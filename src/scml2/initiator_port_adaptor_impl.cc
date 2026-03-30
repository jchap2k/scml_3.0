/***************************************************************************
 * Copyright 1996-2025 Synopsys, Inc.
 *
 * This Synopsys software and all associated documentation are proprietary
 * to Synopsys, Inc. and may only be used pursuant to the terms and
 * conditions of a written license agreement with Synopsys, Inc.
 * All other use, reproduction, modification, or distribution of the
 * Synopsys software or the associated documentation is strictly prohibited.
 ***************************************************************************/

#include <scml2/tagged_message_macros.h>
#include <scml2/initiator_port_adaptor_impl.h>
#include <scml2/tlm2_gp_initiator_port_adaptor.h>

namespace scml2
{

//-----------------------------------------------------------------------------

/** Helper class to acquire and release payload on object scope */
class payload_wrapper {
public:
  payload_wrapper(scml2::mem_manager<scml2::ft_generic_payload>& mm) : pTrans(mm.claim()) {
    pTrans->acquire();
  }
  ~payload_wrapper() {
    pTrans->release();
  }
  scml2::ft_generic_payload* operator->() { return pTrans; }
  operator scml2::ft_generic_payload*() { return pTrans; }
private:
  scml2::ft_generic_payload* pTrans;
};

//-----------------------------------------------------------------------------

initiator_port_adaptor*
initiator_port_adaptor::_create_adaptor(const std::string& protocol, const std::string& name, socket_if* sock)
{
  if (protocol == "TLM2_GP") {
    return new tlm2_gp_initiator_port_adaptor(name, sock);
  }
  else {
    cerr << "ERROR : " << name << " : Failed to create initiator_port_adaptor : no adaptor available for protocol " << protocol << endl;
    //sc_exit();
    return NULL;
  }
}

//-----------------------------------------------------------------------------

initiator_port_adaptor_impl::initiator_port_adaptor_impl(const std::string& name, socket_if* sock)
  : port_adaptor_impl(name)
  , initiator_port_adaptor(name)
  , mSocket(sock)
  // attribute default values
  , m_rd_data_accept_cycles(0)
  , m_wr_data_trigger_cycles(0)
{
  m_is_initiator_adaptor = true;
  // link attributes
  m_attributes["rd_data_accept_cycles"] = (int*)&m_rd_data_accept_cycles;
  m_attributes["wr_data_trigger_cycles"] = (int*)&m_wr_data_trigger_cycles;
  m_attributes["wr_rsp_accept_cycles"] = (int*)&m_wr_rsp_accept_cycles;

  // setup event to signal when ready for clock period to change
  {
    m_sysc_module = this;
    sc_core::sc_spawn_options opts;
    opts.spawn_method();
    opts.set_sensitivity(&m_txn_complete_event);
    opts.dont_initialize();
  }

  mDmiHandler.set_interface((tlm::tlm_fw_direct_mem_if<tlm::tlm_generic_payload>*)mSocket);
}

initiator_port_adaptor_impl::~initiator_port_adaptor_impl()
{
  delete mSocket;
}


void
initiator_port_adaptor_impl::on_txn_complete(payload_info* p_info, const sc_time& t)
{
  port_adaptor_impl::on_txn_complete(p_info, t);

  if(m_support_unique_id_generation) {
    // release ID if no more transactions using it
    std::map<unsigned int, unsigned int>::iterator itr = m_trans_ids.find(p_info->m_id);
    if (itr != m_trans_ids.end()) {
      assert(itr->second > 0);
      if (--itr->second == 0) {
        m_trans_ids.erase(itr);
      }
    }
  }

}

unsigned int
initiator_port_adaptor_impl::get_unique_trans_id() const
{
  if(!m_support_unique_id_generation) {
    PORT_ADAPTOR_ERROR("get_unique_trans_id() only works if attribute \"support_unique_id_generation\" is set to 1");
    return 0;
  }
  unsigned int id = 0;
  while (m_trans_ids.find(id) != m_trans_ids.end())
    ++id;
  return id;
}

tlm::tlm_generic_payload&
initiator_port_adaptor_impl::alloc_and_init_trans(tlm::tlm_command cmd)
{
  scml2::ft_generic_payload* payload = m_pool.claim();

  payload->set_command(cmd);
  payload->set_data_length(mSocket->get_bus_width() / 8);
  payload->set_streaming_width(mSocket->get_bus_width() / 8);
  payload->set_data_ptr(0);
  payload->set_byte_enable_ptr(0);
  payload->set_byte_enable_length(0);
  payload->set_dmi_allowed(false);
  payload->set_response_status(tlm::TLM_INCOMPLETE_RESPONSE);
  /* coverity[leaked_storage] */
  SET_EXT_ATTR(payload, scml2::trans_id_extension, 0);

  return *(tlm::tlm_generic_payload*)payload;
}

//-----------------------------------------------------------------------------

void
initiator_port_adaptor_impl::enable_dmi()
{
  if (!is_dmi_enabled()) {
    mDmiHandler.enable_dmi();
    invalidate_direct_mem_ptr(0, (unsigned long long)-1);
  }
}

void
initiator_port_adaptor_impl::disable_dmi()
{
  if (is_dmi_enabled()) {
    mDmiHandler.disable_dmi();
    invalidate_direct_mem_ptr(0, (unsigned long long)-1);
  }
}

//-----------------------------------------------------------------------------

unsigned int
initiator_port_adaptor_impl::read_debug(unsigned long long address, unsigned char* data, unsigned int count)
{
  payload_wrapper payload(m_pool);

  payload->set_address(address);
  payload->set_command(tlm::TLM_READ_COMMAND);
  payload->set_data_ptr(data);
  payload->set_data_length(count);
  payload->set_dmi_allowed(false);

  return transport_dbg(*(tlm::tlm_generic_payload*)payload);
}

unsigned int
initiator_port_adaptor_impl::write_debug(unsigned long long address, const unsigned char* data, unsigned int count)
{
  payload_wrapper payload(m_pool);

  payload->set_address(address);
  payload->set_command(tlm::TLM_WRITE_COMMAND);
  payload->set_data_ptr(const_cast<unsigned char*>(data));
  payload->set_data_length(count);
  payload->set_dmi_allowed(false);

  return transport_dbg(*(tlm::tlm_generic_payload*)payload);
}

bool
initiator_port_adaptor_impl::read(unsigned long long address, unsigned char* data, unsigned int count, sc_core::sc_time& t)
{
  payload_wrapper payload(m_pool);
  payload->set_address(address);
  payload->set_command(tlm::TLM_READ_COMMAND);
  payload->set_data_ptr(data);
  payload->set_data_length(count);
  payload->set_streaming_width(count);
  payload->set_byte_enable_ptr(0);
  payload->set_byte_enable_length(0);
  payload->set_dmi_allowed(false);
  payload->set_response_status(tlm::TLM_INCOMPLETE_RESPONSE);

  bool lt_mode = true;

  if (can_invoke_timing_callbacks(lt_mode)) {
    payload_info* p_info = alloc_payload_info(*(tlm::tlm_generic_payload*)payload);
    assert(p_info->pTrans == NULL);
    p_info->acquire(this, &m_pending_state_pool, *(tlm::tlm_generic_payload*)payload);
    p_info->m_lt_mode_txn = lt_mode;
    p_info->m_waiting_for_complete = true;

    start_transaction(p_info, t);

    // wait for payload to complete
    while (!p_info->m_complete) {
      wait(m_txn_complete_event);
    }
    t = SC_ZERO_TIME;
  }
  else {
    // no timing callbacks; just forward over the socket
    b_transport(*payload, t);
  }

  return payload->is_response_ok();
}

bool
initiator_port_adaptor_impl::write(unsigned long long address, const unsigned char* data, unsigned int count, sc_core::sc_time& t)
{
  payload_wrapper payload(m_pool);
  payload->set_address(address);
  payload->set_command(tlm::TLM_WRITE_COMMAND);
  payload->set_data_ptr(const_cast<unsigned char*>(data));
  payload->set_data_length(count);
  payload->set_streaming_width(count);
  payload->set_byte_enable_ptr(0);
  payload->set_byte_enable_length(0);
  payload->set_dmi_allowed(false);
  payload->set_response_status(tlm::TLM_INCOMPLETE_RESPONSE);

  bool lt_mode = true;

  if (can_invoke_timing_callbacks(lt_mode)) {
    payload_info* p_info = alloc_payload_info(*(tlm::tlm_generic_payload*)payload);
    assert(p_info->pTrans == NULL);
    p_info->acquire(this, &m_pending_state_pool, *(tlm::tlm_generic_payload*)payload);
    p_info->m_lt_mode_txn = lt_mode;
    p_info->m_waiting_for_complete = true;

    start_transaction(p_info, t);

    // wait for payload to complete
    while (!p_info->m_complete) {
      wait(m_txn_complete_event);
    }
    t = SC_ZERO_TIME;
  }
  else {
    // no timing callbacks; just forward over the socket
    b_transport(*payload, t);
  }

  return payload->is_response_ok();
}

//-----------------------------------------------------------------------------

bool
initiator_port_adaptor_impl::can_send_lt_transaction(bool& is_lt_mode)
{
  is_lt_mode = !m_has_callbacks;   // use AT if timing callbacks have been registered

  if (!can_invoke_timing_callbacks(is_lt_mode)) {
    return true;
  }
  return false;
}

bool
initiator_port_adaptor_impl::send_lt_transaction(tlm::tlm_generic_payload& trans, sc_time& t, bool& is_lt_mode)
{
  if(can_send_lt_transaction(is_lt_mode)) {
    // LT mode, no timing callbacks; just forward over the socket
    b_transport(trans, t);
    return true;
  }
  return false;
}

bool
initiator_port_adaptor_impl::send_lt_transaction(tlm::tlm_generic_payload& trans, const sc_time& t, bool& is_lt_mode)
{
  if(can_send_lt_transaction(is_lt_mode)) {
    // LT mode, no timing callbacks; just forward over the socket
    // User passed a  const time. We can't change it
    sc_time delay(t);
    b_transport(trans, delay);
    return true;
  }
  return false;
}

bool
initiator_port_adaptor_impl::send_transaction(tlm::tlm_generic_payload& payload)
{
  return send_transaction(payload, SC_ZERO_TIME);
}

bool
initiator_port_adaptor_impl::send_transaction(tlm::tlm_generic_payload& trans, sc_time& t)
{
  bool is_lt_mode = false;
  if(send_lt_transaction(trans, t, is_lt_mode)) {
    return true;
  }
  return send_transaction(trans, (const sc_time&)t);
}

bool
initiator_port_adaptor_impl::send_transaction(tlm::tlm_generic_payload& trans, const sc_time &t)
{
  bool is_lt_mode = false;
  if(send_lt_transaction(trans, t, is_lt_mode)) {
    return true;
  }
  ++m_inside_api_call;

  payload_info* p_info = alloc_payload_info(trans);
  if (p_info->pTrans != NULL)
  {
    PORT_ADAPTOR_TXN_FATAL(p_info->pTrans, t, "send_transaction : payload already in progress");
    return false;
  }

  p_info->acquire(this, &m_pending_state_pool, trans);
  p_info->m_lt_mode_txn = is_lt_mode;

  start_transaction(p_info, t);

  --m_inside_api_call;

  // TODO:Currently always return true since the ft_channel handles buffering the payload
  // if it cannot be accepted yet. Could return false in that case instead and force caller
  // to retry sending the transaction
  return true;
}

void
initiator_port_adaptor_impl::start_transaction(payload_info* p_info, const sc_time &t)
{
  if(m_support_unique_id_generation) {
    ++m_trans_ids[p_info->m_id];
  }

  // reset extensions

  // set burst_size_extension to bus width if not set already
  GET_FT_EXT_ATTR(p_info->pTrans, scml2::burst_size_extension, unsigned int, burst_size);
  if (burst_size == 0) {
    /* coverity[leaked_storage] */
    SET_FT_EXT_ATTR(p_info->pTrans, scml2::burst_size_extension, p_info->m_beat_size);
  }

  PORT_ADAPTOR_TRACE(p_info->pTrans, t, (p_info->pTrans->is_read() ? "READ" : (p_info->pTrans->is_write() ? "WRITE" : "IGNORE"))
                                        << " ID: 0x" << hex << p_info->m_id << dec
                                        << ", ADDR: 0x" << hex << p_info->pTrans->get_address() << dec
                                        << ", DL: " << p_info->pTrans->get_data_length()
                                        << ", SZ: " << p_info->m_beat_size
                                        << ", BL: " << p_info->m_beat_count);

  // adjust input annotated time if necessary to respect capacity
  sc_time delay(adjust_request_time(p_info, t));
  align_to_clock_edge(delay);

  {
    push_state(p_info, delay, p_info->pTrans->is_write() ? scml2::WR_ADDR_START : scml2::RD_ADDR_START);
  }
}


//-----------------------------------------------------------------------------

bool
initiator_port_adaptor_impl::update_transaction(tlm::tlm_generic_payload& trans, sc_time& t, scml2::callback_event_enum event_id)
{
  payload_info* p_info = NULL;
  get_payload_info(p_info, &trans);
  if (!p_info)
  {
    PORT_ADAPTOR_TXN_FATAL((&trans), t, "update_transaction : payload is not in progress (event : " << event_name(event_id) << ")");
    return false;
  }

  {
    return port_adaptor_impl::update_transaction(trans, t, event_id);
  }
}

//-----------------------------------------------------------------------------

/* Backward response call from the target */
tlm::tlm_sync_enum
initiator_port_adaptor_impl::nb_transport_bw(tlm::tlm_generic_payload& trans, tlm::tlm_phase& phase, sc_core::sc_time& t)
{

  payload_info* p_info = NULL;
  get_payload_info(p_info, &trans);
  if (!p_info)
  {
    PORT_ADAPTOR_TXN_FATAL((&trans), t, "nb_transport_bw : for payload that is not in progress");
    return tlm::TLM_ACCEPTED;
  }


  scml2::payload_state state = get_payload_state(p_info);

  DEBUG_CHECK(state.m_event != scml2::CBK_EVENT_NONE, &trans, t, "nb_transport_bw invoked with invalid protocol state: " << state_name(state));

  align_to_clock_edge(t);

  PORT_ADAPTOR_TRACE((&trans), t, event_name(p_info->m_current.m_event) << " => " << event_name(state.m_event));

  m_nb_transport_payload = p_info;
  push_state(p_info, t, state);

  // update return status if protocol state was advanced for this payload
  tlm::tlm_sync_enum return_status = tlm::TLM_ACCEPTED;
  if (m_returning_bw_state) {
    t = m_returning_bw_time;


    return_status = tlm::TLM_UPDATED;
  }

  static const char *tlm_sync_enum_str[] = { "TLM_ACCEPTED", "TLM_UPDATED", "TLM_COMPLETED" };
  PORT_ADAPTOR_TRACE((&trans), t, event_name(p_info->m_current.m_event) << ", return: " << tlm_sync_enum_str[return_status]);

  if (p_info->m_complete) {
    {
      release_payload_info(p_info);
    }
  }

  m_nb_transport_payload = NULL;
  m_returning_bw_state = false;

  return return_status;
}

//-----------------------------------------------------------------------------

/** Handle bwd state: invoke timing callback */
void
initiator_port_adaptor_impl::do_bwd_call(payload_info* p_info, sc_time& t)
{
  p_info->m_current.m_time = sc_time_stamp() + t;
  /* coverity[leaked_storage] */
  SET_FT_EXT_ATTR(p_info->pTrans, scml2::trans_id_extension, p_info->m_id);
  set_protocol_state(*p_info->pTrans, p_info->m_current);
  tlm::tlm_sync_enum status = invoke_timing_callback(p_info, p_info->m_current, t);
  if (status != tlm::TLM_ACCEPTED) {
    advance_state(p_info, t);
  }
}

//-----------------------------------------------------------------------------

/** Handle fwd state: invoke timing callback, then send state to initiator or advance internally */
void
initiator_port_adaptor_impl::do_fwd_call(payload_info* p_info, sc_time& t)
{
  p_info->m_current.m_time = sc_time_stamp() + t;
  /* coverity[leaked_storage] */
  SET_FT_EXT_ATTR(p_info->pTrans, scml2::trans_id_extension, p_info->m_id);
  set_protocol_state(*p_info->pTrans, p_info->m_current);

  // invoke timing callback for forward phases; user cannot change protocol state
  // but may want to log something, change extensions or timing
  {
    invoke_fwd_timing_callback(p_info, p_info->m_current, t);
  }

  if (send_state_to_socket(p_info, p_info->m_current)) {
    if (m_nb_transport_payload == NULL) {


      // send new state to target
      if (p_info->m_lt_mode_txn) {
        // LT transaction => attempt DMI, then b_transport
        if (p_info->pTrans->is_write()) {
          scml2::callback_event_enum next_event = has_wr_rsp() ? scml2::WR_RSP_START : scml2::WR_DATA_LAST_END;
          if (mDmiHandler.write(p_info->pTrans->get_address(), p_info->pTrans->get_data_ptr(), p_info->pTrans->get_data_length(), t)) {
            p_info->pTrans->set_dmi_allowed(true);
            push_state(p_info, t, scml2::payload_state(next_event));
          }
          else {
            // spawn thread to perform b_transport for us
            set_protocol_state(*p_info->pTrans, scml2::payload_state(scml2::WR_ADDR_START));
            worker_thread* thread = spawn_worker_thread(p_info, next_event);
            thread->mWakeup.notify(t);
          }
        }
        else {
          scml2::callback_event_enum next_event = scml2::RD_DATA_LAST_START;
          if (mDmiHandler.read(p_info->pTrans->get_address(), p_info->pTrans->get_data_ptr(), p_info->pTrans->get_data_length(), t)) {
            p_info->pTrans->set_dmi_allowed(true);
            push_state(p_info, t, scml2::payload_state(next_event));
          }
          else {
            // spawn thread to perform b_transport for us
            set_protocol_state(*p_info->pTrans, scml2::payload_state(scml2::RD_ADDR_START));
            worker_thread* thread = spawn_worker_thread(p_info, next_event);
            thread->mWakeup.notify(t);
          }
        }
      }
      else {
        PORT_ADAPTOR_TRACE(p_info->pTrans, t, "nb_transport_fw : " << event_name(p_info->m_current.m_event));
        tlm::tlm_sync_enum status = do_nb_transport_fw(p_info, t);
        if (status == tlm::TLM_UPDATED) {
          scml2::payload_state next_state = get_payload_state(p_info);
          PORT_ADAPTOR_TRACE(p_info->pTrans, t, "nb_transport_fw returned TLM_UPDATED : " << event_name(p_info->m_current.m_event) << " => " << event_name(next_state.m_event));
          push_state(p_info, t, next_state);
        }
      }
    }
    else {
      // send this state to target on nb_transport_bw return path
      m_returning_bw_state = true;
      m_returning_bw_time = t;
    }
  }
  else {
    // this state is not sent to the target, so just advance internally
    advance_state(p_info, t);
  }
}

tlm::tlm_sync_enum
initiator_port_adaptor_impl::do_nb_transport_fw(payload_info* p_info, sc_time& t)
{
  tlm::tlm_phase phase(tlm::BEGIN_REQ);
  return mSocket->nb_transport_fw(*p_info->pTrans, phase, t);
}

void
initiator_port_adaptor_impl::
nb2b_thread(scml2::callback_event_enum event_id, payload_info* p_info)
{
  sc_core::sc_time t(sc_core::SC_ZERO_TIME);

  b_transport(*p_info->pTrans, t);

  // wait for downstream components to be finished with payload if we own it
  p_info->wait_for_zero_ref_count(*p_info->pTrans);

  // advance protocol state
  align_to_clock_edge(t);
  push_state(p_info, t, event_id);
}

//-----------------------------------------------------------------------------

std::string
initiator_port_adaptor_impl::get_mapped_name() const
{
  return (*mSocket).name();
}

void
initiator_port_adaptor_impl::b_transport(tlm::tlm_generic_payload& trans, sc_core::sc_time& t)
{
  PORT_ADAPTOR_TRACE((&trans), t, "");
  t = get_latest_time(t, m_req_channel.free_time());

  auto address = trans.get_address();
  if (m_try_dmi_for_b_transport && mDmiHandler.transport(trans, t)) {
    trans.set_dmi_allowed(true);
  }
  else {
    trans.set_address(address);
    (*mSocket).b_transport(trans, t);
  }

  m_req_channel.unlock(t);
}

bool
initiator_port_adaptor_impl::get_direct_mem_ptr(tlm::tlm_generic_payload& trans, tlm::tlm_dmi& dmiData)
{
  return mDmiHandler.get_direct_mem_ptr(trans, dmiData);
}

unsigned int
initiator_port_adaptor_impl::transport_dbg(tlm::tlm_generic_payload& trans)
{
  if (mDmiHandler.transport_debug(trans)) {
    trans.set_dmi_allowed(true);
    return trans.get_data_length();
  }

  return (*mSocket).transport_dbg(trans);
}

void
initiator_port_adaptor_impl::register_bw_direct_mem_if(tlm::tlm_bw_direct_mem_if* bwInterface)
{
  assert(bwInterface);
  mBwDirectMemIfs.insert(bwInterface);
}

void
initiator_port_adaptor_impl::unregister_bw_direct_mem_if(tlm::tlm_bw_direct_mem_if* bwInterface)
{
  mBwDirectMemIfs.erase(bwInterface);
}

void
initiator_port_adaptor_impl::invalidate_direct_mem_ptr(sc_dt::uint64 start_addr, sc_dt::uint64 end_addr)
{
  // invalidate DMI cache
  mDmiHandler.invalidate_direct_mem_ptr(start_addr, end_addr);

  // Invalidate registered DMI handlers
  std::set<tlm::tlm_bw_direct_mem_if*>::iterator it = mBwDirectMemIfs.begin();
  std::set<tlm::tlm_bw_direct_mem_if*>::const_iterator end = mBwDirectMemIfs.end();
  for (; it != end; ++it) {
    (*it)->invalidate_direct_mem_ptr(start_addr, end_addr);
  }
}

} // namespace scml2
