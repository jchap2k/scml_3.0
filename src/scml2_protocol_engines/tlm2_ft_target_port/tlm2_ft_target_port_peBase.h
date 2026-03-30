/***************************************************************************
 * Copyright 1996-2025 Synopsys, Inc.
 *
 * This Synopsys software and all associated documentation are proprietary
 * to Synopsys, Inc. and may only be used pursuant to the terms and
 * conditions of a written license agreement with Synopsys, Inc.
 * All other use, reproduction, modification, or distribution of the
 * Synopsys software or the associated documentation is strictly prohibited.
 ***************************************************************************/


#pragma once

#include "scml2_base.h"
#include "scml2_protocol_engines/tlm2_ft_common/include/tlm2_ft_mode.h"
#include "scml2_protocol_engines/tlm2_ft_common/include/tlm2_intercept_base.h"
#include <string>

namespace scml2 {

typedef unsigned int uint;

template <unsigned int BUSWIDTH=32>
class tlm2_ft_target_port_peBase : public scml2::base::object {
public:

	tlm2_ft_target_port_peBase( const scml2::base::object_name& name, scml2::ft_target_socket<BUSWIDTH>& socket )
		: scml2::base::object(name)
		, protocol("protocol")
		, abstraction("abstraction")
		, read_capacity("read_capacity")
		, write_capacity("write_capacity")
		, total_capacity("total_capacity")
		, rd_cmd_accept_cycles("rd_cmd_accept_cycles")
		, rd_data_trigger_cycles("rd_data_trigger_cycles")
		, wr_cmd_accept_cycles("wr_cmd_accept_cycles")
		, wr_data_accept_cycles("wr_data_accept_cycles")
		, wr_rsp_trigger_cycles("wr_rsp_trigger_cycles")
		, req_accept_cycles("req_accept_cycles")
		, collect_databeats("collect_databeats")
		, dba_supported("dba_supported")
		, clock_port("clock_port")
		, invoke_timing_cbks("invoke_timing_cbks")
		, invoke_behavior_cbks("invoke_behavior_cbks")
		, bus_width("bus_width")
		, consume_annotated_time("consume_annotated_time")
		, get_event_id("get_event_id", this, &tlm2_ft_target_port_peBase::get_event_id_impl)
		, update_transaction("update_transaction", this, &tlm2_ft_target_port_peBase::update_transaction_impl)
		, invalidate_direct_mem_ptr("invalidate_direct_mem_ptr", this, &tlm2_ft_target_port_peBase::invalidate_direct_mem_ptr_impl)
		, nb_transport_bw("nb_transport_bw", this, &tlm2_ft_target_port_peBase::nb_transport_bw_impl)
		, m_socket(socket)
	{
		 protocol.set_getter(this, &tlm2_ft_target_port_peBase::get_protocol_impl);
		 protocol.set_setter(this, &tlm2_ft_target_port_peBase::set_protocol_impl);
		 read_capacity.set_getter(this, &tlm2_ft_target_port_peBase::get_read_capacity_impl);
		 read_capacity.set_setter(this, &tlm2_ft_target_port_peBase::set_read_capacity_impl);
		 write_capacity.set_getter(this, &tlm2_ft_target_port_peBase::get_write_capacity_impl);
		 write_capacity.set_setter(this, &tlm2_ft_target_port_peBase::set_write_capacity_impl);
		 total_capacity.set_getter(this, &tlm2_ft_target_port_peBase::get_total_capacity_impl);
		 total_capacity.set_setter(this, &tlm2_ft_target_port_peBase::set_total_capacity_impl);
		 rd_cmd_accept_cycles.set_getter(this, &tlm2_ft_target_port_peBase::get_rd_cmd_accept_cycles_impl);
		 rd_cmd_accept_cycles.set_setter(this, &tlm2_ft_target_port_peBase::set_rd_cmd_accept_cycles_impl);
		 rd_data_trigger_cycles.set_getter(this, &tlm2_ft_target_port_peBase::get_rd_data_trigger_cycles_impl);
		 rd_data_trigger_cycles.set_setter(this, &tlm2_ft_target_port_peBase::set_rd_data_trigger_cycles_impl);
		 wr_cmd_accept_cycles.set_getter(this, &tlm2_ft_target_port_peBase::get_wr_cmd_accept_cycles_impl);
		 wr_cmd_accept_cycles.set_setter(this, &tlm2_ft_target_port_peBase::set_wr_cmd_accept_cycles_impl);
		 wr_data_accept_cycles.set_getter(this, &tlm2_ft_target_port_peBase::get_wr_data_accept_cycles_impl);
		 wr_data_accept_cycles.set_setter(this, &tlm2_ft_target_port_peBase::set_wr_data_accept_cycles_impl);
		 wr_rsp_trigger_cycles.set_getter(this, &tlm2_ft_target_port_peBase::get_wr_rsp_trigger_cycles_impl);
		 wr_rsp_trigger_cycles.set_setter(this, &tlm2_ft_target_port_peBase::set_wr_rsp_trigger_cycles_impl);
		 req_accept_cycles.set_getter(this, &tlm2_ft_target_port_peBase::get_req_accept_cycles_impl);
		 req_accept_cycles.set_setter(this, &tlm2_ft_target_port_peBase::set_req_accept_cycles_impl);
		 collect_databeats.set_getter(this, &tlm2_ft_target_port_peBase::get_collect_databeats_impl);
		 collect_databeats.set_setter(this, &tlm2_ft_target_port_peBase::set_collect_databeats_impl);
		 dba_supported.set_getter(this, &tlm2_ft_target_port_peBase::get_dba_supported_impl);
		 dba_supported.set_setter(this, &tlm2_ft_target_port_peBase::set_dba_supported_impl);
		 clock_port.set_getter(this, &tlm2_ft_target_port_peBase::get_clock_port_impl);
		 clock_port.set_setter(this, &tlm2_ft_target_port_peBase::set_clock_port_impl);
		 invoke_timing_cbks.set_getter(this, &tlm2_ft_target_port_peBase::get_invoke_timing_cbks_impl);
		 invoke_timing_cbks.set_setter(this, &tlm2_ft_target_port_peBase::set_invoke_timing_cbks_impl);
		 invoke_behavior_cbks.set_getter(this, &tlm2_ft_target_port_peBase::get_invoke_behavior_cbks_impl);
		 invoke_behavior_cbks.set_setter(this, &tlm2_ft_target_port_peBase::set_invoke_behavior_cbks_impl);
		 bus_width.set_getter(this, &tlm2_ft_target_port_peBase::get_bus_width_impl);
		 bus_width.set_setter(this, &tlm2_ft_target_port_peBase::set_bus_width_impl);
	}
	
