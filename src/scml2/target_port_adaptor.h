/***************************************************************************
 * Copyright 1996-2025 Synopsys, Inc.
 *
 * This Synopsys software and all associated documentation are proprietary
 * to Synopsys, Inc. and may only be used pursuant to the terms and
 * conditions of a written license agreement with Synopsys, Inc.
 * All other use, reproduction, modification, or distribution of the
 * Synopsys software or the associated documentation is strictly prohibited.
 ***************************************************************************/
#ifndef __TARGET_PORT_ADAPTOR_H_
#define __TARGET_PORT_ADAPTOR_H_ 

#include <scml2/mappable_if.h>
#include <scml2/memory_select_callback.h>
#include <scml2/port_adaptor.h>

#include <scml2/tagged_message_macros.h>

namespace scml2
{

/*****************************************************************************
 * Name: target_port_adaptor
 *
 * Description: Base class for all target port adaptors.
 ****************************************************************************/
class SNPS_VP_API target_port_adaptor :
  public port_adaptor,
  public tlm::tlm_fw_transport_if<>,
  public tlm::tlm_bw_direct_mem_if
{
protected:
  /** Target socket wrapper class, used to access the templated socket
    * from the non-templated target port adaptor class. */
  class socket_if : public tlm::tlm_bw_nonblocking_transport_if<>,
                    public tlm::tlm_bw_direct_mem_if
  {
  public:
    virtual unsigned int get_bus_width() const = 0;
    virtual std::string name() const = 0;
  };

  template <typename socket_type>
  class socket_wrapper : public socket_if {
  public:
    socket_wrapper(socket_type* socket) : m_target_socket(socket) { }
    virtual std::string name() const { return m_target_socket->name(); }
    virtual unsigned int get_bus_width() const { return m_target_socket->get_bus_width(); }


    virtual void invalidate_direct_mem_ptr(unsigned long long startRange, unsigned long long endRange)
    {
      (*m_target_socket)->invalidate_direct_mem_ptr(startRange, endRange);
    }

    virtual tlm::tlm_sync_enum nb_transport_bw(tlm::tlm_generic_payload& trans, tlm::tlm_phase& phase, sc_time& t)
    {
      return (*m_target_socket)->nb_transport_bw(trans, phase, t);
    }

  public:
    socket_type* m_target_socket;
  };

protected:

  // Port adaptor is not constructed directly - use the create API instead
  target_port_adaptor(const std::string& name) : port_adaptor(name) { }

  static target_port_adaptor* _create_adaptor(const std::string& protocol, const std::string& name, socket_if* sock);

public:

  template <typename socket_type>
  static target_port_adaptor* create(const std::string& protocol, const std::string& name, socket_type* socket, sc_in<bool>* clk=NULL)
  {
    if (is_simple_socket(socket)) {
      SC_REPORT_ERROR("target_port_adaptor", "simple sockets not supported, please use ft_target_socket instead");
    }

    target_port_adaptor* adaptor = _create_adaptor(protocol, name, new socket_wrapper<socket_type>(socket));

    assert(clk == NULL);

    socket->bind(*adaptor);

    return adaptor;
  }

  template <typename socket_type>
  static target_port_adaptor* create(const std::string& name, socket_type* socket, sc_in<bool>* clk=NULL)
  {
    std::string protocol;
      protocol = "TLM2_GP";

    target_port_adaptor* result = create(protocol, name, socket, clk);
    return result;
  }

  template <typename socket_type>
  static target_port_adaptor* create(socket_type* socket, sc_in<bool>* clk=NULL)
  {
    return create(sc_gen_unique_name("adaptor"), socket, clk);
  }

  ~target_port_adaptor() {}

  /** Bind the adaptor to a memory */
  using port_adaptor::operator();
  virtual void operator()(mappable_if& destination) = 0;
  virtual void set_select_callback(memory_select_callback_base* cb) = 0;

  static bool set_target_callback(mappable_if& dest, callback_event_enum event_id, timing_callback_base* cb);

};

// API to set select and timing callbacks
template <typename C>
inline
void
set_select_callback(target_port_adaptor& adaptor, C* c, typename memory_select_callback<C>::CallbackType cb, const std::string& name)
{
  adaptor.set_select_callback(new memory_select_callback<C>(*c, cb, name));
}

template <typename C>
inline
bool set_timing_callback(mappable_if& dest, callback_event_enum event_id, C* c, typename timing_callback0<C>::CallbackType cb)
{
  assert(c);
  return target_port_adaptor::set_target_callback(dest, event_id, create_timing_callback(c, cb));
}

template <typename C, typename A>
inline
bool set_timing_callback(mappable_if& dest, callback_event_enum event_id, C* c, typename timing_callback1<C,A>::CallbackType cb, A a)
{
  assert(c);
  return target_port_adaptor::set_target_callback(dest, event_id, create_timing_callback(c, cb, a));
}

template <typename C, typename A, typename B>
inline
bool set_timing_callback(mappable_if& dest, callback_event_enum event_id, C* c, typename timing_callback2<C,A,B>::CallbackType cb, A a, B b)
{
  assert(c);
  return target_port_adaptor::set_target_callback(dest, event_id, create_timing_callback(c, cb, a, b));
}

} // namespace scml2

#endif  //__TARGET_PORT_ADAPTOR_H_
