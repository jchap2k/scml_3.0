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
class attributeMux : public scml2::base::object {
  public:

	attributeMux(const scml2::base::object_name& name)
        : scml2::base::object(name)
	, values("values")
	, selector("selector")
	, result("result")
	, value_change_only("value_change_only")
    {
		values.set_resized_callback(SCML2_CALLBACK(resize_values));
		selector.set_post_set_callback(SCML2_CALLBACK(update_selection));
    }

    virtual ~attributeMux() {
    }
    
  public:
    scml2::base::vector< scml2::base::attribute<DT> > values;
    scml2::base::value_attribute<unsigned int> selector;
    scml2::base::protected_value_attribute<attributeMux, DT> result;
    scml2::base::value_attribute<bool> value_change_only;

  private:
    std::vector<DT> m_values;

  private:
    void resize_values( size_t old, size_t size) {
    	m_values.resize(size);
    	for (size_t i = old; i < size; i++) {
    		values[i].set_setter(SCML2_CALLBACK(handle_set_value), i);
    		values[i].set_getter(SCML2_CALLBACK(handle_get_value), i);
    	}
    }

    void update_selection() {
    	if (selector < values.size()) {
    		result = m_values[selector];
    	} else {
    		SCML2_MODEL_INTERNAL(LEVEL6) << "Trying to select out of range: size = " << values.size() << ", selector = " << selector << endl;
    	}
    }

    void handle_set_value(int index, DT val) {
    	m_values[index] = val;
    	if (index == (int)selector) {
    		if (value_change_only && val != result) {
    			result = val;
    		}
    	}
    }

    DT handle_get_value(int index) {
    	return m_values[index];
    }

};


} // objects
} // scml2
