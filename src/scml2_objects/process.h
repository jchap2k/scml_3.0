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



#include "process_base.h"
#include <tlm_utils/tlm_quantumkeeper.h>

namespace scml2 { 
namespace objects { 

// convenience wrapper around quantumkeeper for use in process reuse object
class local_quantum : public tlm_utils::tlm_quantumkeeper {
public:
	local_quantum(){ enabled = false;}

	// guard original tlm_quantumkeeper API's to make sure they are only used in a THREAD
	void inc(const sc_core::sc_time& t) { if (enabled) tlm_utils::tlm_quantumkeeper::inc(t); }
	void set(const sc_core::sc_time& t) { if (enabled) tlm_utils::tlm_quantumkeeper::set(t); }
	bool need_sync() const { if (enabled) return tlm_utils::tlm_quantumkeeper::need_sync(); else return false; }
	void sync() { if (enabled) tlm_utils::tlm_quantumkeeper::sync(); }
	void set_and_sync(const sc_core::sc_time& t) { if (enabled) tlm_utils::tlm_quantumkeeper::set_and_sync(t); }
	void reset() { if (enabled) tlm_utils::tlm_quantumkeeper::reset(); }
	sc_core::sc_time get_current_time() const {
		if (enabled) {
			return tlm_utils::tlm_quantumkeeper::get_current_time();
		} else {
			return sc_core::sc_time_stamp();
		}
	}
	sc_core::sc_time get_local_time() const {
		if (enabled) {
			return tlm_utils::tlm_quantumkeeper::get_local_time();
		} else {
			return sc_core::SC_ZERO_TIME;
		}
	}

	// EXTRA API's to allow to use localtime in expressions
	operator sc_core::sc_time() const { return get_local_time(); }
	local_quantum& operator=(sc_core::sc_time _value) { set_and_sync(_value); return *this; }
	local_quantum& operator+=(sc_core::sc_time _value) { inc_and_sync(_value); return *this; }

	void inc_and_sync(sc_core::sc_time value) { inc(value); if (need_sync()) sync(); }


	// EXTRA API's to allow to use local time and 'wait' intermixed
	sc_core::sc_time& get() { return m_local_time; } // allow to operate on internal local time directly
	// enables to use process RO without quantum
	void update() {
		if (need_sync()) sync();
	}
	void update_and_sync() {
		sync();
	}
	// enable to use custom synchronization (wait) in combination with quantum
	// wait shortcuts for the usual 'sync_and_wait' API sequence required when using quantum in combination with sysc wait's
	void wait() { sync(); sc_core::wait(sc_core::sc_get_curr_simcontext());}
    void wait( const sc_event& e ) { sync(); ::sc_core::wait( e, sc_core::sc_get_curr_simcontext() ); }
    void wait( const sc_event_or_list& el ) { sync(); ::sc_core::wait( el, sc_core::sc_get_curr_simcontext() ); }
    void wait( const sc_event_and_list& el ){ sync(); ::sc_core::wait( el, sc_core::sc_get_curr_simcontext() ); }
    void wait( const sc_time& t ){ inc_and_sync(t); }
    void wait( double v, sc_time_unit tu ){ inc_and_sync(sc_core::sc_time( v, tu)); }
    void wait( const sc_time& t, const sc_event& e ){ sync(); ::sc_core::wait( t, e, sc_core::sc_get_curr_simcontext() ); }
    void wait( double v, sc_time_unit tu, const sc_event& e ) { sync(); ::sc_core::wait(sc_time( v, tu ), e, sc_core::sc_get_curr_simcontext() ); }
    void wait( const sc_time& t, const sc_event_or_list& el ) { sync(); ::sc_core::wait( t, el, sc_core::sc_get_curr_simcontext() ); }
    void wait( double v, sc_time_unit tu, const sc_event_or_list& el ){ sync(); ::sc_core::wait( sc_core::sc_time( v, tu ), el, sc_core::sc_get_curr_simcontext() ); }
    void wait( const sc_time& t, const sc_event_and_list& el ){ sync(); ::sc_core::wait( t, el, sc_core::sc_get_curr_simcontext() ); }
    void wait( double v, sc_time_unit tu, const sc_event_and_list& el ) { sync(); ::sc_core::wait( sc_core::sc_time( v, tu ), el, sc_core::sc_get_curr_simcontext() ); }


public:
	bool enabled;
};


class process : public process_base {
public:

	
	process(const scml2::base::object_name& name)
		: process_base(name)
	    , m_finalized(false)
		, m_initialized(false)
	{
	     this->sensitivity.set_resized_callback(SCML2_CALLBACK(resize_sensitivity));
	}

	void finalize_local_construction() 
	{
		std::string name = this->get_name();
		name = name.substr(name.rfind(".")+1);
		if (type != TLM_CALLBACK) {
			if (advanced_options.systemc_events.size() > 0) {
				for (unsigned int i = 0; i < advanced_options.systemc_events.size(); i++) {
					m_event_list |= *(advanced_options.systemc_events[i]);
				}
			}
			if (advanced_options.stack_size.get_initialized()) { opt.set_stack_size(advanced_options.stack_size); }
			if(type == METHOD) {
				opt.spawn_method();
				h = sc_core::sc_spawn( sc_bind(&process::execute_method, this), sc_gen_unique_name(name.c_str()), &opt );
			} else if (type == THREAD) {
				h = sc_core::sc_spawn( sc_bind(&process::execute_thread, this), sc_gen_unique_name(name.c_str()), &opt );
				localtime.enabled = true;
			}
		} 
		m_finalized = true;
	}


public:
	local_quantum localtime;

private:

	sc_core::sc_process_handle h;
	sc_core::sc_spawn_options opt;
	sc_core::sc_event_or_list m_event_list;

	bool get_sensitive() {
	  return (type != TLM_CALLBACK) && (m_event_list.size() != 0 );
	}
	
	bool m_finalized; // indicates finalize_local_construction is done, used to prevent TLM_CALLBACK is called during construction
	bool m_initialized; // inidicates initialize of the SystemC process is done (processes are started with initialize on so get called during initialization

	void execute_method() {
	  if (get_sensitive()) {
			sc_core::next_trigger(m_event_list);
	  }
	  if (m_initialized) {
		  main();
	  } else {
		  if (advanced_options.initialize) main();
		  m_initialized = true;
	  }
	}
	
	void execute_thread() {
		init();
		while(true) {
		  if (get_sensitive()) {
		    wait(m_event_list);
		  }
		  localtime.reset();
		  main();
		  localtime.update_and_sync();
		}
		exit();
	}
	
	void resize_sensitivity( size_t old, size_t size) {
	  for (size_t i = old; i < size; i++) {
	    m_event_list |= sensitivity[i].event;
	    sensitivity[i].set_main_callback_tagged(SCML2_CALLBACK(trigger_main), (int)i);
	  }
	}
	
	void trigger_main(int i) {
		if (!m_finalized) return;
		if (type == TLM_CALLBACK) {
			main();
		} else { // type == THREAD or type == METHOD
			if (sensitivity[i].is_immediate()) {
				sensitivity[i].event.notify();
			} else { 
				sensitivity[i].event.notify(sensitivity[i].delay);
			}
		}
	}
	
	void reset_impl() { if(type == THREAD) { h.reset(); } };
	void suspend_impl() { if(type == THREAD) { h.suspend(); } };
	void resume_impl() { if(type == THREAD) { h.resume(); } };
	void add_sensitivity_impl(sc_core::sc_event& e) { m_event_list |= e; };
	void execute_impl() { main(); };

};



} // objects
} // scml2

