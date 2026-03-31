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

#include "scml2.h"
// some SKI-unittests forcibly disable inclusion of the toplevel header........
#include "scml2/callback_macro.h" 
#include "scml2_protocol_engines/tlm2_ft_target_port/include/tlm2_ft_target_port_peBase.h"

namespace scml2 {

template <unsigned int BUSWIDTH=32>
class tlm2_ft_target_port_pe : public tlm2_ft_target_port_peBase<BUSWIDTH> {
	scml2::target_port_adaptor& get_lt_or_ft_adaptor_for(const std::string& api) {
	  if (this->abstraction != FT && this->abstraction != LT) {
	    SCML2_ERROR(FATAL_ERROR) << "target protocol engine '" << this->get_name() << "' configured for TLM2_GP abstraction, API '" << api << "' not available." << std::endl;
	  }
	  return *m_adaptor;
	}
  
	scml2::target_port_adaptor& get_ft_adaptor_for(const std::string& api) {
	  if (this->abstraction != FT) {
	    SCML2_ERROR(FATAL_ERROR) << "target protocol engine '" << this->get_name() << "' not configured for FT abstraction, API '" << api << "' not available." << std::endl;
	  }
	  return *m_adaptor;
	}	
public:

	tlm2_ft_target_port_pe( const scml2::base::object_name& name, scml2::ft_target_socket<BUSWIDTH>& socket );
	~tlm2_ft_target_port_pe();

	void finalize_local_construction();
protected:
	virtual scml2::callback_event_enum get_event_id_impl(unsigned int protocol_state) ;
	virtual bool update_transaction_impl(tlm::tlm_generic_payload& trans, sc_core::sc_time&  delay, scml2::callback_event_enum event_id) ;
	virtual void invalidate_direct_mem_ptr_impl(sc_dt::uint64 range_start,sc_dt::uint64 range_end);
	virtual tlm::tlm_sync_enum nb_transport_bw_impl(tlm::tlm_generic_payload& trans, tlm::tlm_phase& phase, sc_core::sc_time& time);

	virtual std::string get_protocol_impl() ;
	virtual int get_read_capacity_impl() ;
	virtual void set_read_capacity_impl(int value) ;
	virtual int get_write_capacity_impl() ;
	virtual void set_write_capacity_impl(int value) ;
	virtual int get_total_capacity_impl() ;
	virtual void set_total_capacity_impl(int value) ;
	virtual int get_rd_cmd_accept_cycles_impl() ;
	virtual void set_rd_cmd_accept_cycles_impl(int value) ;
	virtual int get_rd_data_trigger_cycles_impl() ;
	virtual void set_rd_data_trigger_cycles_impl(int value) ;
	virtual int get_wr_cmd_accept_cycles_impl() ;
	virtual void set_wr_cmd_accept_cycles_impl(int value) ;
	virtual int get_wr_data_accept_cycles_impl() ;
	virtual void set_wr_data_accept_cycles_impl(int value) ;
	virtual int get_wr_rsp_trigger_cycles_impl() ;
	virtual void set_wr_rsp_trigger_cycles_impl(int value) ;
	virtual int get_req_accept_cycles_impl() ;
	virtual void set_req_accept_cycles_impl(int value) ;
	virtual bool get_collect_databeats_impl() ;
	virtual void set_collect_databeats_impl(bool value) ;
	virtual bool get_dba_supported_impl() ;
	virtual void set_dba_supported_impl(bool value) ;
	virtual sc_core::sc_in<bool>& get_clock_port_impl() ;
	virtual void set_clock_port_impl(sc_core::sc_in<bool>& value) ;
	virtual int get_invoke_timing_cbks_impl();
	virtual void set_invoke_timing_cbks_impl(int value);
	virtual int get_invoke_behavior_cbks_impl();
	virtual void set_invoke_behavior_cbks_impl(int value);
	virtual unsigned int get_bus_width_impl() ;
public:

	void operator()(scml2::mappable_if& dest);

protected:
	scml2::mappable_if* m_dest{nullptr};
	void handle_binding();

