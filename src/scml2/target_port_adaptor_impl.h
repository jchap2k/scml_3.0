/***************************************************************************
 * Copyright 1996-2025 Synopsys, Inc.
 *
 * This Synopsys software and all associated documentation are proprietary
 * to Synopsys, Inc. and may only be used pursuant to the terms and
 * conditions of a written license agreement with Synopsys, Inc.
 * All other use, reproduction, modification, or distribution of the
 * Synopsys software or the associated documentation is strictly prohibited.
 ***************************************************************************/
#ifndef __TARGET_PORT_ADAPTOR_IMPL_H_
#define __TARGET_PORT_ADAPTOR_IMPL_H_

#include <scml2/port_adaptor_impl.h>
#include <scml2/target_port_adaptor.h>

namespace scml2
{

/*****************************************************************************
 * Name: target_port_adaptor
 *
 * Description: Base class for all target port adaptors.
 ****************************************************************************/
class target_port_adaptor_impl :
  public port_adaptor_impl,
  public target_port_adaptor
{
public:
  target_port_adaptor_impl(const std::string& name, target_port_adaptor::socket_if* sock);
  ~target_port_adaptor_impl();

  /** Port Adaptor interface */
  virtual sc_core::sc_module* get_sc_module() { return dynamic_cast<sc_core::sc_module*>(this); }
  virtual std::string get_name() const { return name(); }
  virtual std::string get_socket_name() const { return mSocket->name(); }
  virtual unsigned int get_bus_width() const { return mSocket->get_bus_width(); }
  virtual unsigned int get_burst_length(tlm::tlm_generic_payload& trans) const { return port_adaptor_impl::get_burst_length(trans); }
  virtual unsigned int get_trans_id(tlm::tlm_generic_payload& trans) const { return port_adaptor_impl::get_trans_id(trans); }

  virtual bool send_transaction(tlm::tlm_generic_payload&) { return true; }
  virtual bool send_transaction(tlm::tlm_generic_payload&, const sc_time&) { return true; }

  virtual bool update_transaction(tlm::tlm_generic_payload& trans, sc_time& t) { return port_adaptor_impl::update_transaction(trans, t); }
  virtual bool update_transaction(tlm::tlm_generic_payload& trans, sc_time& t, scml2::callback_event_enum event_id)  { return port_adaptor_impl::update_transaction(trans, t, event_id); }

  /** Set attributes */
  virtual void set_attribute(const std::string& name, int value) { port_adaptor_impl::set_attribute(name, value); }
  virtual int get_attribute(const std::string& name) { return port_adaptor_impl::get_attribute(name); }

  virtual void execute_command(const std::string& cmd, void* arg1, void* arg2, void* arg3)
  {
    port_adaptor_impl::execute_command(cmd, arg1, arg2, arg3);
  }

  using port_adaptor::before_end_of_elaboration;
  virtual void before_end_of_elaboration() { port_adaptor_impl::before_end_of_elaboration(); }

  /** Clock interface */
  ///@{

  virtual scml2::callback_event_enum get_event_id(unsigned int protocol_state) const { return port_adaptor_impl::get_event_id(protocol_state); }

  /** API to register a callback for the specified event in the transaction life cycle */
  virtual bool set_timing_callback(callback_event_enum event_id, timing_callback_base* cb) { return port_adaptor_impl::set_timing_callback(event_id, cb); }

  /** Convenience function to return if there is any callback registered for a particular event */
  virtual bool has_callback(callback_event_enum event_id) const { return port_adaptor_impl::has_callback(event_id); }

  /** Bind the adaptor to a memory */
  virtual void operator()(mappable_if& destination);
  virtual void set_select_callback(memory_select_callback_base* cb);

  /** fw transport interface */
  virtual tlm::tlm_sync_enum nb_transport_fw(tlm::tlm_generic_payload& trans, tlm::tlm_phase& phase, sc_time& t);
  virtual void b_transport(tlm::tlm_generic_payload& trans, sc_time& delay);
  virtual unsigned int transport_dbg(tlm::tlm_generic_payload& trans);
  virtual bool get_direct_mem_ptr(tlm::tlm_generic_payload& trans, tlm::tlm_dmi& dmi_data);

  virtual void invalidate_direct_mem_ptr(unsigned long long startRange, unsigned long long endRange);

protected:
  mappable_if* get_destination(tlm::tlm_generic_payload& payload) const;
  virtual bool is_behavior_event(scml2::callback_event_enum event_id) const;
  tlm::tlm_sync_enum invoke_behavior_callback(scml2::callback_event_enum event_id, payload_info* p_info, sc_time& t);

  virtual void do_invoke_behavior_callback(mappable_if* destination, payload_info* p_info, sc_time& t);
  virtual void nb2b_thread(scml2::callback_event_enum event_id, payload_info* p_info);

  virtual void do_fwd_call(payload_info* p_info, sc_time& t);
  virtual void do_bwd_call(payload_info* p_info, sc_time& t);
  virtual tlm::tlm_sync_enum do_nb_transport_bw(payload_info* p_info, sc_time& t);

protected:
  socket_if *mSocket;
  mappable_if* mDestination;
  int mBoundDestinations;
  memory_select_callback_base* mSelectCallback;
  mutable logging::stream mError;

  // attributes
  unsigned int m_invoke_behavior_callbacks;
  unsigned int m_auto_behavior;
  unsigned int m_rd_cmd_accept_cycles;
  unsigned int m_rd_data_trigger_cycles;
  unsigned int m_wr_cmd_accept_cycles;
  unsigned int m_wr_data_accept_cycles;
  unsigned int m_wr_rsp_trigger_cycles;
};

} // namespace scml2

#endif  //__TARGET_PORT_ADAPTOR_IMPL_H_
