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


#include "systemc.h"
#include "timer_base.h"
#include "scml_clock.h"

namespace scml2 { 
namespace objects { 

class timer : public timer_base
	, public scml2::clocked_timer
{

  void set_ticks_update() {
    m_tmp_ticks_per_period = m_ticks_per_period;
    m_tmp_remaining_shot_count = clocked_timer::get_remaining_shot_count();
    m_ticks_update = true;
  }

  void reset_ticks_update() {
    m_ticks_update = false;
  }

public:

	timer(const scml2::base::object_name& name) 
		: timer_base(name)
                , scml2::clocked_timer(sc_core::sc_gen_unique_name((std::string(name.c_str()) + "_clocked_timer").c_str(), true))
		, m_clk(0)
		, m_ticks_per_period(0)
                , m_tmp_ticks_per_period(0)
                , m_tmp_remaining_shot_count(0)
                , mDividedClk(sc_core::sc_gen_unique_name((std::string(name.c_str()) + "_divided_clock").c_str(), true))
	{
		this->set_callback(this, &timer::check_alarm);

		// initialize local variables
		m_clock_set = false;
		m_cached_shotcount = 0;
		m_ticks_per_period = 0;
		m_shotcount = 0;
		m_counter = 0;
		m_alarm_ts = SC_ZERO_TIME;
		m_cached_counting_up = true;
		m_count_type = CLOCK;

		// init for manual mode
		m_manual_counter_state = STOPPED;
		m_manual_periodcount = 0;
		m_manual_counter = 0;
		m_manual_shotcount = 0;
		m_manual_shotcount_target = 0;

		setup();

		// attributes that need special care during initialization (avoid error output)
		counter.set_setter(this, &timer::initialize_counter);
	    clock_divider.set_setter(this, &timer::initialize_clock_divider);
	    ticks_per_period.set_setter(this, &timer::initialize_ticks_per_period);

	    configuration_options.count_type.set_setter(this, &timer::set_count_type);
	    configuration_options.count_type.set_getter(this, &timer::get_count_type);

	}

	void finalize_local_construction() {
		// switch to actual setters
		counter.set_setter(this, &timer::set_counter);
	    clock_divider.set_setter(this, &timer::set_clock_divider);
	    ticks_per_period.set_setter(this, &timer::set_ticks_per_period);

	    // No longer checking for m_clk setting at end of construction now that it can be set later...
	    this->set_clock(&mDividedClk); 

	}

	// override some behaviors of clocked_timer so that we can implement additional features
	void start() {
		start(m_ticks_per_period, shot_count);
	}

	

	void start(unsigned long long ticks_per_period, long long shot_count) {
		m_cached_shotcount = shot_count;
		setup();
		if (m_counter != 0 && (m_counter < ticks_per_period)) {
			// if counter was programmed while timer inactive it is used as start value for the first alarm period count
			set_ticks_update();
			local_start(ticks_per_period - m_counter, 1);
		} else {
			local_start(ticks_per_period, shot_count);
			m_counter = 0; // make sure that the counter is set to 0 in case it was larger than ticks_per_period
		}
	}

	bool is_active() {
		if (configuration_options.count_type == CLOCK) {
			return clocked_timer::is_active();
		} else {
			return (m_manual_counter_state == RUNNING);
		}
	}

	void stop() {
		if (configuration_options.count_type == CLOCK) {
			clocked_timer::stop();
		} else {
			m_manual_counter_state = STOPPED;
		}

	}

	// local overrides to ensure that when the API's from the clocked timer are used for the 'OTHER' count type they still work as expected
	unsigned long long get_counter_value() {
		if (configuration_options.count_type == CLOCK) {
			return clocked_timer::get_counter_value();
		} else {
			return m_manual_counter;
		}

	}

	bool resume_stopped() {
		if (configuration_options.count_type == CLOCK) {
			return clocked_timer::resume_stopped();
		} else {
			bool retval = (m_manual_counter_state == STOPPED);
			m_manual_counter_state = RUNNING;
			return retval;
		}

	}

	long long get_remaining_shot_count() {
		if (configuration_options.count_type == CLOCK) {
		  if (m_ticks_update) {
		    return m_tmp_remaining_shot_count;
		  } else {
		    return clocked_timer::get_remaining_shot_count();
		  }
		} else {
			if (m_manual_shotcount_target < 0) {
				return -1;
			} else {
				return (m_manual_shotcount_target - m_manual_shotcount);
			}
		}
	}

	// time based timer settings (not clock based): does not use most features of timer
	void start(const sc_core::sc_time& period) {
		start(period, shot_count);
	}
	void start(const sc_core::sc_time& period, long long shot_count) {
		if (configuration_options.count_type != CLOCK) {
			SCML2_ERROR_TO(this, SCML_INVALID_API_USAGE) << name() << ": timer count mode is manual, it is not allowed to use the start for period API \"start(const sc_core::sc_time& period, long long shot_count)\" " << endl;
		}
		m_cached_shotcount = shot_count;
		setup();
		clocked_timer::start(period, shot_count);
	}

	// API to have timer count through other means than clock
	void count(unsigned long long c) {
		if (configuration_options.count_type == CLOCK) {
			SCML2_ERROR_TO(this, FATAL_ERROR) << name() << ": timer count mode is clocked based, it is not allowed to use the count API" << endl;
			return;
		}
		if (m_manual_counter_state == STOPPED) {
			SCML2_MODEL_INTERNAL_TO(this, LEVEL6) << name() << ": timer counter is not running counter increment ignored, restart the timer before counting" << endl;
			return;
		}
		m_manual_counter += c;
		if (m_manual_counter >= m_manual_periodcount) {
			m_manual_counter = 0;
			m_manual_shotcount++;
			check_alarm();
			if (m_manual_shotcount_target > 0 && m_manual_shotcount >= (unsigned long long)m_manual_shotcount_target) {
				m_manual_counter_state = STOPPED;
			}
		}
	}


private:
	// initial setup for timer start
	void setup() {
		m_current_shotcount = 0;
		m_shotcount_for_trigger = 0;
		reset_ticks_update();
	}

	// tick counter setter and getter plus initialize
	void set_counter(unsigned long long value) {
		// check if timer is configured to allow writing to the counter value else the write will be ignored
		if (configuration_options.tickcounter_write_allowed) {
			if (is_active()) {
				// if active we will lengthen the current pulse based on timer value, or stop the timer if the counter is set to a smaller value
				if (value < ticks_per_period) {
					// run current alarm cycle with new counter value and continue with existing settings after next alarm
					set_ticks_update();
					local_start(ticks_per_period - value, 1);
					m_counter = value; // update counter value so that read/writes from counter take this write into account
				} else {
					// if programming counter to a value larger than the period the alarm will be triggered immediately and the timer will continue from here (start counting from 0)
				        set_ticks_update();
					check_alarm();
					// as part of alarm check m_counter is set to 0, value isn't stored
				}
			} else {
				m_counter = value; // if not active it will be used as start value for first period, next time timer is started
			}
		}
	}
	void initialize_counter(unsigned long long ) {} // at initialization counter value update is not allowed
	unsigned long long get_counter() {
	        unsigned long long total_count = m_counter + local_get_counter_value(); // make sure we start counting relative to the start value
		if (configuration_options.counting_up) {
			return total_count;
		} else {
		  // NOTE: here we need to use the previous ticks per period if we're in the non-immediate transitional period
		  return ((m_ticks_update ? m_tmp_ticks_per_period : m_ticks_per_period) - total_count);
		}
	};
	
	// start getter and setter
	void set_start(bool value) { if (value && trigger_style != TICKS_PER_PERIOD_ONLY) start(); };
	bool get_start() { return false; };

	// shotcount setter and getter
	void set_shot_count(long long value) {
		m_shotcount = value;
		if (configuration_options.shotcount_update_immediate) {
			// if immediate shotcount update is enabled we restat the timer with the new settings
			// only if a new shotcount is programmed!
			if (m_shotcount == m_cached_shotcount) {
				return;
			}
			if (is_active()) {
				start();
			}
		}

	};
	long long get_shot_count() { return m_shotcount; };

	// shotcount_counter setter and getter
	void set_shot_count_counter(long long value ) {
		// check if timer is configured to allow writing to the shotcount counter else the write will be ignored
		if (configuration_options.shotcounter_write_allowed) {
			// if timer is running and is programmed for more than the counter is reprogrammed for, we will continue from the new counter value
			// else the timer will be stopped.
			if (is_active()) {
				if (value <= m_cached_shotcount) {
					m_current_shotcount = value; // update shotcount counter with new value
				} else {
					stop(); // if shotcount is reprogrammed to a value smaller than already counter for, the timer is stopped
				}
			} else {
				m_current_shotcount = value; // will be used as shotcount counter start value next time timer is started
			}
		}
	};
	long long get_shot_count_counter() { return m_current_shotcount; };

	// current_shotcount setter and getter
	void set_current_shot_count(long long ) { /* current_shot_count is read-only */ };
	long long get_current_shot_count() { return m_cached_shotcount; };

	// ticks_per_period setter and getter
	unsigned long long get_ticks_per_period() { return m_ticks_per_period; };
	void initialize_ticks_per_period(unsigned long long value) {
		// during initialization none of the configuration options are checked
		// during initialization the timer isn't started
		m_ticks_per_period = value;
	}
	void set_ticks_per_period(unsigned long long value) {
		if (value == m_ticks_per_period) {
			return; // if value didn't change don't do anything
		}
		bool start_timer = false; // flag to check whether the timer needs to be started
		if (finalized() && configuration_options.tick_update_allowed ) { // extra check for finalized to prevent trying to read from const_attribute tick_update_allowed before it is initialized
			if (is_active()) { // timer is running
				if (configuration_options.tick_update_immediate || m_alarm_ts == sc_time_stamp() ) {
					// if the timer is set in a mode where tick_updates are immediate or when where ticks_per_period is updated at the same SystemC time as the alarm but after the alarm has fired
					// start timer with new ticks_per_period value for the remaining shortcount, if 0 it won't start anymore
					m_ticks_per_period = value;
					continue_shotcount();
					reset_ticks_update(); // reset ticks_update as we've restarted the timer for multiple shotcounts
					m_counter = 0; // reset counter value (is already done in case we're aligned with an alarm but if this is a immediate update we need to reset counter.
					return;
				} else {
					set_ticks_update();
					// if this the first time in this period that ticks_per_period is updated we need to restart the timer to complete the current period
					if (!m_ticks_update) {
						// finish current period before starting with new ticks_per_period value
					        unsigned long long m_accumulated_count = local_get_counter_value();
						m_counter += m_accumulated_count;
						if (m_accumulated_count != ticks_per_period) {
							// run for 1 period -> use underlying clocked_timer API
							local_start(ticks_per_period - m_accumulated_count, 1);
						}
					}
					// indicate what alarm is triggered for, so that timer can be programmed to continue with new ticks_per_period value
					// don't worry about changing the ticks_per_period during alarm we'll not start the timer for the remainder of the period but we'll also still clear the update flag
				}
			} else {
				start_timer = true; // if not active and 'start on write to ticks': start
			}
		} else {
			start_timer = true; // old-style: update ticks_per_period restarts the timer
		}
		m_ticks_per_period = value; // update ticks_per_period value now that everything related to previous setting is handled
		if (start_timer && (trigger_style != DO_START_ONLY)) { // we need an actual start
			start();
		}
	};

	// clock setter and getter
	sc_core::sc_in<bool>& get_clock_port_impl() { return *m_clk; };
	void set_clock_port_impl(sc_core::sc_in<bool>& p) {
		m_clk = &p;
		mDividedClk(*m_clk);
		m_clock_set = true;
	};

	// clock divider getter and setter plus initialization
	unsigned long long get_clock_divider() { return mDividedClk.get_divider(); }
	void initialize_clock_divider(unsigned long long value) {
		if (value != 0) {
		        mDividedClk.set_divider((unsigned)value);
		} else {
			mDividedClk.set_divider(1); // no error check during initialization...
		}
	}
	void set_clock_divider(unsigned long long value) { 
		if (value != 0) {
		        mDividedClk.set_divider((unsigned)value);
		} else {
			SCML2_ERROR_TO(this, SCML_INVALID_API_USAGE) << name() << ": illegal value for clock_divider attribute, cannot be set to 0" << std::endl;
		}
	}
	
	// handle alarm
	void check_alarm() {
		m_current_shotcount++;
		m_shotcount_for_trigger++;
		if (shotcount_trigger != 0) { // check for intermediate shotcount trigger
			if (m_shotcount_for_trigger >= shotcount_trigger) {
				shotcount_trigger = 0; // clear shotcount trigger so a new one can be programmed
				shotcount_alarm();
				m_shotcount_for_trigger = 0; // reset shotcount counter (only used for shotcount_alarm firing)
			}
		}
		alarm_trigger = true;
		m_counter = 0; // counter is reset before alarm, so that alarm cb can reprogram start value for counter
		alarm();
		if (m_ticks_update) { // check if timer needs to be reprogrammed with new ticks per period
			continue_shotcount();
		} else {
			if (get_remaining_shot_count() == 0) {
				m_current_shotcount = 0; // make sure that m_current_shotcount is reset to 0 on last alarm so that the shotcount counter can be reprogrammed with an initial value for the next run
			}
		}
		reset_ticks_update(); // reset value, also clears in case ticks_per_period is updated in the alarm
		m_alarm_ts = sc_time_stamp();
	}

	void continue_shotcount() {
		// reprograms counter but continue with existing shotcount settings
		if (m_cached_shotcount == -1) {
			local_start(m_ticks_per_period, -1); // continue to run forever but now with new ticks_per_period
		} else {
			local_start(m_ticks_per_period, m_cached_shotcount - m_current_shotcount); // start timer with new ticks_per_period value for the remaining shortcount, if 0 it won't start anymore
			if (m_cached_shotcount == m_current_shotcount) {
				m_current_shotcount = 0; // make sure to reset m_current_shotcount on last alarm
			}
		}
	}

	void set_counting_up() {
		if (is_active() && m_cached_counting_up != configuration_options.counting_up) {
			// if we switch from counting up to counting down the alarm will hit if we reach back to 0, i.e as long as we already ran
			// if we switch from counting down to counting up the alarm will hit if we reach the maximum again, so the difference between current value and ticks_per_period
			// so switching count direction is as if we reprogram the counter such that the remaining period is as long as we already ran
			// run current alarm cycle for counter value and continue with existing settings after next alarm
			unsigned long long total_count = m_counter + local_get_counter_value(); // make sure we start counting relative to the start value
			if (configuration_options.counting_up) {
				local_start(total_count, 1);
				m_counter = ticks_per_period - total_count; // this is counter value as get_counter would return before counting_up was changed, we start counting from that value
			} else {
				local_start(ticks_per_period - total_count, 1);
				m_counter = total_count;  // this is counter value as get_counter would return before counting_up was changed, we start counting from that value
			}
			set_ticks_update();
		}
		m_cached_counting_up = configuration_options.counting_up;
	}

	void set_count_type(timer_count_type t) {
		if (configuration_options.count_type == CLOCK) {
			if (finalized() && !m_clk) {
				SCML2_ERROR_TO(this, FATAL_ERROR) << name() << ": no clock specified for timer, please specify value for clock_port attribtue during construction" << endl;
			}
			clocked_timer::stop();
		} else {
			m_manual_counter_state = STOPPED;
		}
		m_count_type = t;
	}

	timer_count_type get_count_type() {
		return m_count_type;
	}

	// local clocked_timer APIs reimplemented to account for manual counter
	void local_start(unsigned long long ticks_per_period, long long shot_count) {
		if (configuration_options.count_type == CLOCK) {
			if (!m_clock_set) {
				SCML2_ERROR_TO(this, SCML_INVALID_API_USAGE) << name() << ": trying to start a timer with count_type == CLOCK without a clock port defined" << std::endl;
			}
			clocked_timer::start(ticks_per_period, shot_count);
		} else {
			// start always restarts from 0
			m_manual_counter = 0;
			m_manual_shotcount = 0;
			if (ticks_per_period == 0 || shot_count == 0) {
				m_manual_counter_state = STOPPED;	// start with nothing to do is same as stop
			} else {
				m_manual_periodcount = ticks_per_period;
				m_manual_shotcount_target = shot_count;
				m_manual_counter_state = RUNNING;
			}
		}
	}

	unsigned long long local_get_counter_value() {
		if (configuration_options.count_type == CLOCK) {
			return clocked_timer::get_counter_value();
		} else {
			return m_manual_counter;
		}
	}

private:	
	sc_core::sc_in<bool>* m_clk;
	unsigned long long m_ticks_per_period;
	unsigned long long m_tmp_ticks_per_period;
	long long m_tmp_remaining_shot_count;
	unsigned long long m_counter;
	scml_divided_clock mDividedClk;
	bool m_clock_set;

	// advanced features
	bool m_ticks_update;
	long long m_current_shotcount;					// keeps track of shotcount progress to enable reprogramming ticks_per_period while active
	unsigned long long m_shotcount_for_trigger;		// keeps track of shotcounts since last shotcount trigger so that multiple triggers can be programmed while active
	long long m_shotcount;							// shotcount value
	long long m_cached_shotcount;					// keeps track of shotcount that was programmed when starting the timer
	sc_core::sc_time m_alarm_ts;					// keeps track of timestamp of last alarm to deal with situation of tick_per_period update at same time but after alarm triggered
	bool m_cached_counting_up;						// keeps track of last setting of counting up
	timer_count_type m_count_type;					// keeps track wether timer is set in mode to count by clock or manual

	// settings for manual counting mode
	unsigned long long m_manual_counter;				// counter value in case counter increments through other means than clock
	unsigned long long m_manual_periodcount;			// keeps track of timeout value for timer in case of manual counter
	unsigned long long m_manual_shotcount;			// keeps track of shotcount in case of manual counter
	long long m_manual_shotcount_target;				// shotcount value in caase of manual counter
	enum manual_counter_state {
		RUNNING, STOPPED
	};
	manual_counter_state m_manual_counter_state;
};



} // objects
} // scml2
