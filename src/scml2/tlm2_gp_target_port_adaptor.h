/***************************************************************************
 * Copyright 1996-2025 Synopsys, Inc.
 *
 * This Synopsys software and all associated documentation are proprietary
 * to Synopsys, Inc. and may only be used pursuant to the terms and
 * conditions of a written license agreement with Synopsys, Inc.
 * All other use, reproduction, modification, or distribution of the
 * Synopsys software or the associated documentation is strictly prohibited.
 ***************************************************************************/
#ifndef __TLM2_GP_TARGET_PORT_ADAPTOR_H__
#define __TLM2_GP_TARGET_PORT_ADAPTOR_H__

#include <tlm.h>
#include <scml2/target_port_adaptor_impl.h>

namespace scml2
{

class tlm2_gp_target_port_adaptor : public target_port_adaptor_impl
{
public:
  tlm2_gp_target_port_adaptor(const std::string& name, target_port_adaptor::socket_if* sock);

  /* Port adaptor protocol details */
  virtual std::string get_protocol() const { return "TLM2_GP"; }
  virtual bool has_data_phase() const { return false; }
  virtual bool has_wr_rsp() const { return true; }

  /* FT initiator state machine interface */
  virtual tlm::tlm_sync_enum nb_transport_fw(tlm::tlm_generic_payload& trans, tlm::tlm_phase& phase, sc_time& t);

protected:
  virtual std::string state_name(scml2::payload_state state) const;

  virtual bool supports_unaligned_beats() const { return true; }

  virtual scml2::payload_state get_payload_state(payload_info* p_info) const;
  virtual void set_protocol_state(tlm::tlm_generic_payload& trans, scml2::payload_state state);

  bool should_simulate_rd_data(payload_info* p_info)
  {
    // only simulate RD_DATA phases if callbacks are registered
    return (p_info->m_beat_count > 1) && (has_callback(scml2::RD_DATA_START) || has_callback(scml2::RD_DATA_END));
  }

  virtual ft_channel* get_channel_for_event(payload_state state);
  virtual bool is_valid_state_transition(payload_info* p_info, scml2::payload_state next_state) const;
  virtual bool send_state_to_socket(payload_info* p_info, scml2::payload_state state) const;
  virtual void push_state(payload_info* p_info, sc_time& t, scml2::payload_state state);
  virtual void advance_state(payload_info* p_info, sc_time& t);
  virtual void process_payload(payload_info* p_info, sc_time& t);
  virtual tlm::tlm_sync_enum do_nb_transport_bw(payload_info* p_info, sc_time& t);

private:
  tlm::tlm_phase_enum event_id_to_phase(scml2::callback_event_enum event_id) const;
  scml2::callback_event_enum phase_to_event_id(tlm::tlm_generic_payload& trans, const tlm::tlm_phase& phase) const;

  ft_channel m_response_channel;
};

} // namespace scml2

#endif  // __TLM2_GP_TARGET_PORT_ADAPTOR_H__
