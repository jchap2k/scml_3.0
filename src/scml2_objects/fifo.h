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

#include <deque>

namespace scml2 { 
namespace objects { 

enum fifo_version {
	V1_0 = 0,
	V2_0 = 1
};

enum fifo_overflow_styles {
	OVERWRITE = 0,
	OVERRUN = 1,
	FORCED_POP = 2
};

enum fifo_size_styles {
	FIXED_SIZE = 0,
	FLEX_SIZE = 1,
	MAX_SIZE = 2
};

template<typename DT>
class fifo;

template<typename DT>
class fifo_ctrl_flags {
public:
	fifo_ctrl_flags()
		: full("full")
		, empty("empty")
		, almost_full("almost_full")
		, almost_empty("almost_empty")
		, insufficient_elements("insufficient_elements")
	{}

	scml2::base::protected_value_attribute<fifo<DT>, bool> full;		// indicates whether fifo is full
	scml2::base::protected_value_attribute<fifo<DT>, bool> empty;		// indicates whether fifo is empty
	scml2::base::protected_value_attribute<fifo<DT>, bool> almost_full;	// indicates whether fifo has more elements than almost_full level
	scml2::base::protected_value_attribute<fifo<DT>, bool> almost_empty;	// indicates whether fifo has fewer elements than almost empty_level
	scml2::base::protected_value_attribute<fifo<DT>, bool> insufficient_elements;	// indicates whether fifo has fewer elements than size in case of FIXED_SIZE style
};


class fifo_configuration_options {
public:
	fifo_configuration_options()
		: fifo_overflow_style("fifo_overflow_style")
		, fifo_size_style("fifo_size_style")
	{}

	scml2::base::value_attribute<fifo_overflow_styles> fifo_overflow_style;	// whether, when fifo is full, new elements overwrite last element or are ignored
	scml2::base::attribute<fifo_size_styles> fifo_size_style;				// whether fifo size is fixed, flexible or flexible with max size (last is default)
};


template<typename DT = unsigned int>
class fifo : public scml2::base::object
{
public:
	fifo(const scml2::base::object_name& name)
	   	: scml2::base::object(name)
	   	, size("size")
		, almost_empty_level("almost_empty_level")
		, almost_full_level("almost_full_level")
		, nr_elements("nr_elements")
		, max_nr_elements("max_nr_elements")
		, version("version")
		, push("push", this, &fifo::push_impl)
	    , pop("pop", this, &fifo::pop_impl)
		, peek("peek", this, &fifo::peek_impl)
		, peek_back("peek_back", this, &fifo::peek_back_impl)
		, front("front", this, &fifo::front_impl)
		, back("back", this, &fifo::back_impl)
		, is_full("is_full", this, &fifo::is_full_impl)
		, is_empty("is_empty", this, &fifo::is_empty_impl)
		, clear("clear", this, &fifo::clear_impl)
		, clear_max_nr_elements("clear_max_nr_elements", this, &fifo::clear_max_nr_elements_impl)
	{
		size.set_getter(this, &fifo::get_fifo_size);
		size.set_setter(this, &fifo::set_fifo_size);
		nr_elements.set_getter(this, &fifo::get_nr_elements);
		nr_elements.set_setter(this, &fifo::set_nr_elements);

		config_options.fifo_size_style.set_getter(this, &fifo::get_fifo_size_style);
		config_options.fifo_size_style.set_setter(this, &fifo::set_fifo_size_style);

		// default settigns
		config_options.fifo_overflow_style = OVERWRITE;		// overwrite on full
		config_options.fifo_size_style = MAX_SIZE;			// max size = flexible fifo size with maximum
		max_nr_elements = 0;								// max_nr_elements starts at 0
		m_size_initialized = false;							// in V1_0 size is const attr, start uninitialiwzed

		// ctrl_flags
		ctrl_flags.full = false;
		ctrl_flags.empty = true;
		ctrl_flags.almost_full = false;
		ctrl_flags.almost_empty = true;
		ctrl_flags.insufficient_elements = false; // only set to true in case of FIXED_SIZE style
	}

	void finalize_local_construction() {}

	// callback definitions
	SCML2_BASE_CALLBACK_IGNORE(full, void);					// called on push() when fifo is full in case of MAX_SIZE or FIXED_SIZE fifo's
	SCML2_BASE_CALLBACK_IGNORE(almost_full, void);			// called on push() when fifo is almosts full in case of MAX_SIZE or FIXED_SIZE fifo's
	SCML2_BASE_CALLBACK_IGNORE(empty, void);				// called on pop() when fifo is empty
	SCML2_BASE_CALLBACK_IGNORE(almost_empty, void);			// called on pop() when fifo is empty
	SCML2_BASE_CALLBACK_IGNORE(new_item_available, void);	// called when new item is successfully pushed into fifo
	SCML2_BASE_CALLBACK_IGNORE(item_popped, void, DT&);		// called when item is successfully popped from fifo
	SCML2_BASE_CALLBACK_IGNORE(overflow, void);				// called before pushing new element when fifo is almost full
	SCML2_BASE_CALLBACK_IGNORE(underflow, void);			// called before popping element when fifo is almost empty
	SCML2_BASE_CALLBACK_IGNORE(forced_pop, void, DT&);		// called when pushing new element in FIXED_SIZE fifo when full and overflow mode is FORCED_POP
	SCML2_BASE_CALLBACK_IGNORE(overrun, void, const DT&);	// called when pushing new element fifo when full and overflow mode is OVERRUN
	SCML2_BASE_CALLBACK_IGNORE(insufficient_elements, void); // called when trying to pop element from a FIXED_SIZE fifo that isn't full

