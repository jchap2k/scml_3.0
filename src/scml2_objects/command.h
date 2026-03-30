/***************************************************************************
 * Copyright 1996-2025 Synopsys, Inc.
 *
 * This Synopsys software and all associated documentation are proprietary
 * to Synopsys, Inc. and may only be used pursuant to the terms and
 * conditions of a written license agreement with Synopsys, Inc.
 * All other use, reproduction, modification, or distribution of the
 * Synopsys software or the associated documentation is strictly prohibited.
 ***************************************************************************/
#pragma once

#include "scml2.h"
#include "scml2_base.h"
#include "scml.h"
#include "object_utils.h"
#include <map>

namespace scml2 {
namespace objects {


class command : public scml2::base::object {
public:

	command(const scml2::base::object_name& name)
		: scml2::base::object(name)
		, arguments("arguments")
		, description("description")
		, min_params("min_params")
                , m_error(false)
	{
		cmd_name = name.c_str();
	}

	SCML2_BASE_CALLBACK(execute, std::string);

	virtual void finalize_local_construction() {
		if (!description.get_initialized()) { // added to avoid error messages when non-initialized but read
			description = "\n";
		}
		synopsis = cmd_name + " ";
		for (unsigned int i = 0; i< arguments.size(); i++) {
			std::string arg_name = arguments[i];
			args[arg_name] = "";
			if (i >= min_params) synopsis += "[" + arg_name + "] ";
			else synopsis += arg_name + " ";
		}
		help = "usage: ";
		help += synopsis + "\n";
		help += description;
		scml2::base::object::finalize_local_construction();
    }

	std::string cmd_impl(const std::vector<std::string>& _args) {
		m_error = false;
		err_message.str("");
		if (_args.empty() && min_params != 0) {
			err_message << "Too few arguments.\n" << "Usage: " << synopsis;
			return err_message.str();
		}
		if (!_args.empty()) {
			if (_args[0] == "help" && _args.size() == 1) {
				return help;
			} else if (_args.size() < min_params) {
				err_message << "Too few arguments.\n" << "Usage: " << synopsis;
				return err_message.str();
			} else if (_args.size() > arguments.size()) {
				err_message << "Too many arguments.\n" << "Usage: " << synopsis;
				return err_message.str();
			}
		}
		for (unsigned int i = 0; i < arguments.size(); i++) {
			std::string arg_name = arguments[i];
			if (!_args.empty() && i<_args.size()) {
				args[arg_name] = _args[i];
			} else {
				args[arg_name] = "null";
			}
		}
		std::string ret = this->execute();
		if (m_error) {
			err_message << ret;
			return err_message.str();
		} else {
			return ret;
		}
	}

	std::string get(std::string arg_name) {
		tArg_map::iterator iter = args.find(arg_name);
		if (iter != args.end()) {
			return args[arg_name];
		} else {
			m_error = true;
			err_message << "Invalid argument '" << arg_name << "' for command '" << get_name() << "'" << endl;
			return "null";
		}
	}

	// generic 'get' implementation: assumes type has a constructor that takes a string....
	template<typename T>
	T get(std::string arg_name) {
		std::string c = get(arg_name);
		if (c != "null") {
			try {
				T ret = T(c);
				return ret;
			} catch (std::exception const &/*x*/) {
				m_error = true;
				err_message << "could not convert '" << c << "' for argument '" << arg_name << "' to integer for command '" << get_name() << "'" << endl;
			}
		} else {
			return T();
		}
	}

	// Type specific 'get' implementations --------------------------
	int getInt(std::string arg_name) {
		std::string c = get(arg_name);
		if (c != "null") {
			int ret = 0;
			try { 
				ret = string_to_val<int>::convertVal(c);
			} catch (std::exception const &/*x*/) {
				m_error = true;
				err_message << "could not convert '" << c << "' for argument '" << arg_name << "' to integer for command '" << get_name() << "'" << endl;
			}
			return ret;
		} else {
			return 0;
		}
	}
	
	// converts arg to long
	long long getLong(std::string arg_name) {
		std::string c = get(arg_name);
		if (c != "null") {
			long long ret = 0;
			try {
				ret = std::stoll(c, 0, 0);
			} catch (std::exception const &/*x*/) {
				m_error = true;
				err_message << "could not convert '" << c << "' for argument '" << arg_name << "' to long for command '" << get_name() << "'" << endl;
			}
			return ret;
		} else {
			return 0;
		}
	}
	
	unsigned long getUnsigned(std::string arg_name) {
		std::string c = get(arg_name);
		if (c != "null") {
			unsigned long ret = 0;
			try {
				ret = string_to_val<unsigned long>::convertVal(c);
			} catch (std::exception const &/*x*/) {
				m_error = true;
				err_message << "could not convert '" << c << "' for argument '" << arg_name << "' to unsigned long for command '" << get_name() << "'" << endl;
			}
			return ret;
		} else {
			return 0;
		}
	}
	
