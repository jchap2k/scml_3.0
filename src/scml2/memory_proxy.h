/***************************************************************************
 * Copyright 1996-2025 Synopsys, Inc.
 *
 * This Synopsys software and all associated documentation are proprietary
 * to Synopsys, Inc. and may only be used pursuant to the terms and
 * conditions of a written license agreement with Synopsys, Inc.
 * All other use, reproduction, modification, or distribution of the
 * Synopsys software or the associated documentation is strictly prohibited.
 ***************************************************************************/
#ifndef SCML2_MEMORY_PROXY_H
#define SCML2_MEMORY_PROXY_H

#include <scml2_logging/snps_vp_dll.h>

#include <scml2/memory.h>
#include <scml2/memory_proxy_base.h>

namespace scml2 {

class SNPS_VP_API toplevel_memory_base_proxy
{
public:
  typedef toplevel_memory_base proxied_type;

  toplevel_memory_base_proxy() {}
  virtual ~toplevel_memory_base_proxy() {}

  virtual operator toplevel_memory_base&() = 0;
};

template <typename DT>
class memory_proxy 
  : public memory_proxy_base<memory<DT>>
  , public toplevel_memory_base_proxy
  , public memory_initialize_api_owner<DT>
{
  virtual std::string type() const override {return "memory";}
  using memory_proxy_base<memory<DT>>::m_proxied;
public:
  typedef memory<DT> proxied_type2;
  using proxy_base<memory<DT>>::proxied_exists;

  typedef DT data_type;

  void init (unsigned long long size) {
    init(size, true);
  }

  void init (unsigned long long size,
	     bool should_exist) {
    if (should_exist) {
      this->m_proxied.reset(new memory<DT>(this->get_basename(), size));
    }
  }

  explicit 
  memory_proxy(const std::string& name)
    : memory_proxy_base<memory<DT>>(name)
  {
  }

  memory_proxy(const std::string& name,
	       unsigned long long size,
	       bool should_exist)
    : memory_proxy_base<memory<DT>>(name)
  {
    init(size, should_exist);
  }

  virtual ~memory_proxy()
  {
  }

  virtual operator toplevel_memory_base&()
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

  SCML2_PROXY_FORWARD_SETTER(initialize, const DT&);
private:
  memory_proxy(const memory_proxy<DT>&);
};

}

#endif