	// size attribute and empty/full levels
	scml2::base::attribute<unsigned long long> size;					// max size of the fifo or fixed size of the fifo
	scml2::base::value_attribute<unsigned long long> almost_empty_level;	// nr elements below which almost_empty is called after popping element
	scml2::base::value_attribute<unsigned long long> almost_full_level;		// nr elements above which almost_full is called after adding element
	scml2::base::protected_attribute<fifo, unsigned long long> nr_elements;	// current nr elements in the fifo
	scml2::base::protected_value_attribute<fifo, unsigned long long> max_nr_elements; 	// max nr elements that were in fifo at certain point in time, clear on write semantics
	scml2::base::const_attribute<fifo_version> version;						// fifo spec version (1.0 or 2.0)
	scml2::objects::fifo_ctrl_flags<DT> ctrl_flags;								// control flags for fifo state
	scml2::objects::fifo_configuration_options config_options;				// configuration options for the fifo

	// behaviors
	scml2::base::behavior<bool, const DT&> push;		// push element in fifo
	scml2::base::behavior<bool, DT&> pop;				// pop element from fifo
	scml2::base::behavior<bool, DT&> peek;				// check front element with bool return value for case Q is empty
	scml2::base::behavior<bool, DT&> peek_back;			// check back element with bool return value for case Q is empty
	scml2::base::behavior<DT> front;					// front element of fifo without pop
	scml2::base::behavior<DT> back;						// back element of fifo without pop
	scml2::base::behavior<bool > is_full;				// checks whether fifo is full
	scml2::base::behavior<bool > is_empty;				// checks whether fifo is empty
	scml2::base::behavior<void > clear;					// clears fifo
	scml2::base::behavior<void > clear_max_nr_elements;	// clears max_nr_elements value


	// public API! (other public API's are available through behavior objects...)
	DT& operator[] (int idx) {
	    return Q[idx];
	}

private: // accessing attribute values

	unsigned long long get_fifo_size() {
		return m_size;
	}
	void set_fifo_size(unsigned long long value) {
		if (version != V1_0 || !m_size_initialized) {
			m_size = value;
			m_size_initialized = true;
			update_size_config(); // update control flags for change in size config
		} else {
			// if version is set to V1_0 and already initialized
			SCML2_ERROR_TO(this, SCML_INVALID_API_USAGE) << " attribute size is initialized more than once. Initial value is '" << m_size << "', duplicate initialization value is '" << value << "'" << std::endl;
		}
	}

	unsigned long long get_nr_elements() {
		return Q.size();
	}
	void set_nr_elements(unsigned long long /*value*/) {}


	fifo_size_styles get_fifo_size_style() {
		return m_fifo_size_style;
	}
	void set_fifo_size_style(fifo_size_styles value) {
		m_fifo_size_style = value;
		update_size_config(); // update control flags for change in size config
	}

private: // behavior implementations

	bool push_impl(const DT & value) {
		bool ret_val = true;
		if (version == V1_0) {
			if (Q.size() == size) {
				full();
				return false;
			} else {
				check_almost_sizes();
				new_item_available();
				Q.push_front(value);
			}
		} else {
			// V2.0 first check overflow then pushed value then calls new item available and then checks for almost full and then for full
			// if size is set to flex size we can 'always push' (until we run out of memory...)
			if (config_options.fifo_size_style == FLEX_SIZE) {
				add_element(value);
				set_ctrl_flags(false); // update all flags except full flag as fifo can never be full in FLEX_SIZE mode
				if (ctrl_flags.almost_full) almost_full(); // almost full still supported, flag updated
				return true;
			}
			// if mode is FIXED_SIZE or MAX_SIZE the fifo grows until max size
			bool element_added = false; // make sure elements are added only once
			set_ctrl_flags(true); // update all flags before checking any thing (check for changes in size etc. inbetween pop/push calls)
			if (ctrl_flags.full) {
				overflow(); // call overflow on full Fifo before deciding what to do
				set_ctrl_flags(true); // check for flag changes after overflow
				// check size again in case overflow changed the fifo state (increase size or remove elements)
				if (ctrl_flags.full) {
					if (config_options.fifo_overflow_style == OVERWRITE) {
						// overwrite the first element
						// modified to better support arbitrary datatypes in the fifo
						//Q[0] = value;
						Q.pop_front();
						add_element(value);
					} else if (config_options.fifo_overflow_style == FORCED_POP) {
						// pop element to make room for new value
						DT & pop_value = Q.back();
						Q.pop_back();
						forced_pop(pop_value);
						item_popped(pop_value);
						add_element(value);
						element_added = true;
					} else {
						// config_options.fifo_overflow_style == OVERRUN
						overrun(value);
						ret_val = false;
					}
					set_ctrl_flags(true); // check for changes due to forced_pop and overrun calbacks
				}
			}
			// if not full we can add element (either it wasn't full or overflow changed full-status)
			// also checks if we haven't added element before
			if (!ctrl_flags.full && !element_added) {
				add_element(value);
				set_ctrl_flags(true); // update all flags in case new_item_available changed size or anything
				if (ctrl_flags.almost_full) almost_full();
				ret_val = true; // in case of overrun that changed fifo state so that element could be added afterall
			}
			if (ctrl_flags.full) full(); // note: using if statement to assign to ctrl_flag
		}
		return ret_val;
	}