	unsigned long long getUnsignedLong(std::string arg_name) {
		std::string c = get(arg_name);
		if (c != "null") {
			unsigned long long ret = 0;
			try {
				ret = string_to_val<unsigned long long>::convertVal(c);
			} catch (std::exception const &/*x*/) {
				m_error = true;
				err_message << "could not convert '" << c << "' for argument '" << arg_name << "' to unsigned long long for command '" << get_name() << "'" << endl;
			}
			return ret;
		} else {
			return 0;
		}
	}
	
	float getFloat(std::string arg_name) {
		std::string c = get(arg_name);
		if (c != "null") {
			float ret = 0;
			try {
				ret = string_to_val<float>::convertVal(c);
			} catch (std::exception const &/*x*/) {
				m_error = true;
				err_message << "could not convert '" << c << "' for argument '" << arg_name << "' to float for command '" << get_name() << "'" << endl;
			}
			return ret;
		} else {
			return 0;
		}
	}
	
	long double getDouble(std::string arg_name) {
		std::string c = get(arg_name);
		if (c != "null") {
			long double ret = 0;
			try {
				ret = string_to_val<long double >::convertVal(c);
			} catch (std::exception const &/*x*/) {
				m_error = true;
				err_message << "could not convert '" << c << "' for argument '" << arg_name << "' to long double for command '" << get_name() << "'" << endl;
			}
			return ret;
		} else {
			return 0;
		}
	}
	
	bool getBool(std::string arg_name) {
		std::string c = get(arg_name);
		if (c != "null") {
			bool ret = false;
			try {
				ret = string_to_val<bool>::convertVal(c);
			} catch (std::exception const &/*x*/) {
				m_error = true;
				err_message << "could not convert '" << c << "' for argument '"<< arg_name << "' to boolean for command '" << get_name() 
					<< "' expecting 'true','false' or '1','0'" << endl;
			}
			return ret;
		} else {
			return false;
		}
	}
	
	sc_core::sc_time getTime(std::string arg_name) {
		std::string c = get(arg_name);
		if (c != "null") {
			sc_core::sc_time ret = SC_ZERO_TIME;
			try {
				// try standard SystemC string representation conversion fist
				ret = string_to_val<sc_core::sc_time>::convertVal(c);
			} catch (std::exception const & /*x*/) {
				// alternatively go for the old-style conversion as originally available with the command RO
				bool error = false;
				std::size_t p = c.find(":");
				std::string s1, s2;
				if (p != std::string::npos) {
					s1 = c.substr(0,p);
					s2 = c.substr(p+1); // drop the ':'
				} else {
					s1 = c;
					s2 = "SC_PS";
				}
				double t = 0;
				try {
					t = std::stod(s1);
				} catch (std::exception const &/*x*/) {
					error = true;
				}
				sc_core::sc_time_unit u;
				if (s2 == "SC_FS") {
					u = sc_core::SC_FS;
				} else if (s2 == "SC_PS") {
					u = sc_core::SC_PS;
				} else if (s2 == "SC_NS") {
					u = sc_core::SC_NS;
				} else if (s2 == "SC_US") {
					u = sc_core::SC_US;
				} else if (s2 == "SC_MS") {
					u = sc_core::SC_MS;
				} else if (s2 == "SC_SEC") {
					u = sc_core::SC_SEC;
				} else {
					error = true;
				}
				if (error) {
					m_error = true;
					err_message << "could not convert '" << c << "' for argument '" << arg_name << "' to sc_time for command '" << get_name()
								<< " expecting standard SystemC notation for sc_time  'value time_unit with value double and time unit one of fs, ps, ns, us, ms, s\n e.g. '10 ns' \n"
								<< " alternatively also supporting the notation 'value:time_unit' with value double and time_unit one of SC_FS, SC_PS, SC_NS, SC_US, SC_MS, SC_SEC, if time unit is omitted SC_PS is assumed";
					return SC_ZERO_TIME;
				} else {
					return sc_core::sc_time(t, u);
				}
			}
			return ret;
		} else {
			return SC_ZERO_TIME;
		}
	}
		
	template<class RET>
	RET lexical_cast(std::string arg_name) {
		RET ret;
		std::stringstream ss;
		
		ss.exceptions( std::ifstream::failbit | std::ifstream::badbit );
		std::string c = get(arg_name);
		ss << c;
		try {
			ss >> ret;
		} catch (std::exception const &/*x*/) {
				m_error = true;
				err_message << "could not convert '" << c << "' for argument '" << arg_name << "' for command '" << get_name() << "'" << endl;
		}
		return ret;
	}

	bool error() {
		return m_error;
	}

public:
	scml2::base::vector<std::string> arguments;
	scml2::base::const_attribute<std::string> description;
	scml2::base::const_attribute<unsigned int> min_params;
private:
	std::string cmd_name;
	std::string synopsis;
	std::string help;
	std::stringstream err_message;
	bool m_error;
	
	typedef std::map< std::string, std::string > tArg_map;
	tArg_map args;
};

}  // end of namespace objects
}  // end of namespace scml2
