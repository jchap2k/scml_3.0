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

#include <fstream>
#include "scml2.h"
#include "scml2_base.h"
#include "object_utils.h"


#include <systemc>
#include <string>
#include <vector>
#include <deque>
#include <cctype> // needed for using std::isspace
#include <typeinfo>
#include <iomanip>

namespace {
  bool convert_to_int(std::string str, unsigned int &val) {
    try {
      val = std::stoi(str, nullptr, 0);
      return true;
    }
    catch (const std::exception&) {
      return false;
    }
  }

  std::vector<std::string> get_data_vec(const std::string& s_data) {
    std::vector<std::string> data_vec;
    if (s_data[0] == '[') {
      if (s_data[s_data.size()-1] != ']') {
	// empty vector indicates error
	return data_vec;
      }
      std::string data_str = s_data.substr(1, s_data.size()-2);
      std::stringstream ss_data(data_str);
      std::string s_byte;
      while(std::getline(ss_data, s_byte, ';')) {
	data_vec.push_back(s_byte);
      }
    } else {
      data_vec.push_back(s_data); // also allow to write plain unsigned ints in the log file (for manual editing)
    }
    return data_vec;
  }
}

namespace scml2 { 
namespace objects { 

// -------------------------------------------------------------------------------------
//
// IO TRACING infrastructure
//
// generic infrastructure to add filebased record/replay to a component
//
// GOAL: record traces for model in a VDK, replay in unittest, avoid the need to setup VDK and get access to SW
//		 traces should be human readable for debugging purposes (e.G. allow to add comments to describe the trace)
//
// APPROACH: tracefile is interface-name based and uses fact that unit test mirror model uses the same names for its interfaces as the model under test
//		recorders and players are created per interface object. For recording and replay there are 2 approaches:
//		- introspection based tracing	: identify interfaces in the set of sc_objects inside a model and create recorder/player per object
//		- instrumentation based tracing	: scml2::objects (protocol engines) contributing to the central infrastructure
//
// CONTAINS:
//	0. socket_monitor			: new monitor on top of tlm trace IPT used for io_tracing of target sockets
//	1. recorder base classes 	: aligns recorder format for any object type that is traced, provides with a basic recorder for scml objects
//	2. player base classes		: takes care of the replay basics for any object type that is to be replayed
//  3. type_tracing_if			: manages recorders and players for a specific object type
//	4. convenience macros		: eases definition of type_trace classes for scml2::objects and protocol_engines
//	5. registry					: registry is used to register scml_object type tracing
//  6. type specializations		: recoder/players and type_trace definitions for signals and tlm sockets
//	7. tracing objects			: recorder and player objects to be instantiated in model and mirror model
// -------------------------------------------------------------------------------------







// -------------------------------------------------------------------------------------
// 1.recorder base class
//
//
// stores object, outfile and type-string
// provides 'record' implementation to ensure common structure in log lines
// derived implementations should create the value-storage in a string stream and pass that to the record API
//
// recorder_base_b  : base class that does not depend on object type, keeps track of name and whether description feature is available for this recorder
// recorder_base  	: generic base class independent of the underlying object type (relies on scml2::object::getBaseName as common API to get to object names)
// basic_recorder	: a recorder class enabling basic instrumentation in scml2 objects, requires that the object implements a 'set_recorder' API
// -------------------------------------------------------------------------------------
template<class BASE_OBJ = sc_core::sc_object>
class recorder_base_b {
public:
	recorder_base_b() {};
	std::string get_name() { return m_name; }
	virtual bool can_add_description() {return false;}
protected:
	std::string m_name;
};
template<class OBJ, class BASE_OBJ = sc_core::sc_object>
class recorder_base : public recorder_base_b<BASE_OBJ> {
public:
	recorder_base(OBJ *_obj, ofstream &_outfile)
		: m_outfile(_outfile)
		, m_obj(_obj)
		, m_descr_col(80)
	{
		this->m_name = scml2::objects::getBaseName(m_obj); // store name , used in recording string and for error messages
	}

	virtual ~recorder_base() {};

	std::string record_init(sc_core::sc_time t = SC_ZERO_TIME) {
		std::stringstream ss_out;
		std::string _n = get_obj_name();
		ss_out << " ps," << type_string << "," << _n << ",";
		return ss_out.str();
	}
	void record(std::string str, sc_core::sc_time t = SC_ZERO_TIME) {
		m_outfile << record_init(t) << str << endl;
	}

	void record(std::string str, std::string descr, sc_core::sc_time t = SC_ZERO_TIME) {
		std::string init = record_init(t);
		unsigned long long w; // windows: cannot init with size_t expression, warning treated as error
		w = (unsigned long long)((init.size() < m_descr_col) ? m_descr_col - init.size() : 0);
		m_outfile << init << std::left << std::setw(w) << str << descr << endl;
	}

	void set_type(std::string type) {
		type_string = type;
	}

	BASE_OBJ* get_object() {
		return (BASE_OBJ*)m_obj;
	}

	//  get name of object, to be overriden for other types...
	std::string get_obj_name()  {
		return this->m_name;
	}

	// override the object name in case recording is done for a sibling object (e.g. protocol_engine for an interface)
	void set_obj_name(std::string n) {
		this->m_name = n;
	}

	virtual void init() {};
protected:
	std::ofstream &m_outfile;
	OBJ* m_obj;
	std::string type_string;
	unsigned int m_descr_col;
};


// -------------------------------------------------------------------------------------
// basic recorder for scml2 objects
//
// requires that the scml2 object implements a 'set_recorder' method to register the recorder with the object
// recording is done through instrumentation of the object implementation with calls to 'record' on the basic recorder obj
//
template<class OBJ>
class basic_recorder : public recorder_base<OBJ, scml2::base::object>  {
public:
	basic_recorder(OBJ *_obj, ofstream &_outfile) : recorder_base<OBJ, scml2::base::object>(_obj, _outfile) {}

	void init() override {
		this->m_obj->set_recorder(this); // postponing set_recorder call so that initial values can be recorded (type_string not set at construction, didn't want to change constructor)
	}
};

// -------------------------------------------------------------------------------------
// Classes to add description support to make the recording more readable
//		kept separate from the generic recorder infra since most recorders will not user this and to avoid too many template arguments
//
// io_tracer_descr_if : interface to be implemented by module or object to add a user description to the recorded trace for an interface
// recorder_descr_base : base class for recorders that manages the description callback
//
template<class PAYLOAD>
class io_tracer_descr_if {
public:
	virtual ~io_tracer_descr_if() {}
	virtual std::string get_iotrace_description(PAYLOAD &payload) = 0;
};

template<class PAYLOAD>
class recorder_descr_base {
public:
	recorder_descr_base() : m_descr_cb(nullptr) {}
	void add_description_cb(io_tracer_descr_if<PAYLOAD>* _cb) { m_descr_cb = _cb; }
protected:
	io_tracer_descr_if<PAYLOAD> *m_descr_cb;
};




// -------------------------------------------------------------------------------------
// 2.base class for players
//
//
// stores object
// provides default implementation for get_object_name as needed by the type_tracer_base process implementation
// derived classes should implement the play method to replay the recorded log
// user needs to decide whether the play is required to be run from a thread, in that case call 'use_thread()' to ensure replay is done from a thread
//
// creates a process per instance to allow for parallel replay of all inputs (e.g. so that TLM2 transactions can start before and finish after a pin IO)
// uses a vector to log all replay requests at the start of the simulation (when the input file is processed), assumes inputs are ordered (not go back in time)
// replays them in timed order through the process
// specializations only need to implement the play method which will be called at the appropriate simulation time
// the toplevel player will wait for the finished event and check whether all instances have finished
//
// player_base 		: base class for specific object types; defaults to sc_core::sc_object but can be specialized for other object types
//					  (relies on scml2::object::getBaseName as common API to get to object names)
// -------------------------------------------------------------------------------------
struct io_tracer_payload {
public:
	io_tracer_payload(unsigned int l,std::vector<std::string> v, sc_core::sc_time _t, bool p=false) : line_nr(l), vec(v), t(_t), processed(p) {}
	~io_tracer_payload() {}
	unsigned int line_nr;
	std::vector<std::string> vec;
	sc_core::sc_time t;
	bool processed;
};
struct io_player_input {
public:
	io_player_input(std::string _t, std::string _n) : type(_t), objname(_n) {}
	~io_player_input() {}
	std::string type;
	std::string objname;
	std::vector<io_tracer_payload*> lines;
};

enum io_player_mode {
	REPLAY,
	STUB,
	OVERLAY
};

template<class OBJ, class BASE_OBJ = sc_core::sc_object>
class player_base {
public:
	player_base(OBJ *_obj)
		: m_obj(_obj)
		, m_mod(nullptr)
		, m_mode(REPLAY)
		, m_enable_checks(true)
		, m_use_method(true)
		, m_player_finished(nullptr)
		, m_delayed_check_event(nullptr)
		, m_check_time(SC_ZERO_TIME)
		, m_nr_errors(0)
		, m_nr_of_checks(0)
		, m_disable(false)
	{
		m_name = scml2::objects::getBaseName(m_obj); // store name , used in recording string and for error messages
	}
	
