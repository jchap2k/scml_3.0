/***************************************************************************
 * Copyright 1996-2025 Synopsys, Inc.
 *
 * This Synopsys software and all associated documentation are proprietary
 * to Synopsys, Inc. and may only be used pursuant to the terms and
 * conditions of a written license agreement with Synopsys, Inc.
 * All other use, reproduction, modification, or distribution of the
 * Synopsys software or the associated documentation is strictly prohibited.
 ***************************************************************************/
#ifndef SCML2_REG_PROXY_H
#define SCML2_REG_PROXY_H

#include <scml2/reg.h>
#include <scml2/memory_proxy_base.h>
#include <scml2/memory_proxy.h>
#include <scml2/memory_alias_proxy.h>

#include <scml2/fallback_value.h>

namespace scml2 {

template <typename DT>
class reg_proxy 
  : public memory_proxy_base<reg<DT>>
{
  fallback_value<DT> m_fallback_value;
  virtual std::string type() const override {return "reg";}

  using memory_proxy_base<reg<DT>>::m_proxied;

public:
  typedef reg<DT> proxied_type2;
  using proxy_base<reg<DT>>::proxied_exists;
  typedef reg<DT> proxied_type;

  typedef DT data_type;

  explicit reg_proxy(const std::string& name)
    : memory_proxy_base<reg<DT>>(name)
    , m_fallback_value([this]() -> bool {return this->m_proxied.get() != nullptr;},
		       [this]() -> DT {return static_cast<DT>(*this->m_proxied);},
		       [this]() -> DT {
			 SCML2_ERROR_TO(&(this->m_parent), FATAL_ERROR) << "empty " << this->type() << " proxy " << this->m_name << " attempted to be accessed."<< std::endl;
			 throw nonexisting_proxy_access_exception(); 
			 return DT();
		       })
  {}

  void init(const proxy_parent_wrapper<toplevel_memory_base_proxy>& parent, unsigned long long offset) {
    init(parent, offset, true);
  }

  template<typename SHOULD_EXIST_TYPE>
  void init(const proxy_parent_wrapper<toplevel_memory_base_proxy>& parent, unsigned long long offset, const SHOULD_EXIST_TYPE& should_exist) {
    if (proxy::should_exist_to_bool(should_exist)) {
      this->m_proxied.reset(new reg<DT>(this->get_basename(), parent.get_parent(), offset));
    }    
  }

  reg_proxy(const std::string& name,
	    const proxy_parent_wrapper<toplevel_memory_base_proxy>& parent,
	    unsigned long long offset)
    : reg_proxy(name, parent, offset, true)
  {
  }

  template<typename SHOULD_EXIST_TYPE>
  reg_proxy(const std::string& name,
	    const proxy_parent_wrapper<toplevel_memory_base_proxy>& parent,
	    unsigned long long offset,
	    const SHOULD_EXIST_TYPE& should_exist)
    : reg_proxy(name)
  {
    init(parent, offset, should_exist);
  }

  template<typename SHOULD_EXIST_TYPE>
  reg_proxy(const std::string& name,
	    const proxy_parent_wrapper<toplevel_memory_base_proxy>& parent,
	    unsigned long long offset,
	    const SHOULD_EXIST_TYPE& should_exist, 
	    DT fallback_value)
    : reg_proxy(name, parent, offset, should_exist)
  {
    set_fallback_value(std::move(fallback_value));
  }

  void init(const proxy_parent_wrapper<memory_alias_base_proxy>& parent,
	    unsigned long long offset)
  {
    init(parent, offset, true);
  }

  template<typename SHOULD_EXIST_TYPE>
  void init(const proxy_parent_wrapper<memory_alias_base_proxy>& parent,
	    unsigned long long offset,
	    const SHOULD_EXIST_TYPE& should_exist)
  {
    if (proxy::should_exist_to_bool(should_exist)) {
      this->m_proxied.reset(new reg<DT>(this->get_basename(), parent.get_parent(), offset));
    }
  }

  reg_proxy(const std::string& name,
	    const proxy_parent_wrapper<memory_alias_base_proxy>& parent,
	    unsigned long long offset)
    : reg_proxy(name)
  {
    init(parent, offset, true);
  }

  template<typename SHOULD_EXIST_TYPE>
  reg_proxy(const std::string& name,
	    const proxy_parent_wrapper<memory_alias_base_proxy>& parent,
	    unsigned long long offset,
	    const SHOULD_EXIST_TYPE& should_exist)
    : reg_proxy(name)
  {
    init(parent, offset, should_exist);
  }

  template<typename SHOULD_EXIST_TYPE>
  void init(const proxy_parent_wrapper<memory_alias_base_proxy>& parent,
	    unsigned long long offset,
	    const SHOULD_EXIST_TYPE& should_exist, 
	    DT fallback_value)
  {
    init(parent, offset, should_exist);
    set_fallback_value(std::move(fallback_value));
  }

  template<typename SHOULD_EXIST_TYPE>
  reg_proxy(const std::string& name,
	    const proxy_parent_wrapper<memory_alias_base_proxy>& parent,
	    unsigned long long offset,
	    const SHOULD_EXIST_TYPE& should_exist, 
	    DT fallback_value)
    : reg_proxy(name)
  {
    init(parent, offset, should_exist, fallback_value);
  }

  void set_fallback_value(DT value) {
    m_fallback_value.set_fallback_value(std::move(value));
  }

  virtual ~reg_proxy() {}

  operator DT() const
  {
    return m_fallback_value.get_value();
  }

  reg_proxy<DT>& operator=(const DT& value)
  {
    SCML2_PROXY_CHECK_WITH_VALUE(*this);
    (*(this->m_proxied)) = value;
    return *this;
  }
  
  SCML2_PROXY_FORWARD(put);
  SCML2_PROXY_FORWARD(get);
  SCML2_PROXY_FORWARD(put_debug);
  SCML2_PROXY_FORWARD(get_debug);
  SCML2_PROXY_FORWARD(put_with_triggering_callbacks);
  SCML2_PROXY_FORWARD(get_with_triggering_callbacks);
  SCML2_PROXY_FORWARD(put_with_triggering_bitfield_callbacks);
  SCML2_PROXY_FORWARD(get_with_triggering_bitfield_callbacks);

  SCML2_PROXY_FORWARD(set_symbolic_name);
  SCML2_PROXY_FORWARD(get_symbolic_name);

  SCML2_PROXY_FORWARD(b_transport);
  SCML2_PROXY_FORWARD(transport_dbg);
  SCML2_PROXY_FORWARD(transport_without_triggering_callbacks);
  SCML2_PROXY_FORWARD(transport_debug_without_triggering_callbacks);
  
  SCML2_PROXY_FORWARD_THIS(operator+=);
  SCML2_PROXY_FORWARD_THIS(operator-=);
  SCML2_PROXY_FORWARD_THIS(operator/=);
  SCML2_PROXY_FORWARD_THIS(operator*=);
  SCML2_PROXY_FORWARD_THIS(operator%=);
  SCML2_PROXY_FORWARD_THIS(operator^=);
  SCML2_PROXY_FORWARD_THIS(operator&=);
  SCML2_PROXY_FORWARD_THIS(operator|=);
  SCML2_PROXY_FORWARD_THIS(operator<<=);
  SCML2_PROXY_FORWARD_THIS(operator>>=);
  reg_proxy<DT>& operator--()
  {
    SCML2_PROXY_CHECK_WITH_VALUE(*this);
    --((reg<DT>&)(*this));    
    return *this;
  }
  reg_proxy<DT>& operator++()
  {
    SCML2_PROXY_CHECK_WITH_VALUE(*this);
    ++((reg<DT>&)(*this));    
    return *this;
  }
  DT operator--(int v)
  {
    SCML2_PROXY_CHECK(operator--,v);
    DT result = (DT)(*this);
    --(*this);
    return result;
  }
  DT operator++(int v)
  {
    SCML2_PROXY_CHECK(operator++,v);
    DT result = (DT)(*this);
    ++(*this);
    return result;
  }
private:
  reg_proxy(const reg_proxy<DT>&);
};

}

#endif