	//! Callbacks 

	SCML2_BASE_CALLBACK_IGNORE(clock_changed, void , scml_clock_if*);
	SCML2_BASE_CALLBACK_IGNORE(rd_addr_start, tlm::tlm_sync_enum , tlm::tlm_generic_payload&, sc_core::sc_time&);
	SCML2_BASE_CALLBACK_IGNORE(rd_data_last_end, tlm::tlm_sync_enum , tlm::tlm_generic_payload&, sc_core::sc_time&);
	SCML2_BASE_CALLBACK_IGNORE(rd_data_last_start, tlm::tlm_sync_enum , tlm::tlm_generic_payload&, sc_core::sc_time&);
	SCML2_BASE_CALLBACK_IGNORE(rd_data_end, tlm::tlm_sync_enum , tlm::tlm_generic_payload&, sc_core::sc_time&);
	SCML2_BASE_CALLBACK_IGNORE(rd_data_start, tlm::tlm_sync_enum , tlm::tlm_generic_payload&, sc_core::sc_time&);
	SCML2_BASE_CALLBACK_IGNORE(rd_addr_end, tlm::tlm_sync_enum , tlm::tlm_generic_payload&, sc_core::sc_time&);
	SCML2_BASE_CALLBACK_IGNORE(wr_rsp_end, tlm::tlm_sync_enum , tlm::tlm_generic_payload&, sc_core::sc_time&);
	SCML2_BASE_CALLBACK_IGNORE(wr_rsp_start, tlm::tlm_sync_enum , tlm::tlm_generic_payload&, sc_core::sc_time&);
	SCML2_BASE_CALLBACK_IGNORE(wr_data_last_end, tlm::tlm_sync_enum , tlm::tlm_generic_payload&, sc_core::sc_time&);
	SCML2_BASE_CALLBACK_IGNORE(wr_data_last_start, tlm::tlm_sync_enum , tlm::tlm_generic_payload&, sc_core::sc_time&);
	SCML2_BASE_CALLBACK_IGNORE(wr_data_end, tlm::tlm_sync_enum , tlm::tlm_generic_payload&, sc_core::sc_time&);
	SCML2_BASE_CALLBACK_IGNORE(wr_data_start, tlm::tlm_sync_enum , tlm::tlm_generic_payload&, sc_core::sc_time&);
	SCML2_BASE_CALLBACK_IGNORE(wr_addr_end, tlm::tlm_sync_enum , tlm::tlm_generic_payload&, sc_core::sc_time&);
	SCML2_BASE_CALLBACK_IGNORE(wr_addr_start, tlm::tlm_sync_enum , tlm::tlm_generic_payload&, sc_core::sc_time&);
	SCML2_BASE_CALLBACK_IGNORE(b_transport, void, tlm::tlm_generic_payload&, sc_core::sc_time&);
	SCML2_BASE_CALLBACK_IGNORE(get_direct_mem_ptr, bool, tlm::tlm_generic_payload&, tlm::tlm_dmi&);
	SCML2_BASE_CALLBACK_IGNORE(nb_transport_fw, tlm::tlm_sync_enum, tlm::tlm_generic_payload&, tlm::tlm_phase&, sc_core::sc_time&);
	SCML2_BASE_CALLBACK_IGNORE(transport_dbg, scml2::uint, tlm::tlm_generic_payload&);
	
