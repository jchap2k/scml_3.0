/***************************************************************************
 * Copyright 1996-2025 Synopsys, Inc.
 *
 * This Synopsys software and all associated documentation are proprietary
 * to Synopsys, Inc. and may only be used pursuant to the terms and
 * conditions of a written license agreement with Synopsys, Inc.
 * All other use, reproduction, modification, or distribution of the
 * Synopsys software or the associated documentation is strictly prohibited.
 ***************************************************************************/
#ifndef SCML2_MEMORY_ALIAS_PROXY_H
#define SCML2_MEMORY_ALIAS_PROXY_H

#include <scml2/memory_alias.h>
#include <scml2/memory_proxy_base.h>
#include <scml2/memory_proxy.h>

namespace scml2 {

class SNPS_VP_API memory_alias_base_proxy
{
public:
  typedef memory_alias_base proxied_type;

  memory_alias_base_proxy() {}
  virtual ~memory_alias_base_proxy() {}

  virtual operator memory_alias_base&() = 0;
};

template <typename DT>
class memory_alias_proxy 
  : public memory_proxy_base<memory_alias<DT>>
  , public memory_alias_base_proxy
{
  virtual std::string type() const override {return "memory alias";}
public:
  typedef memory_alias<DT> proxied_type2;
  using proxy_base<memory_alias<DT>>::proxied_exists;

  typedef DT data_type;

  explicit memory_alias_proxy(const std::string& name)
    : memory_proxy_base<memory_alias<DT>>(name) {
  }

  void init(const proxy_parent_wrapper<toplevel_memory_base_proxy>& parent,
	    unsigned long long offset,
	    unsigned long long size,
	    bool should_exist = true) {
    if (should_exist) {
      this->m_proxied.reset(new memory_alias<DT>(this->get_basename(), parent.get_parent(), offset, size));
    }
  }

  memory_alias_proxy(const std::string& name,
		     const proxy_parent_wrapper<toplevel_memory_base_proxy>& parent,
		     unsigned long long offset,
		     unsigned long long size,
		     bool should_exist)
    : memory_alias_proxy(name)
  {
    init(parent, offset, size, should_exist);
  }

  void init(const proxy_parent_wrapper<memory_alias_base_proxy>& parent,
	    unsigned long long offset,
	    unsigned long long size,
	    bool should_exist = true)
  {
    if (should_exist) {
      this->m_proxied.reset(new memory_alias<DT>(this->get_basename(), parent.get_parent(), offset, size));
    }
  }
  
  memory_alias_proxy(const std::string& name,
		     const proxy_parent_wrapper<memory_alias_base_proxy>& parent,
		     unsigned long long offset,
		     unsigned long long size,
		     bool should_exist)
    : memory_alias_proxy(name)
  {
    init(parent, offset, size, should_exist);
  }

  virtual ~memory_alias_proxy()
  {
  }

  virtual operator memory_alias_base&()
  {
    SCML2_PROXY_CHECK_FATAL();
    return *this->m_proxied;
  }

  memory_index_reference<DT> operator[](unsigned long long index)
  {
    SCML2_PROXY_CHECK_FATAL();
    return (*(this->m_proxied))[index];
  }

  DT operator[](unsigned long long index) const
  {
    SCML2_PROXY_CHECK_WITH_TYPE(DT);
    return (*(this->m_proxied))[index];
  }
  SCML2_PROXY_FORWARD(put);
  SCML2_PROXY_FORWARD(get);
  SCML2_PROXY_FORWARD(put_debug);
  SCML2_PROXY_FORWARD(get_debug);
  SCML2_PROXY_FORWARD(put_with_triggering_callbacks);
  SCML2_PROXY_FORWARD(get_with_triggering_callbacks);

  SCML2_PROXY_FORWARD(set_symbolic_name);
  SCML2_PROXY_FORWARD(get_symbolic_name);

  SCML2_PROXY_FORWARD(b_transport);
  SCML2_PROXY_FORWARD(transport_dbg);
  SCML2_PROXY_FORWARD(transport_without_triggering_callbacks);
  SCML2_PROXY_FORWARD(transport_debug_without_triggering_callbacks);
private:
  memory_alias_proxy(const memory_alias_proxy<DT>&);
};

}

#endif