	bool pop_impl(DT & value) {
		if (version == V1_0) {
			if (Q.empty()) {
				empty();
				return false;
			} else {
				check_almost_sizes();
				value = Q.back();
				Q.pop_back();
			}
		} else {
			// V2.0 first check underflow then pop value then calls item_popped and then checks for almost empty and empty
			// if fifo is set to be FIXED_SIZE elements will only start popping out when full, so as long as it is not full
			set_ctrl_flags(true); // update all flags before checking any thing (check for changes in size etc. inbetween pop/push calls)
			if (ctrl_flags.empty) {
				underflow(); // calling underflow on empty Fifo
				set_ctrl_flags(true); // check flags in case underflow changes fifo state
				if (ctrl_flags.empty) { // check if still empty after the underflow callback
					return false;
				}
			}
			if (ctrl_flags.insufficient_elements) {
				insufficient_elements();
				set_ctrl_flags(true); // check flags in case callback changed state
				if (ctrl_flags.insufficient_elements) {
					return false;
				}
			}

			value = Q.back(); 	// get value
			Q.pop_back(); 		// remove element from deque
			item_popped(value); // call callback

			set_ctrl_flags(true); // update all flags
			if (ctrl_flags.almost_empty) almost_empty();
			if (ctrl_flags.empty) empty();
		}
		return true;
	}
	
	bool peek_impl(DT & value) {
		if (Q.empty()) {
			return false;
		} else {
			value = Q.front();
			return true;
		}
	}

	bool peek_back_impl(DT &value) {
		if (Q.empty()) {
			return false;
		} else {
			value = Q.back();
			return true;
		}
	}

	DT front_impl() { 
		return Q.front();
	}


	DT back_impl() { 
		return Q.back();
	}
	
	bool is_full_impl() {
	    return (Q.size() == size);
	}

	bool is_empty_impl() {
	    return Q.empty();
	}

	void clear_impl() {
	    Q.clear();
	    set_ctrl_flags(true); // update all flags
	}

	void clear_max_nr_elements_impl() {
		max_nr_elements = 0;
	}


private: // internal methods

	void check_almost_sizes() {
	    if (Q.size() <= almost_empty_level) almost_empty();
	    if (Q.size() >= almost_full_level) almost_full();
	}

	void add_element(const DT & value) {
		Q.push_front(value);
		if (Q.size() > max_nr_elements) {
			max_nr_elements = Q.size();
		}
		new_item_available();
	}

	void update_size_config() {
		// insufficient elements is only set in case of fixed_size else needs to be cleared
		if (m_fifo_size_style != FIXED_SIZE) {
			ctrl_flags.insufficient_elements = false;
		} else {
			ctrl_flags.insufficient_elements = (Q.size() != size);
		}
		// for flex size we need to clear the full flag
		// else need to check the full flag and possibly resize the fifo to the size limit
		if (m_fifo_size_style == FLEX_SIZE) {
			ctrl_flags.full = false;
		} else {
			if (Q.size() > size) {
				Q.resize(size);
			}
			ctrl_flags.full = (Q.size() == size);
		}
	}

	void set_ctrl_flags(bool full_included) {
		if (full_included) ctrl_flags.full = (Q.size() == size);
	    ctrl_flags.empty = Q.empty();
	    ctrl_flags.almost_empty = (Q.size() <= almost_empty_level);
	    ctrl_flags.almost_full = (Q.size() >= almost_full_level);
	    ctrl_flags.insufficient_elements = (config_options.fifo_size_style == FIXED_SIZE && Q.size() != size);
	}

private: // local data
	std::deque<DT> Q;
        fifo_size_styles m_fifo_size_style{};
        unsigned long long m_size{};
	bool m_size_initialized; // to model original const_attribute behavior of size attribute
};

} // objects
} // scml2
