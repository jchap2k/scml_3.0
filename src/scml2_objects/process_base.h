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



#include <scml2_base.h>
#include <scml2.h>
#include "event.h"

namespace scml2 { 
namespace objects { 


enum process_type {
	THREAD = 0,
	METHOD = 1,
	TLM_CALLBACK = 2
};


class sensitivity_type {
public:

	sensitivity_type(const std::string& name)
		: m_name(name)
		, trigger("trigger")
		, condition("condition")
		, delay("delay")
		, m_immediate_delay(true)
	{
		delay.set_setter(this, &sensitivity_type::set_delay);
		delay.set_getter(this, &sensitivity_type::get_delay);
		trigger.set_post_set_callback(SCML2_CALLBACK(trigger_main));
		condition.set_post_set_callback(SCML2_CALLBACK(update_condition));
	}

	bool is_immediate() { return m_immediate_delay; };

private:
	std::string m_name;

public:
	sc_core::sc_event event;

	// attributes
	scml2::base::value_attribute<scml2::base::trigger> trigger;
	scml2::base::value_attribute<bool> condition;
	scml2::base::attribute<sc_core::sc_time> delay;

	// callbacks
	SCML2_BASE_CALLBACK(main, void);

private:
	void set_delay(sc_core::sc_time value) { m_delay = value; m_immediate_delay = false; };
	sc_core::sc_time get_delay() { if (m_immediate_delay) return sc_core::SC_ZERO_TIME; else return m_delay; };

	void trigger_main() { main(); };
	void update_condition() { if (condition) main(); };

private:
	sc_core::sc_time m_delay;
	bool m_immediate_delay;
};


class process_advanced_options {
public:
	process_advanced_options()
		: initialize("initialize")
		, stack_size("stack_size")
	{
	}

	scml2::base::const_attribute<bool> initialize;
	scml2::base::const_attribute<unsigned int> stack_size;
	std::vector<sc_core::sc_event*> systemc_events;
};

class process_base : public scml2::base::object {
public:

	process_base(const scml2::base::object_name& name)
		: scml2::base::object(name)
		, type("type")
		, sensitivity("sensitivity")
		, reset("reset", this, &process_base::reset_impl)
		, suspend("suspend", this, &process_base::suspend_impl)
		, resume("resume", this, &process_base::resume_impl)
		, add_sensitivity("add_sensitivity", this, &process_base::add_sensitivity_impl)
	    , execute("execute", this, &process_base::execute_impl)
	{
	}

	// attributes
	scml2::base::const_attribute< process_type > type;
	scml2::base::vector<scml2::objects::sensitivity_type > sensitivity;
	scml2::objects::process_advanced_options advanced_options;


	// callback definitions
	SCML2_BASE_CALLBACK(main, void);
	SCML2_BASE_CALLBACK_IGNORE(init, void);
	SCML2_BASE_CALLBACK_IGNORE(exit, void);

	// behaviors
	scml2::base::behavior<void> reset;
	scml2::base::behavior<void> suspend;
	scml2::base::behavior<void> resume;
	scml2::base::behavior<void, sc_core::sc_event &> add_sensitivity;
	scml2::base::behavior<void> execute;

private:

	virtual void reset_impl() = 0;
	virtual void suspend_impl() = 0;
	virtual void resume_impl() = 0;
	virtual void add_sensitivity_impl(sc_core::sc_event& e) = 0;
	virtual void execute_impl() = 0;
	
};


} // objects
} // scml2

