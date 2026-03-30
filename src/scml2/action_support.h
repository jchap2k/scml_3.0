/***************************************************************************
 * Copyright 1996-2025 Synopsys, Inc.
 *
 * This Synopsys software and all associated documentation are proprietary
 * to Synopsys, Inc. and may only be used pursuant to the terms and
 * conditions of a written license agreement with Synopsys, Inc.
 * All other use, reproduction, modification, or distribution of the
 * Synopsys software or the associated documentation is strictly prohibited.
 ***************************************************************************/
#ifndef SCML2_ACTION_SUPPORT_H
#define SCML2_ACTION_SUPPORT_H

#include "scml2/reg.h"
#include "scml2/memory_put_callback.h"
#include "scml2/bitfield_put_callback.h"
#include "scml2/pin_callback_base.h"
#include "scml2/tagged_message_macros.h"

#include <memory>
#include <functional>

namespace {
using action_t = std::function<void()>;
}

namespace scml2 {

namespace {

// base memory support

template<typename DT, typename MEM_OBJ>
struct mem_cb_action_adapter {
  const action_t m_action;
  const MEM_OBJ& m_mem_obj;
  std::string m_ca_expression;
  
  void operator()(const DT& value,unsigned long long) {
    SCML2_MODEL_INTERNAL_TO(&m_mem_obj, LEVEL6) << "Recomputing \"" << m_ca_expression << "\", due to change of " << m_mem_obj.get_name() << " (0x" << std::hex << value <<")"<< std::endl;
    m_action();
  } 
  void operator()(const DT& value) {
    SCML2_MODEL_INTERNAL_TO(&m_mem_obj, LEVEL6) << "Recomputing \"" << m_ca_expression << "\", due to change of " << m_mem_obj.get_name() << " (0x" << std::hex << value <<")"<< std::endl;
    m_action();
  }
  
};

template<typename DT, template<typename> class MEM_OBJ, typename MEM_CB>
class mem_obj_action_entry 
{
  using this_t = mem_obj_action_entry<DT, MEM_OBJ, MEM_CB>;
  MEM_OBJ<DT>& m_mem_obj;
  MEM_CB* m_put_callback;
public:
  template<typename... Args>
  mem_obj_action_entry(MEM_OBJ<DT>& mem_obj, const action_t& action, const std::string& ca_expression, Args&&... args) 
    : m_mem_obj(mem_obj)
    , m_put_callback(new MEM_CB(&m_mem_obj, mem_cb_action_adapter<DT, MEM_OBJ<DT> >{action, m_mem_obj, ca_expression}, std::forward<Args>(args)...)) {
    m_put_callback->ref();
    add_put_callback(m_mem_obj, m_put_callback);
  }
  mem_obj_action_entry(const this_t&) = delete;
  this_t& operator=(const this_t&) = delete;
  virtual ~mem_obj_action_entry() {
    remove_put_callback(m_mem_obj, m_put_callback);
    m_put_callback->unref();
  }
};

// reg support
  
template<typename DT>
using reg_callback_t = memory_put_callback<DT, scml2::reg, mem_cb_action_adapter<DT, scml2::reg<DT> > >;

template<typename DT>
using reg_action_entry_t = mem_obj_action_entry<DT, scml2::reg, reg_callback_t<DT> >;

// bitfield support

template<typename DT>
using bitfield_callback_t = bitfield_put_callback<DT, mem_cb_action_adapter<DT, scml2::bitfield<DT> > >;

template<typename DT>
using bitfield_action_entry_t = mem_obj_action_entry<DT, scml2::bitfield, bitfield_callback_t<DT> >;

} // anonymous

template<typename DT>
std::unique_ptr<reg_action_entry_t<DT>> create_entry(scml2::reg<DT>& reg, const action_t& action, const std::string& ca_expression) {
  return std::unique_ptr<reg_action_entry_t<DT>>(new reg_action_entry_t<DT>(reg, action, ca_expression, 0, 0));
}

template<typename DT>
std::unique_ptr<reg_action_entry_t<DT>> create_entry(scml2::reg_proxy<DT>& reg, const action_t& action, const std::string& ca_expression) {
  return std::unique_ptr<reg_action_entry_t<DT>>(reg.proxied_exists() ? new reg_action_entry_t<DT>(reg, action, ca_expression,  0, 0) : nullptr);
}

template<typename DT>
std::unique_ptr<bitfield_action_entry_t<DT>> create_entry(scml2::bitfield<DT>& bitfield, const action_t& action, const std::string& ca_expression) {
  return std::unique_ptr<bitfield_action_entry_t<DT>>(new bitfield_action_entry_t<DT>(bitfield, action, ca_expression ));  
}

template<typename DT>
std::unique_ptr<bitfield_action_entry_t<DT>> create_entry(scml2::bitfield_proxy<DT>& bitfield, const action_t& action, const std::string& ca_expression) {
  return std::unique_ptr<bitfield_action_entry_t<DT>>(bitfield.proxied_exists() ? new bitfield_action_entry_t<DT>(bitfield, action, ca_expression) : nullptr);
}

} // scml2

// sc_in support

namespace sc_core {

namespace {

template<typename DT>
class sc_in_action_entry
  : public scml2::pin_callback_base<DT>
{
  using this_t = sc_in_action_entry<DT>;

  const action_t m_action;

  void execute() override {
    m_action();
  }
public:
  sc_in_action_entry(sc_core::sc_in<DT>& pin, const action_t& action)
    : scml2::pin_callback_base<DT>(pin, &pin.value_changed())
    , m_action(action)
  {}
  ~sc_in_action_entry() {
    // no way to remove the dynamic process
  }
  sc_in_action_entry(const this_t&) = delete;
  this_t& operator=(const this_t&) = delete;
};

} // anonymous

template<typename DT>
std::unique_ptr<sc_in_action_entry<DT>> create_entry(sc_core::sc_in<DT>& pin, const action_t& action) {
  return std::unique_ptr<sc_in_action_entry<DT>>(new sc_in_action_entry<DT>(pin, action));
}

} // sc_core

#endif
