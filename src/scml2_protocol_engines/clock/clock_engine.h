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
#include <scml_clock.h>
#include <scml2_objects.h>

#include <ostream>

namespace scml2 {

enum class clock_mode {
  ACTIVE,
  INACTIVE
};

inline std::ostream& operator<<(std::ostream& os, clock_mode cm) {
  os << static_cast<int>(cm);
  return os;
}

class clock_master_engine;
class clock_slave_engine;
class clock_master_engine_player;
class clock_slave_engine_player;

IO_TRACER_DECL_SCML_FULL(clock_master_engine, scml2::objects::basic_recorder<clock_master_engine>,clock_master_engine_player, "SCML_CLOCK_MASTER", "SCML_CLOCK_SLAVE",1,1);
IO_TRACER_DECL_SCML_FULL(clock_slave_engine, scml2::objects::basic_recorder<clock_slave_engine>,clock_slave_engine_player, "SCML_CLOCK_SLAVE", "SCML_CLOCK_MASTER",1,1);

// CLOCK engines
//
class clock_master_engine : public scml2::base::object {
  void check_mode(const std::string& api) {
    if (this->m_mode == clock_mode::INACTIVE) {
      SCML2_ERROR(FATAL_ERROR) << "master clock protocol engine '" << this->get_name() << "' configured for clock_mode::INACTIVE mode, API '" << api << "' not available." << std::endl;
    }
  }	
  
public:
	clock_master_engine(const scml2::base::object_name& name, sc_core::sc_export<sc_core::sc_signal_inout_if<bool> > & clock_output)
	  : scml2::base::object(name)
	  , m_output(clock_output)
	  , m_rec(nullptr)
	  , m_mode("mode")
	  , m_period("period")
	  , m_enabled("enabled")
	  , m_clock_master( sc_gen_unique_name((name.c_str() + std::string("_internal")).c_str()), sc_core::sc_time(10, SC_NS))
	{
		m_period.set_getter(this, &clock_master_engine::get_period);
		m_period.set_setter(this, &clock_master_engine::set_period);
		m_enabled.set_getter(this, &clock_master_engine::is_enabled);
		m_enabled.set_setter(this, &clock_master_engine::enable);
		m_stored_period = sc_core::sc_time(10, SC_NS);
		m_clock_master.set_period(m_stored_period);
		m_stored_enable = true;
		m_clock_master.enable();

		sc_core::sc_spawn_options opts;
		opts.spawn_method();
		opts.set_sensitivity(&m_delay_update);
		opts.dont_initialize();
		m_update_method_handle = sc_core::sc_spawn
				  (sc_bind(&clock_master_engine::update_clock, this),
						  sc_core::sc_gen_unique_name("update_clock"), &opts);

		ADD_IO_TRACE_TYPE(clock_master_engine);
	}

	bool is_enabled(){
	  check_mode("is_enabled");
	  return (m_clock_master.disabled() == false);
	}

	bool is_running(){
	  check_mode("is_running");
	  return (m_clock_master.running());
	}

	void enable(bool enabled = true) {
		check_mode("set_enabled" + std::string( enabled ? "(true)" : "(false)"));
		// extra handling for case where enable gets set from another clock object update
		// clocks cannot handle enable updates during the update phase, so we need to push them out to the next delta-cycle
		if (enabled == m_stored_enable) {
			return;
		}
		m_stored_enable = enabled;
		if (sc_get_curr_simcontext()->update_phase()) {
			m_delay_update.notify(SC_ZERO_TIME);
		} else {
			if (m_stored_enable) {
				m_clock_master.enable();
			} else {
				m_clock_master.disable();
			}
		}
	}

	void disable(){
	  check_mode("set_disabled");
	  enable(false);
	}

	void set_recorder(scml2::objects::basic_recorder<clock_master_engine>* _rec) {
		if (_rec == nullptr) return;
		m_rec = _rec;
		m_rec->set_obj_name(this->get_port_name());
		// also record initial value (in case static clock is used)
		sc_core::sc_time p = m_clock_master.get_period();
		if (m_rec) m_rec->record(p.to_string());

	}

