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
#include <string>

namespace scml2 {

typedef unsigned int uint;


template <unsigned int BUSWIDTH = 32>
class tlm2_ft_initiator_port_peBase : public scml2::base::object {
public:

	tlm2_ft_initiator_port_peBase( const scml2::base::object_name& name, scml2::ft_initiator_socket<BUSWIDTH>& socket )
		: scml2::base::object(name)
		, abstraction("abstraction")
		, read_capacity("read_capacity")
		, write_capacity("write_capacity")
		, total_capacity("total_capacity")
		, rd_data_accept_cycles("rd_data_accept_cycles")
		, wr_data_trigger_cycles("wr_data_trigger_cycles")
		, wr_rsp_accept_cycles("wr_rsp_accept_cycles")
		, handle_tkeep("handle_tkeep")
		, dba_supported("dba_supported")
		, max_beats("max_beats")
		, invoke_timing_cbks("invoke_timing_cbks")
		, protocol("protocol")
		, bus_width("bus_width")
		, clock_port("clock_port")
		, dmi_enabled("dmi_enabled")
		, consume_annotated_time("consume_annotated_time")
		, get_event_id("get_event_id", this, &tlm2_ft_initiator_port_peBase::get_event_id_impl)
		, alloc_and_init_trans("alloc_and_init_trans", this, &tlm2_ft_initiator_port_peBase::alloc_and_init_trans_impl)
		, send_transaction("send_transaction", this, &tlm2_ft_initiator_port_peBase::send_transaction_impl)
		, update_transaction("update_transaction", this, &tlm2_ft_initiator_port_peBase::update_transaction_impl)
		, get_burst_length("get_burst_length", this, &tlm2_ft_initiator_port_peBase::get_burst_length_impl)
		, nb_transport_fw("nb_transport_fw", this, &tlm2_ft_initiator_port_peBase::nb_transport_fw_impl)
		, _socket(socket)
	{
		 read_capacity.set_getter(this, &tlm2_ft_initiator_port_peBase::get_read_capacity_impl);
		 read_capacity.set_setter(this, &tlm2_ft_initiator_port_peBase::set_read_capacity_impl);
		 write_capacity.set_getter(this, &tlm2_ft_initiator_port_peBase::get_write_capacity_impl);
		 write_capacity.set_setter(this, &tlm2_ft_initiator_port_peBase::set_write_capacity_impl);
		 total_capacity.set_getter(this, &tlm2_ft_initiator_port_peBase::get_total_capacity_impl);
		 total_capacity.set_setter(this, &tlm2_ft_initiator_port_peBase::set_total_capacity_impl);
		 rd_data_accept_cycles.set_getter(this, &tlm2_ft_initiator_port_peBase::get_rd_data_accept_cycles_impl);
		 rd_data_accept_cycles.set_setter(this, &tlm2_ft_initiator_port_peBase::set_rd_data_accept_cycles_impl);
		 wr_data_trigger_cycles.set_getter(this, &tlm2_ft_initiator_port_peBase::get_wr_data_trigger_cycles_impl);
		 wr_data_trigger_cycles.set_setter(this, &tlm2_ft_initiator_port_peBase::set_wr_data_trigger_cycles_impl);
		 wr_rsp_accept_cycles.set_getter(this, &tlm2_ft_initiator_port_peBase::get_wr_rsp_accept_cycles_impl);
		 wr_rsp_accept_cycles.set_setter(this, &tlm2_ft_initiator_port_peBase::set_wr_rsp_accept_cycles_impl);
		 handle_tkeep.set_getter(this, &tlm2_ft_initiator_port_peBase::get_handle_tkeep_impl);
		 handle_tkeep.set_setter(this, &tlm2_ft_initiator_port_peBase::set_handle_tkeep_impl);
		 dba_supported.set_getter(this, &tlm2_ft_initiator_port_peBase::get_dba_supported_impl);
		 dba_supported.set_setter(this, &tlm2_ft_initiator_port_peBase::set_dba_supported_impl);
		 max_beats.set_getter(this, &tlm2_ft_initiator_port_peBase::get_max_beats_impl);
		 max_beats.set_setter(this, &tlm2_ft_initiator_port_peBase::set_max_beats_impl);
		 clock_port.set_getter(this, &tlm2_ft_initiator_port_peBase::get_clock_port_impl);
		 clock_port.set_setter(this, &tlm2_ft_initiator_port_peBase::set_clock_port_impl);
		 invoke_timing_cbks.set_getter(this, &tlm2_ft_initiator_port_peBase::get_invoke_timing_cbks_impl);
		 invoke_timing_cbks.set_setter(this, &tlm2_ft_initiator_port_peBase::set_invoke_timing_cbks_impl);
		 protocol.set_getter(this, &tlm2_ft_initiator_port_peBase::get_protocol_impl);
		 bus_width.set_getter(this, &tlm2_ft_initiator_port_peBase::get_bus_width_impl);
		 protocol.set_setter(this, &tlm2_ft_initiator_port_peBase::set_protocol_impl);
		 bus_width.set_setter(this, &tlm2_ft_initiator_port_peBase::set_bus_width_impl);
		 dmi_enabled.set_setter(this, &tlm2_ft_initiator_port_peBase::set_dmi_enabled_impl);
		 dmi_enabled.set_getter(this, &tlm2_ft_initiator_port_peBase::get_dmi_enabled_impl);
		 consume_annotated_time.set_getter(this, &tlm2_ft_initiator_port_peBase::get_consume_annotated_time_impl);
		 consume_annotated_time.set_setter(this, &tlm2_ft_initiator_port_peBase::set_consume_annotated_time_impl);
	}
	
