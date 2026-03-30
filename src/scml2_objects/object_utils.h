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
#include <cctype> // needed for std::isspace()

namespace scml2 { 
namespace objects { 

SNPS_VP_API sc_core::sc_module* getParentModule(sc_core::sc_object* current);
SNPS_VP_API std::string getBaseName(scml2::base::object* _obj);
SNPS_VP_API std::string getBaseName(sc_core::sc_object* _obj);

// -------------------------------------------------------------------------------------
// string_to_val:
//
// class with conversion functions to convert std::string to values of certain type
//need to add a template specialization for each value type that needs to be supported
//
// -------------------------------------------------------------------------------------
template<class T>
class string_to_val {
public:
	static inline T convertVal(std::string s_val) {
		return (T)(std::stoi(s_val, nullptr, 0)); // default convertor
	}
};

template<>
class string_to_val<bool> {
public:
	static inline bool convertVal(std::string s_val) {
		if (s_val == "true" || s_val == "1") {
			return true;
		} else if (s_val == "false" || s_val == "0") {
			return false;
		} else {
			std::stringstream err_message;
			err_message << "could not convert bool argument '" << s_val << "'"<< endl;
			throw std::invalid_argument(err_message.str());
			return false;
		}
	}
};

template<>
class string_to_val<std::string> {
public:
  static inline std::string convertVal(std::string s_val) {
		return s_val;
	}
};

template<>
class string_to_val<unsigned int> {
public:
	static inline unsigned int convertVal(std::string s_val) {
		return (unsigned int)(std::stoul(s_val, nullptr, 0));
	}
};

template<>
class string_to_val<unsigned long> {
public:
	static inline unsigned long convertVal(std::string s_val) {
		return (unsigned long)(std::stoul(s_val, nullptr, 0));
	}
};

template<>
class string_to_val<unsigned long long> {
public:
	static inline unsigned long long convertVal(std::string s_val) {
		return (unsigned long long)(std::stoull(s_val, nullptr, 0));
	}
};

template<>
class string_to_val<double> {
public:
	static inline double convertVal(std::string s_val) {
		return (double)(std::stod(s_val));
	}
};

template<>
class string_to_val<long double> {
public:
	static inline long double convertVal(std::string s_val) {
		return (long double)(std::stold(s_val));
	}
};

template<>
class string_to_val<float> {
public:
	static inline float convertVal(std::string s_val) {
		return (float)(std::stof(s_val));
	}
};

template<>
class string_to_val<sc_core::sc_time> {
public:
	static sc_core::sc_time convertVal(std::string s_val) {
		// free-form copy from SystemC implementation...
		static const char* tst_time_units[] = {
				"fs",
				"ps",
				"ns",
				"us",
				"ms",
				"s"
		};
		char * endptr = NULL;
		double v = std::strtod( s_val.c_str(), &endptr );
		if( s_val == endptr || v < 0.0 ) {
			throw std::invalid_argument("could not parse sc_time value");
			return sc_core::SC_ZERO_TIME;
		}
		while( *endptr && std::isspace( *endptr ) ) ++endptr; // skip whitespace

		unsigned tu = sc_core::SC_FS;
		while( tu <= sc_core::SC_SEC && std::strcmp( endptr, tst_time_units[tu] ) != 0) { ++tu; }

		if( tu > sc_core::SC_SEC ) {
			throw std::invalid_argument("unknown time unit");
			return sc_core::SC_ZERO_TIME;
		}
		return sc_core::sc_time(v, static_cast<sc_time_unit>(tu));
	}
};

template<int W>
class string_to_val<sc_dt::sc_bv<W> > {
public:
	static inline sc_dt::sc_bv<W> convertVal(std::string s_val) {
		sc_dt::sc_bv<W> val = std::stoi(s_val, nullptr, 2);
		return val;
	}
};


// namespaces scml2 and objects
}
}
