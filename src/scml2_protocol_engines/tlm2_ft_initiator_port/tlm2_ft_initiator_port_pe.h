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
#include "scml2_protocol_engines/tlm2_ft_initiator_port/include/tlm2_ft_initiator_port_peBase.h"

#include <memory>

namespace scml2 {

template <unsigned int BUSWIDTH=32>
class tlm2_ft_initiator_port_pe 
	: public tlm2_ft_initiator_port_peBase<BUSWIDTH> 
	, public scml2::mappable_if
{
	using invalidate_dmi_cb_t = scml2::base::callback< void, sc_dt::uint64, sc_dt::uint64 >;
	scml2::initiator_port_adaptor& get_adaptor_for(const std::string& api) {
	  if (this->abstraction == TLM2_GP) {
	    SCML2_ERROR(FATAL_ERROR) << "initiator protocol engine '" << this->get_name() << "' configured for TLM2_GP abstraction, API '" << api << "' not available." << std::endl;
	  }
	  return *m_adaptor;
	}	
	scml2::initiator_port_adaptor& get_ft_adaptor_for(const std::string& api) {
	  if (this->abstraction != FT) {
	    SCML2_ERROR(FATAL_ERROR) << "initiator protocol engine '" << this->get_name() << "' not configured for FT abstraction, API '" << api << "' not available." << std::endl;
	  }
	  return *m_adaptor;
	}	
public:

	tlm2_ft_initiator_port_pe( const scml2::base::object_name& name, scml2::ft_initiator_socket<BUSWIDTH>& socket);

	void finalize_local_construction() override;	
	virtual scml2::callback_event_enum get_event_id_impl(unsigned int protocol_state) ;
	virtual tlm::tlm_generic_payload& alloc_and_init_trans_impl(tlm::tlm_command cmd) ;
	virtual bool send_transaction_impl(tlm::tlm_generic_payload& trans, const sc_core::sc_time& delay) ;
	virtual bool update_transaction_impl(tlm::tlm_generic_payload& trans, sc_core::sc_time&  delay, scml2::callback_event_enum event_id) ;
	virtual unsigned int get_burst_length_impl(tlm::tlm_generic_payload& trans) ;

	bool read(unsigned long long address, unsigned char* data, unsigned int count, sc_core::sc_time& t) {
		return m_adaptor->read(address, data, count, t);
	}
	bool write(unsigned long long address, const unsigned char* data, unsigned int count, sc_core::sc_time& t) {
		return m_adaptor->write(address, data, count, t);
	}
	unsigned int read_debug(unsigned long long address, unsigned char* data, unsigned int count) {
		return read_debug(address, data, count);
	}
	unsigned int write_debug(unsigned long long address, const unsigned char* data, unsigned int count) {
		return write_debug(address, data, count);
	}

	template <typename DT> 
	bool read(unsigned long long address, DT& data) {
		return get_adaptor_for("read").read(address, data);
	}
	template <typename DT> bool read(unsigned long long address, DT* data, unsigned int count) {
		return get_adaptor_for("read").read(address, data, count);
	}
	template <typename DT> bool read(unsigned long long address, DT& data, sc_core::sc_time& t) {
		return get_adaptor_for("read").read(address, data, t);
	}
	template <typename DT> bool read(unsigned long long address, DT* data, unsigned int count, sc_core::sc_time& t) {
		return get_adaptor_for("read").read(address, data, count, t);
	}

	template <typename DT> bool write(unsigned long long address, const DT& data) {
		return get_adaptor_for("write").write(address, data);
	}
	template <typename DT> bool write(unsigned long long address, const DT* data, unsigned int count) {
		return get_adaptor_for("write").write(address, data, count);
	}
	template <typename DT> bool write(unsigned long long address, const DT& data, sc_core::sc_time& t) {
		return get_adaptor_for("write").write(address, data, t);
	}
	template <typename DT> bool write(unsigned long long address, const DT* data, unsigned int count, sc_core::sc_time& t) {
		return get_adaptor_for("write").write(address, data, count, t);
	}
	template <typename DT> bool read_debug(unsigned long long address, DT& data) {
		return get_adaptor_for("read_debug").read_debug(address, data);
	}
	template <typename DT> bool read_debug(unsigned long long address, DT* data, unsigned int count) {
		return get_adaptor_for("read_debug").read_debug(address, data, count);
	}

	template <typename DT> bool write_debug(unsigned long long address, const DT& data) {
		return get_adaptor_for("write_debug").write_debug(address, data);
	}
	template <typename DT> bool write_debug(unsigned long long address, const DT* data, unsigned int count) {
		return get_adaptor_for("write_debug").write_debug(address, data, count);
	}
	virtual int get_read_capacity_impl() ;
	virtual void set_read_capacity_impl(int value) ;
	virtual int get_write_capacity_impl() ;
	virtual void set_write_capacity_impl(int value) ;
	virtual int get_total_capacity_impl() ;
	virtual void set_total_capacity_impl(int value) ;
	virtual int get_rd_data_accept_cycles_impl() ;
	virtual void set_rd_data_accept_cycles_impl(int value) ;
	virtual int get_wr_data_trigger_cycles_impl() ;
	virtual void set_wr_data_trigger_cycles_impl(int value) ;
	virtual int get_wr_rsp_accept_cycles_impl() ;
	virtual void set_wr_rsp_accept_cycles_impl(int value) ;
	virtual int get_handle_tkeep_impl() ;
	virtual void set_handle_tkeep_impl(int value) ;
	virtual int get_dba_supported_impl() ;
	virtual void set_dba_supported_impl(int value) ;
	virtual int get_max_beats_impl() ;
	virtual void set_max_beats_impl(int value) ;
	virtual int get_invoke_timing_cbks_impl() ;
	virtual void set_invoke_timing_cbks_impl(int value) ;
	virtual bool get_consume_annotated_time_impl() ;
	virtual void set_consume_annotated_time_impl(bool value) ;

	virtual std::string get_protocol_impl() ;
	virtual unsigned int get_bus_width_impl() ;
	virtual sc_core::sc_in<bool>& get_clock_port_impl();
	virtual void set_clock_port_impl(sc_core::sc_in<bool>& value);
	bool get_dmi_enabled_impl() override ;
	void set_dmi_enabled_impl(bool value) override;

  // mappable_if
	std::string get_mapped_name() const override;
	void b_transport( tlm::tlm_generic_payload& trans, sc_core::sc_time& time) override;
	bool get_direct_mem_ptr( tlm::tlm_generic_payload& trans, tlm::tlm_dmi& dmi) override;
	scml2::uint transport_dbg( tlm::tlm_generic_payload& trans) override;
	void register_bw_direct_mem_if(tlm::tlm_bw_direct_mem_if* bwInterface) override;
	void unregister_bw_direct_mem_if(tlm::tlm_bw_direct_mem_if* bwInterface) override;

	virtual tlm::tlm_sync_enum  nb_transport_fw_impl( tlm::tlm_generic_payload& trans, tlm::tlm_phase& phase, sc_core::sc_time& time);

  std::unique_ptr<scml2::initiator_port_adaptor> m_adaptor;
protected :
	class bw_interface_wrapper : public scml2::bw_interface_type 
	{
		using nb_transport_bw_cb_t = scml2::base::callback< tlm::tlm_sync_enum, tlm::tlm_generic_payload&, tlm::tlm_phase&, sc_core::sc_time&>;

		nb_transport_bw_cb_t* m_nb_transport_bw_callback{};
		invalidate_dmi_cb_t* m_invalidate_direct_mem_ptr_callback{};

		tlm::tlm_sync_enum nb_transport_bw(tlm::tlm_generic_payload& trans, tlm::tlm_phase& phase, sc_core::sc_time& time) override {
		  if (m_nb_transport_bw_callback) {
		    return m_nb_transport_bw_callback->operator()(trans, phase, time);
		  } else {
		    assert(false);
		    return tlm::TLM_COMPLETED;
		  }
		}

		void invalidate_direct_mem_ptr(sc_dt::uint64 start, sc_dt::uint64 end) override {
		  if (m_invalidate_direct_mem_ptr_callback) {
		    m_invalidate_direct_mem_ptr_callback->operator()(start, end);
		  }
		}
	public:
		bw_interface_wrapper(scml2::ft_initiator_socket<BUSWIDTH>& socket) 
		{
		  socket.bind(*this);
		}

		void register_nb_transport_bw(nb_transport_bw_cb_t* cb){
		  m_nb_transport_bw_callback = cb;
		}

		void register_invalidate_direct_mem_ptr(invalidate_dmi_cb_t* cb){
		  m_invalidate_direct_mem_ptr_callback = cb;
		}
	};
  std::unique_ptr<bw_interface_wrapper> m_ifw;

  class bw_mappable_if 
    : public tlm::tlm_bw_direct_mem_if 
	{
    scml2::mappable_if& m_if;
    scml2::base::callback<void, sc_dt::uint64, sc_dt::uint64>& m_invalidate_direct_mem_ptr_callback;
    
    void invalidate_direct_mem_ptr(sc_dt::uint64 start, sc_dt::uint64 end) override {
      m_invalidate_direct_mem_ptr_callback(start, end);
    }
  public:
    bw_mappable_if(scml2::mappable_if& mif, scml2::base::callback<void, sc_dt::uint64, sc_dt::uint64>& cb) 
      : m_if(mif)
      , m_invalidate_direct_mem_ptr_callback(cb)
    {
      m_if.register_bw_direct_mem_if(this);
		}

    ~bw_mappable_if()
    {
			m_if.unregister_bw_direct_mem_if(this);
    }
  };

  std::unique_ptr<bw_mappable_if> m_mappable_ifw;

	private:
	std::string m_name;
	sc_core::sc_in<bool>* m_clock;

	// temp storage for mappable_if stuff
	tlm::tlm_bw_direct_mem_if* m_bwInterface;

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
        attribute_value<int> rd_data_accept_cycles_value;
        attribute_value<int> wr_data_trigger_cycles_value;
        attribute_value<int> wr_rsp_accept_cycles_value;
        attribute_value<int> handle_tkeep_value;
        attribute_value<int> dba_supported_value;
        attribute_value<int> max_beats_value;
        attribute_value<int> invoke_timing_cbks_value;
        attribute_value<bool> dmi_enabled_value;
        attribute_value<bool> consume_annotated_time_value;

};

template <unsigned int BUSWIDTH>
tlm2_ft_initiator_port_pe<BUSWIDTH>::tlm2_ft_initiator_port_pe( const scml2::base::object_name& name, scml2::ft_initiator_socket<BUSWIDTH>& socket )
	: tlm2_ft_initiator_port_peBase<BUSWIDTH>(name, socket)
  , m_name(sc_core::sc_gen_unique_name(name.c_str()))
  , m_clock(0)
  , m_bwInterface(nullptr)
  {
  }

template <unsigned int BUSWIDTH>
void tlm2_ft_initiator_port_pe<BUSWIDTH>::finalize_local_construction()
{
	if (this->abstraction == TLM2_GP) {
		m_ifw.reset(new bw_interface_wrapper(this->_socket));
		if (this->nb_transport_bw) {
			m_ifw->register_nb_transport_bw(&(this->nb_transport_bw));
		}
		if(this->invalidate_direct_mem_ptr){
			m_ifw->register_invalidate_direct_mem_ptr(&(this->invalidate_direct_mem_ptr));
		}
	} else {
		m_adaptor.reset(scml2::initiator_port_adaptor::create(m_name + "_adaptor", &(this->_socket)));
		if (consume_annotated_time_value.is_initialized()) {
		  m_adaptor->set_attribute("consume_annotated_time", consume_annotated_time_value.get() ? 1 : 0);
		  consume_annotated_time_value.reset();
		}
		if (m_bwInterface != nullptr) {
			m_adaptor->register_bw_direct_mem_if(m_bwInterface);
		}

		if( this->invalidate_direct_mem_ptr){
		  m_mappable_ifw.reset(new bw_mappable_if(*m_adaptor, this->invalidate_direct_mem_ptr));
		}

		if (dmi_enabled_value.is_initialized()) {
		  set_dmi_enabled_impl(dmi_enabled_value.get());
		}
		if (this->abstraction == FT) {

#define SET_FT_VALUE(name)																							\
		  if ((name ## _value).is_initialized()) {		\
		    m_adaptor->set_attribute(#name, (name ## _value).get()); \
		    (name ## _value).reset();				\
		  }
		  SET_FT_VALUE(read_capacity);
		  SET_FT_VALUE(write_capacity);
		  SET_FT_VALUE(total_capacity);
		  SET_FT_VALUE(rd_data_accept_cycles);
		  SET_FT_VALUE(wr_data_trigger_cycles);
		  SET_FT_VALUE(wr_rsp_accept_cycles);
		  SET_FT_VALUE(handle_tkeep);
		  SET_FT_VALUE(dba_supported);
		  SET_FT_VALUE(max_beats);
		  SET_FT_VALUE(invoke_timing_cbks);
#undef SET_FT_VALUE
		  
#define REG_FT_CB(cb, enumvalue)		\
		  if (this->cb) {					\
		    m_adaptor->set_timing_callback(scml2::enumvalue, scml2::create_timing_callback(& (this->cb), &scml2::base::callback< tlm::tlm_sync_enum , tlm::tlm_generic_payload&, sc_core::sc_time& >::operator())); \
		  }
		  REG_FT_CB(rd_addr_start, RD_ADDR_START);
		  REG_FT_CB(rd_data_last_end, RD_DATA_LAST_END);
		  REG_FT_CB(rd_data_last_start, RD_DATA_LAST_START);
		  REG_FT_CB(rd_data_end, RD_DATA_END);
		  REG_FT_CB(rd_data_start, RD_DATA_START);
		  REG_FT_CB(rd_addr_end, RD_ADDR_END);
		  REG_FT_CB(wr_rsp_end, WR_RSP_END);
		  REG_FT_CB(wr_rsp_start, WR_RSP_START);
		  REG_FT_CB(wr_data_last_end, WR_DATA_LAST_END);
		  REG_FT_CB(wr_data_last_start, WR_DATA_LAST_START);
		  REG_FT_CB(wr_data_end, WR_DATA_END);
		  REG_FT_CB(wr_data_start, WR_DATA_START);
		  REG_FT_CB(wr_addr_end, WR_ADDR_END);
		  REG_FT_CB(wr_addr_start, WR_ADDR_START);
#undef REG_FT_CB

		} // abstraction == FT 
		else {
		  assert(this->abstraction == LT);
		  m_adaptor->set_attribute("invoke_timing_cbks", 0); // never invoke timing cbks if component designed for LT mode
		}
	}
}


template <unsigned int BUSWIDTH>
scml2::callback_event_enum tlm2_ft_initiator_port_pe<BUSWIDTH>::get_event_id_impl(unsigned int protocol_state) {
	return get_ft_adaptor_for("get_event_id").get_event_id(protocol_state);
};

template <unsigned int BUSWIDTH>
tlm::tlm_generic_payload& tlm2_ft_initiator_port_pe<BUSWIDTH>::alloc_and_init_trans_impl(tlm::tlm_command cmd) {
	return get_ft_adaptor_for("alloc_and_init_trans").alloc_and_init_trans(cmd);
};

template <unsigned int BUSWIDTH>
bool tlm2_ft_initiator_port_pe<BUSWIDTH>::send_transaction_impl(tlm::tlm_generic_payload& trans, const sc_core::sc_time& delay) {
	return get_ft_adaptor_for("send_transaction").send_transaction(trans, delay);
};

template <unsigned int BUSWIDTH>
bool tlm2_ft_initiator_port_pe<BUSWIDTH>::update_transaction_impl(tlm::tlm_generic_payload& trans, sc_core::sc_time&  delay, scml2::callback_event_enum event_id) {
	if (event_id == scml2::CBK_EVENT_NONE) {
		return get_ft_adaptor_for("update_transaction").update_transaction(trans, delay);
	}
	return get_ft_adaptor_for("update_transaction").update_transaction(trans, delay, event_id);
};

template <unsigned int BUSWIDTH>
unsigned int tlm2_ft_initiator_port_pe<BUSWIDTH>::get_burst_length_impl(tlm::tlm_generic_payload& trans) {
	return get_ft_adaptor_for("get_burst_length").get_burst_length(trans);
};

#define DEFINE_ATTRIBUTE_GETTER_SETTER_WRAPPERS(attr_type, attr_name) \
template <unsigned int BUSWIDTH> \
attr_type tlm2_ft_initiator_port_pe<BUSWIDTH>::get_ ## attr_name ## _impl() { \
	assert(this->m_adaptor != 0); \
        if (this->m_adaptor != 0) { \
                return get_ft_adaptor_for("never_needed").get_attribute(#attr_name); \
        } \
        return attr_type(); \
}; \
template <unsigned int BUSWIDTH> \
void tlm2_ft_initiator_port_pe<BUSWIDTH>::set_ ## attr_name ## _impl(attr_type value) { \
        if (this->m_adaptor != 0) { \
        	get_ft_adaptor_for("never_needed").set_attribute(#attr_name, value); \
        } else {\
                this->attr_name ## _value = value; \
        } \
};

DEFINE_ATTRIBUTE_GETTER_SETTER_WRAPPERS (int, read_capacity)
DEFINE_ATTRIBUTE_GETTER_SETTER_WRAPPERS (int, write_capacity)
DEFINE_ATTRIBUTE_GETTER_SETTER_WRAPPERS (int, total_capacity)
DEFINE_ATTRIBUTE_GETTER_SETTER_WRAPPERS (int, rd_data_accept_cycles)
DEFINE_ATTRIBUTE_GETTER_SETTER_WRAPPERS (int, wr_data_trigger_cycles)
DEFINE_ATTRIBUTE_GETTER_SETTER_WRAPPERS (int, wr_rsp_accept_cycles)
DEFINE_ATTRIBUTE_GETTER_SETTER_WRAPPERS (int, handle_tkeep)
DEFINE_ATTRIBUTE_GETTER_SETTER_WRAPPERS (int, dba_supported)
DEFINE_ATTRIBUTE_GETTER_SETTER_WRAPPERS (int, max_beats)
DEFINE_ATTRIBUTE_GETTER_SETTER_WRAPPERS (int, invoke_timing_cbks)

#undef DEFINE_ATTRIBUTE_GETTER_SETTER_WRAPPERS
template <unsigned int BUSWIDTH>
bool tlm2_ft_initiator_port_pe<BUSWIDTH>::get_consume_annotated_time_impl() {
    if (m_adaptor != nullptr) {
        return (bool)m_adaptor->get_attribute("consume_annotated_time");
    }
    return consume_annotated_time_value.is_initialized() ? consume_annotated_time_value.get() : true;
}

template <unsigned int BUSWIDTH>
void tlm2_ft_initiator_port_pe<BUSWIDTH>::set_consume_annotated_time_impl(bool value) {
    if (m_adaptor != nullptr) {
        m_adaptor->set_attribute("consume_annotated_time", value ? 1 : 0);
    } else {
        consume_annotated_time_value = value;
    }
}


template <unsigned int BUSWIDTH>
std::string tlm2_ft_initiator_port_pe<BUSWIDTH>::get_protocol_impl() {
	return get_ft_adaptor_for("get_protocol").get_protocol();
};

template <unsigned int BUSWIDTH>
unsigned int tlm2_ft_initiator_port_pe<BUSWIDTH>::get_bus_width_impl() {
	return get_ft_adaptor_for("get_bus_width").get_bus_width();
};

template <unsigned int BUSWIDTH>
sc_core::sc_in<bool>& tlm2_ft_initiator_port_pe<BUSWIDTH>::get_clock_port_impl(){
	assert(m_clock);
	return *m_clock;
};

template <unsigned int BUSWIDTH>
void tlm2_ft_initiator_port_pe<BUSWIDTH>::set_clock_port_impl(sc_core::sc_in<bool>& value){
	m_clock = &value;
};

template <unsigned int BUSWIDTH>
bool tlm2_ft_initiator_port_pe<BUSWIDTH>::get_dmi_enabled_impl(){
  return get_adaptor_for("dmi_enabled").is_dmi_enabled();
};

template <unsigned int BUSWIDTH>
void tlm2_ft_initiator_port_pe<BUSWIDTH>::set_dmi_enabled_impl(bool value){
  if (m_adaptor == nullptr) {
    dmi_enabled_value = value;
  } else {
    if (value) {
      get_adaptor_for("dmi_enabled").enable_dmi();
    } else {
      get_adaptor_for("dmi_enabled").disable_dmi();
    }
  }
};


template <unsigned int BUSWIDTH>
tlm::tlm_sync_enum tlm2_ft_initiator_port_pe<BUSWIDTH>::nb_transport_fw_impl( tlm::tlm_generic_payload& trans, tlm::tlm_phase& phase, sc_core::sc_time& time) {
	return this->_socket->nb_transport_fw(trans, phase, time);
};

// mappable_if

template <unsigned int BUSWIDTH>
std::string tlm2_ft_initiator_port_pe<BUSWIDTH>::get_mapped_name() const 
{
  return this->get_name();
}

template <unsigned int BUSWIDTH>
void tlm2_ft_initiator_port_pe<BUSWIDTH>::b_transport( tlm::tlm_generic_payload& trans, sc_core::sc_time& time) {
	this->_socket->b_transport(trans, time);
};

template <unsigned int BUSWIDTH>
bool tlm2_ft_initiator_port_pe<BUSWIDTH>::get_direct_mem_ptr( tlm::tlm_generic_payload& trans, tlm::tlm_dmi& dmi) {
	return this->_socket->get_direct_mem_ptr(trans, dmi);
};

template <unsigned int BUSWIDTH>
scml2::uint tlm2_ft_initiator_port_pe<BUSWIDTH>::transport_dbg( tlm::tlm_generic_payload& trans) {
	return this->_socket->transport_dbg(trans);
}

template <unsigned int BUSWIDTH>
void tlm2_ft_initiator_port_pe<BUSWIDTH>::register_bw_direct_mem_if(tlm::tlm_bw_direct_mem_if* bwInterface)
{
  assert(!this->invalidate_direct_mem_ptr);
  if (m_adaptor != nullptr) {
	  m_adaptor->register_bw_direct_mem_if(bwInterface);
  } else {
	  m_bwInterface = bwInterface;
  }
}

template <unsigned int BUSWIDTH>
void tlm2_ft_initiator_port_pe<BUSWIDTH>::unregister_bw_direct_mem_if(tlm::tlm_bw_direct_mem_if* bwInterface)
{
  assert(!this->invalidate_direct_mem_ptr);
  if (m_adaptor != nullptr) {
  	  m_adaptor->unregister_bw_direct_mem_if(bwInterface);
  } else {
	  m_bwInterface = nullptr;
  }
}



} // namespace scml2