	//! Callbacks 

	SCML2_BASE_CALLBACK_IGNORE(clock_changed, void, scml_clock_if* );
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
	SCML2_BASE_CALLBACK_IGNORE(nb_transport_bw, tlm::tlm_sync_enum , tlm::tlm_generic_payload&, tlm::tlm_phase&, sc_core::sc_time&);
	SCML2_BASE_CALLBACK_IGNORE(invalidate_direct_mem_ptr, void, sc_dt::uint64, sc_dt::uint64);

	//! Attributes
	scml2::base::value_attribute<tlm2_ft_mode> abstraction;
	scml2::base::attribute<int> read_capacity;
	scml2::base::attribute<int> write_capacity;
	scml2::base::attribute<int> total_capacity;
	scml2::base::attribute<int> rd_data_accept_cycles;
	scml2::base::attribute<int> wr_data_trigger_cycles;
	scml2::base::attribute<int> wr_rsp_accept_cycles;
	scml2::base::attribute<int> handle_tkeep;
	scml2::base::attribute<int> dba_supported;
	scml2::base::attribute<int> max_beats;
	scml2::base::attribute<int> invoke_timing_cbks;
	scml2::base::protected_attribute<tlm2_ft_initiator_port_peBase<BUSWIDTH>, std::string > protocol;
	scml2::base::protected_attribute<tlm2_ft_initiator_port_peBase<BUSWIDTH>, unsigned int > bus_width;
	scml2::base::attribute<sc_core::sc_in<bool>&> clock_port;
	scml2::base::attribute<bool> dmi_enabled;
	scml2::base::const_attribute<bool> consume_annotated_time;
	
	//! Behaviors

	scml2::base::behavior<scml2::callback_event_enum , unsigned int> get_event_id;
	scml2::base::behavior<tlm::tlm_generic_payload& , tlm::tlm_command> alloc_and_init_trans;
	scml2::base::behavior<bool , tlm::tlm_generic_payload&, const sc_core::sc_time&> send_transaction;
	scml2::base::behavior<bool , tlm::tlm_generic_payload&, sc_core::sc_time& , scml2::callback_event_enum> update_transaction;
	scml2::base::behavior<unsigned int , tlm::tlm_generic_payload&> get_burst_length;
        
