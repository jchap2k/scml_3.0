/***************************************************************************
 * Copyright 1996-2025 Synopsys, Inc.
 *
 * This Synopsys software and all associated documentation are proprietary
 * to Synopsys, Inc. and may only be used pursuant to the terms and
 * conditions of a written license agreement with Synopsys, Inc.
 * All other use, reproduction, modification, or distribution of the
 * Synopsys software or the associated documentation is strictly prohibited.
 ***************************************************************************/
#ifndef SCML2_BITFIELD_PROXY_H
#define SCML2_BITFIELD_PROXY_H

#include <scml2_logging/snps_vp_dll.h>

#include <scml2/bitfield.h>
#include <scml2/proxy_base.h>
#include <scml2/reg_proxy.h>

#include <scml2/fallback_value.h>

#include <string>
#include <memory>
#include <cstdlib>

namespace scml2 {

template <typename DT>
class bitfield_proxy 
  : public proxy_base<bitfield<DT>>
  , public bitfield_restriction_api_owner<DT>
  , public bitfield_callback_observers_api_owner<DT>
  , public bitfield_callback_api_owner<DT>
  , public bitfield_debug_callback_api_owner<DT>
  , public bitfield_description_api_owner
{
  fallback_value<DT> m_fallback_value;

  using proxy_base<bitfield<DT>>::m_proxied;
  virtual std::string type() const override {return "bitfield";}

public:
  explicit bitfield_proxy(const std::string& name)
    : proxy_base<bitfield<DT>>(name) 
    , m_fallback_value([this]() -> bool {return this->m_proxied.get() != nullptr;},
		       [this]() -> DT {return static_cast<DT>(*this->m_proxied);},
		       [this]() -> DT {
			 SCML2_ERROR_TO(&(this->m_parent), FATAL_ERROR) << "empty " << this->type() << " proxy " << this->m_name << " attempted to be accessed."<< std::endl;
			 throw nonexisting_proxy_access_exception(); 
			 return DT();
		       })    
  {}

  typedef bitfield<DT> proxied_type2;
  using proxy_base<bitfield<DT>>::proxied_exists;
  typedef bitfield<DT> proxied_type;

  typedef DT data_type;

  void init(const proxy_parent_wrapper<reg_proxy<DT>>& parent,
	    unsigned int offset,
	    unsigned int size) {
    init(parent, offset, size, true);
  }

  template<typename SHOULD_EXIST_TYPE>
  void init(const proxy_parent_wrapper<reg_proxy<DT>>& parent,
	    unsigned int offset,
	    unsigned int size,
	    SHOULD_EXIST_TYPE should_exist) {
    if (proxy::should_exist_to_bool(should_exist)) {
      this->m_proxied.reset(new bitfield<DT>(this->get_basename(), parent.get_parent(), offset, size));
    }    
  }

  void init(const proxy_parent_wrapper<bitfield_proxy<DT>>& parent,
	    unsigned int offset,
	    unsigned int size) {
    init(parent, offset, size, true);
  }

  template<typename SHOULD_EXIST_TYPE>
  void init(const proxy_parent_wrapper<bitfield_proxy<DT>>& parent,
	    unsigned int offset,
	    unsigned int size,
	    SHOULD_EXIST_TYPE should_exist) {
    if (proxy::should_exist_to_bool(should_exist)) {
      this->m_proxied.reset(new bitfield<DT>(this->get_basename(), parent.get_parent(), offset, size));
    }    
  }

  template<typename SHOULD_EXIST_TYPE>
  bitfield_proxy(const std::string& name,
		 const proxy_parent_wrapper<reg_proxy<DT>>& parent,
		 unsigned int offset,
		 unsigned int size,
		 SHOULD_EXIST_TYPE should_exist)
    : bitfield_proxy(name)
  {
    init(parent, offset, size, should_exist);
  }

  template<typename SHOULD_EXIST_TYPE>
  bitfield_proxy(const std::string& name,
		 const proxy_parent_wrapper<reg_proxy<DT>>& parent,
		 unsigned int offset,
		 unsigned int size,
		 SHOULD_EXIST_TYPE should_exist, 
		 DT fallback_value)
    : bitfield_proxy(name, parent, offset, size, should_exist)
  {
    set_fallback_value(std::move(fallback_value));
  }

  template<typename SHOULD_EXIST_TYPE>
  bitfield_proxy(const std::string& name,
		 const proxy_parent_wrapper<bitfield_proxy<DT>>& parent,
		 unsigned long long offset,
		 unsigned int size,
		 SHOULD_EXIST_TYPE should_exist)
    : bitfield_proxy(name)
  {
    if (proxy::should_exist_to_bool(should_exist)) {
      this->m_proxied.reset(new bitfield<DT>(name, parent.get_parent(), offset, size));
    }
  }

  template<typename SHOULD_EXIST_TYPE>
  bitfield_proxy(const std::string& name,
		 const proxy_parent_wrapper<bitfield_proxy<DT>>& parent,
		 unsigned long long offset,
		 unsigned int size,
		 SHOULD_EXIST_TYPE should_exist, 
		 DT fallback_value)
    : bitfield_proxy(name, parent, offset, size, should_exist)
  {
    set_fallback_value(std::move(fallback_value));
  }
  
  void set_fallback_value(DT value) {
    m_fallback_value.set_fallback_value(std::move(value));
  }

  ~bitfield_proxy()
  {
  }

  operator DT() const
  {
    return m_fallback_value.get_value();
  }

  bitfield_proxy<DT>& operator=(const DT& value)
  {
    SCML2_PROXY_CHECK_WITH_VALUE(*this);
    (*this->m_proxied) = value;
    return *this;
  }

  SCML2_PROXY_FORWARD(put);
  SCML2_PROXY_FORWARD(get);
  SCML2_PROXY_FORWARD(put_debug);
  SCML2_PROXY_FORWARD(get_debug);
  SCML2_PROXY_FORWARD(put_with_triggering_callbacks);
  SCML2_PROXY_FORWARD(get_with_triggering_callbacks);

  SCML2_PROXY_FORWARD(set_symbolic_name);
  SCML2_PROXY_FORWARD(get_symbolic_name);

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
  bitfield_proxy<DT>& operator--()
  {
    SCML2_PROXY_CHECK_WITH_VALUE(*this);
    --((bitfield<DT>&)(*this));    
    return *this;
  }
  bitfield_proxy<DT>& operator++()
  {
    SCML2_PROXY_CHECK_WITH_VALUE(*this);
      ++((bitfield<DT>&)(*this));    
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

  // bitfield_restriction_api_owner
  SCML2_PROXY_FORWARD_SETTER(set_restriction, bitfield_access_restriction_base<DT>*);
  SCML2_PROXY_FORWARD_SETTER(set_read_restriction, bitfield_access_restriction_base<DT>*);
  SCML2_PROXY_FORWARD_SETTER(set_write_restriction, bitfield_access_restriction_base<DT>*);
  SCML2_PROXY_FORWARD_GETTER(bitfield_access_restriction_base<DT>*, get_read_restriction);
  SCML2_PROXY_FORWARD_GETTER(bitfield_access_restriction_base<DT>*, get_write_restriction);
  SCML2_PROXY_FORWARD_MUTATOR(remove_restriction);
  SCML2_PROXY_FORWARD_MUTATOR(remove_read_restriction);
  SCML2_PROXY_FORWARD_MUTATOR(remove_write_restriction);

  // bitfield_callback_observers_api_owner
  SCML2_PROXY_FORWARD_SETTER(register_callback_observer, bitfield_callback_observer<DT>*);
  SCML2_PROXY_FORWARD_MUTATOR(unregister_all_callback_observers);
  SCML2_PROXY_FORWARD_SETTER(unregister_callback_observer, bitfield_callback_observer<DT>*);

  // bitfield_callback_observers_api_owner
  SCML2_PROXY_FORWARD_SETTER(set_read_callback, bitfield_read_callback_base<DT>* );
  SCML2_PROXY_FORWARD_SETTER(set_write_callback, bitfield_write_callback_base<DT>* );
  SCML2_PROXY_FORWARD_SETTER(set_post_write_callback, bitfield_post_write_callback_base<DT>* );
  SCML2_PROXY_FORWARD_SETTER2(set_post_write_callback, DT, bitfield_post_write_callback_base<DT>* );
  SCML2_PROXY_FORWARD_SETTER(set_default_enum_write_callback, bitfield_post_write_callback_base<DT>*);
  SCML2_PROXY_FORWARD_MUTATOR(remove_callback);
  SCML2_PROXY_FORWARD_MUTATOR(remove_read_callback);
  SCML2_PROXY_FORWARD_MUTATOR(remove_write_callback);
  SCML2_PROXY_FORWARD_MUTATOR(remove_post_write_callback);
  SCML2_PROXY_FORWARD_GETTER(bitfield_read_callback_base<DT>*,get_read_callback);
  SCML2_PROXY_FORWARD_GETTER(bitfield_write_callback_base<DT>*,get_write_callback);

  // bitfield_debug_callback_api_owner
  SCML2_PROXY_FORWARD_SETTER(set_debug_read_callback, bitfield_debug_read_callback_base<DT>*);
  SCML2_PROXY_FORWARD_SETTER(set_debug_write_callback, bitfield_debug_write_callback_base<DT>*);
  SCML2_PROXY_FORWARD_MUTATOR(remove_debug_callback);
  SCML2_PROXY_FORWARD_MUTATOR(remove_debug_read_callback);
  SCML2_PROXY_FORWARD_MUTATOR(remove_debug_write_callback);
  SCML2_PROXY_FORWARD_GETTER(bitfield_debug_read_callback_base<DT>*, get_debug_read_callback);
  SCML2_PROXY_FORWARD_GETTER(bitfield_debug_write_callback_base<DT>*, get_debug_write_callback);

  // bitfield_description_api_owner
  SCML2_PROXY_FORWARD_SETTER(set_description, const std::string&);
  SCML2_PROXY_FORWARD_GETTER(const std::string&, get_description);

  // more functions from bitfield_base that we need binary compatibility for
  unsigned int get_offset() const {
    SCML2_PROXY_CHECK(get_offset);
    return ((bitfield<DT>&)(*this)).get_offset();
  }
  unsigned int get_size() const {
    SCML2_PROXY_CHECK(get_size);
    return ((bitfield<DT>&)(*this)).get_size();
  }

  const reg<DT>& get_register() const {
    SCML2_PROXY_CHECK_FATAL();
    return ((const bitfield<DT>&)(*this)).get_register();
  }

  reg<DT>& get_register() {
    SCML2_PROXY_CHECK_FATAL();
    return ((bitfield<DT>&)(*this)).get_register();
  }


 private:
  bitfield_proxy(const bitfield_proxy&);
};

}

#endif