	class fw_interface_wrapper : public scml2::fw_interface_type {
	public:
		fw_interface_wrapper(scml2::ft_target_socket<BUSWIDTH>& socket) :
			m_socket(socket),
			m_b_transport_callback(0),
			m_get_direct_mem_ptr_callback(0),
			m_nb_transport_fw_callback(0),
			m_transport_dbg_callback(0)
		{
			m_socket.bind(*this);
		}

		~fw_interface_wrapper(){
		}

		scml2::ft_target_socket<BUSWIDTH>& m_socket;
		
		scml2::base::callback< void, tlm::tlm_generic_payload&, sc_core::sc_time& >* m_b_transport_callback;
		scml2::base::callback< bool, tlm::tlm_generic_payload&, tlm::tlm_dmi&>* m_get_direct_mem_ptr_callback;
		scml2::base::callback< tlm::tlm_sync_enum, tlm::tlm_generic_payload&, tlm::tlm_phase&, sc_core::sc_time&>* m_nb_transport_fw_callback;
		scml2::base::callback< unsigned int, tlm::tlm_generic_payload&>* m_transport_dbg_callback;


		void register_b_transport(scml2::base::callback< void , tlm::tlm_generic_payload&, sc_core::sc_time& > * cb) {
		  m_b_transport_callback = cb;
		}

		virtual void b_transport(tlm::tlm_generic_payload& trans,
		                           sc_core::sc_time& t){
		  if (m_b_transport_callback) {
		    return m_b_transport_callback->operator()(trans, t);
		  } else {
		    assert(false);
		  }
		}


		void register_get_direct_mem_ptr(scml2::base::callback< bool, tlm::tlm_generic_payload&, tlm::tlm_dmi&>* cb) {
		  m_get_direct_mem_ptr_callback = cb;
		}

		virtual bool get_direct_mem_ptr(tlm::tlm_generic_payload& trans, tlm::tlm_dmi&  dmi_data){
		  if (m_get_direct_mem_ptr_callback) {
		    return m_get_direct_mem_ptr_callback->operator()(trans, dmi_data);
		  } else {
		    dmi_data.allow_read_write();
		    dmi_data.set_start_address(0x0ull);
		    dmi_data.set_end_address((sc_dt::uint64)-1);
		    return false;
		  }
		}

		void register_nb_transport_fw(scml2::base::callback<tlm::tlm_sync_enum, tlm::tlm_generic_payload&, tlm::tlm_phase&, sc_core::sc_time&>* cb){
		  m_nb_transport_fw_callback = cb;
		}

		virtual tlm::tlm_sync_enum nb_transport_fw(tlm::tlm_generic_payload& trans, tlm::tlm_phase& phase, sc_core::sc_time& time){
		  if (m_nb_transport_fw_callback) {
		    return m_nb_transport_fw_callback->operator()(trans, phase, time);
		  } else {
		    return tlm::TLM_COMPLETED;
		  }
		}

		void register_transport_dbg(scml2::base::callback<unsigned int, tlm::tlm_generic_payload&>* cb){
		  m_transport_dbg_callback = cb;
		}

		virtual unsigned int transport_dbg(tlm::tlm_generic_payload& trans) {
		  if (m_transport_dbg_callback) {
		    return m_transport_dbg_callback->operator()(trans);
		  } else {
		    return 0;
		  }
		}

	};

public:
	scml2::target_port_adaptor *m_adaptor;
protected:
	virtual void callback_registered(scml2::base::callback_base& cbb);

	fw_interface_wrapper* m_ifw;

private :
        bool get_direct_mem_ptr_stub(tlm::tlm_generic_payload& trans, tlm::tlm_dmi& dmi_data);