	virtual ~player_base() { m_inputs.clear(); } // pointers owned by io_trace_player

	// default implementation of play(), allows to create tracers that do not support replay
	virtual bool play(std::vector<std::string>) { return false; }

	// default implementation of check(), available for use in objects that have no analysis or tracing support and may need to call a checker implemented by a custom player
	virtual void check(std::string) {}

	// check if object name from trace is referring to this object, override if recording was done for a sibling object (e.g. Protocol engine for interface)
	bool is_name(const std::string objname) {
		return (get_obj_name() == objname);
	}

	void use_method() { m_use_method = true; }
	void use_thread() {	m_use_method = false; }

	void set_finished_event(sc_core::sc_event *_ev) {
		m_player_finished = _ev;
	}

	void set_stub(io_player_mode mode) {
		m_mode = mode;
	}
	void set_module(sc_core::sc_module* _mod) {
		m_mod = _mod;
	}

	void set_delayed_check_event(const sc_core::sc_event *_ev) {
		m_delayed_check_event = _ev;
	}

	void process_play(io_tracer_payload *l, sc_core::sc_time delay) {
		l->t = l->t+delay;
		if (m_inputs.size() == 0) {
			SCML2_INFO_TO(m_mod, TEST_INFO) << "setting playback for " << get_obj_name() << " to start at " << l->t << endl;
			m_next_input_ev.notify(l->t - delay); // in case of delay (i.e. multiple TLMC unit tests with replay) we need to take relative time since we already waited up to delay
			m_player_start_time = l->t - delay;
		} else {
			if (m_inputs.back()->t > l->t) {
				SCML2_ERROR_TO(m_mod, GENERIC_ERROR)  << " inputs for " << get_obj_name() << " go back in time at line " << l->line_nr << ", this is not supported." << endl;
				m_nr_errors++;
				return;
			}
		}
		m_inputs.push_back(l);
		m_nr_of_checks++;
	}

	bool is_finished() { return (m_inputs.size() == 0 && m_check_list.size() == 0); }

	void enable_checks(bool on) {
		m_enable_checks = on;
	}
	
	unsigned int get_nr_errors()
	{
		if (m_nr_of_checks != 0) { // only report result for players that had checks registered
			if (m_nr_errors != 0) {
				SCML2_INFO_TO(m_mod, TEST_INFO) << get_obj_name() << ": had " << m_nr_errors << " check failures, out of " << m_nr_of_checks << " checks" << endl;
			} else {
				SCML2_INFO_TO(m_mod, TEST_INFO) << get_obj_name() << ": passed all " << m_nr_of_checks << " checks" << endl;
			}
			return m_nr_errors;
		} else {
			return 0;
		}
	}

	unsigned int get_nr_checks() { return m_nr_of_checks; }

	std::vector<io_tracer_payload*>* get_inputs() {
		return &m_inputs;
	}

	void disable() {
		m_disable = true;
		m_next_input_ev.cancel();
	}

public:
	void finalize_construction()  {
		sc_core::sc_spawn_options opt;
		if (m_use_method) {
			opt.spawn_method();
			opt.set_sensitivity(&m_next_input_ev);
			opt.dont_initialize();
			m_h = sc_core::sc_spawn( sc_bind(&player_base<OBJ, BASE_OBJ>::process_line, this), sc_gen_unique_name("player"), &opt );
		} else {
			m_h = sc_core::sc_spawn( sc_bind(&player_base<OBJ, BASE_OBJ>::play_thread, this), sc_gen_unique_name("player"), &opt );
		}

		if (m_delayed_check_event) {
			sc_core::sc_spawn_options opt2;
			opt2.spawn_method();
			opt2.set_sensitivity(m_delayed_check_event);
			opt2.dont_initialize();
			m_h = sc_core::sc_spawn( sc_bind(&player_base<OBJ, BASE_OBJ>::check_proc, this), sc_gen_unique_name("checker"), &opt2 );
		}
	}

        virtual void player_started() {}
        virtual void player_finished() {}
private:
	void play_thread() {
		while(1) {
			wait(m_next_input_ev);
			process_line();
		}
	}

	void process_line() {
		io_tracer_payload* arg;

		if (m_disable) {
			m_next_input_ev.cancel();
			return;
		}

		// keep trak of time when line is to be processed (sc_time_stamp may move forward if this is run from a thread)
		sc_core::sc_time current_time = sc_time_stamp();

		// verify all delayed checks have finished
		if (m_enable_checks && m_check_list.size() != 0 && m_check_time != current_time) {
			m_nr_errors++;
			SCML2_ERROR_TO(m_mod, GENERIC_ERROR)  << get_obj_name() << " CHECK Failed: unsuccessfull replay for " << get_obj_name() << " previous delayed check failed" << endl;
			// clear the delayed check list as it has become useless
			for(unsigned int i = 0; i<m_check_list.size(); i++) {
				delete(m_check_list[i]);
			}
			m_check_list.clear();
			// if this is a failed test for the final line we need to notify the end of the test
			if (m_inputs.size() == 0) {
				m_player_finished->notify();
				player_finished();
				return; // we're done here, no further processing needed...
			}
		}

		arg = m_inputs[0];
		// process all lines for this timestamp
		while (arg->t == current_time) {
			m_inputs.erase(m_inputs.begin());
			if (arg->processed) continue;
			bool result = play(arg->vec);
			if (m_enable_checks && !result) {
				// if check failed on 1st delta cycle, see if we need to delay the check to later delta cycles.
				if (m_delayed_check_event) {
					m_check_time = current_time;
					m_check_list.push_back(arg);
				} else {
					m_nr_errors++;
					SCML2_ERROR_TO(m_mod, GENERIC_ERROR)  << get_obj_name() << " TEST Failed: unsuccessfull replay in line " << arg->line_nr << endl;
					delete arg;
				}
			} else {
				delete arg;
			}
			if (m_inputs.size() == 0) {
				if (is_finished()) {
					SCML2_INFO_TO(m_mod, TEST_INFO) << current_time<< ": " << get_obj_name() << ", completely finished!!" << endl;
					m_player_finished->notify();
					player_finished();
				} else {
					SCML2_INFO_TO(m_mod, TEST_INFO) << current_time<< ": " << get_obj_name() << ", finished processing inputs." << endl;
					// need to exit if we finished processing all input lines but are still waiting for a delayed check
					// make sure we validate that the delayed check is validated for the last line
					//(we need to assume no more events are coming so check_proc won't be called anymore)
					if (m_enable_checks && m_check_list.size() != 0) {
						m_next_input_ev.notify(sc_core::sc_time(1, SC_PS)); // will be canceled if check_proc is called...
					}
				}
				return; // we're done here, no further processing needed...
			}

			// get next element
			arg = m_inputs[0];
		}

		// update current time so that we check relative to actual systemc time (in case this is run from thread and time advanced)
		current_time = sc_time_stamp();

		// schedule next processing but skip any events going back in time
		while (arg->t < current_time) {
			m_nr_errors++;
			SCML2_ERROR_TO(m_mod, GENERIC_ERROR)  << get_obj_name() << " ERROR in replay: going back int time in line " << arg->line_nr << endl;
			m_inputs.erase(m_inputs.begin());
			if (m_inputs.size() == 0) {
				// make sure we validate that the delayed check is validated for the last line
				//(we need to assume no more events are coming so check_proc won't be called anymore)
				if (m_enable_checks && m_check_list.size() != 0) {
					m_next_input_ev.notify(sc_core::sc_time(1, SC_PS)); // will be canceled if check_proc is called...
				}
				return;
			}

			// get next element
			arg = m_inputs[0];
		}
		m_next_input_ev.notify(arg->t - current_time);
	}

	void check_proc() {
		if (m_check_list.size() == 0 || !m_enable_checks) return; // unlogged event?! or no checking enabled
		io_tracer_payload* arg = m_check_list[0];
		m_check_list.erase(m_check_list.begin());
		if (sc_time_stamp() != m_check_time || !play(arg->vec)) {
			m_nr_errors++;
			SCML2_ERROR_TO(m_mod, GENERIC_ERROR) << get_obj_name() << " CHECK Failed: unsuccessfull delayed check for replay in line " << arg->line_nr << ", " << m_check_time << endl;
		}
		delete arg;
		if (is_finished()) {
			SCML2_INFO_TO(m_mod, TEST_INFO) << sc_time_stamp() << ": " << get_obj_name() << " finished!!" << endl;
			m_player_finished->notify();
			player_finished();
			m_next_input_ev.cancel(); // make sure process_line isn't called anymore to check final line did finish
		}
	}

protected:
	//  get name of object, to be overriden for other types...
	std::string get_obj_name()  {
		return m_name;
	}

	// override the object name in case recording is done for a sibling object (e.g. protocol_engine for an interface)
	void set_obj_name(std::string n) {
		m_name = n;
	}

