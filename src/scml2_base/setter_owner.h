/***************************************************************************
 * Copyright 1996-2025 Synopsys, Inc.
 *
 * This Synopsys software and all associated documentation are proprietary
 * to Synopsys, Inc. and may only be used pursuant to the terms and
 * conditions of a written license agreement with Synopsys, Inc.
 * All other use, reproduction, modification, or distribution of the
 * Synopsys software or the associated documentation is strictly prohibited.
 ***************************************************************************/

#ifndef SCML2_BASE_SETTER_OWNER_H_INCLUDED
#define SCML2_BASE_SETTER_OWNER_H_INCLUDED

#include "scml2_base/utils.h"

#ifdef SNPS_SLS_VP_BASE

#include "scml2_base/callback.h"
#include "scml2_base/callback_impl.h"
#include "scml2_base/setter_owner_observer.h"

#include <string>
#include <vector>

namespace scml2 { namespace base {

    template<typename T>
    class getter_owner;
    
    template<typename T>
    class setter_owner
    {
    public:
      setter_owner(const std::string& name) 
	: m_setter_callback(name + " setter")	  
      {}

      virtual ~setter_owner() {
	for (auto o : m_observers) {
	  delete o;
	}
      }

      void set(T value)
      {
	try {
	  m_setter_callback(value);
	  for (auto o : m_observers) {
	    o->handle_set(value);
	  }
	} catch (scml2::base::empty_callback_exception&) {
	  SCML2_ERROR_TO(m_setter_callback.get_parent(), FATAL_ERROR)
	    << "no implementation for " << m_setter_callback.get_name() << std::endl;
	}
      }

      setter_owner<T>& operator=(T value)
      {
	set(value);
	return *this;
      }

      setter_owner<T>& operator=(const getter_owner<T>& rhs)
      {
	return (*this=rhs.get());
      }
     
      template<typename C>
      void set_setter(C* c, typename callback_impl<C, void, T>::callback_type memfun, const std::string& n = "setter")
      {
	m_setter_callback.set_behavior(create_callback_impl<C, void, T>(n, *c, memfun));
      }

      template<typename C, typename F, typename... Args>
      void set_setter(C* c, F memfun, const std::string& n, Args... args)
      {
	m_setter_callback.set_behavior(create_callback_impl<C, void, T>(n, *c, memfun, args...));
      }

      void add_observer(setter_owner_observer<T>* observer) {
	m_observers.push_back(observer);
      }

      void remove_observer(setter_owner_observer<T>* observer) {
	auto oi = std::find(m_observers.begin(), m_observers.end(), observer);
	assert(oi != m_observers.end());
	m_observers.erase(oi);
      }
    private:
      callback<void, T> m_setter_callback;
      std::vector<setter_owner_observer<T>*> m_observers;
    };
    
  }}

#endif // SNPS_SLS_VP_BASE

#endif // SCML2_BASE_SETTER_OWNER_H_INCLUDED
