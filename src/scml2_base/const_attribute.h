/***************************************************************************
 * Copyright 1996-2025 Synopsys, Inc.
 *
 * This Synopsys software and all associated documentation are proprietary
 * to Synopsys, Inc. and may only be used pursuant to the terms and
 * conditions of a written license agreement with Synopsys, Inc.
 * All other use, reproduction, modification, or distribution of the
 * Synopsys software or the associated documentation is strictly prohibited.
 ***************************************************************************/

#ifndef SCML2_BASE_CONST_ATTRIBUTE_H_INCLUDED
#define SCML2_BASE_CONST_ATTRIBUTE_H_INCLUDED

#include "scml2_base/utils.h"

#ifdef SNPS_SLS_VP_BASE

#ifdef _WIN32
#pragma warning( disable : 4522 )
#endif

#include <string>
#include "scml2_base/attribute.h"
#include "scml2_base/attribute_constant_initializer.h"

namespace scml2 { namespace base {
    
    template<typename T>
    class const_attribute
      : public attribute<T>
      , private attribute_constant_initializer<T>
    {
    public:

      const_attribute(const std::string& name) 
	: attribute<T>(name)
	, attribute_constant_initializer<T>(*((attribute<T>*)this), name)
      {}

      const const_attribute<T>& operator=(T value)
      {
	this->set(value);
	return *this;
      }

      const const_attribute<T>& operator=(const const_attribute<T>& rhs)
      {
	((*this) = rhs.get());
	return *this;
      }

      T get() const throw() { 
	// coverity[fun_call_w_exception]
	return attribute<T>::get(); 
      }
      operator T() const throw() {
	// coverity[fun_call_w_exception]
	return  attribute<T>::operator T();
      }

      using attribute_constant_initializer<T>::get_initialized;
      using attribute_constant_initializer<T>::set_post_set_callback;
    };
    
  }}

#endif // SNPS_SLS_VP_BASE

#endif // SCML2_BASE_CONST_ATTRIBUTE_H_INCLUDED
