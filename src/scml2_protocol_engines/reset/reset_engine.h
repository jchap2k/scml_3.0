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

#define SNPS_SLS_VP_BASE

#include <systemc>
#include <scml2_base.h>
#include <scml2.h>

namespace scml2 {

class reset_slave_engine : public scml2::base::object
{
private :

public:
	enum ResetState {BEFORE_RESET=0, DURING_RESET=1, AFTER_RESET=2};

	reset_slave_engine(const scml2::base::object_name& name, sc_in<bool>& reset_input)
	: scml2::base::object(name)
	, active_level("active_level")
	, m_reset_input(reset_input)
	, m_state(BEFORE_RESET)

	{
		scml2::set_change_callback(reset_input, SCML2_CALLBACK(reset_change));
	}


	void reset_change() {
		if (active_level == m_reset_input){
			m_state = DURING_RESET;
			reset_enter();
		} else {
			m_state = AFTER_RESET;
			reset_exit();
		}
	}

	//callback definitions
	SCML2_BASE_CALLBACK_IGNORE(reset_enter, void);
	SCML2_BASE_CALLBACK_IGNORE(reset_exit, void);

	//attributes
	scml2::base::const_attribute<bool> active_level;

	//behaviors

private:

	const sc_in<bool>& m_reset_input;
	ResetState m_state;
};

enum reset_engine_mode {
    DURATION,
    DIRECT,
};

class reset_master_engine : public scml2::base::object
{
public:
	reset_master_engine(const scml2::base::object_name& name, sc_out<bool> & reset_output) :
	  scml2::base::object(name)
	, m_do_start_pulse()
	, m_reset_pulse_end()
	, m_output(reset_output)
	, mode("mode")
	, active_level("active_level")
	, duration("duration")
	, enabled("enabled")
	, enable_debug_trigger("enable_debug_trigger")
    , start_reset_pulse("start_reset_pulse", this, &reset_master_engine::start_reset_pulse_impl)
	, do_reset_pulse("do_reset_pulse", this, &reset_master_engine::do_reset_pulse_impl)
	{
		enable_debug_trigger = false; // force default value
		enabled.set_setter(this, &reset_master_engine::enabled_setter);
		enabled.set_getter(this, &reset_master_engine::enabled_getter);
	}

	void start_reset_pulse_impl() {
	  if (mode == DURATION) {
	    m_do_start_pulse.notify();
	  } else {
	    SCML2_ERROR(FATAL_ERROR) << "reset master protocol engine '" << this->get_name() << "' configured for DIRECT mode, behavior 'start_reset_pulse' not available." << std::endl;
	  }
	}

	void do_reset_pulse_impl() {
	  if (mode == DURATION) {
	    pulse_output();
	  } else {
	    SCML2_ERROR(FATAL_ERROR) << "reset master protocol engine '" << this->get_name() << "' configured for DIRECT mode, behavior 'do_reset_pulse' not available." << std::endl;
	  }
	}

	int dbg_trigger_reset(const std::string& arg) {
		(void) arg; // no args implemented
		if (enabled) {
			SCML2_INFO(GENERIC_INFO) << "reset already enabled, debugger action ignored" << endl;
			return 1;
		}
		if (mode == DURATION) {
			m_do_start_pulse.notify(); // trigger event (from debugger process!)
		} else {
			enabled = true; // forward handling to enabled_setter
		}
		return 0;
	}

protected:
	virtual void finalize_local_construction() {
	  if (!mode.get_initialized()) {
	    mode = DURATION;
	  }
	  if (!active_level.get_initialized()) {
	    active_level = false;
	  }
	  if (mode == DURATION) {
	    sc_core::sc_spawn_options spawn_opts;
	    spawn_opts.set_sensitivity(&m_do_start_pulse);
	    sc_core::sc_spawn(sc_bind(&reset_master_engine::pulse, this), sc_core::sc_gen_unique_name((get_basename() + "_pulse").c_str()), &spawn_opts);
	    m_output.initialize(!active_level.get());
	  } else {
	    // the 'enabled' setter is initializing the reset pin
	  }
	  if (enable_debug_trigger) {
		  // only register master in case it is a true master
		  scml2::register_reset_trigger(this->get_name(), this, &reset_master_engine::dbg_trigger_reset, "dbg_trigger_reset");
	  }
	}
	
	void enabled_setter(bool value) {
	  if (mode == DIRECT) {
	    if (sc_core::sc_is_running()) {
	      m_output.write(value == active_level.get());
	    } else {
	      m_output.initialize(value == active_level.get());
	    }
	  } else {
	    SCML2_ERROR(FATAL_ERROR) << "reset master protocol engine '" << this->get_name() << "' configured for DURATION mode, writing to 'enabled' attribute not allowed." << std::endl;
	  }
	}

	bool enabled_getter() {
	  return m_output.read() == active_level.get();
	}
	
	void pulse() {
	  while (true) {
	    wait();
	    pulse_output();
	  }
	}

	void pulse_output() {
	  assert(mode == DURATION);
	  m_output = active_level;
	  wait(duration);
	  m_output = !active_level;
	  m_reset_pulse_end.notify();
	}

	sc_core::sc_event m_do_start_pulse;
	sc_core::sc_event m_reset_pulse_end;
	sc_out<bool> & m_output;

public:
	//attributes
	scml2::base::const_attribute<reset_engine_mode> mode;
	scml2::base::const_attribute<bool> active_level;
	scml2::base::value_attribute<sc_core::sc_time> duration;
	scml2::base::attribute<bool> enabled;
	scml2::base::value_attribute<bool> enable_debug_trigger;

	//behaviors
	scml2::base::behavior<void> start_reset_pulse;
	scml2::base::behavior<void> do_reset_pulse;
};

} //namespace scml2



