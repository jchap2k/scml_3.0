/***************************************************************************
 * Copyright 1996-2025 Synopsys, Inc.
 *
 * This Synopsys software and all associated documentation are proprietary
 * to Synopsys, Inc. and may only be used pursuant to the terms and
 * conditions of a written license agreement with Synopsys, Inc.
 * All other use, reproduction, modification, or distribution of the
 * Synopsys software or the associated documentation is strictly prohibited.
 ***************************************************************************/
#ifndef __INITIATOR_PORT_ADAPTOR_IMPL_H__
#define __INITIATOR_PORT_ADAPTOR_IMPL_H__

#include <scml2/port_adaptor_impl.h>
#include <scml2/initiator_port_adaptor.h>
#include <scml2/dmi_handler.h>

namespace scml2
{

class initiator_port_adaptor_impl : public port_adaptor_impl,
                                    public initiator_port_adaptor {
public:
  initiator_port_adaptor_impl(const std::string& name, socket_if* sock);
  ~initiator_port_adaptor_impl();

  /** Port Adaptor interface */
  virtual sc_core::sc_module* get_sc_module() { return dynamic_cast<sc_core::sc_module*>(this); }
  virtual std::string get_name() const { return name(); }
  virtual std::string get_socket_name() const { return mSocket->name(); }
  virtual unsigned int get_bus_width() const { return mSocket->get_bus_width(); }
  virtual unsigned int get_burst_length(tlm::tlm_generic_payload& trans) const { return port_adaptor_impl::get_burst_length(trans); }
  virtual unsigned int get_trans_id(tlm::tlm_generic_payload& trans) const { return port_adaptor_impl::get_trans_id(trans); }

  /** Set attributes */
  virtual void set_attribute(const std::string& name, int value) { port_adaptor_impl::set_attribute(name, value); }
  virtual int get_attribute(const std::string& name) { return port_adaptor_impl::get_attribute(name); }

  virtual void execute_command(const std::string& cmd, void* arg1, void* arg2, void* arg3)
  {
    port_adaptor_impl::execute_command(cmd, arg1, arg2, arg3);
  }

  using port_adaptor::before_end_of_elaboration;
  virtual void before_end_of_elaboration() { port_adaptor_impl::before_end_of_elaboration(); }


  virtual scml2::callback_event_enum get_event_id(unsigned int protocol_state) const { return port_adaptor_impl::get_event_id(protocol_state); }

  /** API to register a callback for the specified event in the transaction life cycle */
  virtual bool set_timing_callback(callback_event_enum event_id, timing_callback_base* cb) { return port_adaptor_impl::set_timing_callback(event_id, cb); }

  /** Convenience function to return if there is any callback registered for a particular protocol state */
  virtual bool has_callback(callback_event_enum event_id) const { return port_adaptor_impl::has_callback(event_id); }

  virtual unsigned int get_unique_trans_id() const;

  /** API to request a transaction object */
  virtual tlm::tlm_generic_payload& alloc_and_init_trans(tlm::tlm_command cmd);

  /** DMI */
  virtual void enable_dmi();
  virtual void disable_dmi();
  virtual bool is_dmi_enabled() const { return mDmiHandler.is_dmi_enabled(); }

  /** API to send a transaction over the bus */
  virtual unsigned int read_debug(unsigned long long address, unsigned char* data, unsigned int count);
  virtual unsigned int write_debug(unsigned long long address, const unsigned char* data, unsigned int count);
  virtual bool read(unsigned long long address, unsigned char* data, unsigned int count, sc_core::sc_time& t);
  virtual bool write(unsigned long long address, const unsigned char* data, unsigned int count, sc_core::sc_time& t);

  using initiator_port_adaptor::send_transaction;
  virtual bool send_transaction(tlm::tlm_generic_payload& trans);
  virtual bool send_transaction(tlm::tlm_generic_payload& trans, sc_time& t);
  virtual bool send_transaction(tlm::tlm_generic_payload& trans, const sc_time& t);

  virtual bool update_transaction(tlm::tlm_generic_payload& trans, sc_time& t) { return port_adaptor_impl::update_transaction(trans, t); }
  virtual bool update_transaction(tlm::tlm_generic_payload& trans, sc_time& t, scml2::callback_event_enum event_id);


  /** mappable_if */
  virtual std::string get_mapped_name() const;

  virtual void b_transport(tlm::tlm_generic_payload& trans, sc_core::sc_time& t);
  virtual bool get_direct_mem_ptr(tlm::tlm_generic_payload& trans, tlm::tlm_dmi& dmiData);
  virtual unsigned int transport_dbg(tlm::tlm_generic_payload& trans);

  virtual void register_bw_direct_mem_if(tlm::tlm_bw_direct_mem_if* bwInterface);
  virtual void unregister_bw_direct_mem_if(tlm::tlm_bw_direct_mem_if* bwInterface);

  /** bw transport interface */
  virtual void invalidate_direct_mem_ptr(sc_dt::uint64 start, sc_dt::uint64 end);
  virtual tlm::tlm_sync_enum nb_transport_bw(tlm::tlm_generic_payload& trans, tlm::tlm_phase& phase, sc_time& t);

protected:

  bool can_send_lt_transaction(bool& is_lt_mode);
  bool send_lt_transaction(tlm::tlm_generic_payload& trans, sc_time& t, bool& is_lt_mode);
  bool send_lt_transaction(tlm::tlm_generic_payload& trans, const sc_time& t, bool& is_lt_mode);
  void start_transaction(payload_info* p_info, const sc_time &t);
  virtual void on_txn_complete(payload_info* p_info, const sc_time& t);
  virtual void do_fwd_call(payload_info* p_info, sc_time& t);
  virtual void do_bwd_call(payload_info* p_info, sc_time& t);
  virtual tlm::tlm_sync_enum do_nb_transport_fw(payload_info* p_info, sc_time& t);
  virtual void nb2b_thread(scml2::callback_event_enum event_id, payload_info* p_info);

protected:

  socket_if* mSocket;

  // attributes
  unsigned int m_rd_data_accept_cycles;
  unsigned int m_wr_data_trigger_cycles;
  unsigned int m_wr_rsp_accept_cycles;

  std::map<unsigned int, unsigned int> m_trans_ids;

  scml2::dmi_handler mDmiHandler;
  std::set<tlm::tlm_bw_direct_mem_if*> mBwDirectMemIfs;
};

} // namespace scml2

#endif  //__INITIATOR_PORT_ADAPTOR_IMPL_H__
