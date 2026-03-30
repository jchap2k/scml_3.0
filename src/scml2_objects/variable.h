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

#include <scml2.h>
#include <scml2_base.h>
#include "object_utils.h"

namespace scml2 { 
namespace objects { 


class variable_advanced_options {
public:
	variable_advanced_options()
		: analysis_on("analysis_on")
		, analysis_name("analysis_name")
		, command_enabled("command_enabled")
		, command_name("command_name")
	{}

	scml2::base::const_attribute<bool> analysis_on;
	scml2::base::const_attribute<std::string> analysis_name;
	scml2::base::const_attribute<bool> command_enabled;
	scml2::base::const_attribute<std::string> command_name;
};

template<typename DT>
class variable_base : public scml2::base::object {
public:

	variable_base(const scml2::base::object_name& name)
		: scml2::base::object(name)
		, value("value")
		, base_name(name.c_str())
		, index(0)
	{}

	void finalize_local_construction() {
		this->value.set_post_set_callback(SCML2_CALLBACK(update_value));
	}

	//! Attributes
	scml2::base::value_attribute<DT> value;
	scml2::objects::variable_advanced_options advanced_options;

	//! Behaviors

	//! callbacks
	SCML2_BASE_CALLBACK_IGNORE(value_update, void);

private:
	virtual void update_value() = 0;
	virtual void add_analysis() = 0;

	std::string cmd_impl(const std::vector<std::string>& _args) {
		std::stringstream msg;
		msg.str("");
		if (_args[0] == "get" && _args.size() == 1) {
		  msg << static_cast<const DT&>(value);
		} else if (_args[0] == "set" && _args.size() == 2) {
			DT val;
			try {
				val = string_to_val<DT>::convertVal(_args[1]);
			} catch(const std::exception& e) {
				msg << " incorrect argument (" << e.what() <<"), could not convert input to expected type for " << get_name() << endl;
				return msg.str();
			}
			value = val;
		} else {
			msg << "incorrect arguments, usage:" << synopsis;
		}
		return msg.str();
	}

protected:
	bool first_change{true};
	DT initial_value{};

	std::string synopsis;
	std::string base_name;
	unsigned int index;
};


template<typename DT>
class variable : public variable_base<DT> {
public:

	variable(const scml2::base::object_name& name)
		: variable_base<DT>(name)
	{}

	operator DT() const { return this->value; }

	variable<DT>& operator=(DT _value) { this->value = _value; return *this; }
	variable<DT>& operator =(const variable<DT>& r) { this->value = r.value; return *this; }

	variable<DT>& operator+=(DT _value) { this->value += _value; return *this; }
	variable<DT>& operator-=(DT _value) { this->value -= _value; return *this; }
	variable<DT>& operator/=(DT _value) { this->value /= _value; return *this; }
	variable<DT>& operator*=(DT _value) { this->value *= _value; return *this; }
	variable<DT>& operator%=(DT _value) { this->value %= _value; return *this; }
	variable<DT>& operator^=(DT _value) { this->value ^= _value; return *this; }
	variable<DT>& operator&=(DT _value) { this->value &= _value; return *this; }
	variable<DT>& operator|=(DT _value) { this->value |= _value; return *this; }
	variable<DT>& operator>>=(DT _value) { this->value >>= _value; return *this; }
	variable<DT>& operator<<=(DT _value) { this->value <<= _value; return *this; }

	variable<DT>& operator--() { this->value--; return *this; }
	DT operator--(int) { DT tmp = this->value; this->value--; return tmp; }
	variable<DT>& operator++() { this->value++; return *this; }
	DT operator++(int) { DT tmp = this->value; this->value++; return tmp; }

private:
	void update_value() {
		this->value_update();
	}

	void add_analysis() {
	}

private:
};


template<>
class variable<double> : public variable_base<double> {
public:

	variable(const scml2::base::object_name& name)
		: variable_base<double>(name)
	{}

	operator double() const { return this->value; }

	variable<double>& operator=(double _value) { this->value = _value; return *this; }
	variable<double>& operator =(const variable<double>& r) { this->value = r.value; return *this; }

	variable<double>& operator+=(double _value) { this->value += _value; return *this; }
	variable<double>& operator-=(double _value) { this->value -= _value; return *this; }
	variable<double>& operator/=(double _value) { this->value /= _value; return *this; }
	variable<double>& operator*=(double _value) { this->value *= _value; return *this; }

	variable<double>& operator--() { this->value--; return *this; }
	double operator--(int) { double tmp = this->value; this->value--; return tmp; }
	variable<double>& operator++() { this->value++; return *this; }
	double operator++(int) { double tmp = this->value; this->value++; return tmp; }

private:
	void update_value() {
		this->value_update();
	}

	void add_analysis() {
	}

private:
};


template<>
class variable<sc_core::sc_time> : public variable_base<sc_core::sc_time> {
public:

	variable(const scml2::base::object_name& name)
		: variable_base<sc_core::sc_time>(name)
	{}

	operator sc_core::sc_time() const { return this->value; }

	variable<sc_core::sc_time>& operator=(sc_core::sc_time _value) { this->value = _value; return *this; }
	variable<sc_core::sc_time>& operator =(const variable<sc_core::sc_time>& r) { this->value = r.value; return *this; }

	variable<sc_core::sc_time>& operator+=(sc_core::sc_time _value) { this->value += _value; return *this; }
	variable<sc_core::sc_time>& operator-=(sc_core::sc_time _value) { this->value -= _value; return *this; }
	variable<sc_core::sc_time>& operator%=(sc_core::sc_time _value) { this->value %= _value; return *this; }

private:
	void update_value() {
		this->value_update();
	}

	void add_analysis() {
	}

private:
};

template<>
class variable<std::string> : public variable_base<std::string> {
public:

	variable(const scml2::base::object_name& name)
		: variable_base<std::string>(name)
	{}

	operator std::string() const { return this->value; }

	variable<std::string>& operator=(std::string _value) { this->value = _value; return *this; }
	variable<std::string>& operator =(const variable<std::string>& r) { this->value = r.value; return *this; }

	variable<std::string>& operator+=(std::string _value) { this->value += _value; return *this; }
private:
	void update_value() {
		this->value_update();
	}

	void add_analysis() {
	}

private:
};

inline std::ostream& operator<<(std::ostream& os, const variable<std::string>& value) {
	os << (std::string)value;
	return os;
}

} // objects
} // scml2
