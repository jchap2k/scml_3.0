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


#include <scml2_base.h>
#include <scml2.h>
#include "router_binder_attribute_payload.h"

namespace scml2 { 
namespace objects { 

enum router_mapping_type {
	RW = 0,
	READ = 1,
	WRITE = 2
};

class Mapping  {
public:

	Mapping(const std::string& _name)
		: m_name(_name)
		, name("name")
		, base("base")
		, size("size")
		, destination("destination")
		, offset("offset")
		, type("type")
		, active("active")
		, m_base(0)
		, m_size(0)
		, m_destination(0)
		, m_offset(0)
		, m_type(RW)
		, m_active(false)
	{
		active.set_getter(SCML2_CALLBACK(get_active));
		active.set_setter(SCML2_CALLBACK(set_active));

		base.set_setter(SCML2_CALLBACK(set_base));
		size.set_setter(SCML2_CALLBACK(set_size));
		destination.set_setter(SCML2_CALLBACK(set_destination));
		offset.set_setter(SCML2_CALLBACK(set_offset));
		type.set_setter(SCML2_CALLBACK(set_type));

		base.set_getter(SCML2_CALLBACK(get_base));
		size.set_getter(SCML2_CALLBACK(get_size));
		destination.set_getter(SCML2_CALLBACK(get_destination));
		offset.set_getter(SCML2_CALLBACK(get_offset));
		type.set_getter(SCML2_CALLBACK(get_type));
	}

	~Mapping() {
		if (active) unmap();
	}
private:
	std::string m_name;

public:

	// attributes
	scml2::base::const_attribute<std::string> name;
	scml2::base::attribute<unsigned long long> base;
	scml2::base::attribute<unsigned long long> size;
	scml2::base::attribute<scml2::mappable_if& > destination;
	scml2::base::attribute<unsigned long long> offset;
	scml2::base::attribute<scml2::objects::router_mapping_type> type;
	scml2::base::attribute<bool> active;

	// callbacks
	SCML2_BASE_CALLBACK(map, bool);
	SCML2_BASE_CALLBACK(unmap, bool);

	// behaviors
	std::string print() {
		std::string ret ="";
		ret += name;
		ret += ":";
		ret += "\n      base   = " + std::to_string(base);
		ret += "\n      size   = " + std::to_string(size);
		ret += "\n      offset = " + std::to_string(offset);
		ret += "\n      type   = ";
		if (type == RW) ret += "RW"; else if (type == READ) ret += "READ"; else ret += "WRITE";
		ret += "\n      active = ";
		if (active) ret += "true"; else ret += "false";
		ret += "\n";

		return ret;
	}

private:
	void set_base(unsigned long long value){
		if (value != m_base) {
			if (m_active) unmap();
			m_base = value;
			if (m_active) map();
		}
	}

	void set_size(unsigned long long value){
		if (value != m_size) {
			if (m_active) unmap();
			m_size = value;
			if (m_active) map();
		}
	}

	void set_destination(scml2::mappable_if& value){
		// destination can only be set once, there is no support to update the destination for a given mapping
		// if you want to change destination you need to delete and recreate the mapping
		m_destination = &value;
		if (m_active) map();
	}

	void set_offset(unsigned long long value){
		if (value != m_offset) {
			if (m_active) unmap();
			m_offset = value;
			if (m_active) map();
		}
	}

	void set_type(scml2::objects::router_mapping_type value){
		if (value != m_type) {
			if (m_active) unmap();
			m_type = value;
			if (m_active) map();
		}
	}

	unsigned long long get_base() { return m_base; }
	unsigned long long get_size() { return m_size; }
	scml2::mappable_if& get_destination() { return *m_destination; }
	unsigned long long get_offset() { return m_offset; }
	scml2::objects::router_mapping_type get_type() { return m_type; }
	bool get_active() { return m_active; }

	void set_active(bool value) {
		if (value != m_active) {
			m_active = value;
			if (value) map(); else unmap();
		}
	}

private:
	unsigned long long m_base;
	unsigned long long m_size;
	scml2::mappable_if* m_destination;
	unsigned long long m_offset;
	scml2::objects::router_mapping_type m_type;
	bool m_active;
};


class router_base : public scml2::base::object {
public:

	router_base(const scml2::base::object_name& name)
		: scml2::base::object(name)
		, mappings("mappings")
		  , bound_on("bound_on")
	{
	}

	// attributes
	scml2::base::vector<Mapping> mappings;
	scml2::base::value_attribute<utils::binder> bound_on;
	
	// behaviors

	// callback definitions
	SCML2_BASE_CALLBACK_IGNORE(Callback, void, tlm::tlm_generic_payload&, sc_core::sc_time&);
	SCML2_BASE_CALLBACK_IGNORE(DebugCallback, unsigned int, tlm::tlm_generic_payload&);
};




} // objects
} // scml2