	// used by io_trace
	std::string get_port_name() { return m_output.basename(); }

private:
	void finalize_local_construction() override {
	  switch (m_mode) {
	  case clock_mode::ACTIVE:
	    m_output(m_clock_master);	    
	    break;
	  case clock_mode::INACTIVE:
	    break;
	  }
	}

	void set_period(sc_core::sc_time p){
	  check_mode("set_period");
	  // extra handling for case where period gets set from another clock object update
	  // clocks cannot handle period updates during the update phase, so we need to push them out to the next delta-cycle
	  if (p == m_stored_period) {
		  return;
	  }
	  m_stored_period = p;
	  if (sc_get_curr_simcontext()->update_phase()) {
		  m_delay_update.notify(SC_ZERO_TIME);
	  } else {
		m_clock_master.set_period(p);
		if (m_rec) m_rec->record(p.to_string());
	  }
	}

	sc_core::sc_time get_period() {
	  check_mode("get_period");
		return m_clock_master.get_period();
	}

	void update_clock() {
		m_clock_master.set_period(m_stored_period);
		if (m_stored_enable) {
			m_clock_master.enable();
		} else {
			m_clock_master.disable();
		}
	}

	sc_core::sc_export<sc_core::sc_signal_inout_if<bool> >& m_output;
	sc_core::sc_event m_delay_update;
	sc_core::sc_process_handle m_update_method_handle;
	sc_core::sc_time m_stored_period;
	bool m_stored_enable;
	scml2::objects::basic_recorder<clock_master_engine>* m_rec;
public:
	scml2::base::const_attribute<clock_mode> m_mode;
	scml2::base::attribute<sc_core::sc_time> m_period;
	scml2::base::attribute<bool > m_enabled;
public:
	scml_clock m_clock_master;
};

class clock_slave_engine : public scml2::base::object , public scml_clock_observer {
public:
	clock_slave_engine(const scml2::base::object_name& name, sc_in<bool> & clock_input)
		: scml2::base::object(name)
		, enable("enable", this, &clock_slave_engine::enable_impl)
		, disable("disable", this, &clock_slave_engine::disable_impl)
		, divider("divider")
		, ticking("ticking")
		, enabled("enabled")
		, period("period")
		, clock_if("clock_if")
		, m_input(clock_input)
			, m_clock_proxy(sc_core::sc_gen_unique_name((name.c_str() + std::string("clock_proxy")).c_str()), clock_input, 1u, 1u)
		, m_previous_period(SC_ZERO_TIME)
		, m_was_running(false)
	  	, m_rec(nullptr)
	{
		divider.set_setter(this, &clock_slave_engine::set_divider);
		divider.set_getter(this, &clock_slave_engine::get_divider);
		enabled.set_setter(this, &clock_slave_engine::set_enabled);
		enabled.set_getter(this, &clock_slave_engine::get_enabled);
		m_clock_proxy.register_observer(this);
		period = sc_core::sc_time(10, SC_NS); // set default value to avoid that continuous assignments into other clocks lead to period = 0 errors
		clock_if = &m_clock_proxy;

		ADD_IO_TRACE_TYPE(clock_slave_engine);
	}

	~clock_slave_engine() {
	  m_clock_proxy.unregister_observer(this);
	}

	bool is_enabled() const {
		return !(m_clock_proxy.disabled());
	}

	bool is_running() const {
		return m_clock_proxy.running();
	}

	sc_core::sc_time get_period() const {
		return m_clock_proxy.get_period();
	}

	void disable_impl() {
		enabled = false;
	}

	void enable_impl() {
		enabled = true;
	}

	void set_recorder(scml2::objects::basic_recorder<clock_slave_engine>* _rec) {
		if (_rec == nullptr) return;
		m_rec = _rec;
		m_rec->set_obj_name(this->get_port_name());
		// also record initial value (in case static clock is used)
		sc_core::sc_time p = m_clock_proxy.get_period();
		if (m_rec) m_rec->record(p.to_string());
	}

	// used by io_trace
	std::string get_port_name() { return m_input.basename(); }

private:
	void set_enabled(bool value) {
	  if (value) {
	    m_clock_proxy.enable();
	  } else {
	    m_clock_proxy.disable();
	  }
	}

	bool get_enabled() {
	  return !m_clock_proxy.disabled();
	}
	