	// to support non-blocking TLM APIs we need to allow flexibility in order of execution of player processing and incoming TLM APIs in players
	// an incoming TLM API can check whether there is an unprocessed input for a certain timestamp (allows for annotated time calls)
	// NOTE: it is not possible to interleave non-blocking m_inputs with external events at same timestamp since all m_inputs for one timepoint are processed together in while-loop
	//
	io_tracer_payload* get_next_unprocessed_input_at(sc_core::sc_time t) {
		for (auto i: m_inputs) {
			if (i->t == t && i->processed == false) {
				return i;
			}
		}
		return nullptr;
	}

	void incr_errors() {
		m_nr_errors++;
	}

protected:
	OBJ* m_obj;
	sc_core::sc_module* m_mod;
	io_player_mode m_mode;

	bool m_enable_checks;

private:
	sc_core::sc_event m_play_event;
	sc_core::sc_process_handle m_h;
	std::string m_name;

	bool m_use_method;

	std::vector<io_tracer_payload*> m_inputs;
	sc_core::sc_time m_player_start_time;
	sc_core::sc_event m_next_input_ev;
	sc_core::sc_event* m_player_finished;

	const sc_core::sc_event* m_delayed_check_event;
	std::vector<io_tracer_payload*> m_check_list;
	sc_core::sc_time m_check_time;

	unsigned int m_nr_errors;
	unsigned int m_nr_of_checks;

	bool m_disable;
};




// -------------------------------------------------------------------------------------
// 3. type_tracing_if
//
//
//type_tracing_if
//			: interface class defining the set of API's as used by the recorder and player
//
// type_tracer_base
// 			: base class for record replay infra for certain type of object in module
//
// both are templetized for the base type of objects that can be traced by the recorder/player set managed by the type_tracer_base
// default is for sc_core::sc_object, but can also work for e.g. scml2::base::objects (hence the specialized base classes for scml2 objects)
//
// to add support for a certian object type a type_tracer class should be created that derves from this class:
// with specific template classes and an implementation of the process() call
// the type specific class should also store the type name in the type_name member (via typeid.name info) this allows to check for duplicates
//
// type_tracer_base template arguments:
// - OBJ is type of object to check for
//		OBJ should be derived from an sc_core::sc_object so that it is part of the children of the module
// - REC is a class that will be created to record for objects of type OBJ
//		REC should be derived from recorder_base and have a constructor with 3 arguments (name, object instance pointer and outfile)
// - PLAY is a class that will be created to replay for objects of type OBJ
//		PLAY should be derived from player_base and have a constructor with 2 arguments (name and object instance pointer)
//
// this base class takes care of the basic management of recorders and players
//  - recorders and players are created based on a 'check_for..' call implemented by the type specialization else they fall back to a dynamic cast
//  - there is a call to remove a recorder to disable recording
//	- there is a call to process a line for this type, which will look for the appropriate player and hand the actual replay to the player
//  - the base class also takes care of keeping track of error and check metrics as well as whether the replay has finished for this type
//	- recorders and players should manage their type tracer lists via the 'add_tracer_to_list' API, which allows the type_tracer itself to check whether a duplicate exists
// -------------------------------------------------------------------------------------

// -------------------------------------------------------------------------------------
// type_tracer_base_if: interface definition linking trace_types with toplevel recorder and player objects
template<class BASE_OBJ = sc_core::sc_object>
class type_tracer_base_if
{
public:
	virtual ~type_tracer_base_if() {};
	virtual bool createRecorder(BASE_OBJ* _obj, std::ofstream &outfile) = 0;
	virtual bool removeRecorder(BASE_OBJ* _obj) = 0;
	virtual void deleteRecorders() =0;
	virtual bool createPlayer(BASE_OBJ* _obj, sc_core::sc_module* _mod, sc_core::sc_event *_ev, io_player_mode mode) = 0;
	virtual bool process(io_player_input*, sc_core::sc_time delay, io_player_mode mode) = 0;
	virtual bool allPlayersDone() = 0;
	virtual void enable_checks(bool on = true) = 0;
	virtual unsigned int get_errors() = 0;
	virtual unsigned int get_checks() = 0;
	virtual std::vector<io_tracer_payload*> *get_input_list(std::string objname) = 0;
	virtual bool disable_player(std::string objname) = 0;
	virtual bool player_exists(std::string objname) = 0;
	virtual bool add_tracer_to_list(std::vector<type_tracer_base_if<BASE_OBJ>*>* trace_types) =0;
	virtual std::string get_type_name() = 0;
	virtual bool check_obj_type(BASE_OBJ* _obj) = 0;
	virtual recorder_base_b<BASE_OBJ>* get_recorder_for_obj(std::string _name) = 0;
        virtual void player_started() = 0;
};


// -------------------------------------------------------------------------------------
// type_tracer_base: shared infrastructure to setup recorders and players of a certian type
template<class OBJ, class REC, class PLAY, class BASE_OBJ = sc_core::sc_object>
class type_tracer_base : public type_tracer_base_if<BASE_OBJ>
{
public:
	type_tracer_base(std::string _r_type, std::string _p_type)
		: type_name("none")
		, record_type(_r_type)
		, play_type(_p_type)
	{}

	~type_tracer_base() {
		deleteRecorders();
		deletePlayers();
	}

	// checks if the object is of the right type to be able to record
	// a pre-check 'check_can_record' can be used to avoid the dynamic cast
	// creates a recorder of type REC
	bool createRecorder(BASE_OBJ* _obj, std::ofstream &outfile) {
		if (check_can_record(_obj)) {
			OBJ* m_obj = dynamic_cast<OBJ* >(_obj);
			if (m_obj) {
				REC *m_recorder = new REC(m_obj, outfile);
				m_recorder->set_type(record_type);
				m_recorder->init();
				recorders.push_back(m_recorder);
				return true;
			}
		}
		return false;
	}

	// remove recorder: is used to disable recording for a certain object
	// the recorder should take care of cleanly removing all tracing helpers
	bool removeRecorder(BASE_OBJ* _obj) {
		for(auto *r : recorders) {
			if (r->get_object() == _obj) {
				delete(r);
				return true;
			}
		}
		return false;
	}

	// delete all recorders: is used to disable all recording
	void deleteRecorders() {
		for(auto* r : recorders) delete(r);
		recorders.clear();
	}

	// delete all players: is used to disable all playing
	void deletePlayers() {
		for(auto* p : players) delete(p);
		players.clear();
	}

        void player_started() override {
	  for(auto* p : players) p->player_started();
	}


	// checks if the object is of the right type to be able to record
	// a pre-check 'check_can_play' can be used to avoid the dynamic cast
	// creates a player of type PLAY and sets the finished event for the player to notify when it is done
	bool createPlayer(BASE_OBJ* _obj, sc_core::sc_module* _mod, sc_core::sc_event *finished_ev, io_player_mode mode = REPLAY) {
		if (check_can_play(_obj)) {
			OBJ* m_obj = dynamic_cast<OBJ* >(_obj);
			if (m_obj) {
				PLAY* m_play = new PLAY(m_obj);
				m_play->set_finished_event(finished_ev);
				m_play->set_module(_mod);
				m_play->set_stub(mode);
				m_play->finalize_construction();
				players.push_back(m_play);
				return true;
			}
		}
		return false;
	}

	// checks if the type string in the log is what we're looking for
	// calls check_can_process which needs to be implemented in the actual type tracer: should check the arguments of the trace-line
	// looks for a player for the object and calls play which should store the input log to be processed as time t, line_nr is passed for error reporting
	// returns true if a player was found
	bool process(io_player_input* ip, sc_core::sc_time delay, io_player_mode mode) {
		if ((mode == REPLAY && ip->type != play_type) || (mode == STUB && ip->type != record_type) || (mode == OVERLAY && ip->type != record_type)) {
			return false;
		}
		for (player_base<OBJ, BASE_OBJ>* p : players ) {
			if (p->is_name(ip->objname)) {
				for (auto l: ip->lines) {
					if (!check_can_process(l->vec)) {
						cerr  << "unable to replay: failed to process line " << l->line_nr << endl;
						return false;
					}
					p->process_play(l, delay);
				}
				return true;
			}
		}
		return false;
	}

	// checks whether all instances of the player type have finished replaying the input file
	bool allPlayersDone() {
		for (player_base<OBJ, BASE_OBJ>* p : players ) {
			if (!p->is_finished()) return false;
		}
		return true;
	}

	// enable/disable checks for players
	void enable_checks(bool on = true) {
		for (player_base<OBJ, BASE_OBJ>* p : players ) {
			p->enable_checks(on);
		}
	}
	
	// gets the number of errors that occured during replay
	unsigned int get_errors() {
		unsigned int nr_errors = 0;
		for (player_base<OBJ, BASE_OBJ>* p : players ) {
			nr_errors += p->get_nr_errors();
		}
		return nr_errors;
	}

	// gets the total number of checks that were done during replay
	unsigned int get_checks() {
		unsigned int nr_checks = 0;
		for (player_base<OBJ, BASE_OBJ>* p : players ) {
			nr_checks += p->get_nr_checks();
		}
		return nr_checks;
	}

	std::string get_type_name() {
		return type_name;
	}

