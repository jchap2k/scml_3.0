/***************************************************************************
 * Copyright 1996-2025 Synopsys, Inc.
 *
 * This Synopsys software and all associated documentation are proprietary
 * to Synopsys, Inc. and may only be used pursuant to the terms and
 * conditions of a written license agreement with Synopsys, Inc.
 * All other use, reproduction, modification, or distribution of the
 * Synopsys software or the associated documentation is strictly prohibited.
 ***************************************************************************/

#ifndef SCML2_BASE_VALUE_ATTRIBUTE_H_INCLUDED
#define SCML2_BASE_VALUE_ATTRIBUTE_H_INCLUDED

#include "scml2_base/utils.h"

#ifdef SNPS_SLS_VP_BASE

#include <string>
#include "scml2_base/setter_owner.h"
#include "scml2_base/getter_owner.h"
#include "scml2_base/attribute_value.h"
#include "scml2_base/atom.h"

namespace scml2 { namespace base {
    
    template<typename T>
    class value_attribute
      : public atom
      , public setter_owner<T>
      , private getter_owner<T>
      ,	private attribute_value<T>
    {
    public:
      value_attribute(const std::string& name) 
	: atom(name)
	, setter_owner<T>(name)
	, getter_owner<T>(name)
	, attribute_value<T>(*this, *this)
	, m_post_set_callback(name + ".post_set_callback", [](){})
      {}

      value_attribute(const std::string& name, const T& value) 
	: value_attribute(name)
      {
	set(value);
      }

      T get() const throw() { 
	// coverity[fun_call_w_exception]
	return getter_owner<T>::get(); 
      }
      operator T() const throw() {
	// coverity[fun_call_w_exception]
	return  getter_owner<T>::operator T();
      }
      value_attribute<T>& operator=(T value) { set(value); return *this; }
      value_attribute<T>& operator=(const value_attribute<T>& rhs) {return ((*this) = rhs.get());}

      value_attribute<T>& operator++() { T tmp = get(); ++tmp; set(tmp); return *this; }
      T operator++(int) { T result = get(); T tmp = result; tmp++; set(tmp); return result; }
      value_attribute<T>& operator--() { T tmp = get(); --tmp; set(tmp); return *this;}
      T operator--(int) { T result = get(); T tmp = result; tmp--; set(tmp); return result; }

      value_attribute<T>& operator+=(const T& value) { set(get()+value); return *this; }
      value_attribute<T>& operator-=(const T& value) { set(get()-value); return *this; }
      value_attribute<T>& operator/=(const T& value) { set(get()/value); return *this; }
      value_attribute<T>& operator*=(const T& value) { set(get()*value); return *this; }
      value_attribute<T>& operator%=(const T& value) { set(get()%value); return *this; }
      value_attribute<T>& operator^=(const T& value) { set(get()^value); return *this; }
      value_attribute<T>& operator&=(const T& value) { set(get()&value); return *this; }
      value_attribute<T>& operator|=(const T& value) { set(get()|value); return *this; }
      value_attribute<T>& operator<<=(const T& value) { set(get()<<value); return *this; }
      value_attribute<T>& operator>>=(const T& value) { set(get()>>value); return *this; }

      
      
      template<typename C>
      void set_post_set_callback(C* c, typename callback_impl<C, void>::callback_type memfun, const std::string& n = "post_set_callback_impl")
      {
	m_post_set_callback.set_behavior(scml2::base::callback_impl<C, void>(n, *c, memfun));
      }
      template<typename C>
      void set_post_set_callback_tagged(C* c, typename callback_impl_tagged<C, void>::callback_type memfun, const std::string& n, int tag)
      {
	m_post_set_callback.set_behavior(scml2::base::callback_impl_tagged<C, void>(n, *c, memfun, tag));
      }

      void set(T value) {
	// coverity[fun_call_w_exception]
	setter_owner<T>::set(value);
	if (m_post_set_callback.has_behavior()) {
	  // coverity[fun_call_w_exception]
	  m_post_set_callback();
	}
      }
    private:
      callback<void> m_post_set_callback;
    };
    
  }}

#endif // SNPS_SLS_VP_BASE

#endif // SCML2_BASE_VALUE_ATTRIBUTE_H_INCLUDED
