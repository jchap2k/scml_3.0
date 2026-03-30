/***************************************************************************
 * Copyright 1996-2025 Synopsys, Inc.
 *
 * This Synopsys software and all associated documentation are proprietary
 * to Synopsys, Inc. and may only be used pursuant to the terms and
 * conditions of a written license agreement with Synopsys, Inc.
 * All other use, reproduction, modification, or distribution of the
 * Synopsys software or the associated documentation is strictly prohibited.
 ***************************************************************************/
#ifndef __INITIATOR_PORT_ADAPTOR_H__
#define __INITIATOR_PORT_ADAPTOR_H__ 

#include <set>
#include <scml2/mappable_if.h>
#include <scml2/port_adaptor.h>
#include <scml2/utils.h>

#include <scml2/tagged_message_macros.h>

namespace scml2
{

/*****************************************************************************
 * Name: initiator_port_adaptor
 *
 * Description: Base class for all initiator port adaptors.
 ****************************************************************************/
class SNPS_VP_API initiator_port_adaptor :  public port_adaptor,
                                            public mappable_if,
                                            public tlm::tlm_bw_transport_if<> {

protected:
  template <typename T> struct SizeOfBits { enum { result = SizeOf<T>::result * 8 }; };

  /** Initiator socket wrapper class, used to access the templated socket
    * from the non-templated initiator port adaptor class. */
  class socket_if : public tlm::tlm_fw_transport_if<>
  {
  public:
    virtual std::string name() const = 0;
    virtual unsigned int get_bus_width() const = 0;
  };

  template <typename socket_type>
  class socket_wrapper : public socket_if {
  public:
    socket_wrapper(socket_type* socket) : m_init_socket(socket) { }
    virtual std::string name() const { return m_init_socket->name(); }
    virtual unsigned int get_bus_width() const { return m_init_socket->get_bus_width(); }


    virtual bool get_direct_mem_ptr(tlm::tlm_generic_payload& trans, tlm::tlm_dmi& dmiData)
    {
      return (*m_init_socket)->get_direct_mem_ptr(trans, dmiData);
    }

    virtual unsigned int transport_dbg(tlm::tlm_generic_payload& trans)
    {
      return (*m_init_socket)->transport_dbg(trans);
    }

    virtual void b_transport(tlm::tlm_generic_payload& trans, sc_time& t)
    {
      (*m_init_socket)->b_transport(trans, t);
    }

    virtual tlm::tlm_sync_enum nb_transport_fw(tlm::tlm_generic_payload& trans, tlm::tlm_phase& phase, sc_time& t)
    {
      return (*m_init_socket)->nb_transport_fw(trans, phase, t);
    }

  public:
    socket_type* m_init_socket;
  };

protected:
  // Port adaptor is not constructed directly - use the create API instead
  initiator_port_adaptor(const std::string& name) : port_adaptor(name) { }

  static initiator_port_adaptor* _create_adaptor(const std::string& protocol, const std::string& name, socket_if* socket);

public:

  template <typename socket_type>
  static initiator_port_adaptor* create(const std::string& protocol, const std::string& name, socket_type* socket, sc_in<bool>* clk=NULL)
  {
    if (is_simple_socket(socket)) {
      SC_REPORT_ERROR("initiator_port_adaptor", "simple sockets not supported, please use ft_initiator_socket instead");
    }

    initiator_port_adaptor* adaptor = _create_adaptor(protocol, name, new socket_wrapper<socket_type>(socket));

    assert(clk == NULL);

    socket->bind(*adaptor);

    return adaptor;
  }

  template <typename socket_type>
  static initiator_port_adaptor* create(const std::string& name, socket_type* socket, sc_in<bool>* clk=NULL)
  {
    // try to lookup protocol from the port property
    std::string protocol;
      protocol = "TLM2_GP";

    initiator_port_adaptor* result = create(protocol, name, socket, clk);
    return result;
  }

  template <typename socket_type>
  static initiator_port_adaptor* create(socket_type* socket, sc_in<bool>* clk=NULL)
  {
    return create(sc_gen_unique_name("adaptor"), socket, clk);
  }

  ~initiator_port_adaptor() { }

  /** Clock interface */
  virtual bool is_idle() const = 0;   /** Indicates whether there are any active transfers on the port adaptor */

  /** API to get a transaction ID not already in use by an active transaction */
  virtual unsigned int get_unique_trans_id() const = 0;

  /** DMI */
  virtual void enable_dmi() = 0;
  virtual void disable_dmi() = 0;
  virtual bool is_dmi_enabled() const = 0;

  /** API to request a transaction object */
  virtual tlm::tlm_generic_payload& alloc_and_init_trans(tlm::tlm_command cmd) = 0;

  /** API to send a transaction over the bus */
  virtual bool send_transaction(tlm::tlm_generic_payload& payload) = 0;
  virtual bool send_transaction(tlm::tlm_generic_payload& payload, sc_time& delay) = 0;
  virtual bool send_transaction(tlm::tlm_generic_payload& payload, const sc_time& delay) = 0;

  /** LT interface */
  virtual bool read(unsigned long long address, unsigned char* data, unsigned int count, sc_core::sc_time& t) = 0;
  virtual bool write(unsigned long long address, const unsigned char* data, unsigned int count, sc_core::sc_time& t) = 0;
  virtual unsigned int read_debug(unsigned long long address, unsigned char* data, unsigned int count) = 0;
  virtual unsigned int write_debug(unsigned long long address, const unsigned char* data, unsigned int count) = 0;

  template <typename DT> bool read(unsigned long long address, DT& data) {
    sc_core::sc_time t(SC_ZERO_TIME);
    bool result = read(address, dt_from_array<DT>(data), SizeOf<DT>::result, t);
    if (t != SC_ZERO_TIME) {
      wait(t);
    }
    return result;
  }
  template <typename DT> bool read(unsigned long long address, DT* data, unsigned int count) {
    sc_core::sc_time t(SC_ZERO_TIME);
    bool result = read(address, reinterpret_cast<unsigned char*>(data), count * SizeOf<DT>::result, t);
    if (t != SC_ZERO_TIME) {
      wait(t);
    }
    return result;
  }
  template <typename DT> bool read(unsigned long long address, sc_dt::sc_biguint<SizeOfBits<DT>::result>* data, unsigned int count) {
    sc_core::sc_time t(SC_ZERO_TIME);
    bool result = read(address, data, count);
    if (t != SC_ZERO_TIME) {
      wait(t);
    }
    return result;
  }
  template <typename DT> bool read(unsigned long long address, DT& data, sc_core::sc_time& t) {
    return read(address, dt_from_array<DT>(data), sizeof(DT), t);
  }
  template <typename DT> bool read(unsigned long long address, DT* data, unsigned int count, sc_core::sc_time& t) {
    return read(address, reinterpret_cast<unsigned char*>(data), count * SizeOf<DT>::result, t);
  }
  template <typename DT> bool read(unsigned long long address, sc_dt::sc_biguint<SizeOfBits<DT>::result>* data, unsigned int count, sc_core::sc_time& t) {
    std::vector<unsigned char> array(SizeOf<DT>::result * count);
    bool result = read(address, &array[0], count, t);
    if (result) {
      for (unsigned int i = 0; i < count; ++i) {
        data[i] = array_to_dt<DT>(&array[i * SizeOf<DT>::result]);
      }
    }
    return result;
  }

  template <typename DT> bool write(unsigned long long address, const DT& data) {
    sc_core::sc_time t(SC_ZERO_TIME);
    bool result = write(address, const_dt_from_array<DT>(data), SizeOf<DT>::result, t);
    if (t != SC_ZERO_TIME) {
      wait(t);
    }
    return result;
  }
  template <typename DT> bool write(unsigned long long address, const DT* data, unsigned int count) {
    sc_core::sc_time t(SC_ZERO_TIME);
    bool result = write(address, reinterpret_cast<const unsigned char*>(data), count * SizeOf<DT>::result, t);
    if (t != SC_ZERO_TIME) {
      wait(t);
    }
    return result;
  }
  template <typename DT> bool write(unsigned long long address, const sc_dt::sc_biguint<SizeOfBits<DT>::result>* data, unsigned int count) {
    sc_core::sc_time t(SC_ZERO_TIME);
    bool result = write(address, data, count, t);
    if (t != SC_ZERO_TIME) {
      wait(t);
    }
    return result;
  }
  template <typename DT> bool write(unsigned long long address, const DT& data, sc_core::sc_time& t) {
    return write(address, const_dt_from_array<DT>(data), SizeOf<DT>::result, t);
  }
  template <typename DT> bool write(unsigned long long address, const DT* data, unsigned int count, sc_core::sc_time& t) {
    return write(address, reinterpret_cast<const unsigned char*>(data), count * SizeOf<DT>::result, t);
  }
  template <typename DT> bool write(unsigned long long address, const sc_dt::sc_biguint<SizeOfBits<DT>::result>* data, unsigned int count, sc_core::sc_time& t) {
    std::vector<unsigned char> array(SizeOf<DT>::result * count);
    for (unsigned int i = 0; i < count; ++i) {
      to_char_array(data[i], &array[i * SizeOf<DT>::result], SizeOf<DT>::result, 0);
    }
    return write<SizeOf<DT>::result>(address, &array[0], count, t);
  }

  template <typename DT> bool read_debug(unsigned long long address, DT& data) {
    return read_debug(address, dt_from_array<DT>(data), SizeOf<DT>::result);
  }
  template <typename DT> bool read_debug(unsigned long long address, DT* data, unsigned int count) {
    return read_debug(address, reinterpret_cast<unsigned char*>(data), count * SizeOf<DT>::result);
  }
  template <typename DT> bool read_debug(unsigned long long address, sc_dt::sc_biguint<SizeOfBits<DT>::result>* data, unsigned int count) {
    std::vector<unsigned char> array(SizeOf<DT>::result * count);
    bool result = read_debug<SizeOf<DT>::result>(address, &array[0], count);
    if (result) {
      for (unsigned int i = 0; i < count; ++i) {
        data[i] = array_to_dt<DT>(&array[i * SizeOf<DT>::result]);
      }
    }
    return result;
  }

  template <typename DT> bool write_debug(unsigned long long address, const DT& data) {
    return write_debug(address, const_dt_from_array<DT>(data), SizeOf<DT>::result);
  }
  template <typename DT> bool write_debug(unsigned long long address, const DT* data, unsigned int count) {
    return write_debug(address, reinterpret_cast<const unsigned char*>(data), count * SizeOf<DT>::result);
  }
  template <typename DT> bool write_debug(unsigned long long address, const sc_dt::sc_biguint<SizeOfBits<DT>::result>* data, unsigned int count) {
    std::vector<unsigned char> array(SizeOf<DT>::result * count);
    for (unsigned int i = 0; i < count; ++i) {
      to_char_array(data[i], &array[i * SizeOf<DT>::result], SizeOf<DT>::result, 0);
    }
    return write_debug<SizeOf<DT>::result>(address, &array[0], count);
  }
};

} // namespace scml2

#endif  //__INITIATOR_PORT_ADAPTOR_H__
