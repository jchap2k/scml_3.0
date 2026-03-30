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

template<typename DT>
class attributeSplit : public scml2::base::object {
  public:

	attributeSplit(const scml2::base::object_name& name)
        : scml2::base::object(name)
	, value("value")
	, result("result")
	, value_change_only("value_change_only")
    {
		value.set_setter(SCML2_CALLBACK(handle_set_value));
    	value.set_getter(SCML2_CALLBACK(handle_get_value));
    }

    virtual ~attributeSplit() {
    }
    
  public:
    scml2::base::attribute<DT> value;
    scml2::base::vector<scml2::base::protected_value_attribute<attributeSplit, bool> > result;
    scml2::base::value_attribute<bool> value_change_only;

  private:
    DT m_value;

  private:
    void handle_set_value(DT val) {
    	m_value = val;
    	for (unsigned int i = 0; i < result.size(); i++) {
    		bool res = val & 0x1;
    		if (!value_change_only || res != result[i]) {
    			result[i] = res;
    		}
    		val = val >> 1;
    	}
    }

    DT handle_get_value() {
    	return m_value;
    }

};


} // objects
} // scml2
