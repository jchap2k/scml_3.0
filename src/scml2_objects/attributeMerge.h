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
class attributeMerge : public scml2::base::object {
  public:

	attributeMerge(const scml2::base::object_name& name)
        : scml2::base::object(name)
	, values("values")
	, result("result")
	, value_change_only("value_change_only")
    {
		values.set_resized_callback(SCML2_CALLBACK(resize_values));
    }

    virtual ~attributeMerge() {
    }
    
  public:
    scml2::base::vector< scml2::base::attribute<bool> > values;
    scml2::base::protected_value_attribute<attributeMerge, DT> result;
    scml2::base::value_attribute<bool> value_change_only;

  private:
    std::vector<bool> m_values;

  private:
    void resize_values( size_t old, size_t size) {
    	m_values.resize(size);
    	for (size_t i = old; i < size; i++) {
    		values[i].set_setter(SCML2_CALLBACK(handle_set_value), i);
    		values[i].set_getter(SCML2_CALLBACK(handle_get_value), i);
    	}
    }

    void handle_set_value(int index, bool val) {
    	m_values[index] = val;
		if (val) {
			result = result | (1<<index);
			} else {
			result = result & ~(1<<index);
		}
    }

    bool handle_get_value(int index) {
    	return m_values[index];
    }

};


} // objects
} // scml2
