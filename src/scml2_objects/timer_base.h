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


namespace scml2 { 
namespace objects { 

enum timer_trigger_type {
	DO_START_ONLY = 0,
	TICKS_PER_PERIOD_ONLY = 1,
	BOTH = 2
};

enum timer_count_type {
	CLOCK = 0,
	OTHER
};

class timer_configuration_options {
public:
	timer_configuration_options()
		: counting_up("counting_up")
		, shotcount_update_immediate("shotcount_update_immediate")
		, tick_update_immediate("tick_update_immediate")
		, shotcounter_write_allowed("shotcounter_write_allowed")
		, tickcounter_write_allowed("tickcounter_write_allowed")
		, tick_update_allowed("tick_update_allowed")
		, count_type("count_type")
	{
		// init values (should be the same as in .tlmclib)
		counting_up = true;
		shotcount_update_immediate = false;
		tick_update_immediate = false;
		shotcounter_write_allowed = false;
		tickcounter_write_allowed = false;
		//tick_update_allowed = true; // don't initialize since this is a const attribtue
	}

	scml2::base::value_attribute<bool> counting_up;					// whether timer counter is counting up or down
	scml2::base::value_attribute<bool> shotcount_update_immediate;	// indicates whether change in shotcount will reprogram the timer immediately or only next time it is enabled
	scml2::base::value_attribute<bool> tick_update_immediate;		// indicaes whether change in ticks_per_period will reprogram the alarm period immediately or only after the next alarm
	scml2::base::value_attribute<bool> shotcounter_write_allowed;	// indicates it is allowed to update the shotcount_counter while the timer is running
	scml2::base::value_attribute<bool> tickcounter_write_allowed;	// indicaes it is allowed to update the tickcounter while the timer is running
	scml2::base::const_attribute<bool> tick_update_allowed;			// whether it is allows to change the ticks_per_period while the timer is running
	scml2::base::attribute<timer_count_type> count_type;		// whether the timer counter increments based on clock ticks or through 'count()' API
};

class timer; // forward declaration

class timer_base : public scml2::base::object {
public:

	timer_base(const scml2::base::object_name& name)
		: scml2::base::object(name)
		, counter("counter")
		, do_start("do_start")
		, shot_count("shot_count")
		, current_shot_count("current_shot_count")
		, shot_count_counter("shot_count_counter")
		, ticks_per_period("ticks_per_period")
		, alarm_trigger("alarm_trigger")
		, clock_port("clock_port")
		, clock_divider("clock_divider")
		, trigger_style("trigger_style")
		, shotcount_trigger("shotcount_trigger")
	{
	    counter.set_setter(this, &timer_base::set_counter);
	    counter.set_getter(this, &timer_base::get_counter);
	    clock_port.set_getter(this, &timer_base::get_clock_port_impl);
	    clock_port.set_setter(this, &timer_base::set_clock_port_impl);
	    do_start.set_setter(this, &timer_base::set_start);
	    do_start.set_getter(this, &timer_base::get_start);
	    shot_count.set_setter(this, &timer_base::set_shot_count);
	    shot_count.set_getter(this, &timer_base::get_shot_count);
	    current_shot_count.set_setter(this, &timer_base::set_current_shot_count);
	    current_shot_count.set_getter(this, &timer_base::get_current_shot_count);
	    shot_count_counter.set_setter(this, &timer_base::set_shot_count_counter);
	    shot_count_counter.set_getter(this, &timer_base::get_shot_count_counter);
	    ticks_per_period.set_setter(this, &timer_base::set_ticks_per_period);
	    ticks_per_period.set_getter(this, &timer_base::get_ticks_per_period);
	    clock_divider.set_getter(this, &timer_base::get_clock_divider);
	    clock_divider.set_setter(this, &timer_base::set_clock_divider);

	    configuration_options.counting_up.set_post_set_callback(this, &timer_base::set_counting_up);
	}

	// attributes
	scml2::base::attribute<unsigned long long> counter;						// counts clock ticks up to ticks_per_period, read only access
	scml2::base::attribute<bool> do_start;									// attribute to enable/start the timer
	scml2::base::attribute<long long> shot_count;							// defines number of periods the timer should fire, allows to update shotcount while timer is running
	scml2::base::protected_attribute<::scml2::objects::timer_base, long long> current_shot_count;			// provides current set of periods the timer is running for, read only access
	scml2::base::attribute<long long> shot_count_counter;					// counts the number of alarms that have been fired, read-only access
	scml2::base::attribute<unsigned long long> ticks_per_period;			// defines the length of the timer period
	scml2::base::protected_value_attribute<::scml2::objects::timer, bool> alarm_trigger;	// bool that will be set when alarm is to be triggered, can hook into trigger attributes elsewhere
	scml2::base::attribute<sc_core::sc_in<bool>&> clock_port;				// the clock that is used for ticks per period. the clock can change frequency while the timer is running, the  timer will adjust accordingly
	scml2::base::attribute<unsigned long long> clock_divider;				// allows to divide the clock frequency
	scml2::base::value_attribute<scml2::objects::timer_trigger_type> trigger_style; // indicates whether timer is started by do_start attribute, or setting ticks_per_period or both
	scml2::base::value_attribute<unsigned long long> shotcount_trigger; 	// indicates when to trigger the intermediate shotcount_alarm next to the regular alarm, counts in shotcount increments
	scml2::objects::timer_configuration_options configuration_options;		// advanced options
	
	// behaviors
	virtual void count(unsigned long long) = 0;

	// callback definitions
	SCML2_BASE_CALLBACK_IGNORE(alarm, void);
	SCML2_BASE_CALLBACK_IGNORE(shotcount_alarm, void);

private:

	virtual void set_counter(unsigned long long) = 0;
	virtual void set_start(bool) = 0;
	virtual bool get_start() = 0;
	virtual void set_shot_count(long long) = 0;
	virtual long long get_shot_count() = 0;
	virtual void set_current_shot_count(long long) = 0;
	virtual long long get_current_shot_count() = 0;
	virtual void set_shot_count_counter(long long) = 0;
	virtual long long get_shot_count_counter() = 0;
	virtual void set_ticks_per_period(unsigned long long) = 0;
	virtual unsigned long long get_ticks_per_period() = 0;
	virtual unsigned long long get_counter() = 0;
	virtual sc_core::sc_in<bool>& get_clock_port_impl() = 0;
	virtual void set_clock_port_impl(sc_core::sc_in<bool>& value) = 0;
	virtual void set_clock_divider(unsigned long long value) = 0;
	virtual unsigned long long get_clock_divider() = 0;

	virtual void set_counting_up() = 0;
};




} // objects
} // scml2

