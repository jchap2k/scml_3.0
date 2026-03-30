/***************************************************************************
 * Copyright 1996-2025 Synopsys, Inc.
 *
 * This Synopsys software and all associated documentation are proprietary
 * to Synopsys, Inc. and may only be used pursuant to the terms and
 * conditions of a written license agreement with Synopsys, Inc.
 * All other use, reproduction, modification, or distribution of the
 * Synopsys software or the associated documentation is strictly prohibited.
 ***************************************************************************/
#ifndef SCML2_PROXY_BASE_H
#define SCML2_PROXY_BASE_H

#include <scml2_logging/snps_vp_dll.h>
#include <scml2/streams.h>

#include <systemc>

#include <memory>
#include <string>
#include <type_traits>

namespace scml2 {

#define SCML2_PROXY_FORWARD_GETTER(RTYPE, NAME)		\
  virtual RTYPE NAME() const override			\
  {							\
    SCML2_PROXY_CHECK(NAME);				\
    return m_proxied->NAME();				\
  }

#define SCML2_PROXY_FORWARD_SETTER(NAME, TYPE)				\
  virtual auto NAME(TYPE v) -> decltype(((proxied_type2*)0)->NAME(v)) override \
  {									\
    SCML2_PROXY_CHECK(NAME, v);						\
    return m_proxied->NAME(v);						\
  }
  
#define SCML2_PROXY_FORWARD_SETTER2(NAME, TYPE1, TYPE2)			\
  virtual auto NAME(TYPE1 v1, TYPE2 v2) -> decltype(((proxied_type2*)0)->NAME(v1, v2)) override \
  {								\
    SCML2_PROXY_CHECK(NAME, v1, v2);				\
    return m_proxied->NAME(v1, v2);				\
  }

#define SCML2_PROXY_FORWARD_MUTATOR(NAME)			\
  virtual auto NAME() -> decltype(((proxied_type2*)0)->NAME()) override	\
  {								\
    SCML2_PROXY_CHECK(NAME);					\
    return m_proxied->NAME();					\
  }

#define SCML2_PROXY_FORWARD(NAME)					\
  template<typename... Args>						\
  auto NAME(Args&&... args) -> decltype(((proxied_type2*)0)->NAME(args...))	\
  {									\
    SCML2_PROXY_CHECK(NAME, args...);					\
    return (this->m_proxied)->NAME(args...);				\
  }

#define SCML2_PROXY_FORWARD_THIS(NAME)					\
  template<typename... Args>						\
  auto NAME(Args&&... args) -> decltype(((proxied_type2*)0)->NAME(args...))	\
  {									\
    SCML2_PROXY_CHECK_WITH_VALUE(*this);					\
    (this->m_proxied)->NAME(args...);				\
    return *this;						\
  }

class SNPS_VP_API nonexisting_proxy_access_exception
  : public std::runtime_error
{
public:
  nonexisting_proxy_access_exception() 
    : std::runtime_error("Access to nonexisting scml2::proxy_base")
  {}
};

#ifdef _WIN32
#define __func__ __FUNCTION__
#endif

namespace {
  template<typename RETURN_TYPE>
  RETURN_TYPE helper() { static typename std::remove_reference<RETURN_TYPE>::type result; return result; }
  template<> inline void helper<void>() {}
}

#define SCML2_PROXY_CHECK_WITH_VALUE(VALUE)					\
  if (this->m_proxied == 0) {						\
    if (::getenv("SNPS_VP_EMPTY_PROXY_NONFATAL") != nullptr) {		\
      SCML2_ERROR_TO(&(this->m_parent), GENERIC_ERROR) << "empty " << this->type() << " proxy " << this->m_name << " attempted to be accessed in " << __func__ << std::endl; \
      return VALUE; \
    } else {								\
      SCML2_ERROR_TO(&(this->m_parent), FATAL_ERROR) << "empty " << this->type() << " proxy " << this->m_name << " attempted to be accessed in " << __func__ << std::endl; \
      throw nonexisting_proxy_access_exception();			\
    }									\
  }									\

#define SCML2_PROXY_CHECK_WITH_TYPE(TYPE)				\
  SCML2_PROXY_CHECK_WITH_VALUE(helper<TYPE>())

#define SCML2_PROXY_CHECK(NAME, ...)					\
  SCML2_PROXY_CHECK_WITH_TYPE(decltype((this->m_proxied)->NAME(__VA_ARGS__)))	

#define SCML2_PROXY_CHECK_FATAL()					\
  if (this->m_proxied == 0) {						\
    SCML2_ERROR_TO(&(this->m_parent), FATAL_ERROR) << "empty " << this->type() << " proxy " << this->m_name << " attempted to be accessed in " << __func__ << std::endl; \
    throw nonexisting_proxy_access_exception();				\
  }									\

template <typename MEMORY_OBJECT>
class proxy_base
{
protected:
  std::string get_basename() const {
    std::string::size_type pos = m_name.rfind('.');
    if (pos == std::string::npos) {
      return m_name;
    } else {
      return m_name.substr(pos +1);
    }
  }