	void set_divider(unsigned int value) 
	{ 	
	    if (value == 0) {
		SCML2_MODEL_INTERNAL(LEVEL3) << "Cannot set divider value 0 hence setting divider equals to 1" << endl;
		m_clock_proxy.set_divider(1);
	    } else {
		m_clock_proxy.set_divider(value);
	    }
	} 

	unsigned int get_divider() { return m_clock_proxy.get_divider(); }


	virtual void handle_clock_deleted(scml_clock_if* clock_if){
                (void)clock_if;
	}

	virtual void handle_clock_parameters_updated(scml_clock_if* clock_if) {
                (void)clock_if;
		//check if period changed and notify callback
		sc_core::sc_time p = m_clock_proxy.get_period();
		if (m_rec) m_rec->record(p.to_string());
		bool r = m_clock_proxy.running();
		if (m_previous_period!= p) {
			period = p;
			//only invoke period updated callback while running.
			if (r == true){
				period_updated(p);
			}
			m_previous_period = p;
		}

		//running changed
		if (m_was_running != r ) {
			if (r) {
				//notify started callback.
				started(p);
				ticking = true;
			} else {
				stopped();
				ticking = false;
			}
			m_was_running = r;
		}
	}

public:

	//callbacks
	SCML2_BASE_CALLBACK_IGNORE(period_updated, void, sc_core::sc_time);
	SCML2_BASE_CALLBACK_IGNORE(started, void, sc_core::sc_time);
	SCML2_BASE_CALLBACK_IGNORE(stopped, void);

	//behaviors
	scml2::base::behavior<void> enable;
	scml2::base::behavior<void> disable;

	//attributes
    scml2::base::attribute<unsigned int> divider;
	scml2::base::protected_value_attribute<clock_slave_engine,bool> ticking;
	scml2::base::attribute<bool> enabled;
	scml2::base::protected_value_attribute<clock_slave_engine,sc_core::sc_time> period;
    scml2::base::protected_value_attribute<clock_slave_engine,scml_clock*> clock_if;


private:
	const sc_in<bool> & m_input;
	scml_divided_clock m_clock_proxy;

	sc_core::sc_time m_previous_period;
	bool m_was_running;

	scml2::objects::basic_recorder<clock_slave_engine>* m_rec;
};

// IO_TRACE players for clock protocol engines
//
class clock_master_engine_player : public scml2::objects::player_base<clock_master_engine, scml2::base::object> {
public:
	clock_master_engine_player(clock_master_engine *_obj) : scml2::objects::player_base<clock_master_engine, scml2::base::object>(_obj) {
		this->set_obj_name(this->m_obj->get_port_name());
	}
	~clock_master_engine_player() {};

	bool play(std::vector<std::string> vec) override {
		sc_core::sc_time v;
		try {
			v = scml2::objects::string_to_val<sc_core::sc_time>::convertVal(vec[0]);
		} catch(const std::exception&e ) {
			SCML2_INFO_TO(this->m_mod, TEST_INFO) << "parsing problem(" << e.what() << "), could not convert input to sc_time for " << this->m_obj->get_name() << endl;
			return false;
		}
		if (v != SC_ZERO_TIME) {
			this->m_obj->m_period = v; // setting period to 0 results in an error, even though we record that....
		}
		return true;
	}
};

class clock_slave_engine_player : public scml2::objects::player_base<clock_slave_engine, scml2::base::object> {
public:
	clock_slave_engine_player(clock_slave_engine *_obj) : scml2::objects::player_base<clock_slave_engine, scml2::base::object>(_obj) {
		this->set_obj_name(this->m_obj->get_port_name());
	}
	~clock_slave_engine_player() {};

	bool play(std::vector<std::string> vec) override {
		sc_core::sc_time v;
		try {
			v = scml2::objects::string_to_val<sc_core::sc_time>::convertVal(vec[0]);
		} catch(const std::exception&e ) {
			SCML2_INFO_TO(this->m_mod, TEST_INFO) << "parsing problem(" << e.what() << "), could not convert input to sc_time for " << this->m_obj->get_name() << endl;
			return false;
		}
		sc_core::sc_time c = m_obj->period;
		return (c == v);
	}
};

}