	bool add_tracer_to_list(std::vector<type_tracer_base_if<BASE_OBJ>*>* trace_types) {
		for (type_tracer_base_if<BASE_OBJ>*i: *trace_types ) {
			std::string _n = i->get_type_name();
			if ( _n != "none" && _n == this->get_type_name()) {
				return false; // tracer of same type was already added
			}
		}
		trace_types->push_back(this);
		return true;
	}

	// check object type
	bool check_obj_type(BASE_OBJ* _obj) {
		OBJ* m_obj = dynamic_cast<OBJ* >(_obj);
		return (m_obj != nullptr);
	}

	// API to fetch recorders that need to be passed into findRecorder API
	recorder_base_b<BASE_OBJ>* get_recorder_for_obj(std::string _name) {
		for(auto* r : recorders) {
			if (r->get_name() == _name) {
				return r;
			}
		}
		return nullptr;
	}

	// default implementation of checkers
	virtual inline bool check_can_record(BASE_OBJ* ) { return false; }
	virtual inline bool check_can_play(BASE_OBJ* ) { return false; }
	virtual inline bool check_can_process(std::vector<std::string> ) { return false; }

	std::vector<io_tracer_payload*>* get_input_list(std::string objname) {
		for (auto *p: players) {
			if (p->is_name(objname)) {
				return p->get_inputs();
			}
		}
		return nullptr;
	}

	bool disable_player(std::string objname) {
		for (auto *p: players) {
			if (p->is_name(objname)) {
				p->disable();
				return true;
			}
		}
		return false;
	}

	bool player_exists(std::string objname) {
		for (auto *p: players) {
			if (p->is_name(objname)) {
				return true;
			}
		}
		return false;
	}
protected:
	std::vector<recorder_base<OBJ, BASE_OBJ>*> recorders;
	std::vector<player_base<OBJ, BASE_OBJ>*> players;

	// name used to check for duplicate type_tracer classes
	std::string type_name;
private:
	std::string record_type; // string used in log file to indicate type of log-line when recording
	std::string play_type;   // log-type that is to be replayed for this C++ type (NOTE: usually MIRROR type of record type: e.g. sc_in versus sc_out)
};





// -------------------------------------------------------------------------------------
// 4. CONVENIENCE MACROS
//
//
// -------------------------------------------------------------------------------------


// shorthand for the base recorder and player class for a certain scml2 object type
#define IO_TRACER_BASE_REC(OBJ) scml2::objects::recorder_base<OBJ, scml2::base::object>
#define IO_TRACER_BASE_PLAY(OBJ) scml2::objects::player_base<OBJ, scml2::base::object>

// -------------------------------------------------------------------------------------
// IO_TRACER_DECL_SCML: creates a basic io_trace_type for an SCML object with only recording support and using the basic_recorder as recorder for the object
//						recording will be done by calling 'record' on this basic recorder from within the object
//
//	OBj 	= object type
//	ID		= ID string identifier, used by recorder,  to be printed in the IO_trace file, should be unique for an object type
#define IO_TRACER_DECL_SCML(OBJ, ID) 																																	\
class  io_trace_##OBJ : public scml2::objects::type_tracer_base< OBJ, scml2::objects::basic_recorder<OBJ>, IO_TRACER_BASE_PLAY(OBJ), scml2::base::object> {				\
public:																																									\
	io_trace_ ##OBJ () : scml2::objects::type_tracer_base<OBJ, scml2::objects::basic_recorder<OBJ>, IO_TRACER_BASE_PLAY(OBJ), scml2::base::object >(ID, ID "_TEST") {	\
		this->type_name = typeid(this).name();																															\
	};																																									\
	inline bool check_can_record(scml2::base::object* _obj) override { return true; }																					\
};


// -------------------------------------------------------------------------------------
// IO_TRACER_DECL_SCML_REC: creates a new io_trace_type for an SCML object with only recording support and using a dedicated recorder for the object
//							allows for introspection based recording (no instrumentation in the object implementation) or more complex recording mechanisms
//
//	OBj = object type
//	REC = recorder type
//	ID	= ID string identifier, used by recorder, to be printed in the IO_trace file, should be unique for an object type
#define IO_TRACER_DECL_SCML_REC(OBJ, REC, ID) 																							\
class  io_trace_##OBJ : public scml2::objects::type_tracer_base< OBJ, REC, IO_TRACER_BASE_PLAY(OBJ), scml2::base::object> {				\
public:																																	\
	io_trace_ ##OBJ () : scml2::objects::type_tracer_base<OBJ, REC, IO_TRACER_BASE_PLAY(OBJ), scml2::base::object >(ID, ID "_TEST") {	\
		this->type_name = typeid(this).name();																							\
	};																																	\
	inline bool check_can_record(scml2::base::object* _obj) override { return true; }													\
};


// -------------------------------------------------------------------------------------
// IO_TRACER_DECL_SCML_PLAY: creates a new io_trace_type for an SCML object with only player support
//
//	OBJ  = object type
//	PLAY = player type
//	ID	 = ID string identifier used by player
//	MIN  = minimal number of arguments in string for playing
//	MAX  = maximum number of arguments in string for replay
#define IO_TRACER_DECL_SCML_PLAY(OBJ, PLAY, ID, MIN, MAX)																			\
class  io_trace_##OBJ : public scml2::objects::type_tracer_base<OBJ, IO_TRACER_BASE_REC(OBJ), PLAY, scml2::base::object > {			\
public:																																\
	io_trace_##OBJ () : scml2::objects::type_tracer_base<OBJ, IO_TRACER_BASE_REC(OBJ), PLAY, scml2::base::object>(ID "_TEST", ID) {	\
		this->type_name = typeid(this).name();																						\
	};																																\
	inline bool check_can_play(scml2::base::object* _obj) override { return true; }													\
	inline bool check_can_process(std::vector<std::string> vec) override { return (vec.size() >= MIN && vec.size() <= MAX); }		\
};


// -------------------------------------------------------------------------------------
// IO_TRACER_DECL_SCML_FULL: creates a new io_trace_type for an SCML object with recorder and player support
//
//	OBJ  = object type
//  REC  = recorder type
//	PLAY = player type
//	ID	 = ID string identifier used by recorder (representing the OBJ type in the trace)
//  ID2  = ID string identifier used by player (representing the mirror object type in the trace)
//	MIN  = minimal number of arguments in string for playing
//	MAX  = maximum number of arguments in string for replay
#define IO_TRACER_DECL_SCML_FULL(OBJ, REC, PLAY, ID, ID2, MIN, MAX)																\
class  io_trace_##OBJ : public scml2::objects::type_tracer_base<OBJ, REC, PLAY, scml2::base::object > {							\
public:																															\
	io_trace_##OBJ () : scml2::objects::type_tracer_base<OBJ, REC, PLAY, scml2::base::object>(ID , ID2) {						\
		this->type_name = typeid(this).name();																					\
	};																															\
	inline bool check_can_record(scml2::base::object*) override { return true; }											\
	inline bool check_can_play(scml2::base::object*) override { return true; }												\
	inline bool check_can_process(std::vector<std::string> vec) override { return (vec.size() >= MIN && vec.size() <= MAX); }	\
};