  virtual std::string type() const = 0;
  sc_core::sc_module& m_parent;
  const std::string m_name;
  std::unique_ptr<MEMORY_OBJECT> m_proxied;
  mutable streams* m_streams;
public:
  bool proxied_exists() const {
    return (bool)m_proxied;
  }

  proxy_base(const std::string& name)
#if (!defined CWR_SYSTEMC && defined SYSTEMC_VERSION && SYSTEMC_VERSION <= 20221128 /*2.3.4*/)
    : m_parent(*sc_core::sc_get_curr_simcontext()->hierarchy_curr())
#elif (defined CWR_SYSTEMC)
    : m_parent(*static_cast<sc_core::sc_module*>(sc_core::sc_get_curr_simcontext()->hierarchy_curr()))
#else
    : m_parent(*static_cast<sc_core::sc_module*>(sc_core::sc_get_current_object()))
#endif
    , m_name(sc_core::sc_get_current_object() != 0 ? (sc_core::sc_get_current_object()->name() + ("." + name)) : name )
    , m_streams(0)
  {
#if (!defined CWR_SYSTEMC && defined SYSTEMC_VERSION && SYSTEMC_VERSION <= 20221128 /*2.3.4*/)
#elif (defined CWR_SYSTEMC)
    sc_core::sc_object_host* host = sc_core::sc_get_curr_simcontext()->hierarchy_curr();
    assert(dynamic_cast<sc_core::sc_module*>(host) != nullptr || host == nullptr);
    (void)(host);
#else
    sc_core::sc_object* host = sc_core::sc_get_current_object();
    assert(dynamic_cast<sc_core::sc_module*>(host) != nullptr || host == nullptr);
    (void)(host);
#endif
  }

  ~proxy_base()
  {
    if (m_streams != 0) {
      delete m_streams;
    }
  }

  operator MEMORY_OBJECT&() const
  {
    SCML2_PROXY_CHECK_FATAL();
    return *m_proxied;
  }

  streams& get_streams() const {
    if ((bool)m_proxied) {
      return m_proxied->get_streams();
    }
    if (m_streams == 0) {
      m_streams = new streams(m_name);
    }
    return *m_streams;
  }
};

template<typename PROXY_TYPE>
class proxy_parent_wrapper
{
  PROXY_TYPE* m_proxy_parent;
  typename PROXY_TYPE::proxied_type* m_real_parent;
public:
  proxy_parent_wrapper(PROXY_TYPE& parent)
    : m_proxy_parent(&parent)
    , m_real_parent(0)
  {}
  proxy_parent_wrapper(typename PROXY_TYPE::proxied_type& parent)
    : m_proxy_parent(0)
    , m_real_parent(&parent)
  {}
  typename PROXY_TYPE::proxied_type& get_parent() const {
    if (m_proxy_parent != 0) {
      return (typename PROXY_TYPE::proxied_type&)(*m_proxy_parent);
    } else {
      return *m_real_parent;
    }
  }
};

}

#endif
