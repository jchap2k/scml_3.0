/***************************************************************************
 * Copyright 1996-2025 Synopsys, Inc.
 *
 * This Synopsys software and all associated documentation are proprietary
 * to Synopsys, Inc. and may only be used pursuant to the terms and
 * conditions of a written license agreement with Synopsys, Inc.
 * All other use, reproduction, modification, or distribution of the
 * Synopsys software or the associated documentation is strictly prohibited.
 ***************************************************************************/

#include <scml2/memory_base.h>
#include <scml2/memory_region_registry.h>
#include <scml2/thread_pool.h>
#include <sysc/kernel/sc_dynamic_processes.h>
#include <scml2/target_port_adaptor_impl.h>
#include <scml2/tlm2_gp_target_port_adaptor.h>

#include <iostream>

typedef std::map<scml2::mappable_if*, scml2::target_port_adaptor_impl*> memory_adaptor_map;
static memory_adaptor_map adaptor_map;

namespace scml2
{

target_port_adaptor*
target_port_adaptor::_create_adaptor(const std::string& protocol, const std::string& name, socket_if* sock)
{
  if (protocol == "TLM2_GP") {
    return new tlm2_gp_target_port_adaptor(name, sock);
  }
  else {
    cerr << "ERROR : " << name << " : Failed to create target_port_adaptor : no adaptor available for protocol " << protocol << endl;
    //sc_exit();
    return NULL;
  }
}

bool
target_port_adaptor::set_target_callback(mappable_if& dest, callback_event_enum event_id, timing_callback_base* cb)
{
  memory_adaptor_map::iterator itr = adaptor_map.find(&dest);
  if (itr == adaptor_map.end()) {
    std::cout << "set_target_callback failed: mappable_if object not bound to a target_port_adaptor_impl" << endl;
    return false;
  }
  else {
    return itr->second->port_adaptor_impl::set_timing_callback(event_id, cb);
  }
}


//-----------------------------------------------------------------------------

target_port_adaptor_impl::target_port_adaptor_impl(const std::string& name, socket_if* sock)
  : port_adaptor_impl(name),
    target_port_adaptor(name),
    mSocket(sock),
    mDestination(0),
    mBoundDestinations(0),
    mSelectCallback(0),
    mError(name, logging::severity::error()),
    // attribute default values
    m_invoke_behavior_callbacks(1),
    m_auto_behavior(0),
    m_rd_cmd_accept_cycles(0),
    m_rd_data_trigger_cycles(0),
    m_wr_cmd_accept_cycles(0),
    m_wr_data_accept_cycles(0),
    m_wr_rsp_trigger_cycles(0)
{
  m_sysc_module = this;

  // link attributes
  m_attributes["invoke_behavior_cbks"] = (int*)&m_invoke_behavior_callbacks;
  m_attributes["auto_behavior"] = (int*)&m_auto_behavior;
  m_attributes["rd_cmd_accept_cycles"] = (int*)&m_rd_cmd_accept_cycles;
  m_attributes["rd_data_trigger_cycles"] = (int*)&m_rd_data_trigger_cycles;
  m_attributes["wr_cmd_accept_cycles"] = (int*)&m_wr_cmd_accept_cycles;
  m_attributes["wr_data_accept_cycles"] = (int*)&m_wr_data_accept_cycles;
  m_attributes["wr_rsp_trigger_cycles"] = (int*)&m_wr_rsp_trigger_cycles;
}

target_port_adaptor_impl::~target_port_adaptor_impl()
{
  delete mSocket;

  // remove any memories bound to this adaptor
  memory_adaptor_map::iterator itr = adaptor_map.begin();
  while (itr != adaptor_map.end()) {
    memory_adaptor_map::iterator check = itr++;
    if (check->second == this)
      adaptor_map.erase(check);
  }

  if (mSelectCallback) {
    mSelectCallback->unref();
  }
}

void
target_port_adaptor_impl::operator()(mappable_if& destination)
{
  if (!mDestination) {
    // The first bound destination is the default destination
    mDestination = &destination;
  }
  mDestination->register_bw_direct_mem_if(this);
  mBoundDestinations++;

  adaptor_map[&destination] = this;
}


bool
target_port_adaptor_impl::is_behavior_event(scml2::callback_event_enum event_id) const
{
  return event_id == RD_ADDR_START || event_id == WR_DATA_LAST_START;
}

void
target_port_adaptor_impl::
set_select_callback(memory_select_callback_base* cb)
{
  if (mSelectCallback) {
    mSelectCallback->unref();
  }
  mSelectCallback = cb;
  if (mSelectCallback) {
    mSelectCallback->ref();
  }
}

mappable_if*
target_port_adaptor_impl::get_destination(tlm::tlm_generic_payload& payload) const
{
  if (mSelectCallback) {
    return mSelectCallback->execute(payload);
  }
  else {
    if (mBoundDestinations > 1) {
      PORT_ADAPTOR_ERROR("Multiple mappable_if interfaces bound, and no select callback registered");
    }
    return mDestination;
  }
}


void
target_port_adaptor_impl::b_transport(tlm::tlm_generic_payload& trans, sc_time& t)
{
  PORT_ADAPTOR_TRACE((&trans), t, (trans.is_read() ? "READ" : (trans.is_write() ? "WRITE" : "IGNORE")) << " ADDR: 0x" << hex << trans.get_address() << dec);

  if (can_invoke_timing_callbacks(true)) {
    payload_info *p_info = alloc_payload_info(trans);
    p_info->acquire(this, &m_pending_state_pool, trans);
    p_info->m_lt_mode_txn = true;
    p_info->m_waiting_for_complete = true;

    align_to_clock_edge(t);

    push_state(p_info, t, trans.is_write() ? scml2::WR_ADDR_START : scml2::RD_ADDR_START);
    while (!p_info->m_complete)
      wait(m_txn_complete_event);

    // wait for downstream components to be finished with payload if we own it
    p_info->wait_for_zero_ref_count(trans);

    t = get_latest_time(sc_core::SC_ZERO_TIME, p_info->m_current.m_time);
  }
  else if (m_invoke_behavior_callbacks) {
    mappable_if* destination = get_destination(trans);
    if (destination) {
      destination->b_transport(trans, t);
    }
    else {
      static bool error_logged = false;
      if (!error_logged) {
        PORT_ADAPTOR_ERROR("Received b_transport to address 0x" << std::hex << trans.get_address() << std::dec
                            << ", but no mappable_if interface was bound. Ignoring.");
        error_logged = true;
      }
    }
  }

  PORT_ADAPTOR_TRACE((&trans), t, "return");
}

unsigned int
target_port_adaptor_impl::transport_dbg(tlm::tlm_generic_payload& trans)
{
  mappable_if* destination = get_destination(trans);
  if (destination) {
    return destination->transport_dbg(trans);
  }
  else {
    static bool error_logged = false;
    if (!error_logged) {
      PORT_ADAPTOR_ERROR("Received transport_dbg to address 0x" << std::hex << trans.get_address() << std::dec
                          << ", but no mappable_if interface was bound. Ignoring.");
      error_logged = true;
    }
    return 0;
  }
}

bool
target_port_adaptor_impl::get_direct_mem_ptr(tlm::tlm_generic_payload& trans, tlm::tlm_dmi& dmi_data)
{
  mappable_if* destination = get_destination(trans);
  if (destination) {
    return destination->get_direct_mem_ptr(trans, dmi_data);
  }
  else {
    dmi_data.set_start_address(0);
    dmi_data.set_end_address(~0ULL);
    return false;
  }
}

void
target_port_adaptor_impl::invalidate_direct_mem_ptr(unsigned long long startRange, unsigned long long endRange)
{
  mSocket->invalidate_direct_mem_ptr(startRange, endRange);
}


tlm::tlm_sync_enum
target_port_adaptor_impl::invoke_behavior_callback(scml2::callback_event_enum event_id, payload_info* p_info, sc_time& t)
{
  if (!m_invoke_behavior_callbacks) {
    return tlm::TLM_UPDATED;
  }

  // Invoke the bound object's b_transport callback, performing nb2b if necessary

  mappable_if* destination = get_destination(*p_info->pTrans);
  if (!destination) {
    return tlm::TLM_UPDATED;
  }

  bool sync_required = true;
  memory_base* base = dynamic_cast<memory_base*>(
    memory_region_registry::get_instance().find_memory_region_by_name(destination->get_mapped_name())
  );
  // if the memory and all aliases are NEVER_SYNCING we don't need to sync
  if (base) {
    if (p_info->pTrans->is_write() && base->has_never_syncing_write_behavior()) {
      sync_required = false;
      for (memory_base::AliasesIterator itr = base->begin_alias(); itr != base->end_alias(); ++itr) {
        if (!(*itr)->has_never_syncing_write_behavior()) {
          sync_required = true;
          break;
        }
      }
    }
    else if (!p_info->pTrans->is_write() && base->has_never_syncing_read_behavior()) {
      sync_required = false;
      for (memory_base::AliasesIterator itr = base->begin_alias(); itr != base->end_alias(); ++itr) {
        if (!(*itr)->has_never_syncing_read_behavior()) {
          sync_required = true;
          break;
        }
      }
    }
  }
  if (sync_required) {
    PORT_ADAPTOR_TRACE(p_info->pTrans, t, "sync before calling behavior callback");
    p_info->m_waiting_for_b_transport = true;
    worker_thread* thread = spawn_worker_thread(p_info, event_id);
    thread->mWakeup.notify(t);
    return tlm::TLM_ACCEPTED;
  }

  do_invoke_behavior_callback(destination, p_info, t);
  return tlm::TLM_UPDATED;
}

void
target_port_adaptor_impl::do_invoke_behavior_callback(mappable_if* destination, payload_info* p_info, sc_time& t)
{
  assert(destination != NULL);
  destination->b_transport(*p_info->pTrans, t);
}

void
target_port_adaptor_impl::
nb2b_thread(scml2::callback_event_enum event_id, payload_info* p_info)
{
  sc_core::sc_time t = sc_core::SC_ZERO_TIME;
  mappable_if* destination = get_destination(*p_info->pTrans);
  if (destination) {
    do_invoke_behavior_callback(destination, p_info, t);
  }

  // unblock payload
  p_info->m_waiting_for_b_transport = false;
  align_to_clock_edge(t);

  // advance protocol state
  if (m_auto_behavior) {
    p_info->m_current.m_time = std::max(p_info->m_current.m_time, sc_time_stamp() + t);
    process_state_machine();
  }
  else {
    push_state(p_info, t, event_id);
  }
}

//-----------------------------------------------------------------------------

tlm::tlm_sync_enum
target_port_adaptor_impl::nb_transport_fw(tlm::tlm_generic_payload& trans, tlm::tlm_phase& phase, sc_core::sc_time& t)
{
  // for OSCI; only tlm2_gp_target_port_adaptor is supported, and it takes care of
  // allocating and initialising p_info
  payload_info* p_info = alloc_payload_info(trans);
  if (!p_info) {
    PORT_ADAPTOR_TXN_FATAL((&trans), t, "nb_transport_fw : for payload that is not in progress");
    return tlm::TLM_ACCEPTED;
  }

  scml2::payload_state state = get_payload_state(p_info);

  DEBUG_CHECK(state.m_event != scml2::CBK_EVENT_NONE, &trans, t, "nb_transport_fw invoked with invalid protocol state: " << state_name(state));

  align_to_clock_edge(t);

  PORT_ADAPTOR_TRACE((&trans), t, event_name(p_info->m_current.m_event) << " => " << event_name(state.m_event));

  m_nb_transport_payload = p_info;
  push_state(p_info, t, state);

  // update return status if protocol state was advanced for this payload
  tlm::tlm_sync_enum return_status = tlm::TLM_ACCEPTED;
  if (m_returning_bw_state) {
    t = m_returning_bw_time;


    return_status = tlm::TLM_UPDATED;
    m_returning_bw_state = false;
  }

  static const char *tlm_sync_enum_str[] = { "TLM_ACCEPTED", "TLM_UPDATED", "TLM_COMPLETED" };
  PORT_ADAPTOR_TRACE((&trans), t, event_name(p_info->m_current.m_event) << ", return: " << tlm_sync_enum_str[return_status]);

  if (p_info->m_complete) {
    release_payload_info(p_info);
  }

  m_nb_transport_payload = NULL;

  return return_status;
}

//-----------------------------------------------------------------------------

/** Handle fwd state: invoke timing callback */
void
target_port_adaptor_impl::do_fwd_call(payload_info* p_info, sc_time& t)
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

/** Handle bwd state: invoke timing callback, then send state to initiator or advance internally */
void
target_port_adaptor_impl::do_bwd_call(payload_info* p_info, sc_time& t)
{
  p_info->m_current.m_time = sc_time_stamp() + t;
  /* coverity[leaked_storage] */
  SET_FT_EXT_ATTR(p_info->pTrans, scml2::trans_id_extension, p_info->m_id);

  set_protocol_state(*p_info->pTrans, p_info->m_current);

  // invoke timing callback for backward phases; user cannot change protocol state
  // but may want to log something, change extensions or timing
  {
    invoke_fwd_timing_callback(p_info, p_info->m_current, t);
  }

  if (!p_info->m_lt_mode_txn && send_state_to_socket(p_info, p_info->m_current.m_event)) {
    if (m_nb_transport_payload == NULL) {


      // send new state to initiator
      PORT_ADAPTOR_TRACE(p_info->pTrans, t, "nb_transport_bw : " << event_name(p_info->m_current.m_event));
      tlm::tlm_sync_enum status = do_nb_transport_bw(p_info, t);
      if (status == tlm::TLM_UPDATED) {
        scml2::payload_state next_state = get_payload_state(p_info);
        PORT_ADAPTOR_TRACE(p_info->pTrans, t, "nb_transport_bw returned TLM_UPDATED : " << event_name(p_info->m_current.m_event) << " => " << event_name(next_state.m_event));
        push_state(p_info, t, next_state);
      }
    }
    else {
      // send this state to initiator on nb_transport_fw return path
      m_returning_bw_state = true;
      m_returning_bw_time = t;
    }
  }
  else {
    // this state is not sent to the initiator, so just advance internally
    advance_state(p_info, t);
  }
}

tlm::tlm_sync_enum
target_port_adaptor_impl::do_nb_transport_bw(payload_info* p_info, sc_time& t)
{
  tlm::tlm_phase phase(scml2::FT_TIMING);
  return mSocket->nb_transport_bw(*p_info->pTrans, phase, t);
}

} // namespace scml2
