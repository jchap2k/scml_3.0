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
#include "scml2_protocol_engines/tlm2_ft_common/include/tlm2_intercept_base.h"

namespace scml2 { 
namespace objects {

class tlm2TargetIntercept :
	public scml2::base::object, 
    public tlm2_intercept_base {
public:
    tlm2TargetIntercept(const scml2::base::object_name& name) 
		: scml2::base::object(name)
		, m_destination(nullptr)	
		, invalidate_dmi_cb_active(false)
	{
		m_mapped_name = this->get_name();
	}
	
	SCML2_BASE_CALLBACK_IGNORE_W_VAL(payload_intercept, bool, true, tlm::tlm_generic_payload&);
	SCML2_BASE_CALLBACK_IGNORE(payload_intercept_post, void, tlm::tlm_generic_payload&);
	SCML2_BASE_CALLBACK_IGNORE(invalidate_dmi_range_update, void, unsigned long long&, unsigned long long&);

	void operator() (scml2::mappable_if& dest) {
	    if (!m_destination || m_destination == &dest) {
			m_destination = &dest;
			m_destination->register_bw_direct_mem_if(this);
		} else {
			SCML2_ERROR(FATAL_ERROR) << "TLM2 Target Intercept object " << this->get_name() <<
			"bound to multiple destinations" << endl;
		}
	};
	
	std::string get_mapped_name() const {
		return m_mapped_name;
	}
		
    void b_transport(tlm::tlm_generic_payload& trans, sc_core::sc_time& t) {
		if (payload_intercept(trans)) {
		    m_destination->b_transport(trans, t);
		    payload_intercept_post(trans);
		}
	};
	bool get_direct_mem_ptr(tlm::tlm_generic_payload& trans, tlm::tlm_dmi& dmiData) {
		if (payload_intercept(trans)) {
		    return m_destination->get_direct_mem_ptr(trans, dmiData);
		}
		return false;
	};
	unsigned int transport_dbg(tlm::tlm_generic_payload& trans) {
		if (payload_intercept(trans)) {
		    unsigned int ret = m_destination->transport_dbg(trans);
		    payload_intercept_post(trans);
		    return ret;
		}
		return 0;
	};
	void register_bw_direct_mem_if(tlm::tlm_bw_direct_mem_if* bwInterface) {
		mBwDirectMemIfs.insert(bwInterface);
	};
	void unregister_bw_direct_mem_if(tlm::tlm_bw_direct_mem_if* bwInterface) {
		mBwDirectMemIfs.erase(bwInterface);
	};

	void invalidate_direct_mem_ptr(unsigned long long startRange, unsigned long long endRange) {
	    if (invalidate_dmi_cb_active) {
		    invalidate_dmi_range_update(startRange, endRange);
		}
	    std::set<tlm::tlm_bw_direct_mem_if*>::iterator it = mBwDirectMemIfs.begin();
		std::set<tlm::tlm_bw_direct_mem_if*>::const_iterator end = mBwDirectMemIfs.end();
		for (; it != end; ++it) {
		    (*it)->invalidate_direct_mem_ptr(startRange, endRange);
		}
	};
	
private:
    void callback_registered(scml2::base::callback_base& cbb) {
	    if (&cbb == dynamic_cast<scml2::base::callback_base*>(&(this->invalidate_dmi_range_update))) {
		    invalidate_dmi_cb_active = true;
	    }
	}
		
private:
	scml2::mappable_if* m_destination;
    std::set<tlm::tlm_bw_direct_mem_if*> mBwDirectMemIfs;
	std::string m_mapped_name; 
	bool invalidate_dmi_cb_active;
};

} // namespace objects
} // namespace scml2