	scml2::base::behavior<tlm::tlm_sync_enum, tlm::tlm_generic_payload&, tlm::tlm_phase&, sc_core::sc_time&> nb_transport_fw;


protected:
	template <unsigned int>
	friend class tlm2_ft_initiator_port_peProxy;

	scml2::ft_initiator_socket<BUSWIDTH>& _socket;

private:
	virtual scml2::callback_event_enum get_event_id_impl(unsigned int protocol_state) = 0;

// mappable_if
	virtual std::string get_mapped_name() const = 0;
	virtual void b_transport( tlm::tlm_generic_payload& trans, sc_core::sc_time& time) = 0 ;
	virtual bool get_direct_mem_ptr( tlm::tlm_generic_payload& trans, tlm::tlm_dmi& dmi) = 0 ;
	virtual scml2::uint transport_dbg( tlm::tlm_generic_payload& trans) = 0 ;
	virtual void register_bw_direct_mem_if(tlm::tlm_bw_direct_mem_if* bwInterface) = 0;
	virtual void unregister_bw_direct_mem_if(tlm::tlm_bw_direct_mem_if* bwInterface) = 0;

	virtual tlm::tlm_generic_payload& alloc_and_init_trans_impl(tlm::tlm_command cmd) = 0;
	virtual bool send_transaction_impl(tlm::tlm_generic_payload& trans, const sc_core::sc_time& delay) = 0;
	virtual bool update_transaction_impl(tlm::tlm_generic_payload& trans, sc_core::sc_time&  delay, scml2::callback_event_enum event_id) = 0;
	virtual tlm::tlm_sync_enum nb_transport_fw_impl( tlm::tlm_generic_payload& trans, tlm::tlm_phase& phase, sc_core::sc_time& time) = 0 ;
	virtual unsigned int get_burst_length_impl( tlm::tlm_generic_payload& trans) = 0 ;

	virtual int get_read_capacity_impl() = 0;
	virtual void set_read_capacity_impl(int value) = 0;
	virtual int get_write_capacity_impl() = 0;
	virtual void set_write_capacity_impl(int value) = 0;
	virtual int get_total_capacity_impl() = 0;
	virtual void set_total_capacity_impl(int value) = 0;
	virtual int get_rd_data_accept_cycles_impl() = 0;
	virtual void set_rd_data_accept_cycles_impl(int value) = 0;
	virtual int get_wr_data_trigger_cycles_impl() = 0;
	virtual void set_wr_data_trigger_cycles_impl(int value) = 0;
	virtual int get_wr_rsp_accept_cycles_impl() = 0;
	virtual void set_wr_rsp_accept_cycles_impl(int value) = 0;
	virtual int get_handle_tkeep_impl() = 0;
	virtual void set_handle_tkeep_impl(int value) = 0;
	virtual int get_dba_supported_impl() = 0;
	virtual void set_dba_supported_impl(int value) = 0;
	virtual int get_max_beats_impl() = 0;
	virtual void set_max_beats_impl(int value) = 0;

	virtual int get_invoke_timing_cbks_impl() = 0;
	virtual void set_invoke_timing_cbks_impl(int value) = 0;
	virtual std::string get_protocol_impl() = 0;
	virtual unsigned int get_bus_width_impl() = 0;
	void set_protocol_impl(std::string){};
	void set_bus_width_impl(unsigned int){};
	virtual sc_core::sc_in<bool>& get_clock_port_impl() = 0;
	virtual void set_clock_port_impl(sc_core::sc_in<bool>& value) = 0;
	virtual bool get_dmi_enabled_impl() = 0;
	virtual void set_dmi_enabled_impl(bool value) = 0;
	virtual bool get_consume_annotated_time_impl() = 0;
	virtual void set_consume_annotated_time_impl(bool value) = 0;
	
};

} // namespace scml2
