/***************************************************************************
 * Copyright 1996-2025 Synopsys, Inc.
 *
 * This Synopsys software and all associated documentation are proprietary
 * to Synopsys, Inc. and may only be used pursuant to the terms and
 * conditions of a written license agreement with Synopsys, Inc.
 * All other use, reproduction, modification, or distribution of the
 * Synopsys software or the associated documentation is strictly prohibited.
 ***************************************************************************/
#ifndef __PORT_ADAPTOR_H__
#define __PORT_ADAPTOR_H__

#include <scml_clock/scml_clock.h>
#include <scml2_tlm2/snps_tlm2_extensions/snps_tlm2_generic_payload.h>
#include <scml2_tlm2/snps_tlm2_extensions/snps_tlm2_extensions.h>
#include <scml2_tlm2/snps_tlm2_utils/ft_initiator_socket.h>
#include <scml2_tlm2/snps_tlm2_utils/ft_target_socket.h>
#include <scml2_tlm2/snps_tlm2_utils/simple_initiator_socket.h>
#include <scml2_tlm2/snps_tlm2_utils/simple_target_socket.h>
#include <scml2/timing_callback.h>

namespace scml2
{

/** Enumeration for events in transaction life cycle */
typedef enum {
  CBK_EVENT_NONE = 0,

  WR_ADDR_START,
  WR_ADDR_END,

  WR_DATA_START,
  WR_DATA_END,

  WR_DATA_LAST_START,
  WR_DATA_LAST_END,

  WR_RSP_START,
  WR_RSP_END,

  RD_ADDR_START,
  SNOOP_ADDR_START = RD_ADDR_START,

  RD_ADDR_END,

  RD_DATA_START,
  RD_DATA_END,

  RD_DATA_LAST_START,
  SNOOP_RSP_START = RD_DATA_LAST_START,
  RD_DATA_LAST_END,


  BEFORE_TRANS_START,
  AFTER_TRANS_END,

  MAX_CBK_EVENT_ID_E
} callback_event_enum;

/** Extension indicating the index of the current beat. Only valid for data phase events */
DECLARE_EXTENSION(beat_index_extension, unsigned int, 0);

/** Macros to simplify timing and clock changed callbacks registration */
#define SCML2_SET_TIMING_CBK(adaptor, event_id, obj, func) \
  (adaptor)->set_timing_callback(event_id, scml2::create_timing_callback(obj, func))

#define SCML2_SET_TIMING_CBK_WITH_ARG(adaptor, event_id, obj, func, arg) \
  (adaptor)->set_timing_callback(event_id, scml2::create_timing_callback(obj, func, arg))

// old style macros without the '&' on func. Doesn't work with templated arguments
#define SCML2_REGISTER_TIMING_CBK(adaptor, event_id, obj, func) \
  (adaptor)->set_timing_callback(event_id, scml2::create_timing_callback(obj, &func))

#define SCML2_REGISTER_TIMING_CBK_WITH_ARG(adaptor, event_id, obj, func, arg) \
  (adaptor)->set_timing_callback(event_id, scml2::create_timing_callback(obj, &func, arg))


template <typename T> inline bool is_simple_socket(T*) { return false; }
template <typename M, unsigned int BUSWIDTH> inline bool is_simple_socket(tlm_utils::simple_initiator_socket<M, BUSWIDTH>*) { return true; }
template <typename M, unsigned int BUSWIDTH> inline bool is_simple_socket(tlm_utils::simple_target_socket<M, BUSWIDTH>*) { return true; }
template <typename M, unsigned int BUSWIDTH> inline bool is_simple_socket(scml2::simple_initiator_socket<M, BUSWIDTH>*) { return true; }
template <typename M, unsigned int BUSWIDTH> inline bool is_simple_socket(scml2::simple_target_socket<M, BUSWIDTH>*) { return true; }

/*****************************************************************************
 * Name: port_adaptor
 *
 * Description: Base class to provide callback based semantics for implementing
 * initiators and targets. This is a generic utility class that can be used and
 * leveraged by various port_adaptors irrespective of the communication
 * interface implemented by them whether LT/AT or CA.
 *
 * Each port_adaptor is expected to derive from this class in order to use the
 * given interface.
 ****************************************************************************/
class SNPS_VP_API port_adaptor : public sc_core::sc_module
{
public:

  // values for invoke_timing_cbks attribute
  enum EInvokeTimingCallbacks
  {
    eInvokeCbksForFT = -1,
    eNeverInvokeCbks = 0,
    eAlwaysInvokeCbks = 1
  };


public:
  port_adaptor(const std::string& name) : sc_module(sc_module_name(name.c_str())) {}
  virtual ~port_adaptor() {}

  virtual std::string get_name() const = 0;
  virtual std::string get_socket_name() const = 0;


  /* Port adaptor protocol details */
  virtual std::string get_protocol() const = 0;
  virtual bool has_data_phase() const = 0;
  virtual bool has_wr_rsp() const = 0;

  /** Set attributes */
  virtual void set_attribute(const std::string& name, int value) = 0;
  virtual int get_attribute(const std::string& name) = 0;

  virtual void execute_command(const std::string& cmd, void* arg1 = NULL, void* arg2 = NULL, void* arg3 = NULL) = 0;

  /** API to register a callback for the specified event in the transaction life cycle */
  virtual bool set_timing_callback(callback_event_enum event_id, timing_callback_base* cb) = 0;

  template <typename C>
  bool set_timing_callback(callback_event_enum event_id, C* c, typename timing_callback0<C>::CallbackType cb)
  {
    assert(c);
    return set_timing_callback(event_id, create_timing_callback(c, cb));
  }

  template <typename C, typename A>
  bool set_timing_callback(callback_event_enum event_id, C* c, typename timing_callback1<C,A>::CallbackType cb, A a)
  {
    assert(c);
    return set_timing_callback(event_id, create_timing_callback(c, cb, a));
  }

  template <typename C, typename A, typename B>
  bool set_timing_callback(callback_event_enum event_id, C* c, typename timing_callback2<C,A,B>::CallbackType cb, A a, B b)
  {
    assert(c);
    return set_timing_callback(event_id, create_timing_callback(c, cb, a, b));
  }

  /** Lookup callback event ID by protocol state */
  virtual scml2::callback_event_enum get_event_id(unsigned int protocol_state) const = 0;

  /** Convenience function to return if there is any callback registered for a particular event */
  virtual bool has_callback(callback_event_enum event_id) const = 0;

  /** API to advance transaction state machine */
  virtual bool update_transaction(tlm::tlm_generic_payload& trans, sc_time& t) = 0;
  virtual bool update_transaction(tlm::tlm_generic_payload& trans, sc_time& t, scml2::callback_event_enum event_id) = 0;

  virtual unsigned int get_bus_width() const = 0;
  virtual unsigned int get_burst_length(tlm::tlm_generic_payload& trans) const = 0;
  virtual unsigned int get_trans_id(tlm::tlm_generic_payload& trans) const = 0;
};

} // namespace scml2

#endif  //__PORT_ADAPTOR_H__
