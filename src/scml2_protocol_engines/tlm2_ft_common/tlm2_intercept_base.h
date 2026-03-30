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

#include <systemc>
#include <scml2.h>
#include <scml2_base.h>

namespace scml2 { 
namespace objects {

class tlm2_intercept_base : 
	public scml2::mappable_if,
	public tlm::tlm_bw_direct_mem_if {
public:
	// bind with mappable_if destination
	virtual void operator() (scml2::mappable_if& dest) =0;
	
	// mappable_if
	virtual std::string get_mapped_name() const = 0;
    virtual void b_transport(tlm::tlm_generic_payload& trans, sc_core::sc_time& t)=0;
	virtual bool get_direct_mem_ptr(tlm::tlm_generic_payload& trans, tlm::tlm_dmi& dmiData)=0;
	virtual unsigned int transport_dbg(tlm::tlm_generic_payload& trans)=0;
	virtual void register_bw_direct_mem_if(tlm::tlm_bw_direct_mem_if* bwInterface) =0;
	virtual void unregister_bw_direct_mem_if(tlm::tlm_bw_direct_mem_if* bwInterface) =0;
	
	// tlm_bw_direct_mem_if
	virtual void invalidate_direct_mem_ptr(unsigned long long startRange, unsigned long long endRange)=0;
};

}
}