// -------------------------------------------------------------------------------------
// ADD_IO_TRACE: creates an instance of the io_trace_type for an SCML object (assumes the type name is as in the earlier macro's
//
// should be called in the constructor of the reuse object
// OBJ  = object type
#define ADD_IO_TRACE_TYPE(OBJ)																			\
		{																								\
		scml2::objects::io_trace::registry* inst = scml2::objects::io_trace::registry::getInstance();	\
		inst->add(new io_trace_##OBJ (), this);															\
		}





// -------------------------------------------------------------------------------------
// 5. registry
//
//
// Registry to add scml2 based instrumentation based tracing
//
// Objects that want to contribut to the tracing register with the registry
// When tracer object (io_trace_recorder or io_tracer_player) goes through its 'invoke callback' to construct the actual recorders and players the registrants will be
// -------------------------------------------------------------------------------------

// forward declarations:
//		registry depends both on type_tracer_base as well as on io_tracers_base
class io_tracers_base;


// -------------------------------------------------------------------------------------
// registry sits in extra level of namespace io_trace (so scml2::objects::io_trace::registry)
namespace io_trace {

class SNPS_VP_API registry {
public:
	typedef type_tracer_base_if<scml2::base::object> t_type_tracer;
	typedef std::vector<t_type_tracer*> t_trace_list;

	static registry* getInstance() {
		if (!instance) instance = new registry();
		return instance;
	}

	~registry() {
		cleanup_lists();
	}

	void add(t_type_tracer* _t, scml2::base::object* _o);
	bool set_tracer(io_tracers_base* _tracer);
	void clear_tracer();

private:
	registry()
		: m_trace_types(nullptr)
		, m_trace_objects(nullptr)
		, m_tracer(nullptr)
		, m_module(nullptr)
		, m_list_is_transferred(false)
	{
		new_lists();
	}

	bool check_module();
	void cleanup_lists();
	void new_lists();

private:
	static registry* instance;
	t_trace_list* m_trace_types;
	std::vector<scml2::base::object*>* m_trace_objects;
	io_tracers_base* m_tracer;
	const sc_core::sc_module* m_module;
	bool m_list_is_transferred;
};


} // namespace io_trace






// -------------------------------------------------------------------------------------
// 6. type specializations
//
//
// for sc_objects : signals and tlm sockets
//
// for each type they define:
//	- recorder
//	- player
//	- io_trace_type class
// -------------------------------------------------------------------------------------


// -------------------------------------------------------------------------------------
// signal port infrastructure
//
// classes:
//		sc_port_recorder: REC class for sc_in and sc_out ports
// 						  uses a SystemC method to prints message on signal change
//
//		sc_in_port_player, sc_out_port_player : specialized PLAY classes for sc_in and sc_out ports
//
//		io_trace_sc_in_port, io_trace_sc_out_port: type_tracer classes for sc_in and sc_out ports
//
// -------------------------------------------------------------------------------------
template<template <typename> class P, class T>
class sc_port_recorder : public recorder_base<P<T>>, public recorder_descr_base<P<T>*> {
public:
	sc_port_recorder( P<T> *_obj, ofstream &_outfile)
		: recorder_base<P<T> >(_obj, _outfile)
	{
		sc_core::sc_spawn_options opt;
		opt.spawn_method();
		//opt.dont_initialize();
		opt.set_sensitivity(&this->m_obj->value_changed());
		std::string nm = std::string(this->m_obj->basename()) + "_io_tracing";
		m_h = sc_core::sc_spawn( sc_bind(&sc_port_recorder<P, T>::record_method, this), sc_gen_unique_name(nm.c_str()), &opt );
	};

	~sc_port_recorder() {
		// m_h.kill(); // removed this because got issues due to 'kill an uninitialized process' in SystemC even though the process ran...
	}

	void record_method() {
		std::stringstream ss;
		ss << this->m_obj->read();
		if (this->m_descr_cb) {
			this->record(ss.str(), this->m_descr_cb->get_iotrace_description(this->m_obj));
		} else {
			this->record(ss.str());
		}
	}

	bool can_add_description() {return true;}

private:
	sc_core::sc_process_handle m_h;
};
// specialization for bool ports, to ensure scml_clock ports do not get recorded (leads to recording on every edge)
template<>
class sc_port_recorder<sc_core::sc_in, bool> : public recorder_base<sc_core::sc_in<bool>>, public recorder_descr_base<sc_core::sc_in<bool>*> {
public:
	sc_port_recorder(sc_core::sc_in<bool> *_obj, ofstream &_outfile)
		: recorder_base<sc_core::sc_in<bool> >(_obj, _outfile)
	{
		// recorders are created at end of elab so we can check whether the bool port is a clock
		scml_clock_if* c = scml2::get_scml_clock(*this->m_obj, true);
		if (c == nullptr) {
			sc_core::sc_spawn_options opt;
			opt.spawn_method();
			//opt.dont_initialize();
			opt.set_sensitivity(&this->m_obj->value_changed());
			std::string nm = std::string(this->m_obj->basename()) + "_io_tracing";
			m_h = sc_core::sc_spawn( sc_bind(&sc_port_recorder<sc_core::sc_in, bool>::record_method, this), sc_gen_unique_name(nm.c_str()), &opt );
		}
	};

	~sc_port_recorder() {
		// m_h.kill(); // removed this because got issues due to 'kill an uninitialized process' in SystemC even though the process ran...
	}

	void record_method() {
		std::stringstream ss;
		ss << this->m_obj->read();
		if (this->m_descr_cb) {
			this->record(ss.str(), this->m_descr_cb->get_iotrace_description(this->m_obj));
		} else {
			this->record(ss.str());
		}
	}

	bool can_add_description() {return true;}

private:
	sc_core::sc_process_handle m_h;
};


// -------------------------------------------------------------------------------------
// in and out port players: use conversion classes for string to type conversion
//
template<template <typename> class P, class T>
class sc_in_port_player : public player_base<P<T> > {
public:
	sc_in_port_player(sc_core::sc_in<T> *_obj) : player_base<P<T> >(_obj), m_overlay_sig(nullptr)
	{
		this->set_delayed_check_event(&this->m_obj->value_changed_event());
	}

	~sc_in_port_player() {};


	bool play(std::vector<std::string> vec) override {
		T val;
		try {
			val = string_to_val<T>::convertVal(vec[0]);
		} catch(const std::exception& e) {
			SCML2_INFO_TO(this->m_mod, TEST_INFO) << " parsing problem(" << e.what() <<"), could not convert input to expected type for " << this->m_obj->name() << endl;
			return false;
		}
		if (this->m_mode == STUB) {
			return true; // ignore value
		} else if (this->m_mode == REPLAY){
			return (val == this->m_obj->read());
		} else {
			if (m_overlay_sig == nullptr) {
				m_overlay_sig = dynamic_cast<sc_core::sc_signal<T>*>(this->m_obj->get_interface());
				if (m_overlay_sig == nullptr) {
					SCML2_INFO_TO(this->m_mod, TEST_INFO) << " could not find connecting signal for " << this->m_obj->name() << ": cannot perform overlay" << endl;
				}
			}
			if (m_overlay_sig != nullptr) {
				m_overlay_sig->write(val);
				return true;
			}
		}
		return false;
	}
private:
	sc_core::sc_signal<T> *m_overlay_sig;
};

// -------------------------------------------------------------------------------------
template<template <typename> class P, class T>
class sc_out_port_player : public player_base<P<T> > {
public:
	sc_out_port_player(sc_core::sc_out<T> *_obj) : player_base<P<T> >(_obj) {}

	~sc_out_port_player() {};

	bool play(std::vector<std::string> vec) override {
		T val;
		try {
			val = string_to_val<T>::convertVal(vec[0]);
		} catch(const std::exception& e) {
			SCML2_INFO_TO(this->m_mod, TEST_INFO) << " parsing problem(" << e.what() <<"), could not convert input to expected type for " << this->m_obj->name() << endl;
			return false;
		}
		this->m_obj->write(val);
		return true;
	}
};


// -------------------------------------------------------------------------------------
// actual type_tracers specializations for sc_in and sc_out ports
//
// record/replay for in ports: records model in port side, replay model out port side (checker)
// stub for in side: ignore updates
//
template<class TYPE>
class io_trace_sc_in_port : public type_tracer_base<sc_core::sc_in<TYPE>, sc_port_recorder<sc_core::sc_in, TYPE>, sc_in_port_player<sc_core::sc_in, TYPE> >
{
public:
	// define type tracer string types 'IN' and 'OUT': recording 'IN' and replay 'OUT'
	io_trace_sc_in_port() : type_tracer_base<sc_core::sc_in<TYPE>, sc_port_recorder<sc_core::sc_in, TYPE>, sc_in_port_player<sc_core::sc_in, TYPE> >("IN", "OUT") {
		this->type_name = typeid(this).name();
	};

	inline bool check_can_record(sc_core::sc_object* _obj) override { return (!strcmp(_obj->kind(), "sc_in")); }
	inline bool check_can_play(sc_core::sc_object* _obj) override { return (!strcmp(_obj->kind(), "sc_in")); }
	inline bool check_can_process(std::vector<std::string> vec) override { return (vec.size() == 1); }
};

// -------------------------------------------------------------------------------------
// record/replay for out ports: records model out port side, replay model in port side (update values)
// stub for out port side: send value updates
//
template<class TYPE>
class io_trace_sc_out_port : public type_tracer_base<sc_core::sc_out<TYPE>, sc_port_recorder<sc_core::sc_out, TYPE>, sc_out_port_player<sc_core::sc_out, TYPE> >
{
public:
	// define type tracer string types 'OUT' and 'IN': recording 'OUT' and replay 'IN'
	io_trace_sc_out_port() : type_tracer_base<sc_core::sc_out<TYPE>, sc_port_recorder<sc_core::sc_out, TYPE>, sc_out_port_player<sc_core::sc_out, TYPE> >("OUT", "IN") {
		this->type_name = typeid(this).name();
	};

	inline bool check_can_record(sc_core::sc_object* _obj) override { return (!strcmp(_obj->kind(), "sc_out")); }
	inline bool check_can_play(sc_core::sc_object* _obj) override { return (!strcmp(_obj->kind(), "sc_out")); }
	inline bool check_can_process(std::vector<std::string> vec) override { return (vec.size() == 1); }
};


// -------------------------------------------------------------------------------------
// tlm2 sockets infrastructure
//
// classes
//		sc_socket_recorder: for initiator and target sockets
//						    uses dedicated socket tracing monitor to intercept incoming b_transports
//
//		sc_target_socket_player: for initiator sockets only
//
//		io_trace_sc_target_socket : target port tracer, only supports recording
//		io_trace_sc_initiator_socket : initiator port tracer, only supports replay
//
// -------------------------------------------------------------------------------------
template<class SOCKET>
class sc_socket_recorder : public recorder_base<SOCKET>, public recorder_descr_base<const tlm::tlm_generic_payload>
{
public:
	sc_socket_recorder(SOCKET* _obj, ofstream &_outfile)
		: recorder_base<SOCKET>(_obj, _outfile)
	{
	}

	~sc_socket_recorder() {
	}

	void transaction(const tlm::tlm_generic_payload* trans, const sc_core::sc_time &tt) {
		// for correct replay we need to log the timing annotation when transaction starts
		// time passed in the monitor API is absolute time, so need to check whether timing annotation is needed
		sc_core::sc_time t = sc_time_stamp();
		if (tt != SC_ZERO_TIME && tt > t) {
			m_annot_t = (tt - t);
		} else {
			m_annot_t = SC_ZERO_TIME;
		}
		if (trans->get_command() == tlm::TLM_WRITE_COMMAND) {
			m_start_t = SC_ZERO_TIME;
			record_trans(*trans);
		} else {
			m_start_t = sc_time_stamp();
		}
	}

	void end_transaction(const tlm::tlm_generic_payload* trans) {
		if (trans->get_command() == tlm::TLM_READ_COMMAND) {
			record_trans(*trans);
		}
	}

	void record_trans(const tlm::tlm_generic_payload &trans) {
		std::stringstream ss;
		ss << std::setw(5) << ((trans.get_command() == tlm::TLM_READ_COMMAND) ? "READ" : "WRITE" );
		ss << ",0x" << hex << std::setfill('0') << std::setw(8) << trans.get_address();
		ss << ",[";
		unsigned char *data = trans.get_data_ptr();
		for (unsigned int i = 0; i< trans.get_data_length(); i++) {
			if (i!=0) ss << ";";
			ss << "0x" << std::setfill('0') << std::setw(2) << (unsigned int)(data[i]);
		}
		ss << "]";
		if (m_annot_t != SC_ZERO_TIME) {
			ss << "," << m_annot_t;
		}
		ss << dec;
		if (this->m_descr_cb) {
			this->record(ss.str(), this->m_descr_cb->get_iotrace_description(trans), m_start_t);
		} else {
			this->record(ss.str(), m_start_t);
		}
	}

	bool can_add_description() {return true;}

private:
	sc_core::sc_time m_annot_t;
	sc_core::sc_time m_start_t;
};

// -------------------------------------------------------------------------------------
template<class SOCKET>
class sc_target_socket_player : public player_base<SOCKET>
{
public:
	sc_target_socket_player(SOCKET* _obj): player_base<SOCKET>(_obj) {
		this->use_thread(); // TLM2 transactions should be emitted from a thread...
	}

	~sc_target_socket_player() {};

	bool play(std::vector<std::string> vec) override {
		std::string cmd = vec[0];
		std::string str_addr = vec[1]; // s_addr is redefined by windows headers...
		std::string s_data = vec[2];
		sc_core::sc_time t_annot = SC_ZERO_TIME;
		if (vec.size() == 4) {
			t_annot = string_to_val<sc_core::sc_time>::convertVal(vec[3]);
		}

		unsigned long long addr = std::stoull(str_addr, nullptr, 0);
		std::vector<std::string> data_vec = get_data_vec(s_data);
		if (data_vec.empty()) {
		  SCML2_ERROR_TO(this->m_mod, GENERIC_ERROR)  << " could not process data vector for transaction for " << (*this->m_obj).name() << endl;
		  return false;
		}
		unsigned int nr_bytes = (unsigned int)data_vec.size();
		unsigned char* data_ptr = new unsigned char[nr_bytes];

		m_trans.set_address(addr);
		m_trans.set_data_ptr(data_ptr);
		m_trans.set_data_length(nr_bytes);
		m_trans.set_streaming_width(nr_bytes);
		m_trans.set_byte_enable_length(0);
		m_trans.set_byte_enable_ptr(nullptr);
		m_trans.set_response_status(tlm::TLM_INCOMPLETE_RESPONSE);
		m_trans.set_dmi_allowed(false);

		bool success = true;
		if (cmd == "WRITE") {
			for (unsigned int i =0; i< nr_bytes; i++) {
				unsigned int val;
				if (convert_to_int(data_vec[i], val)) {
					data_ptr[i] = (unsigned char)val;
				} else {
					SCML2_ERROR_TO(this->m_mod, GENERIC_ERROR) << "could not convert transaction data to unsigned int for value: \"" << data_vec[i] << "\" for " << (*this->m_obj).name() << endl;
					delete[](data_ptr);
					return false;
				}
			}
			m_trans.set_command(tlm::TLM_WRITE_COMMAND);
			(*this->m_obj)->b_transport(m_trans, t_annot);
		}
		else if (cmd == "READ") {
			m_trans.set_command(tlm::TLM_READ_COMMAND);
			(*this->m_obj)->b_transport(m_trans, t_annot); 
			for (unsigned int i =0; i< nr_bytes; i++) {
				unsigned int val;
				if (!convert_to_int(data_vec[i], val)) {
					SCML2_ERROR_TO(this->m_mod, GENERIC_ERROR) << "could not convert transaction data to unsigned int for value: \"" << data_vec[i] << "\" for " << (*this->m_obj).name() << endl;
					delete[](data_ptr);
					return false;
				}
				if (data_ptr[i] != (unsigned char)val) {
					success = false;
					break;
				}
			}
		}
		delete[](data_ptr);
		return success;
	}
protected:
	tlm::tlm_generic_payload m_trans;
};

// -------------------------------------------------------------------------------------
template<class SOCKET>
class sc_initiator_socket_player : public player_base<SOCKET>
{
public:

  sc_core::sc_event transaction_ready;
  sc_core::sc_event transaction_done;

  typedef std::vector<std::string> command;
  std::deque<command> cmds;
  unsigned transaction_match_failure_cnt{};

  class my_fw_if : public scml2::mappable_if {
  public:
    sc_initiator_socket_player* owner;

    my_fw_if(sc_initiator_socket_player* _owner) : owner(_owner) {}

    void b_transport(tlm::tlm_generic_payload& trans, sc_core::sc_time& t) override {
    	sc_core::wait(t);
    	t = sc_core::SC_ZERO_TIME;
    	if (owner->cmds.empty()) {
    		sc_core::wait(owner->transaction_ready);
    	}
    	assert(!owner->cmds.empty());
    	decode_command(owner->cmds.front(), trans);
    	owner->cmds.pop_front();
    	owner->transaction_done.notify(SC_ZERO_TIME);
    }

    void set_error_response(tlm::tlm_generic_payload& trans, const std::string& message) {
    	SCML2_ERROR_TO(owner->m_mod, GENERIC_ERROR) << message << "\" for " << (*owner->m_obj).name() << endl;
    	owner->transaction_match_failure_cnt++;
    	trans.set_response_status(tlm::TLM_GENERIC_ERROR_RESPONSE);
    }

    // checker only...
    void decode_command(const std::vector<std::string>& vec, tlm::tlm_generic_payload& trans) {
		std::string cmd = vec[0];
		std::string str_addr = vec[1]; // s_addr is redefined by windows headers...
		std::string s_data = vec[2];
		sc_core::sc_time t_annot = SC_ZERO_TIME;
		if (vec.size() == 4) {
			t_annot = string_to_val<sc_core::sc_time>::convertVal(vec[3]);
		}

		unsigned long long addr = std::stoull(str_addr, nullptr, 0);
		std::vector<std::string> data_vec = get_data_vec(s_data);
		if (data_vec.empty()) {
		  return set_error_response(trans, "could not process data vector for transaction");
		}
		unsigned int nr_bytes = (unsigned int)data_vec.size();
		if (trans.get_data_length() != nr_bytes) {
		  return set_error_response(trans, "mismatching transaction length");
		}
		unsigned char* data_ptr = trans.get_data_ptr();

		if (trans.get_address() != addr) {
		  return set_error_response(trans, "mismatching address");
		}
		if (trans.get_streaming_width() < nr_bytes) {
		  return set_error_response(trans, "streaming width<data length not supported");
		}
		if (trans.get_byte_enable_ptr() != 0) {
		  return set_error_response(trans, "byte enable not supported");
		}
		if (cmd == "WRITE") {
		  trans.set_response_status(tlm::TLM_OK_RESPONSE);
		  if (!trans.is_write()) {
		    return set_error_response(trans, "write transaction expected");
		  }
		  for (unsigned int i =0; i< nr_bytes; i++) {
		    unsigned int val;
		    if (convert_to_int(data_vec[i], val)) {
		      if (data_ptr[i] != (unsigned char)val) {
			return set_error_response(trans, "write data mismatch");
		      }
		    } else {
		      return set_error_response(trans, "could not convert transaction data to unsigned int");
		    }
		  }
		}
		else if (cmd == "READ") {
		  trans.set_response_status(tlm::TLM_OK_RESPONSE);
		  if (!trans.is_read()) {
		    return set_error_response(trans, "read transaction expected");
		  }
		  for (unsigned int i =0; i< nr_bytes; i++) {
		    unsigned int val;
		    if (!convert_to_int(data_vec[i], val)) {
		      return set_error_response(trans, "could not convert transaction data to unsigned int");
		    }
		    data_ptr[i] = (unsigned char)val;
		  }
		}
		return;
    }
    
    unsigned int transport_dbg(tlm::tlm_generic_payload& /*trans*/) override {return 0;}
    bool get_direct_mem_ptr(tlm::tlm_generic_payload& /*trans*/, tlm::tlm_dmi& /*dmi_data*/) override {return false;}
    std::string get_mapped_name() const override {return "my_fw_if";}
    void register_bw_direct_mem_if(tlm::tlm_bw_direct_mem_if* /*bwInterface*/) override {}
    void unregister_bw_direct_mem_if(tlm::tlm_bw_direct_mem_if* /*bwInterface*/) override {}
  };

  class my_select_callback : public scml2::memory_select_callback_base {
	  my_fw_if* result;
  public:
	  my_select_callback(my_fw_if* myif) : result(myif) {}
	  mappable_if* execute(tlm::tlm_generic_payload& /*trans*/) override {
		  return result;
	  }
  };


  sc_initiator_socket_player(SOCKET* _obj) : player_base<SOCKET>(_obj)
					, m_select_callback(new my_select_callback(new my_fw_if(this))) {
	  this->use_thread(); // we'll be waiting on our transaction results being processed
	  m_select_callback->ref();
  }

  virtual ~sc_initiator_socket_player() {
	  m_select_callback->unref();
  }

  bool play(std::vector<std::string> vec) override {
	  if (this->m_mode != STUB) {
		  cmds.push_back(vec);
		  transaction_ready.notify(SC_ZERO_TIME);
		  sc_core::wait(transaction_done);
		  return (transaction_match_failure_cnt==0);
	  }
	  return true;
  }

private:
  my_select_callback* m_select_callback{};

  template<typename T>
  bool configure_select_callback(bool set) {
	  if (T* pa = dynamic_cast<T*>(this->m_obj->get_interface())) {
		  pa->set_select_callback(set ? m_select_callback : 0);
		  return true;
	  } else {
		  return false;
	  }
  }

  void player_started() override {
	  if (this->m_mode != STUB) {
		  if (!configure_select_callback<scml2::target_port_adaptor>(true) &&
				  !configure_select_callback<scml2::tlm2_gp_target_adapter<32>>(true) &&
				  !configure_select_callback<scml2::tlm2_gp_target_adapter<64>>(true)) {
			  SCML2_ERROR_TO(this->m_mod, FATAL_ERROR) << "could not register replayer for " << this->m_obj->name() << endl;
		  }
	  }
  }
  void player_finished() override {
	  if (this->m_mode != STUB) {
		  if (!configure_select_callback<scml2::target_port_adaptor>(false) &&
				  !configure_select_callback<scml2::tlm2_gp_target_adapter<32>>(false) &&
				  !configure_select_callback<scml2::tlm2_gp_target_adapter<64>>(false)) {
			  SCML2_ERROR_TO(this->m_mod, FATAL_ERROR) << "could not unregister replayer for " << this->m_obj->name() << endl;
		  }
	  }
  }
};

// -------------------------------------------------------------------------------------
// actual type tracers for sockets
//
// record/replay for target sockets: records model target side, replay model initiator side (checker)
// stub for target side: ignore transactions
//
template<class SOCKET>
class io_trace_sc_target_socket : public type_tracer_base<SOCKET, sc_socket_recorder<SOCKET>, sc_initiator_socket_player<SOCKET> >
{
public:
	// define type tracer string types 'TARGET' : recording 'TARGET' and no-replay ('-')
	io_trace_sc_target_socket() : type_tracer_base<SOCKET, sc_socket_recorder<SOCKET>, sc_initiator_socket_player<SOCKET> >("TARGET", "INITIATOR") {
		this->type_name = typeid(this).name();
	};

	inline bool check_can_record(sc_core::sc_object* _obj) override { return (!strcmp(_obj->kind(), "tlm_target_socket")); }
	inline bool check_can_play(sc_core::sc_object* _obj) override { return (!strcmp(_obj->kind(), "tlm_target_socket") || !strcmp(_obj->kind(), "cwr_tlm_base_target_socket")); }
	inline bool check_can_process(std::vector<std::string> vec) override { return (vec.size() == 3 || vec.size() == 4); }
};

// record/replay for initiator sockets: records model initiator side, replay model target side (send transactions)
// stub for initiator side: emit transactions
//
template<class SOCKET>
class io_trace_sc_initiator_socket : public type_tracer_base<SOCKET, sc_socket_recorder<SOCKET>, sc_target_socket_player<SOCKET> >
{
public:
	// define type tracer string types 'TARGET' : no-recording ('-') and replay for 'TARGET'
	io_trace_sc_initiator_socket() : type_tracer_base<SOCKET, sc_socket_recorder<SOCKET>, sc_target_socket_player<SOCKET> >("INITIATOR", "TARGET") {
		this->type_name = typeid(this).name();
	};

	inline bool check_can_record(sc_core::sc_object* _obj) override { return (!strcmp(_obj->kind(), "tlm_initiator_socket") || !strcmp(_obj->kind(), "cwr_tlm_base_initiator_socket")); }
	inline bool check_can_play(sc_core::sc_object* _obj) override { return (!strcmp(_obj->kind(), "tlm_initiator_socket") || !strcmp(_obj->kind(), "cwr_tlm_base_initiator_socket")); }
	inline bool check_can_process(std::vector<std::string> vec) override { return (vec.size() == 3 || vec.size() == 4); }
};








// -------------------------------------------------------------------------------------
// 7. Tracing objects
//
//
// these are the actual record/replay objects to be instantiated in a model
//
// classes
//		- io_tracers_base: common base class for recorder and player
//						   provides a common registry of type_tracers (not global, want to avoid too long lists: is model specific)
//
//		- io_trace_recorder: the object to instantiate in a model to enable recording
//
//		- io_trace_player: object to instantiate in unit test to be able to replay log files
//
// -------------------------------------------------------------------------------------
class SNPS_VP_API io_tracers_base : public scml2::base::object {
  template<unsigned BW>
  void add_socket_tracers() {
    add_trace_type(new io_trace_sc_target_socket<scml2::ft_target_socket<BW> >());
    add_trace_type(new io_trace_sc_target_socket<tlm::tlm_target_socket<BW> >());
    add_trace_type(new io_trace_sc_initiator_socket<scml2::ft_initiator_socket<BW> >());
    add_trace_type(new io_trace_sc_initiator_socket<scml2::initiator_socket<BW> >());
  }
public:
	io_tracers_base(const scml2::base::object_name& _name, sc_core::sc_module *_mod)
			: scml2::base::object(_name)
			, m_mod(_mod)
	{
	  add_socket_tracers<32>();
	  add_socket_tracers<64>();

		add_signal_type<bool>();
		add_signal_type<bool>();
		add_signal_type<char>();
		add_signal_type<unsigned int>();
		add_signal_type<int>();
		add_signal_type<double>();

		// indicate new tracer is being created
		io_trace::registry* inst = io_trace::registry::getInstance();
		inst->set_tracer(this);
	}

	~io_tracers_base()
	{
		for (auto *i : trace_types) delete(i);
		trace_types.clear();
		if (scml_trace_types) {
			for (auto* i : *scml_trace_types) delete(i);
			scml_trace_types->clear();
			delete(scml_trace_types); // created in registry
		}
		if (scml_obj_to_trace) {
			scml_obj_to_trace->clear();
			delete(scml_obj_to_trace); // created in registry
		}
	}

	// API to add your own trace types to the list (for sc_core::sc_object)
	void add_trace_type(type_tracer_base_if<>* tracer) {
		if (!tracer->add_tracer_to_list(&trace_types)) {
			delete(tracer); // if failed to add we delete immediately, io_tracer takes ownership of the trace_types that get registered
		}
	}

	// API to add your own trace types to the list (for scml2::base::object)
	void add_scml_object_trace_type(type_tracer_base_if<scml2::base::object>* tracer, scml2::base::object* _o) {
		if (!tracer->add_tracer_to_list(scml_trace_types)) {
			delete(tracer); // if failed to add we delete immediately, io_tracer takes ownership of the trace_types that get registered
		}
		scml_obj_to_trace->push_back(_o);
	}

	// copy list of scml_trace_types from registry, transfers ownership to the io_tracer_base
	void set_scml_object_trace_types(std::vector<type_tracer_base_if<scml2::base::object>*>* type_traces, std::vector<scml2::base::object*>* objs) {
		scml_trace_types = type_traces;
		scml_obj_to_trace = objs;
	}

	template<class T>
	void add_signal_type() {
		add_trace_type(new io_trace_sc_in_port<T>());
		add_trace_type(new io_trace_sc_out_port<T>());
	}

	// disable tracing for an object (available for recording and replay)
	// should be used before end-of-elaboration
	// is intended for use in the model code itself (i.e. not through user control)
	template<class OBJ>
	bool disable_tracing_t(OBJ* _obj, std::vector<OBJ*>& _list) {
		if (find_disabled(_obj)) return false;
		SCML2_INFO(TEST_INFO) << "disabling tracing for " << scml2::objects::getBaseName(_obj) << endl;
		_list.push_back(_obj);
		return true;
	}
	bool disable_tracing(sc_core::sc_object* _obj) {
		return disable_tracing_t<sc_core::sc_object>(_obj, disabled_list);
	}
	bool disable_tracing(scml2::base::object* _obj) {
		return disable_tracing_t<scml2::base::object>(_obj, disabled_scml_list);
	}

	template<class OBJ>
	bool find_disabled_t(OBJ* _obj, std::vector<OBJ*>& _list) {
		for (auto* e: _list) {
			if (scml2::objects::getBaseName(e) == scml2::objects::getBaseName(_obj)) return true;
		}
		return false;
	}
	bool find_disabled(sc_core::sc_object* _obj) {
		return find_disabled_t<sc_core::sc_object>(_obj, disabled_list);
	}
	bool find_disabled(scml2::base::object* _obj) {
		return find_disabled_t<scml2::base::object>(_obj, disabled_scml_list);
	}

protected:
	sc_core::sc_module *m_mod;
	std::vector<type_tracer_base_if<>*> trace_types;
	std::vector<type_tracer_base_if<scml2::base::object>*>* scml_trace_types;
	std::vector<sc_core::sc_object*> disabled_list;
	std::vector<scml2::base::object*> disabled_scml_list;
	std::vector<scml2::base::object*>* scml_obj_to_trace; // list of scml objects that should be processed
};




// -------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------
class SNPS_VP_API io_trace_recorder : public io_tracers_base {
public:
	io_trace_recorder(const scml2::base::object_name& _name, sc_core::sc_module *_mod, bool enable, std::string filename, bool add_commands = true)
			: io_tracers_base(_name, _mod)
			, m_filename(filename)
			, m_enable(enable)
			, m_recorders_created(false)
	{
		if (m_enable) {
			init(m_filename);
		}

		if (add_commands) {
		}

	}

	~io_trace_recorder() {
		// recorders and players are deleted per trace_type no need to do that here
		if (m_enable) {
			outfile.close();
		}
	}

	// enable recording for all, should be set before end-of-elaboration
	// is to be used in model code e.g. if tracing is to be enabled after construction via an env.var.
	void enable();

	// disable recording for all, should be set before end-of-elaboration
	// is to be used in model code e.g. if tracing is to be disabled after construction via an env.var.
	void disable() {
		m_enable = false;
	}

	// switch to new output file, to be used when output needs to be split (can be used in unit testing to record per unit test)
	void new_outfile(std::string filename) {
		sync();
		if (m_enable) {
			outfile.close();
			outfile.clear();
		}
		m_filename = filename;
		init(m_filename);
	}

	// command implementation for stop_recording command
	// either stops all recording or only for a specific object
	std::string stop_recording_cmd_impl(const std::vector<std::string>& _args);

	// command implementation for start_recording command
	// either starts all recording or only for a specific object
	std::string start_recording_cmd_impl(const std::vector<std::string>& _args);

	void sync();

	// stop recording for a certain object
	// removes a specific recorder object, therefore should be used after end-of-elab
	// allows to use commands to disable recording
	// can be used at runtime as well, will simply stop recording for the specified object
	bool stop_recording(std::string obj_name);

	// start recording for a certain object
	// adds a specific recorder object, can be used anytime
	// allows to use commands to enable recording
	// can be used at runtime as well, will simply start recording for the specified object
	bool start_recording(std::string obj_name);

	// adds a description callback to the recorder for more readable output
	// searches the recorder with description support associated with the object and adds the callback
	// NOTE: can only be called when recorders are created, so you should call this from the add_descriptions callback (see below)
	template<class PAYLOAD>
	bool add_description_cb(sc_core::sc_object* _obj, io_tracer_descr_if<PAYLOAD>* _cb) {
		for(auto* t: trace_types) {
			if (t->check_obj_type(_obj)) {
				std::string n = scml2::objects::getBaseName(_obj);
				recorder_base_b<sc_core::sc_object>* rec = t->get_recorder_for_obj(n);
				if (rec != nullptr && rec->can_add_description()) {
					recorder_descr_base<PAYLOAD>* r = dynamic_cast<recorder_descr_base<PAYLOAD>* >(rec);
					if (r != nullptr) {
						r->add_description_cb(_cb);
						return true;
					}
					return false;
				}
			}
		}
		return false;
	}

	// callback from which to call the add_description_cb (to ensure this is done at the right time)
	SCML2_BASE_CALLBACK_IGNORE(add_descriptions, void);

private:
	void invoke_callback() override;
	void setup_recorders();
	void init(std::string filename);

	template<class OBJ> bool add_recorder_t(OBJ* _obj, std::vector<type_tracer_base_if<OBJ>*>* _list);
	bool add_recorder(sc_core::sc_object* _obj);
	bool add_recorder(scml2::base::object* _obj);

	// stops all recording
	// in case recording is to be stopped during simulation
	void stop_recording();
private:
	std::ofstream outfile;
	std::string m_filename;
	bool m_enable;
	bool m_recorders_created;
};


// -------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------
class SNPS_VP_API io_trace_player : public io_tracers_base {
public :
	io_trace_player(const scml2::base::object_name& _name, sc_core::sc_module *_mod, bool enable = true, io_player_mode mode = REPLAY)
		: io_tracers_base(_name, _mod)
		, m_line_nr(0)
		, m_test_result(0)
		, m_test_checks(0)
		, m_enable(enable)
		, m_mode(mode)
	{
		m_accumulated_test_time = SC_ZERO_TIME;
		m_time_alignment = sc_core::sc_time(1, SC_SEC); // default test alignment is on 1 sec boundaries

		std::string m_t_name = "fileTraceTesterThread_for_";
		m_t_name += m_mod->basename();
		sc_core::sc_spawn_options opt;
		m_h = sc_core::sc_spawn( sc_bind(&io_trace_player::traceThread, this), sc_gen_unique_name(m_t_name.c_str()), &opt );
	}

	~io_trace_player() {}

	// enable player, should be set before end-of-elaboration
	// is to be used in model code e.g. if player is to be enabled after construction via an env.var.
	void enable() {
		m_enable = true;
	}

	// disable replay for all, should be set before end-of-elaboration
	// is to be used in model code e.g. if player is to be disabled after construction via an env.var.
	void disable() {
		m_enable = false;
	}

	// process trace file, will launch all necessary player methods/threads for replay
	void processTrace(const char* filename, bool do_checks = true);

	// one call to process trace, wait for test to finish and get the result
	unsigned int processTraceAndGetResult(const char* filename) {
		processTrace(filename);
		wait(test_finished);
		return getResult();
	}

	unsigned int processTraceAndGetResult(std::vector<const char*> filenames, bool do_checks = true) {
		unsigned int total = 0;
		for (auto filename: filenames) {
			cout << "IO TRACER: processing: " << filename << endl;
			m_test_result = 0;
			processTrace(filename, do_checks);
			wait(test_finished);
			total += getResult();
			cout << "IO TRACER: finished: " << filename << endl;
		}
		return total;
	}

	// returns number of errors
	unsigned int getResult() {
		return m_test_result;
	}

	// return total number of checks performed
	unsigned int getChecks() {
		return m_test_checks;
	}

	// define start time alignment for consecutive calls to process trace
	void set_test_start_alignment(sc_core::sc_time _time_alignment) {
		m_time_alignment = _time_alignment;
	}

	// get the input list for a specific object (in case replay implementation could be optimized for a certain interface e.g. target sockets)
	std::vector<io_tracer_payload*> *get_input_list(std::string objname);

	// disables a player after creation (allows to get e.g. the input list but then disable all further activity from player
	void disable_player(std::string objname);

	// disable replay based on object name
	void disable_replay(std::string n) {
		if (find_disabled_by_name(n)) return;
		disabled_obj.push_back(n);
	}

private:
	void invoke_callback() override;
	bool find_disabled_by_name(std::string _basename);
	template<class OBJ> void add_players(const std::vector<OBJ*>* obj_list, const std::vector<type_tracer_base_if<OBJ>*>* trace_types, std::vector<type_tracer_base_if<OBJ>*> &players);

private:
	void traceThread();
	std::string trim(const std::string& str);
	void process_inputfile();

public:
	sc_core::sc_event test_finished;
	sc_core::sc_event m_finished_ev;
private:
	std::vector<type_tracer_base_if<>*> sc_object_players;
	std::vector<type_tracer_base_if<scml2::base::object>*> scml_object_players;
	std::vector<io_player_input*> m_inputs;
	std::vector<std::string> disabled_obj;
    std::string m_filename;
	std::ifstream m_inputfile;
	unsigned int m_line_nr;
	sc_core::sc_time m_accumulated_test_time;
	sc_core::sc_time m_time_alignment;
	sc_core::sc_process_handle m_h;
	sc_core::sc_event m_launched_ev;

	unsigned int m_test_result;
	unsigned int m_test_checks;

	bool m_enable;
	io_player_mode m_mode; // for the case the player is intended to run in the same model as where the trace is taken from, if mode = REPLAY a mirror model is assumed
};




// namespaces scml2 and objects
}
}
