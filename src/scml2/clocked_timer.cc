// -*- C++ -*-
/***************************************************************************
 * Copyright 1996-2025 Synopsys, Inc.
 *
 * This Synopsys software and all associated documentation are proprietary
 * to Synopsys, Inc. and may only be used pursuant to the terms and
 * conditions of a written license agreement with Synopsys, Inc.
 * All other use, reproduction, modification, or distribution of the
 * Synopsys software or the associated documentation is strictly prohibited.
 ***************************************************************************/

#include "clocked_timer.h"
#include <systemc>
#include <assert.h>

const char* const scml2::clocked_timer::gStateString[] = {
   "Idle", "Running", "Stopped"
};
const char* const scml2::clocked_timer::gEventString[] = {
   "Start", "Expired", "Stop", "ResumeStopped", "Delete"
};

scml2::clocked_timer::clocked_timer(const std::string& name, scml_clock_if* clock)
: sc_core::sc_object(name.c_str()), clocked_module(clock), mCallback(0),
  mState(eS_Idle), mCallbackPending(false), mCacheTimeStamp(~0ull
   , false
  ),
#if (!defined CWR_SYSTEMC && defined SYSTEMC_VERSION && SYSTEMC_VERSION <= 20221128 /*2.3.4*/)
  mOwner(sc_core::sc_get_curr_simcontext()->hierarchy_curr())
#elif (defined CWR_SYSTEMC)
  mOwner(nullptr)
#else
  mOwner(nullptr)
#endif
{
#if (!defined CWR_SYSTEMC && defined SYSTEMC_VERSION && SYSTEMC_VERSION <= 20221128 /*2.3.4*/)
#elif (defined CWR_SYSTEMC)
  sc_core::sc_object_host* host = sc_core::sc_get_curr_simcontext()->hierarchy_curr();
  mOwner = dynamic_cast<sc_core::sc_module*>(host);
  assert(mOwner != nullptr || host == nullptr);
#else
  sc_core::sc_object* host = sc_core::sc_get_current_object();
  if (host != nullptr && !dynamic_cast<sc_core::sc_module*>(host)) {
    SC_REPORT_WARNING("clocked_timer", "No owner is set because no active module was found.");
    mOwner = nullptr;
  } else {
    mOwner = static_cast<sc_core::sc_module*>(host);
  }
#endif
   // coverity[uninit_member] uninitialized by intention: mTicksPerPeriod, mRemainingShotCount, mRemainingTicksWhenResuming, mCachedState, mCachedRemainingShotCount
}

scml2::clocked_timer::~clocked_timer() {
   cancel_clock_trigger();
   mState = eS_Idle;
   if (mCallback != 0) {
      mCallback->unref();
   }
}

void scml2::clocked_timer::set_callback(clocked_method_callback_base* callback) {
   if (mCallback != 0) {
      mCallback->unref();
   }
   mCallback = callback;
   if (mCallback != 0) {
      mCallback->ref();
   }
}

void scml2::clocked_timer::update_cache(bool from_callback) {
   sc_core::sc_time now = sc_core::sc_time_stamp();
   if (mCacheTimeStamp == now) {
      return;
   }
   mCacheTimeStamp = now;
   mCachedRemainingShotCount = mRemainingShotCount;
   mCachedState = mState;

   if (from_callback || (is_clock_trigger_requested() && get_scheduled_clock_tick()==get_clock()->get_tick_count())) {
      assert(mState == eS_Running);
      mCallbackPending = true;
      if (mRemainingShotCount > 0) {
	 --mRemainingShotCount;
	 if (mRemainingShotCount == 0) {
	    mState = eS_Idle;
	 }
      }
   } else {
      mCallbackPending = false;
   }
}


void scml2::clocked_timer::start(long long ticks_per_period, long long shot_count) {
   update_cache();
   mRemainingShotCount = shot_count;
   mTicksPerPeriod = ticks_per_period;
   if (mRemainingShotCount==0 || mTicksPerPeriod==0) {
      mState = eS_Idle;
   } else {
      mState = eS_Running;
   }
   if (!mCallbackPending) {
      cancel_clock_trigger();
      if (mState == eS_Running) {
	 request_clock_trigger(mTicksPerPeriod-1);
      }
   }

}

void scml2::clocked_timer::start(const sc_core::sc_time& period, long long shot_count) {
   long long ticks_per_period = period.value() / get_clock()->get_period().value();
   if (ticks_per_period*get_clock()->get_period().value() != period.value()) {
      ++ticks_per_period;
   }
   start(ticks_per_period, shot_count);
}

void scml2::clocked_timer::stop() {
   update_cache();
   if (mState == eS_Running) {
      if (mCallbackPending) {
	 mRemainingTicksWhenResuming = mTicksPerPeriod-1;
      } else {
	 mRemainingTicksWhenResuming = get_scheduled_clock_tick()-get_clock()->get_tick_count()-1;
	 assert(mRemainingTicksWhenResuming >= 0);
	 cancel_clock_trigger();
      }
      mState = eS_Stopped;
   }
}

bool scml2::clocked_timer::resume_stopped() {
   if (mState != eS_Stopped) {
      return false;
   }
   if (!mCallbackPending) {
      request_clock_trigger(mRemainingTicksWhenResuming);
   }
   mState = eS_Running;
   return true;
}

unsigned long long scml2::clocked_timer::get_counter_value() {
   update_cache();
   if (mCallbackPending || mState==eS_Idle) {
      return 0;
   }
   return mTicksPerPeriod - (mState==eS_Running? (get_scheduled_clock_tick()-get_clock()->get_tick_count()) : (mRemainingTicksWhenResuming+1));
}

bool scml2::clocked_timer::get_counter_value(const sc_core::sc_time delay, unsigned long long& value) {
   update_cache();
   if (mCallbackPending) {
      return false;
   }
   switch (mState) {
    case eS_Idle:
      value = 0;
      break;
    case eS_Running:
      {
         unsigned long long scheduled_clock_tick = get_scheduled_clock_tick();
         unsigned long long tick_count = get_clock()->get_tick_count(delay);
         if (tick_count >= scheduled_clock_tick) {
            return false;
         }
         value = mTicksPerPeriod - (scheduled_clock_tick-tick_count);
      }
      break;
    case eS_Stopped:
      value = mTicksPerPeriod - (mRemainingTicksWhenResuming+1);
      break;
   }
   return true;
}

void scml2::clocked_timer::handle_clock_tick() {
   update_cache(true);
   assert(mCallbackPending);
   if (mCallback != 0) {
      mCallback->notify();
   }
   mCallbackPending = false;
   if (mState == eS_Running) {
      request_clock_trigger(mTicksPerPeriod-1);
   }
}

const char* scml2::clocked_timer::kind() const {
   return "scml2::clocked_timer";
}


// {{{ Emacs local variables

// Local variables:
// folded-file: t
// c-file-style: "coware"
// End:

// }}}
