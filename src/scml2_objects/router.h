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



#include "router_base.h"
#include "scml2_protocol_engines/tlm2_ft_common/include/tlm2_intercept_base.h"

#include <cassert>

namespace scml2 { 
namespace objects { 

template<typename DT>
class router : public router_base, public scml2::router<DT>
{
  bool shutdown{};
 public:

	router(const scml2::base::object_name& name) : ::scml2::objects::router_base(name), scml2::router<DT>(sc_gen_unique_name((router_base::get_name().substr(router_base::get_name().rfind(".")+1)+ "_router").c_str()), ULLONG_MAX)
	{
		this->set_callback( create_router_callback(create_transport_adapter(*this, &router<DT>::Callback_impl),"Callback",AUTO_SYNCING));
		this->set_debug_callback(create_router_debug_callback(create_debug_transport_adapter(*this, &router<DT>::DebugCallback_impl), "DebugCallback"));

		mappings.set_resized_callback(SCML2_CALLBACK(resize_mappings));
	}

	virtual ~router() {
	  shutdown = true;
	}
	
	void finalize_local_construction() {
	  this->bound_on.get().finalize(*this);
	}
	
	void clear() { mappings.resize(0); }

	std::string print() {
		std::string ret = this->name();
		ret += " has following mappings defined:\n";
		for (unsigned int i = 0; i < mappings.size(); i++) {
			ret += "mapping: ";
			ret += mappings[i].print();
		}
		return ret;
	}

private:
	void Callback_impl(tlm::tlm_generic_payload& p, sc_core::sc_time& t) { Callback(p, t); };
	unsigned int DebugCallback_impl(tlm::tlm_generic_payload& p) { return DebugCallback(p); };

	void resize_mappings( size_t old, size_t size) {
    	for (size_t i = old; i < size; i++) {
    		mappings[i].set_map_callback_tagged(SCML2_CALLBACK(range_map), i);
    		mappings[i].set_unmap_callback_tagged(SCML2_CALLBACK(range_unmap), i);
    	}
	}


	bool range_unmap(int i) {
	  if (!shutdown) {
		Mapping& range = mappings[i];
		if (range.type == RW) {
			return this->unmap(range.base);
		} else if (range.type == READ) {
			return this->unmap_read(range.base);
		} else if (range.type == WRITE) {
			return this->unmap_write(range.base);
		}
		assert(false);
	  }
	  return false;
	}

	bool range_map(int i) {
		Mapping& range = mappings[i];
		if (&static_cast<scml2::mappable_if&>(range.destination) == nullptr) {
		  SCML2_ERROR_TO(static_cast<scml2::base::object*>(this), FATAL_ERROR) << "activating mapping " << i << " for unset destination" << std::endl;
		  return false;
		}
		if (range.type == RW) {
			return this->map(range.base, range.size, range.destination, range.offset);
		} else if (range.type == READ) {
			return this->map_read(range.base, range.size, range.destination, range.offset);
		} else if (range.type == WRITE) {
			return this->map_write(range.base, range.size, range.destination, range.offset);
		}
		assert(false);
		return false;
	}

};



} // objects
} // scml2
