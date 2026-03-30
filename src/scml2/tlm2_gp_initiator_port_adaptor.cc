/***************************************************************************
 * Copyright 1996-2025 Synopsys, Inc.
 *
 * This Synopsys software and all associated documentation are proprietary
 * to Synopsys, Inc. and may only be used pursuant to the terms and
 * conditions of a written license agreement with Synopsys, Inc.
 * All other use, reproduction, modification, or distribution of the
 * Synopsys software or the associated documentation is strictly prohibited.
 ***************************************************************************/
#include <scml2_tlm2/snps_tlm2_extensions/snps_tlm2_extensions.h>
#include <scml2/tlm2_gp_initiator_port_adaptor.h>

namespace scml2
{

tlm2_gp_initiator_port_adaptor::tlm2_gp_initiator_port_adaptor(const std::string& name, initiator_port_adaptor::socket_if* sock)
 :  initiator_port_adaptor_impl(name, sock)
{
  m_protocol_id = TLM2_GP_PROTOCOL_ID;

  // address channel is shared for read+write so only support total capacity
  m_attributes["read_capacity"] = (int*)&m_total_capacity;
  m_attributes["write_capacity"] = (int*)&m_total_capacity;

  // override default attribute values
  m_read_capacity = m_write_capacity = 0;
  m_total_capacity = 1;
  m_rd_data_accept_cycles = 0;
  m_wr_rsp_accept_cycles = 0;

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
  m_channels.push_back(&m_data_channel);
  m_channels.push_back(&m_response_channel);
}

tlm::tlm_phase_enum
tlm2_gp_initiator_port_adaptor::event_id_to_phase(scml2::callback_event_enum event_id) const
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
tlm2_gp_initiator_port_adaptor::phase_to_event_id(tlm::tlm_generic_payload& trans, tlm::tlm_phase& phase) const
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
tlm2_gp_initiator_port_adaptor::state_name(scml2::payload_state state) const
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
tlm2_gp_initiator_port_adaptor::get_payload_state(payload_info* p_info) const
{
  return scml2::payload_state(phase_to_event_id(*p_info->pTrans, p_info->m_tlm_phase));
}

void
tlm2_gp_initiator_port_adaptor::set_protocol_state(tlm::tlm_generic_payload&, scml2::payload_state)
{
  // do nothing; the tlm_phase will be set when calling nb_transport_fw
}

//-----------------------------------------------------------------------------

ft_channel*
tlm2_gp_initiator_port_adaptor::get_channel_for_event(payload_state state)
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
      return &m_data_channel; // TODO: replace with response channel?
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
tlm2_gp_initiator_port_adaptor::is_valid_state_transition(payload_info* p_info, scml2::payload_state next_state) const
{
  switch (p_info->m_current.m_event) {
    case scml2::CBK_EVENT_NONE:     return ((next_state.m_event == scml2::RD_ADDR_START) || (next_state.m_event == scml2::WR_ADDR_START));
    case scml2::RD_ADDR_START:      return ((next_state.m_event == scml2::RD_ADDR_END) || (next_state.m_event == scml2::RD_DATA_START) || (next_state.m_event == scml2::RD_DATA_LAST_START));
    case scml2::RD_ADDR_END:        return ((next_state.m_event == scml2::RD_DATA_START) || (next_state.m_event == scml2::RD_DATA_LAST_START));
    case scml2::RD_DATA_START:      return (next_state.m_event == scml2::RD_DATA_END);
    case scml2::RD_DATA_END:        return ((next_state.m_event == scml2::RD_DATA_START) || (next_state.m_event == scml2::RD_DATA_LAST_START));
    case scml2::RD_DATA_LAST_START: return (next_state.m_event == scml2::RD_DATA_LAST_END);

    case scml2::WR_ADDR_START:      return ((next_state.m_event == scml2::WR_ADDR_END) || (next_state.m_event == scml2::WR_DATA_LAST_END) || (next_state.m_event == scml2::WR_RSP_START));
    case scml2::WR_ADDR_END:        return ((next_state.m_event == scml2::WR_DATA_START) || (next_state.m_event == scml2::WR_DATA_LAST_START));
    case scml2::WR_DATA_START:      return (next_state.m_event == scml2::WR_DATA_END);
    case scml2::WR_DATA_END:        return ((next_state.m_event == scml2::WR_DATA_START) || (next_state.m_event == scml2::WR_DATA_LAST_START));
    case scml2::WR_DATA_LAST_START: return (next_state.m_event == scml2::WR_DATA_LAST_END);
    case scml2::WR_DATA_LAST_END:   return (next_state.m_event == scml2::WR_RSP_START);
    case scml2::WR_RSP_START:       return (next_state.m_event == scml2::WR_RSP_END);
    default:
      return false;
  }
}

//-----------------------------------------------------------------------------

/** Return true if this state should be sent to the target (via return path or nb_transport_fw) */
bool
tlm2_gp_initiator_port_adaptor::send_state_to_socket(payload_info* p_info, scml2::payload_state state) const
{
  if (p_info->m_lt_mode_txn) {
    switch (state.m_event) {
      case scml2::RD_ADDR_START:
      case scml2::WR_ADDR_START:
        return true;
      default:
        return false;
    }
  }
  else {
    switch (state.m_event) {
      case scml2::RD_ADDR_START:
      case scml2::RD_DATA_LAST_END:
      case scml2::WR_ADDR_START:
      case scml2::WR_RSP_END:
        return true;
      default:
        return false;
    }
  }
}

//-----------------------------------------------------------------------------

void
tlm2_gp_initiator_port_adaptor::push_state(payload_info* p_info, sc_time& t, scml2::payload_state state)
{
  switch (state.m_event) {
    case scml2::RD_DATA_LAST_START:
      // check for skipped phases, and simulate them internally if necessary
      if (m_req_channel.is_captured_by(p_info) && !p_info->is_state_pending(scml2::RD_ADDR_END)) {
        p_info->m_pending.push_back(state_time_pair(scml2::RD_ADDR_END, sc_time_stamp() + t));
      }

      // simulate read data phases only if model has callbacks registered
      if ((p_info->m_beat_index + 1 < p_info->m_beat_count) &&
          (has_callback(scml2::RD_DATA_START) || has_callback(scml2::RD_DATA_END))) {
        state.m_event = scml2::RD_DATA_START;
      }
      break;

    case scml2::RD_DATA_START:
    case scml2::RD_DATA_END:
    case scml2::WR_DATA_START:
    case scml2::WR_DATA_END:
    case scml2::WR_DATA_LAST_START:
      p_info->m_pending.push_front(state_time_pair(state, sc_time_stamp() + t));
      process_state_machine();
      return;

    case scml2::WR_DATA_LAST_END:
      // check for skipped WR_ADDR_END
      if ((p_info->m_current.m_event == scml2::WR_ADDR_START) && !p_info->is_state_pending(scml2::WR_ADDR_END)) {
        p_info->m_pending.push_back(state_time_pair(scml2::WR_ADDR_END, sc_time_stamp() + t));
      }
      break;

    case scml2::WR_RSP_START:
      // check for skipped phases, and simulate them internally if necessary
      if (m_req_channel.is_captured_by(p_info) && !p_info->is_state_pending(scml2::WR_DATA_LAST_END)) {
        push_state(p_info, t, scml2::WR_DATA_LAST_END);
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
tlm2_gp_initiator_port_adaptor::advance_state(payload_info* p_info, sc_time& t)
{
  scml2::payload_state next_state(scml2::CBK_EVENT_NONE);

  switch (p_info->m_current.m_event) {
    case scml2::RD_DATA_START:
      next_state.m_event = scml2::RD_DATA_END;
      break;

    case scml2::RD_DATA_END:
      next_state.m_event = (p_info->m_beat_index + 1 < p_info->m_beat_count) ? scml2::RD_DATA_START : scml2::RD_DATA_LAST_START;
      break;

    case scml2::RD_DATA_LAST_START:
      t += m_clk_period * m_rd_data_accept_cycles;
      next_state.m_event = scml2::RD_DATA_LAST_END;
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
      next_state.m_event = (p_info->m_beat_index + 1 < p_info->m_beat_count) ? scml2::WR_DATA_START : scml2::WR_DATA_LAST_START;
      break;

    case scml2::RD_ADDR_END:
    case scml2::WR_DATA_LAST_START:
    case scml2::WR_DATA_LAST_END:
      break;

    case scml2::WR_RSP_START:
      t += m_clk_period * m_wr_rsp_accept_cycles;
      next_state.m_event = scml2::WR_RSP_END;
      break;

    case scml2::RD_DATA_LAST_END:
    case scml2::WR_RSP_END:
      // transaction is complete - nothing else to do
      break;

    default:
      PORT_ADAPTOR_TXN_ERROR(p_info->pTrans, t, "Cannot advance protocol state; invalid current state: " << event_name(p_info->m_current.m_event));
      abort();
      break;
  }

  if (next_state.m_event != scml2::CBK_EVENT_NONE) {
    PORT_ADAPTOR_TRACE(p_info->pTrans, t, event_name(p_info->m_current.m_event) << " => " << event_name(next_state.m_event));
    push_state(p_info, t, next_state);
  }
}

//-----------------------------------------------------------------------------

void
tlm2_gp_initiator_port_adaptor::process_payload(payload_info* p_info, sc_time& t)
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
      m_req_channel.release(t + m_clk_period);
      do_bwd_call(p_info, t);
      break;

    case scml2::RD_DATA_START:
      do_bwd_call(p_info, t);
      break;

    case scml2::RD_DATA_END:
  	  p_info->m_beat_index++;
      do_fwd_call(p_info, t);
      break;

    case scml2::RD_DATA_LAST_START:
      PORT_ADAPTOR_TRACE(p_info->pTrans, t, "read data: " << byte_array_to_str(p_info->pTrans->get_data_ptr(), p_info->pTrans->get_data_length()));
      do_bwd_call(p_info, t);
      break;

    case scml2::RD_DATA_LAST_END:
      {
        m_data_channel.release(t + m_clk_period);
        do_fwd_call(p_info, t);
        on_txn_complete(p_info, t);   // TODO: gives same timing as K-2015.12, but should add 1 clock cycle
      }
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
    case scml2::WR_DATA_END:
    case scml2::WR_RSP_START:
      do_bwd_call(p_info, t);
      break;

    case scml2::WR_DATA_START:
    case scml2::WR_DATA_LAST_START:
      p_info->m_beat_index++;
      do_fwd_call(p_info, t);
      break;

    case scml2::WR_DATA_LAST_END:
      m_req_channel.release(t + m_clk_period);
      p_info->m_beat_index = p_info->m_beat_count - 1;
      do_bwd_call(p_info, t);
      break;

    case scml2::WR_RSP_END:
      m_response_channel.release(t + m_clk_period);
      do_fwd_call(p_info, t);
      on_txn_complete(p_info, t);   // TODO: gives same timing as K-2015.12, but should add 1 clock cycle
      break;

    default:
      break;
  }
}
//-----------------------------------------------------------------------------

/* Backward response call from the target */
tlm::tlm_sync_enum
tlm2_gp_initiator_port_adaptor::nb_transport_bw(tlm::tlm_generic_payload& trans, tlm::tlm_phase& phase, sc_time& t)
{
  payload_info* p_info = NULL;
  get_payload_info(p_info, &trans);
  if (!p_info) {
    return tlm::TLM_ACCEPTED;
  }

  p_info->m_tlm_phase = phase;
  phase = scml2::FT_TIMING;
  tlm::tlm_sync_enum status = initiator_port_adaptor_impl::nb_transport_bw(trans, phase, t);
  phase = p_info->m_tlm_phase = event_id_to_phase(p_info->m_current.m_event);
  return status;
}

//-----------------------------------------------------------------------------

tlm::tlm_sync_enum
tlm2_gp_initiator_port_adaptor::do_nb_transport_fw(payload_info* p_info, sc_time& t)
{
  p_info->m_tlm_phase = event_id_to_phase(p_info->m_current.m_event);
  tlm::tlm_sync_enum status = mSocket->nb_transport_fw(*p_info->pTrans, p_info->m_tlm_phase, t);
  if (status == tlm::TLM_COMPLETED) {
    if ((p_info->m_current.m_event == scml2::RD_ADDR_START) || (p_info->m_current.m_event == scml2::WR_ADDR_START)) {
      // jump to last state, and mark payload as complete so no further states are sent to target
      //TODO p_info->m_complete = true;
      p_info->m_tlm_phase = tlm::BEGIN_RESP;
      status = tlm::TLM_UPDATED;
    }
    else {
      status = tlm::TLM_ACCEPTED;
    }
  }

  return status;
}

} // namespace scml2

