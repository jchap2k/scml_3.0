/***************************************************************************
 * Copyright 1996-2025 Synopsys, Inc.
 *
 * This Synopsys software and all associated documentation are proprietary
 * to Synopsys, Inc. and may only be used pursuant to the terms and
 * conditions of a written license agreement with Synopsys, Inc.
 * All other use, reproduction, modification, or distribution of the
 * Synopsys software or the associated documentation is strictly prohibited.
 ***************************************************************************/
#include <scml2/tlm2_gp_target_port_adaptor.h>

namespace scml2
{

tlm2_gp_target_port_adaptor::tlm2_gp_target_port_adaptor(const std::string& name, target_port_adaptor::socket_if* sock)
 :  target_port_adaptor_impl(name, sock)
{
  m_protocol_id = TLM2_GP_PROTOCOL_ID;

  // address channel is shared for read+write so only support total capacity
  m_attributes["read_capacity"] = (int*)&m_total_capacity;
  m_attributes["write_capacity"] = (int*)&m_total_capacity;

  // override default attribute values
  m_read_capacity = m_write_capacity = 0;
  m_total_capacity = 1;
  m_rd_cmd_accept_cycles = 0;
  m_wr_cmd_accept_cycles = 0;
  m_wr_rsp_trigger_cycles = 0;

  // Just re-use the callback states
  m_eventId2ProtoState[RD_ADDR_START]       = RD_ADDR_START;
  m_eventId2ProtoState[RD_ADDR_END]         = RD_ADDR_END;
  m_eventId2ProtoState[RD_DATA_START]       = RD_DATA_START;
  m_eventId2ProtoState[RD_DATA_END]         = RD_DATA_END;
  m_eventId2ProtoState[RD_DATA_LAST_START]  = RD_DATA_LAST_START;
  m_eventId2ProtoState[RD_DATA_LAST_END]    = RD_DATA_LAST_END;
  m_eventId2ProtoState[WR_ADDR_START]       = WR_ADDR_START;
  m_eventId2ProtoState[WR_ADDR_END]         = WR_ADDR_END;
  m_eventId2ProtoState[WR_DATA_START]       = WR_DATA_START;
  m_eventId2ProtoState[WR_DATA_END]         = WR_DATA_END;
  m_eventId2ProtoState[WR_DATA_LAST_START]  = WR_DATA_LAST_START;
  m_eventId2ProtoState[WR_DATA_LAST_END]    = WR_DATA_LAST_END;
  m_eventId2ProtoState[WR_RSP_START]        = WR_RSP_START;
  m_eventId2ProtoState[WR_RSP_END]          = WR_RSP_END;

  m_channels.push_back(&m_req_channel);
  m_channels.push_back(&m_response_channel);
}

tlm::tlm_phase_enum
tlm2_gp_target_port_adaptor::event_id_to_phase(scml2::callback_event_enum event_id) const
{
  switch (event_id)
  {
    case RD_ADDR_START:
    case WR_ADDR_START:
      return tlm::BEGIN_REQ;

    case RD_ADDR_END:
    case WR_DATA_LAST_END:
      return tlm::END_REQ;

    case RD_DATA_LAST_START:
    case WR_RSP_START:
      return tlm::BEGIN_RESP;

    case RD_DATA_LAST_END:
    case WR_RSP_END:
      return tlm::END_RESP;

    default:
      return tlm::UNINITIALIZED_PHASE;
  }
}

scml2::callback_event_enum
tlm2_gp_target_port_adaptor::phase_to_event_id(tlm::tlm_generic_payload& trans, const tlm::tlm_phase& phase) const
{
  switch (phase)
  {
    case tlm::BEGIN_REQ:  return trans.is_write() ? scml2::WR_ADDR_START : scml2::RD_ADDR_START;
    case tlm::END_REQ:    return trans.is_write() ? scml2::WR_DATA_LAST_END : scml2::RD_ADDR_END;
    case tlm::BEGIN_RESP: return trans.is_write() ? scml2::WR_RSP_START : scml2::RD_DATA_LAST_START;
    case tlm::END_RESP:   return trans.is_write() ? scml2::WR_RSP_END : scml2::RD_DATA_LAST_END;
    default:
      return scml2::CBK_EVENT_NONE;
  }
}

std::string
tlm2_gp_target_port_adaptor::state_name(scml2::payload_state state) const
{
  switch ((scml2::callback_event_enum)m_eventId2ProtoState[state.m_event])
  {
    case RD_ADDR_START:
    case WR_ADDR_START:
      return "BEGIN_REQ";

    case RD_ADDR_END:
    case WR_DATA_LAST_END:
      return "END_REQ";

    case RD_DATA_LAST_START:
    case WR_RSP_START:
      return "BEGIN_RESP";

    case RD_DATA_LAST_END:
    case WR_RSP_END:
      return "END_RESP";

    default:
      return port_adaptor_impl::state_name(state);
  }
}

scml2::payload_state
tlm2_gp_target_port_adaptor::get_payload_state(payload_info* p_info) const
{
  return scml2::payload_state(phase_to_event_id(*p_info->pTrans, p_info->m_tlm_phase));
}

void
tlm2_gp_target_port_adaptor::set_protocol_state(tlm::tlm_generic_payload&, scml2::payload_state)
{
  // do nothing; the tlm_phase will be set when calling nb_transport_bw
}

ft_channel*
tlm2_gp_target_port_adaptor::get_channel_for_event(payload_state state)
{
  assert(state.m_event > CBK_EVENT_NONE && state.m_event < MAX_CBK_EVENT_ID_E);

  switch (state.m_event) {
  case RD_ADDR_START:
  case RD_ADDR_END:
  case WR_ADDR_START:
  case WR_ADDR_END:
  case WR_DATA_START:
  case WR_DATA_END:
  case WR_DATA_LAST_START:
  case WR_DATA_LAST_END:
    return &m_req_channel;
  case RD_DATA_START:
  case RD_DATA_END:
  case RD_DATA_LAST_START:
  case RD_DATA_LAST_END:
  case WR_RSP_START:
  case WR_RSP_END:
    return &m_response_channel;
  default:
    assert(false);
    return NULL;
  }
}

//-----------------------------------------------------------------------------

bool
tlm2_gp_target_port_adaptor::is_valid_state_transition(payload_info* p_info, scml2::payload_state next_state) const
{
  switch (p_info->m_current.m_event) {
    case scml2::CBK_EVENT_NONE:     return ((next_state.m_event == scml2::RD_ADDR_START) || (next_state.m_event == scml2::WR_ADDR_START));
    case scml2::RD_ADDR_START:      return (next_state.m_event == scml2::RD_ADDR_END);
    case scml2::RD_ADDR_END:        return !p_info->m_waiting_for_b_transport && ((next_state.m_event == scml2::RD_DATA_START) || (next_state.m_event == scml2::RD_DATA_LAST_START));
    case scml2::RD_DATA_START:      return (next_state.m_event == scml2::RD_DATA_END);
    case scml2::RD_DATA_END:        return ((next_state.m_event == scml2::RD_DATA_START) || (next_state.m_event == scml2::RD_DATA_LAST_START));
    case scml2::RD_DATA_LAST_START: return (next_state.m_event == scml2::RD_DATA_LAST_END);

    case scml2::WR_ADDR_START:      return ((next_state.m_event == scml2::WR_ADDR_END) || (next_state.m_event == scml2::WR_DATA_LAST_END));
    case scml2::WR_ADDR_END:        return ((next_state.m_event == scml2::WR_DATA_START) || (next_state.m_event == scml2::WR_DATA_LAST_START));
    case scml2::WR_DATA_START:      return (next_state.m_event == scml2::WR_DATA_END);
    case scml2::WR_DATA_END:        return ((next_state.m_event == scml2::WR_DATA_START) || (next_state.m_event == scml2::WR_DATA_LAST_START));
    case scml2::WR_DATA_LAST_START: return (next_state.m_event == scml2::WR_DATA_LAST_END);
    case scml2::WR_DATA_LAST_END:   return !p_info->m_waiting_for_b_transport && (next_state.m_event == scml2::WR_RSP_START);
    case scml2::WR_RSP_START:       return (next_state.m_event == scml2::WR_RSP_END);
    default:
      return false;
  }
}

//-----------------------------------------------------------------------------

/** Return true if this state should be sent to the initiator (via return path or nb_transport_bw) */
bool
tlm2_gp_target_port_adaptor::send_state_to_socket(payload_info*, scml2::payload_state state) const
{
  if (m_eventId2ProtoState[state.m_event] == scml2::INVALID_PROTOCOL_STATE) {     // only send to initiator if this state maps to a protocol_state
    return false;
  }

  switch (state.m_event) {
    case scml2::RD_ADDR_END:
    case scml2::RD_DATA_LAST_START:
    case scml2::WR_DATA_LAST_END:
    case scml2::WR_RSP_START:
      return true;

    default:
      return false;         // these states are never sent to the initiator
  }
}

//-----------------------------------------------------------------------------
tlm::tlm_sync_enum
tlm2_gp_target_port_adaptor::nb_transport_fw(tlm::tlm_generic_payload& trans, tlm::tlm_phase& phase, sc_time& t)
{
  // need to get p_info here so we can store tlm_phase before calling target_port_adaptor_impl::nb_transport_fw
  payload_info* p_info = alloc_payload_info(trans);
  if (p_info->pTrans == NULL) {
    p_info->acquire(this, &m_pending_state_pool, trans);

    PORT_ADAPTOR_TRACE((&trans), t, (trans.is_read() ? "READ" : (trans.is_write() ? "WRITE" : "IGNORE"))
                       << " ID: 0x" << hex << p_info->m_id << dec
                       << ", ADDR: 0x" << hex << trans.get_address() << dec
                       << ", DL: " << trans.get_data_length()
                       << ", SZ: " << p_info->m_beat_size
                       << ", BL: " << p_info->m_beat_count);

    // adjust input annotated time if necessary to respect capacity
    t = adjust_request_time(p_info, t);
  }

  p_info->m_tlm_phase = phase;

  phase = scml2::FT_TIMING;
  tlm::tlm_sync_enum status = target_port_adaptor_impl::nb_transport_fw(trans, phase, t);
  phase = p_info->m_tlm_phase = event_id_to_phase(p_info->m_current.m_event);
  return status;
}

//-----------------------------------------------------------------------------

/** Advance protocol state (if possible) for this payload */
void
tlm2_gp_target_port_adaptor::advance_state(payload_info* p_info, sc_time& t)
{
  scml2::payload_state next_state(scml2::CBK_EVENT_NONE);

  switch (p_info->m_current.m_event) {
    // ---- READ transfers ----
    case scml2::RD_ADDR_START:
      t += m_clk_period * m_rd_cmd_accept_cycles;

      // TODO: 9000937524: adaptor should automatically advance to RD_DATA_LAST_START
      // state, but (for now) we still need to support the legacy behavior
      // where this is only done if there is no RD_ADDR_START callback registered.

      if (m_auto_behavior) {
        next_state.m_event = should_simulate_rd_data(p_info) ? scml2::RD_DATA_START : scml2::RD_DATA_LAST_START;
        t += m_clk_period * m_rd_data_trigger_cycles;
      }
      else {
        if (has_callback(scml2::RD_ADDR_START)) {
          next_state.m_event = scml2::RD_ADDR_END;
        }
        else {
          // invoke behavior callback internally and return read data
          next_state.m_event = should_simulate_rd_data(p_info) ? scml2::RD_DATA_START : scml2::RD_DATA_LAST_START;
          if (invoke_behavior_callback(next_state.m_event, p_info, t) == tlm::TLM_ACCEPTED)
            return;

          t += m_clk_period * m_rd_data_trigger_cycles;
        }
      }
      break;

    case scml2::RD_ADDR_END:
      // do not auto advance to read data phases
      break;

    case scml2::RD_DATA_START:
      next_state.m_event = scml2::RD_DATA_END;
      break;

    case scml2::RD_DATA_END:
      next_state.m_event = (p_info->m_beat_index + 1 < p_info->m_beat_count) ? scml2::RD_DATA_START : scml2::RD_DATA_LAST_START;
      break;

    case scml2::RD_DATA_LAST_START:
      next_state.m_event = scml2::RD_DATA_LAST_END;
      break;

    case scml2::RD_DATA_LAST_END:
      // transaction is complete - nothing else to do
      break;

    // ---- WRITE transfers ----
    case scml2::WR_ADDR_START:
      t += m_clk_period * m_wr_cmd_accept_cycles;
      next_state.m_event = scml2::WR_ADDR_END;
      break;

    case scml2::WR_ADDR_END:
      // simulate write data phases if needed
      if ((p_info->m_beat_count > 1) && (has_callback(scml2::WR_DATA_START) || has_callback(scml2::WR_DATA_END))) {
        next_state.m_event = scml2::WR_DATA_START;
      }
      else {
        next_state.m_event = scml2::WR_DATA_LAST_START;
      }
      break;

    case scml2::WR_DATA_START:
      next_state.m_event = scml2::WR_DATA_END;
      break;

    case scml2::WR_DATA_END:
      next_state.m_event = (p_info->m_beat_index+1 < p_info->m_beat_count) ? scml2::WR_DATA_START : scml2::WR_DATA_LAST_START;
      break;

    case scml2::WR_DATA_LAST_START:
      // TODO: 9000937524: adaptor should automatically advance to WR_RSP_START
      // state, but (for now) we still need to support the legacy behavior
      // where this is only done if there is no WR_ADDR_START callback registered.
      if (m_auto_behavior) {
        next_state.m_event = scml2::WR_RSP_START;
        t += m_clk_period * m_wr_rsp_trigger_cycles;
      }
      else {
        if (has_callback(scml2::WR_ADDR_START) || has_callback(scml2::WR_DATA_LAST_START)) {
          next_state.m_event = scml2::WR_DATA_LAST_END;
        }
        else {
          // invoke behavior callback internally and return write reponse
          next_state.m_event = scml2::WR_RSP_START;
          if (invoke_behavior_callback(next_state.m_event, p_info, t) == tlm::TLM_ACCEPTED) {
            return;
          }
          t += m_clk_period * m_wr_rsp_trigger_cycles;
        }
      }
      break;

    case scml2::WR_DATA_LAST_END:
      next_state.m_event = scml2::WR_RSP_START;
      t += m_clk_period * m_wr_rsp_trigger_cycles;
      break;

    case scml2::WR_RSP_START:
      next_state.m_event = scml2::WR_RSP_END;
      break;

    case scml2::WR_RSP_END:
      // transaction is complete - nothing else to do
      break;

    default:
      PORT_ADAPTOR_TXN_ERROR(&p_info->pTrans, t, "Cannot advance protocol state; invalid current state: " << event_name(p_info->m_current.m_event));
      abort();
      break;
  }

  if ((next_state.m_event != scml2::CBK_EVENT_NONE) && !p_info->is_state_pending(next_state.m_event)) {
    PORT_ADAPTOR_TRACE(p_info->pTrans, t, event_name(p_info->m_current.m_event) << " => " << event_name(next_state.m_event));
    push_state(p_info, t, next_state);
  }
}

//-----------------------------------------------------------------------------

/** Prepare to handle a new event by either requesting the channel that controls
  * when the event can be handled, OR adding the event to the pending event list
  * for this payload. State machine will be kicked if the event can be processed
  * immediately.
  */
void
tlm2_gp_target_port_adaptor::push_state(payload_info* p_info, sc_time& t, scml2::payload_state state)
{
  switch (state.m_event) {
    case scml2::RD_DATA_START:
    case scml2::RD_DATA_LAST_START:
      // check for skipped RD_ADDR_END
      if ((p_info->m_current.m_event == scml2::RD_ADDR_START) && !p_info->is_state_pending(scml2::RD_ADDR_END)) {
        p_info->m_pending.push_back(state_time_pair(scml2::RD_ADDR_END, p_info->m_current.m_time + m_clk_period * m_rd_cmd_accept_cycles));
      }
      break;

    case scml2::RD_DATA_END:
    case scml2::WR_DATA_START:
    case scml2::WR_DATA_END:
    case scml2::WR_DATA_LAST_START:
      p_info->m_pending.push_front(state_time_pair(state, sc_time_stamp() + t));
      process_state_machine();
      return;

    case scml2::WR_DATA_LAST_END:
      if (!p_info->is_state_pending(scml2::WR_DATA_LAST_END)) {
        p_info->m_pending.push_front(state_time_pair(state, sc_time_stamp() + t));
        process_state_machine();
      }
      return;

    case scml2::WR_RSP_START:
      // check for skipped WR_ADDR_END
      if ((p_info->m_current.m_event == scml2::WR_ADDR_START) && !p_info->is_state_pending(scml2::WR_ADDR_END)) {
        p_info->m_pending.push_back(state_time_pair(scml2::WR_ADDR_END, p_info->m_current.m_time + m_clk_period * m_wr_cmd_accept_cycles));
      }
      if ((p_info->m_current.m_event < scml2::WR_DATA_LAST_END) && !p_info->is_state_pending(scml2::WR_DATA_LAST_END)) {
        p_info->m_pending.push_back(state_time_pair(scml2::WR_DATA_LAST_END, sc_time_stamp() + t));
      }
      break;

    default:
      break;
  }

  p_info->m_pending.push_back(state_time_pair(state, sc_time_stamp() + t));
  if (get_channel_for_event(state)->request(p_info)) {
    process_state_machine();
  }
}

//-----------------------------------------------------------------------------

void
tlm2_gp_target_port_adaptor::process_payload(payload_info* p_info, sc_time& t)
{
  switch (p_info->m_current.m_event) {
    // ---- READ transfers ----
    case scml2::RD_ADDR_START:
      ++m_active_reads;
      if (!can_accept_read()) {
        m_req_channel.lock();
      }
      do_fwd_call(p_info, t);
      break;

    case scml2::RD_ADDR_END:
      m_req_channel.release(t);
      do_bwd_call(p_info, t);

      if (m_auto_behavior) {
        // invoke behavior callback (may block payload from processing the next state
        // if a context change is required)
        invoke_behavior_callback(scml2::CBK_EVENT_NONE, p_info, t);
      }
      break;

    case scml2::RD_DATA_START:
      p_info->m_beat_index++;
      do_bwd_call(p_info, t);
      break;

    case scml2::RD_DATA_END:
      do_fwd_call(p_info, t);
      break;

    case scml2::RD_DATA_LAST_START:
      PORT_ADAPTOR_TRACE(p_info->pTrans, t, "read data: " << byte_array_to_str(p_info->pTrans->get_data_ptr(), p_info->pTrans->get_data_length()));
      p_info->m_beat_index = p_info->m_beat_count -1;
      do_bwd_call(p_info, t);
      break;

    case scml2::RD_DATA_LAST_END:
      m_response_channel.release(t);
      do_fwd_call(p_info, t);
      on_txn_complete(p_info, t);   // TODO: gives same timing as K-2015.12, but should add 1 clock cycle
      break;

    // ---- WRITE transfers ----
    case scml2::WR_ADDR_START:
      PORT_ADAPTOR_TRACE(p_info->pTrans, t, "write data: " << byte_array_to_str(p_info->pTrans->get_data_ptr(), p_info->pTrans->get_data_length()));
      ++m_active_writes;
      if (!can_accept_write()) {
        m_req_channel.lock();
      }
      do_fwd_call(p_info, t);
      break;

    case scml2::WR_ADDR_END:
    case scml2::WR_RSP_START:
      do_bwd_call(p_info, t);
      break;

    case scml2::WR_DATA_START:
      do_fwd_call(p_info, t);
      break;

    case scml2::WR_DATA_END:
      p_info->m_beat_index++;
      do_bwd_call(p_info, t);
      break;

    case scml2::WR_DATA_LAST_START:
      p_info->m_beat_index = p_info->m_beat_count - 1;
      do_fwd_call(p_info, t);
      break;

    case scml2::WR_DATA_LAST_END:
      m_req_channel.release(t);
      p_info->m_beat_index = p_info->m_beat_count - 1;
      do_bwd_call(p_info, t);
      if (m_auto_behavior) {
        // invoke behavior callback (may block payload from processing the next state
        // if a context change is required)
        invoke_behavior_callback(scml2::CBK_EVENT_NONE, p_info, t);
      }
      break;

    case scml2::WR_RSP_END:
      m_response_channel.release(t);
      do_fwd_call(p_info, t);
      on_txn_complete(p_info, t);   // TODO: gives same timing as K-2015.12, but should add 1 clock cycle
      break;

    default:
      break;
  }
}

//-----------------------------------------------------------------------------

tlm::tlm_sync_enum
tlm2_gp_target_port_adaptor::do_nb_transport_bw(payload_info* p_info, sc_time& t)
{
  p_info->m_tlm_phase = event_id_to_phase(p_info->m_current.m_event);
  tlm::tlm_sync_enum status = mSocket->nb_transport_bw(*p_info->pTrans, p_info->m_tlm_phase, t);
  if (status == tlm::TLM_COMPLETED) {
    // jump to last state, and mark payload as complete so no further states are sent to target
    //TODO p_info->m_complete = true;
    p_info->m_tlm_phase = tlm::END_RESP;
    status = tlm::TLM_UPDATED;
  }

  return status;
}

} // namespace scml2