	//! Attributes

	scml2::base::protected_attribute<tlm2_ft_target_port_peBase<BUSWIDTH>, std::string > protocol;
	scml2::base::value_attribute<tlm2_ft_mode> abstraction;
	std::vector<scml2::objects::tlm2_intercept_base*> intercepts;
	scml2::base::attribute<int> read_capacity;
	scml2::base::attribute<int> write_capacity;
	scml2::base::attribute<int> total_capacity;
	scml2::base::attribute<int> rd_cmd_accept_cycles;
	scml2::base::attribute<int> rd_data_trigger_cycles;
	scml2::base::attribute<int> wr_cmd_accept_cycles;
	scml2::base::attribute<int> wr_data_accept_cycles;
	scml2::base::attribute<int> wr_rsp_trigger_cycles;
	scml2::base::attribute<int> req_accept_cycles;
	scml2::base::attribute<bool> collect_databeats;
	scml2::base::attribute<bool> dba_supported;
	scml2::base::attribute<sc_core::sc_in<bool>&> clock_port;
	scml2::base::attribute<int> invoke_timing_cbks;
	scml2::base::attribute<int> invoke_behavior_cbks;
	scml2::base::protected_attribute<tlm2_ft_target_port_peBase<BUSWIDTH>, unsigned int > bus_width;
	scml2::base::const_attribute<bool> consume_annotated_time;

	//! Behaviors
	scml2::base::behavior<scml2::callback_event_enum , unsigned int> get_event_id;
	scml2::base::behavior<bool , tlm::tlm_generic_payload&, sc_core::sc_time& , scml2::callback_event_enum> update_transaction;
	scml2::base::behavior<void, sc_dt::uint64, sc_dt::uint64> invalidate_direct_mem_ptr;
	scml2::base::behavior<tlm::tlm_sync_enum, tlm::tlm_generic_payload&, tlm::tlm_phase&, sc_core::sc_time&> nb_transport_bw;

protected:

	scml2::ft_target_socket<BUSWIDTH>& m_socket;

private:

	virtual scml2::callback_event_enum get_event_id_impl(unsigned int protocol_state) = 0;
	virtual bool update_transaction_impl(tlm::tlm_generic_payload& trans, sc_core::sc_time&  delay, scml2::callback_event_enum event_id) = 0;
	virtual void invalidate_direct_mem_ptr_impl( sc_dt::uint64 range_start,sc_dt::uint64 range_end) = 0;
	virtual tlm::tlm_sync_enum nb_transport_bw_impl(tlm::tlm_generic_payload& trans, tlm::tlm_phase& phase, sc_core::sc_time& time) = 0;

	virtual std::string get_protocol_impl() = 0;
	virtual int get_read_capacity_impl() = 0;
	virtual void set_read_capacity_impl(int value) = 0;
	virtual int get_write_capacity_impl() = 0;
	virtual void set_write_capacity_impl(int value) = 0;
	virtual int get_total_capacity_impl() = 0;
	virtual void set_total_capacity_impl(int value) = 0;
	virtual int get_rd_cmd_accept_cycles_impl() = 0;
	virtual void set_rd_cmd_accept_cycles_impl(int value) = 0;
	virtual int get_rd_data_trigger_cycles_impl() = 0;
	virtual void set_rd_data_trigger_cycles_impl(int value) = 0;
	virtual int get_wr_cmd_accept_cycles_impl() = 0;
	virtual void set_wr_cmd_accept_cycles_impl(int value) = 0;
	virtual int get_wr_data_accept_cycles_impl() = 0;
	virtual void set_wr_data_accept_cycles_impl(int value) = 0;
	virtual int get_wr_rsp_trigger_cycles_impl() = 0;
	virtual void set_wr_rsp_trigger_cycles_impl(int value) = 0;
	virtual int get_req_accept_cycles_impl() = 0;
	virtual void set_req_accept_cycles_impl(int value) = 0;
	virtual bool get_collect_databeats_impl() = 0;
	virtual void set_collect_databeats_impl(bool value) = 0;
	virtual bool get_dba_supported_impl() = 0;
	virtual void set_dba_supported_impl(bool value) = 0;
	virtual sc_core::sc_in<bool>& get_clock_port_impl() = 0;
	virtual void set_clock_port_impl(sc_core::sc_in<bool>& value) = 0;
	void set_protocol_impl(std::string){};
	virtual int get_invoke_timing_cbks_impl() = 0;
	virtual void set_invoke_timing_cbks_impl(int value) = 0;
	virtual int get_invoke_behavior_cbks_impl() = 0;
	virtual void set_invoke_behavior_cbks_impl(int value) = 0;
	virtual unsigned int get_bus_width_impl() = 0;
	void set_bus_width_impl(unsigned int) {};
};

} // namespace scml2
