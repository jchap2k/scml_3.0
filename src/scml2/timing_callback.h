/***************************************************************************
 * Copyright 1996-2025 Synopsys, Inc.
 *
 * This Synopsys software and all associated documentation are proprietary
 * to Synopsys, Inc. and may only be used pursuant to the terms and
 * conditions of a written license agreement with Synopsys, Inc.
 * All other use, reproduction, modification, or distribution of the
 * Synopsys software or the associated documentation is strictly prohibited.
 ***************************************************************************/
#ifndef SCML2_TIMING_CALLBACK_H
#define SCML2_TIMING_CALLBACK_H

namespace tlm {
  class tlm_generic_payload;
}

namespace sc_core {
  class sc_time;
}

namespace scml2
{

class timing_callback_base
{
public:
  virtual tlm::tlm_sync_enum execute(tlm::tlm_generic_payload& trans, sc_time& t) = 0;
};

template <typename C>
class timing_callback0 : public timing_callback_base
{
public:
  typedef tlm::tlm_sync_enum (C::*CallbackType)(tlm::tlm_generic_payload&, sc_core::sc_time&);

  timing_callback0(C* c, CallbackType cb) : mClass(c), mCallback(cb) { }
  tlm::tlm_sync_enum execute(tlm::tlm_generic_payload& trans, sc_core::sc_time& t) { return (*mClass.*mCallback)(trans, t); }

private:
  C* mClass;
  CallbackType mCallback;
};

template <typename C, typename A>
class timing_callback1 : public timing_callback_base
{
public:
  typedef tlm::tlm_sync_enum (C::*CallbackType)(tlm::tlm_generic_payload&, sc_core::sc_time&, A);

  timing_callback1(C* c, CallbackType cb, A a) : mClass(c), mCallback(cb), mArg1(a) { }
  tlm::tlm_sync_enum execute(tlm::tlm_generic_payload& trans, sc_core::sc_time& t) { return (*mClass.*mCallback)(trans, t, mArg1); }

private:
  C* mClass;
  CallbackType mCallback;
  A mArg1;
};

template <typename C, typename A, typename B>
class timing_callback2 : public timing_callback_base
{
public:
  typedef tlm::tlm_sync_enum (C::*CallbackType)(tlm::tlm_generic_payload&, sc_core::sc_time&, A, B);

  timing_callback2(C* c, CallbackType cb, A a, B b) : mClass(c), mCallback(cb), mArg1(a), mArg2(b) { }
  tlm::tlm_sync_enum execute(tlm::tlm_generic_payload& trans, sc_core::sc_time& t) { return (*mClass.*mCallback)(trans, t, mArg1, mArg2); }

private:
  C* mClass;
  CallbackType mCallback;
  A mArg1;
  B mArg2;
};

// provide consistent interface to create timing callbacks with variable number of arguments
template <typename C>
timing_callback_base* create_timing_callback(C* c, typename timing_callback0<C>::CallbackType cb)
{
  return new timing_callback0<C>(c, cb);
}

template <typename C, typename A>
timing_callback_base* create_timing_callback(C* c, typename timing_callback1<C, A>::CallbackType cb, A a)
{
  return new timing_callback1<C, A>(c, cb, a);
}

template <typename C, typename A, typename B>
timing_callback_base* create_timing_callback(C* c, typename timing_callback2<C, A, B>::CallbackType cb, A a, B b)
{
  return new timing_callback2<C, A, B>(c, cb, a, b);
}

}

#endif