	std::string m_name;
	sc_core::sc_in<bool>* m_clock;
	bool m_clock_changed_cb_active;
	bool m_rd_addr_start_cb_active;
	bool m_rd_data_last_end_cb_active;
	bool m_rd_data_last_start_cb_active;
	bool m_rd_data_end_cb_active;
	bool m_rd_data_start_cb_active;
	bool m_rd_addr_end_cb_active;
	bool m_wr_rsp_end_cb_active;
	bool m_wr_rsp_start_cb_active;
	bool m_wr_data_last_end_cb_active;
	bool m_wr_data_last_start_cb_active;
	bool m_wr_data_end_cb_active;
	bool m_wr_data_start_cb_active;
	bool m_wr_addr_end_cb_active;
	bool m_wr_addr_start_cb_active;
	bool m_b_transport_cb_active;
	bool m_get_direct_mem_ptr_cb_active;
	bool m_nb_transport_fw_cb_active;
	bool m_transport_dbg_cb_active;
	bool m_has_socket_callbacks;
	bool m_has_adaptor_callbacks;
        // Temporary storage for port adaptor attribute values

        template<typename T>
        class attribute_value {
        public:
          attribute_value() : m_initialized(false), m_value(T()) {}
          attribute_value(const attribute_value& other) : m_initialized(other.m_initialized), m_value(other.m_value) {}
          attribute_value& operator=(const T& other) {
            m_value = other;
            m_initialized = true;
            return *this;
          }
          const T& get() const {
            assert(m_initialized == true);
            return m_value;
          }
          bool is_initialized() {
            return m_initialized;
          }
          void reset() {
            m_initialized = false;
          }
        private:
          bool m_initialized;
          T m_value;
        };
        attribute_value<int> read_capacity_value;
        attribute_value<int> write_capacity_value;
        attribute_value<int> total_capacity_value;
        attribute_value<int> rd_cmd_accept_cycles_value;
        attribute_value<int> rd_data_trigger_cycles_value;
        attribute_value<int> wr_cmd_accept_cycles_value;
        attribute_value<int> wr_data_accept_cycles_value;
        attribute_value<int> wr_rsp_trigger_cycles_value;
        attribute_value<int> req_accept_cycles_value;
        attribute_value<bool> collect_databeats_value;
        attribute_value<bool> dba_supported_value;
        attribute_value<int> invoke_timing_cbks_value;
        attribute_value<int> invoke_behavior_cbks_value;
};

template <unsigned int BUSWIDTH>
tlm2_ft_target_port_pe<BUSWIDTH>::~tlm2_ft_target_port_pe(){
	if (m_adaptor) {
		delete m_adaptor;
	}

	if (m_ifw) {
		delete m_ifw;
	}
}


template <unsigned int BUSWIDTH>
void tlm2_ft_target_port_pe<BUSWIDTH>::callback_registered(scml2::base::callback_base& cbb) {
	if (&cbb == dynamic_cast<scml2::base::callback_base*>(&(this->clock_changed))) {
		m_clock_changed_cb_active = true;
		m_has_adaptor_callbacks = true;
	}

	if (&cbb == dynamic_cast<scml2::base::callback_base*>(&(this->rd_addr_start))) {
		m_rd_addr_start_cb_active = true;
		m_has_adaptor_callbacks = true;
	}

	if (&cbb == dynamic_cast<scml2::base::callback_base*>(&(this->rd_data_last_end))) {
		m_rd_data_last_end_cb_active = true;
		m_has_adaptor_callbacks = true;
	}

	if (&cbb == dynamic_cast<scml2::base::callback_base*>(&(this->rd_data_last_start))) {
		m_rd_data_last_start_cb_active = true;
		m_has_adaptor_callbacks = true;
	}

	if (&cbb == dynamic_cast<scml2::base::callback_base*>(&(this->rd_data_end))) {
		m_rd_data_end_cb_active = true;
		m_has_adaptor_callbacks = true;
	}

	if (&cbb == dynamic_cast<scml2::base::callback_base*>(&(this->rd_data_start))) {
		m_rd_data_start_cb_active = true;
		m_has_adaptor_callbacks = true;
	}

	if (&cbb == dynamic_cast<scml2::base::callback_base*>(&(this->rd_addr_end))) {
		m_rd_addr_end_cb_active = true;
		m_has_adaptor_callbacks = true;
	}

	if (&cbb == dynamic_cast<scml2::base::callback_base*>(&(this->wr_rsp_end))) {
		m_wr_rsp_end_cb_active = true;
		m_has_adaptor_callbacks = true;
	}

	if (&cbb == dynamic_cast<scml2::base::callback_base*>(&(this->wr_rsp_start))) {
		m_wr_rsp_start_cb_active = true;
		m_has_adaptor_callbacks = true;
	}

	if (&cbb == dynamic_cast<scml2::base::callback_base*>(&(this->wr_data_last_end))) {
		m_wr_data_last_end_cb_active = true;
		m_has_adaptor_callbacks = true;
	}

	if (&cbb == dynamic_cast<scml2::base::callback_base*>(&(this->wr_data_last_start))) {
		m_wr_data_last_start_cb_active = true;
		m_has_adaptor_callbacks = true;
	}

	if (&cbb == dynamic_cast<scml2::base::callback_base*>(&(this->wr_data_end))) {
		m_wr_data_end_cb_active = true;
		m_has_adaptor_callbacks = true;
	}

	if (&cbb == dynamic_cast<scml2::base::callback_base*>(&(this->wr_data_start))) {
		m_wr_data_start_cb_active = true;
		m_has_adaptor_callbacks = true;
	}

	if (&cbb == dynamic_cast<scml2::base::callback_base*>(&(this->wr_addr_end))) {
		m_wr_addr_end_cb_active = true;
		m_has_adaptor_callbacks = true;
	}

	if (&cbb == dynamic_cast<scml2::base::callback_base*>(&(this->wr_addr_start))) {
		m_wr_addr_start_cb_active = true;
		m_has_adaptor_callbacks = true;
	}
	
	if (&cbb == dynamic_cast<scml2::base::callback_base*>(&(this->b_transport))) {
		m_b_transport_cb_active = true;
		m_has_socket_callbacks = true;
	}
	
	if (&cbb == dynamic_cast<scml2::base::callback_base*>(&(this->get_direct_mem_ptr))) {
		m_get_direct_mem_ptr_cb_active = true;
		m_has_socket_callbacks = true;
	}
	
	if (&cbb == dynamic_cast<scml2::base::callback_base*>(&(this->nb_transport_fw))) {
		m_nb_transport_fw_cb_active = true;
		m_has_socket_callbacks = true;
	}

	if (&cbb == dynamic_cast<scml2::base::callback_base*>(&(this->transport_dbg))) {
		m_transport_dbg_cb_active = true;
		m_has_socket_callbacks = true;
	}
}


template <unsigned int BUSWIDTH>
tlm2_ft_target_port_pe<BUSWIDTH>::tlm2_ft_target_port_pe( const scml2::base::object_name& name, scml2::ft_target_socket<BUSWIDTH>& socket )
	: tlm2_ft_target_port_peBase<BUSWIDTH>(name, socket)
	  ,m_adaptor(0)
	  ,m_ifw(0)
	  ,m_name(sc_core::sc_gen_unique_name(name.c_str()))
	  ,m_clock(0)
	  ,m_clock_changed_cb_active(false)
	  ,m_rd_addr_start_cb_active(false)
	  ,m_rd_data_last_end_cb_active(false)
	  ,m_rd_data_last_start_cb_active(false)
	  ,m_rd_data_end_cb_active(false)
	  ,m_rd_data_start_cb_active(false)
	  ,m_rd_addr_end_cb_active(false)
	  ,m_wr_rsp_end_cb_active(false)
	  ,m_wr_rsp_start_cb_active(false)
	  ,m_wr_data_last_end_cb_active(false)
	  ,m_wr_data_last_start_cb_active(false)
	  ,m_wr_data_end_cb_active(false)
	  ,m_wr_data_start_cb_active(false)
	  ,m_wr_addr_end_cb_active(false)
	  ,m_wr_addr_start_cb_active(false)
	  ,m_b_transport_cb_active(false)
	  ,m_get_direct_mem_ptr_cb_active(false)
	  ,m_nb_transport_fw_cb_active(false)
	  ,m_transport_dbg_cb_active(false)
	  ,m_has_socket_callbacks(false)
	  ,m_has_adaptor_callbacks(false)
{

}

template <unsigned int BUSWIDTH>
bool tlm2_ft_target_port_pe<BUSWIDTH>::get_direct_mem_ptr_stub(tlm::tlm_generic_payload& /*trans*/, tlm::tlm_dmi& dmi_data)
{
  dmi_data.set_dmi_ptr(0);
  dmi_data.set_granted_access(tlm::tlm_dmi::DMI_ACCESS_READ_WRITE);
  dmi_data.set_start_address(0ull);
  dmi_data.set_end_address(~0ull);
  return false;
}

template <unsigned int BUSWIDTH>
void tlm2_ft_target_port_pe<BUSWIDTH>::finalize_local_construction()
{
  if (this->abstraction == TLM2_GP) {

	if (m_has_socket_callbacks) {
		
		m_ifw = new fw_interface_wrapper(this->m_socket);
		
		if (m_b_transport_cb_active) {
			m_ifw->register_b_transport(&(this->b_transport));
		}
		if (m_nb_transport_fw_cb_active) {
			m_ifw->register_nb_transport_fw(&(this->nb_transport_fw));
		}
		if (m_transport_dbg_cb_active) {
			m_ifw->register_transport_dbg(&(this->transport_dbg));
		}
		if (!m_get_direct_mem_ptr_cb_active) {
		  // link to a valid implementation of the interface
		  this->set_get_direct_mem_ptr_callback(SCML2_CALLBACK(get_direct_mem_ptr_stub));
		}
		m_ifw->register_get_direct_mem_ptr(&(this->get_direct_mem_ptr));
	}
  } else {
		m_adaptor = scml2::target_port_adaptor::create(m_name + "_adaptor", &(this->m_socket));
		if (this->consume_annotated_time.get_initialized()) {
		  m_adaptor->set_attribute("consume_annotated_time", this->consume_annotated_time ? 1 : 0);
		}
		if (this->abstraction == FT) {
		  if (read_capacity_value.is_initialized()) {
		    m_adaptor->set_attribute("read_capacity", read_capacity_value.get());
		    read_capacity_value.reset();
		  }
		  if (write_capacity_value.is_initialized()) {
		    m_adaptor->set_attribute("write_capacity", write_capacity_value.get());
		    write_capacity_value.reset();
		  }
		  if (total_capacity_value.is_initialized()) {
		    m_adaptor->set_attribute("total_capacity", total_capacity_value.get());
		    total_capacity_value.reset();
		  }
		  if (rd_cmd_accept_cycles_value.is_initialized()) {
		    m_adaptor->set_attribute("rd_cmd_accept_cycles", rd_cmd_accept_cycles_value.get());
		    rd_cmd_accept_cycles_value.reset();
		  }
		  if (rd_data_trigger_cycles_value.is_initialized()) {
		    m_adaptor->set_attribute("rd_data_trigger_cycles", rd_data_trigger_cycles_value.get());
		    rd_data_trigger_cycles_value.reset();
		  }
		  if (wr_cmd_accept_cycles_value.is_initialized()) {
		    m_adaptor->set_attribute("wr_cmd_accept_cycles", wr_cmd_accept_cycles_value.get());
		    wr_cmd_accept_cycles_value.reset();
		  }
		  if (wr_data_accept_cycles_value.is_initialized()) {
		    m_adaptor->set_attribute("wr_data_accept_cycles", wr_data_accept_cycles_value.get());
		    wr_data_accept_cycles_value.reset();
		  }
		  if (wr_rsp_trigger_cycles_value.is_initialized()) {
		    m_adaptor->set_attribute("wr_rsp_trigger_cycles", wr_rsp_trigger_cycles_value.get());
		    wr_rsp_trigger_cycles_value.reset();
		  }
		  if (req_accept_cycles_value.is_initialized()) {
		    m_adaptor->set_attribute("req_accept_cycles", req_accept_cycles_value.get());
		    req_accept_cycles_value.reset();
		  }
		  if (collect_databeats_value.is_initialized()) {
		    m_adaptor->set_attribute("collect_databeats", collect_databeats_value.get());
		    collect_databeats_value.reset();
		  }
		  if (dba_supported_value.is_initialized()) {
		    m_adaptor->set_attribute("dba_supported", dba_supported_value.get());
		    dba_supported_value.reset();
		  }
		  if (invoke_timing_cbks_value.is_initialized()) {
		    m_adaptor->set_attribute("invoke_timing_cbks", invoke_timing_cbks_value.get());
		    invoke_timing_cbks_value.reset();
		  }
		  if (invoke_behavior_cbks_value.is_initialized()) {
		    m_adaptor->set_attribute("invoke_behavior_cbks", invoke_behavior_cbks_value.get());
		    invoke_behavior_cbks_value.reset();
		  }

		  if (m_rd_addr_start_cb_active) {
		    (m_adaptor)->set_timing_callback(scml2::RD_ADDR_START, scml2::create_timing_callback(& (this->rd_addr_start), &scml2::base::callback< tlm::tlm_sync_enum , tlm::tlm_generic_payload&, sc_core::sc_time& >::operator()));
		  }

		  if (m_rd_data_last_end_cb_active) {
		    (m_adaptor)->set_timing_callback(scml2::RD_DATA_LAST_END, scml2::create_timing_callback(& (this->rd_data_last_end), &scml2::base::callback< tlm::tlm_sync_enum , tlm::tlm_generic_payload&, sc_core::sc_time& >::operator()));
		  }

		  if (m_rd_data_last_start_cb_active) {
		    (m_adaptor)->set_timing_callback(scml2::RD_DATA_LAST_START, scml2::create_timing_callback(& (this->rd_data_last_start), &scml2::base::callback< tlm::tlm_sync_enum , tlm::tlm_generic_payload&, sc_core::sc_time& >::operator()));
		  }

		  if (m_rd_data_end_cb_active) {
		    (m_adaptor)->set_timing_callback(scml2::RD_DATA_END, scml2::create_timing_callback(& (this->rd_data_end), &scml2::base::callback< tlm::tlm_sync_enum , tlm::tlm_generic_payload&, sc_core::sc_time& >::operator()));
		  }

		  if (m_rd_data_start_cb_active) {
		    (m_adaptor)->set_timing_callback(scml2::RD_DATA_START, scml2::create_timing_callback(& (this->rd_data_start), &scml2::base::callback< tlm::tlm_sync_enum , tlm::tlm_generic_payload&, sc_core::sc_time& >::operator()));
		  }

		  if (m_rd_addr_end_cb_active) {
		    (m_adaptor)->set_timing_callback(scml2::RD_ADDR_END, scml2::create_timing_callback(& (this->rd_addr_end), &scml2::base::callback< tlm::tlm_sync_enum , tlm::tlm_generic_payload&, sc_core::sc_time& >::operator()));
		  }

		  if (m_wr_rsp_end_cb_active) {
		    (m_adaptor)->set_timing_callback(scml2::WR_RSP_END, scml2::create_timing_callback(& (this->wr_rsp_end), &scml2::base::callback< tlm::tlm_sync_enum , tlm::tlm_generic_payload&, sc_core::sc_time& >::operator()));
		  }

		  if (m_wr_rsp_start_cb_active) {
		    (m_adaptor)->set_timing_callback(scml2::WR_RSP_START, scml2::create_timing_callback(& (this->wr_rsp_start), &scml2::base::callback< tlm::tlm_sync_enum , tlm::tlm_generic_payload&, sc_core::sc_time& >::operator()));
		  }

		  if (m_wr_data_last_end_cb_active) {
		    (m_adaptor)->set_timing_callback(scml2::WR_DATA_LAST_END, scml2::create_timing_callback(& (this->wr_data_last_end), &scml2::base::callback< tlm::tlm_sync_enum , tlm::tlm_generic_payload&, sc_core::sc_time& >::operator()));
		  }

		  if (m_wr_data_last_start_cb_active) {
		    (m_adaptor)->set_timing_callback(scml2::WR_DATA_LAST_START, scml2::create_timing_callback(& (this->wr_data_last_start), &scml2::base::callback< tlm::tlm_sync_enum , tlm::tlm_generic_payload&, sc_core::sc_time& >::operator()));
		  }

		  if (m_wr_data_end_cb_active) {
		    (m_adaptor)->set_timing_callback(scml2::WR_DATA_END, scml2::create_timing_callback(& (this->wr_data_end), &scml2::base::callback< tlm::tlm_sync_enum , tlm::tlm_generic_payload&, sc_core::sc_time& >::operator()));
		  }

		  if (m_wr_data_start_cb_active) {
		    (m_adaptor)->set_timing_callback(scml2::WR_DATA_START, scml2::create_timing_callback(& (this->wr_data_start), &scml2::base::callback< tlm::tlm_sync_enum , tlm::tlm_generic_payload&, sc_core::sc_time& >::operator()));
		  }

		  if (m_wr_addr_end_cb_active) {
		    (m_adaptor)->set_timing_callback(scml2::WR_ADDR_END, scml2::create_timing_callback(& (this->wr_addr_end), &scml2::base::callback< tlm::tlm_sync_enum , tlm::tlm_generic_payload&, sc_core::sc_time& >::operator()));
		  }

		  if (m_wr_addr_start_cb_active) {
		    (m_adaptor)->set_timing_callback(scml2::WR_ADDR_START, scml2::create_timing_callback(& (this->wr_addr_start), &scml2::base::callback< tlm::tlm_sync_enum , tlm::tlm_generic_payload&, sc_core::sc_time& >::operator()));
		  }
		} else {
		  assert(this->abstraction == LT);
		  m_adaptor->set_attribute("invoke_timing_cbks", 0); // never invoke timing cbks if component designed for LT mode
		  if (this->intercepts.size() > 0) {
			  (*m_adaptor)(*(this->intercepts[0]));
		      for (size_t i = 1; i < this->intercepts.size(); i++) {
		    	  (*(this->intercepts[i-1]))(*(this->intercepts[i]));
			  }
		  }
		}
	}

  handle_binding();
}


template <unsigned int BUSWIDTH>
scml2::callback_event_enum tlm2_ft_target_port_pe<BUSWIDTH>::get_event_id_impl(unsigned int protocol_state) {
	return get_ft_adaptor_for("get_event_id").get_event_id(protocol_state);
};

template <unsigned int BUSWIDTH>
bool tlm2_ft_target_port_pe<BUSWIDTH>::update_transaction_impl(tlm::tlm_generic_payload& trans, sc_core::sc_time&  delay, scml2::callback_event_enum event_id) {
	if (event_id == scml2::CBK_EVENT_NONE) {
		return get_ft_adaptor_for("update_transaction").update_transaction(trans, delay);
	}
	return get_ft_adaptor_for("update_transaction").update_transaction(trans, delay, event_id);
};

template <unsigned int BUSWIDTH>
void tlm2_ft_target_port_pe<BUSWIDTH>::invalidate_direct_mem_ptr_impl(sc_dt::uint64 range_start,sc_dt::uint64 range_end) {
	this->m_socket->invalidate_direct_mem_ptr(range_start, range_end);
}

template <unsigned int BUSWIDTH>
tlm::tlm_sync_enum tlm2_ft_target_port_pe<BUSWIDTH>::nb_transport_bw_impl(tlm::tlm_generic_payload& trans, tlm::tlm_phase& phase, sc_core::sc_time& time) {
	return this->m_socket->nb_transport_bw(trans, phase, time);
}

template <unsigned int BUSWIDTH>
void tlm2_ft_target_port_pe<BUSWIDTH>::operator() (scml2::mappable_if& dest) {
  m_dest = &dest;
  if (this->finalized()) {
    handle_binding();
  }
};

template <unsigned int BUSWIDTH>
void tlm2_ft_target_port_pe<BUSWIDTH>::handle_binding() {
  if (m_dest != nullptr) {
    assert(this->abstraction != TLM2_GP);
    if (this->intercepts.size() == 0 || this->abstraction == FT) {
      get_lt_or_ft_adaptor_for("bind to memory (or other scml2::mappable_if)")(*m_dest);
    } else {
      (*(this->intercepts[this->intercepts.size()-1]))(*m_dest);
    }  
    m_dest = nullptr;
  }
}

template <unsigned int BUSWIDTH>
std::string tlm2_ft_target_port_pe<BUSWIDTH>::get_protocol_impl(){
  return get_ft_adaptor_for("get_protocol").get_protocol();
}

#define DEFINE_ATTRIBUTE_GETTER_SETTER_WRAPPERS(attr_type, attr_name) \
template <unsigned int BUSWIDTH> \
attr_type tlm2_ft_target_port_pe<BUSWIDTH>::get_ ## attr_name ## _impl() { \
	assert(this->m_adaptor != 0); \
        if (this->m_adaptor != 0) { \
                return get_ft_adaptor_for("never_needed").get_attribute(#attr_name); \
        } \
        return attr_type(); \
}; \
template <unsigned int BUSWIDTH> \
void tlm2_ft_target_port_pe<BUSWIDTH>::set_ ## attr_name ## _impl(attr_type value) { \
        if (this->m_adaptor != 0) { \
        	get_ft_adaptor_for("never_needed").set_attribute(#attr_name, value); \
        } else {\
                this->attr_name ## _value = value; \
        } \
};

DEFINE_ATTRIBUTE_GETTER_SETTER_WRAPPERS (int, read_capacity)
DEFINE_ATTRIBUTE_GETTER_SETTER_WRAPPERS (int, write_capacity)
DEFINE_ATTRIBUTE_GETTER_SETTER_WRAPPERS (int, total_capacity)
DEFINE_ATTRIBUTE_GETTER_SETTER_WRAPPERS (int, rd_cmd_accept_cycles)
DEFINE_ATTRIBUTE_GETTER_SETTER_WRAPPERS (int, rd_data_trigger_cycles)
DEFINE_ATTRIBUTE_GETTER_SETTER_WRAPPERS (int, wr_cmd_accept_cycles)
DEFINE_ATTRIBUTE_GETTER_SETTER_WRAPPERS (int, wr_data_accept_cycles)
DEFINE_ATTRIBUTE_GETTER_SETTER_WRAPPERS (int, wr_rsp_trigger_cycles)
DEFINE_ATTRIBUTE_GETTER_SETTER_WRAPPERS (int, req_accept_cycles)
DEFINE_ATTRIBUTE_GETTER_SETTER_WRAPPERS (bool, collect_databeats)
DEFINE_ATTRIBUTE_GETTER_SETTER_WRAPPERS (bool, dba_supported)
DEFINE_ATTRIBUTE_GETTER_SETTER_WRAPPERS (int, invoke_timing_cbks)
DEFINE_ATTRIBUTE_GETTER_SETTER_WRAPPERS (int, invoke_behavior_cbks)

#undef DEFINE_ATTRIBUTE_GETTER_SETTER_WRAPPERS

template <unsigned int BUSWIDTH>
sc_core::sc_in<bool>& tlm2_ft_target_port_pe<BUSWIDTH>::get_clock_port_impl(){
	assert(m_clock);
	return *m_clock;
}
template <unsigned int BUSWIDTH>
void tlm2_ft_target_port_pe<BUSWIDTH>::set_clock_port_impl(sc_core::sc_in<bool>& value){
  m_clock = &value;
}

template <unsigned int BUSWIDTH>
unsigned int tlm2_ft_target_port_pe<BUSWIDTH>::get_bus_width_impl(){
  return get_ft_adaptor_for("get_bus_width").get_bus_width();
}

} // namespace scml2
