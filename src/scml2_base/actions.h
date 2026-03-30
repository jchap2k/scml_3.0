/***************************************************************************
 * Copyright 1996-2025 Synopsys, Inc.
 *
 * This Synopsys software and all associated documentation are proprietary
 * to Synopsys, Inc. and may only be used pursuant to the terms and
 * conditions of a written license agreement with Synopsys, Inc.
 * All other use, reproduction, modification, or distribution of the
 * Synopsys software or the associated documentation is strictly prohibited.
 ***************************************************************************/

#ifndef SCML2_OBJECT_BASE_ACTIONS_H_INCLUDED
#define SCML2_OBJECT_BASE_ACTIONS_H_INCLUDED

#include "scml2_base/utils.h"

#ifdef SNPS_SLS_VP_BASE

#include "scml2_base/setter_owner.h"

#include <vector>
#include <memory>
#include <functional>

namespace scml2 { namespace base {

class action_entry_base {
public:
  action_entry_base() = default;
  action_entry_base(const action_entry_base&) = delete;
  action_entry_base& operator=(const action_entry_base&) = delete;
  virtual ~action_entry_base()  = default;
};

namespace {

template<typename T>
class action_entry 
  : public action_entry_base
{
  std::unique_ptr<T> m_wrapped;
public:
  action_entry(std::unique_ptr<T>&& entry) 
    : m_wrapped(std::move(entry)) 
  {}
};

template<typename T>
std::unique_ptr<action_entry_base> wrap(std::unique_ptr<T>&& unwrapped)
{
  return std::unique_ptr<action_entry_base>(new action_entry<T>(std::move(unwrapped)));
}

}

class actions {
  std::vector<std::unique_ptr<action_entry_base> > m_action_entries;
public:
  using action_t = std::function<void()>;

  actions() = default;
  ~actions() = default;

  template<typename T0, typename... Ts>
  void add_action(const action_t& action, T0& t0, Ts&... ts) {
    add_action(action, t0);
    add_action(action, ts...);
  }

  template<typename T0>
  void add_action(const action_t& action, T0& entry) {
    m_action_entries.emplace_back(wrap(create_entry(entry, action, "unknown")));
  }
  
  template<typename T0, typename... Ts>
  void add_action(const std::string& ca_expression, const action_t& action,  T0& t0, Ts&... ts) {
    add_action(ca_expression, action, t0);
    add_action(ca_expression, action, ts...);
  }

  template<typename T0>
  void add_action(const std::string& ca_expression, const action_t& action, T0& entry) {
    m_action_entries.emplace_back(wrap(create_entry(entry, action, ca_expression)));
  }


  void clear() {
    m_action_entries.clear();
  }
};

}}

#endif // SNPS_SLS_VP_BASE

#endif // SCML2_OBJECT_BASE_ACTIONS_H_INCLUDED
