/***************************************************************************
 * Copyright 1996-2025 Synopsys, Inc.
 *
 * This Synopsys software and all associated documentation are proprietary
 * to Synopsys, Inc. and may only be used pursuant to the terms and
 * conditions of a written license agreement with Synopsys, Inc.
 * All other use, reproduction, modification, or distribution of the
 * Synopsys software or the associated documentation is strictly prohibited.
 ***************************************************************************/

#ifndef SCML2_BASE_SETTER_OWNER_ACTION_H_INCLUDED
#define SCML2_BASE_SETTER_OWNER_ACTION_H_INCLUDED

#include "scml2_base/utils.h"

#ifdef SNPS_SLS_VP_BASE

#include "scml2_base/setter_owner.h"
#include "scml2_base/named_base.h"
#include "scml2/tagged_message_macros.h"

#include <string>
#include <functional>
#include <memory>

namespace scml2 { namespace base {

namespace {
using action_t = std::function<void()>;

template<typename DT>
class setter_owner_action_entry
  : public setter_owner_observer<DT>
{
  using this_t = setter_owner_action_entry<DT>;

  const action_t m_action;
  setter_owner<DT>& m_setter_owner;
  const std::string m_ca_expression;
  std::string m_owner_name;
  object* m_parent;

  void handle_set(const DT& value) override {
    SCML2_MODEL_INTERNAL_TO(m_parent, LEVEL6) 
        << "Recomputing \"" << m_ca_expression << "\",  due to update of " << m_owner_name << " (0x" << std::hex << impl::to_string(value) <<")"<< std::endl;
    m_action();
  }
public:
  setter_owner_action_entry(setter_owner<DT>& setter_owner, const action_t& action, const std::string& ca_expression)
    : m_action(action)
    , m_setter_owner(setter_owner)
    , m_ca_expression(ca_expression)
    , m_owner_name("unknown owner")
    , m_parent(0)
  {
    m_setter_owner.add_observer(this);
    
    if (atom* at = dynamic_cast<atom*>(&setter_owner)) {
      m_owner_name = at->get_name();
      m_parent = at->get_parent();
    }

  }
  
  ~setter_owner_action_entry() {
    m_setter_owner.remove_observer(this);
  }
  setter_owner_action_entry(this_t&) = delete;
  this_t& operator=(this_t&) = delete;
};

} // anonymous

template<typename DT>
std::unique_ptr<setter_owner_action_entry<DT>> create_entry(setter_owner<DT>& setter_owner, const action_t& action, const std::string& ca_expression) {
  return std::unique_ptr<setter_owner_action_entry<DT>>(new setter_owner_action_entry<DT>(setter_owner, action, ca_expression));
}

template<typename DT>
std::unique_ptr<setter_owner_action_entry<DT>> create_entry(setter_owner<DT>& setter_owner, const action_t& action) {
  return std::unique_ptr<setter_owner_action_entry<DT>>(new setter_owner_action_entry<DT>(setter_owner, action, "unknown"));
}


}}

#endif // SNPS_SLS_VP_BASE

#endif // SCML2_BASE_SETTER_OWNER_ACTION_H_INCLUDED
