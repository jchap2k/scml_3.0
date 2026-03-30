/***************************************************************************
 * Copyright 1996-2025 Synopsys, Inc.
 *
 * This Synopsys software and all associated documentation are proprietary
 * to Synopsys, Inc. and may only be used pursuant to the terms and
 * conditions of a written license agreement with Synopsys, Inc.
 * All other use, reproduction, modification, or distribution of the
 * Synopsys software or the associated documentation is strictly prohibited.
 ***************************************************************************/

#include "scml2_objects/io_tracer.h"

namespace scml2 {
namespace objects {



// -------------------------------------------------------------------------------------
//
// IO_TRACE_RECORDER
//
// -------------------------------------------------------------------------------------

std::string io_trace_recorder::stop_recording_cmd_impl(const std::vector<std::string>& _args) {
	std::stringstream msg;
	msg.str("");
	if (!m_recorders_created) {
		msg << "no recordering enabled yet, cannot stop recording\n";
		return msg.str();
	}
	if (_args.empty()) {
		stop_recording();
	} else if (_args.size() == 1) {
		if (!stop_recording(_args[0])) {
			msg << "failed to remove recorder for " << _args[0] << "\n";
		}
	} else {
		msg << "wrong number of arguments\n";
	}
	return msg.str();
}

std::string io_trace_recorder::start_recording_cmd_impl(const std::vector<std::string>& _args) {
	std::stringstream msg;
	msg.str("");
	if (_args.empty()) {
		if (m_recorders_created) {
			msg << "recordering is already enabled, nothing done\n";
		} else {
			setup_recorders();
		}
	} else if (_args.size() == 1) {
		if (!start_recording(_args[0])) {
			msg << "failed to add recorder for " << _args[0] << "\n";
		}
	} else {
		msg << "wrong number of arguments\n";
	}
	return msg.str();
}


void io_trace_recorder::enable() {
	if (!m_enable && !outfile.is_open()) {
		init(m_filename);
	}
	m_enable = true;
}

void io_trace_recorder::invoke_callback() {
	if (m_enable) {
		 setup_recorders();
	}
	// indicate new tracer is finalized
	io_trace::registry* inst = io_trace::registry::getInstance();
	inst->clear_tracer();
}

void io_trace_recorder::setup_recorders() {
	for (auto* obj : m_mod->get_child_objects()) {
		add_recorder(obj);
	}
	for (auto* obj : *scml_obj_to_trace) {
		add_recorder(obj);
	}
	add_descriptions(); // call callback to allow users to add description cb to recorders
	m_recorders_created = true;
}

void io_trace_recorder::init(std::string filename) {
	outfile.open(filename.c_str(), ios::trunc);
	if (!outfile.is_open()) {
		SCML2_ERROR(CONFIGURATION_ERROR) << "could not open " << filename << " as logging file for " << m_mod->name() << endl;
		return;
	}
	outfile << "# START RECORDING for " << get_name() << endl;
}

void io_trace_recorder::sync() {
	if (outfile.is_open()) {
	  outfile.flush();
	}
}


bool io_trace_recorder::stop_recording(std::string obj_name) {
	bool removed = false;
	for (auto* obj : m_mod->get_child_objects()) {
		if (obj->basename() == obj_name) {
			for (auto* t : trace_types) {
				if (t->removeRecorder(obj)) {
					removed = true;
					break;
				}
			}
			break;
		}
	}
	if (removed) return removed;
	for (auto* obj : *scml_obj_to_trace) {
		if (scml2::objects::getBaseName(obj) == obj_name) {
			for (auto* t : *scml_trace_types) {
				if (t->removeRecorder(obj)) {
					removed = true;
					break;
				}
			}
		}
	}
	if (!removed) {
		SCML2_ERROR(CONFIGURATION_ERROR) << "stop recording failed for " << obj_name << ", in instance: " << m_mod->name() << endl;
	}
	return removed;
}


bool io_trace_recorder::start_recording(std::string obj_name) {
	for (auto* obj : m_mod->get_child_objects()) {
		if (scml2::objects::getBaseName(obj) == obj_name) return add_recorder(obj);
	}
	for (auto* obj : *scml_obj_to_trace) {
		if (scml2::objects::getBaseName(obj) == obj_name) return add_recorder(obj);
	}
	return false;
}


template<class OBJ>
bool io_trace_recorder::add_recorder_t(OBJ* _obj, std::vector<type_tracer_base_if<OBJ>*>* _list) {
	for (auto* t : *_list) {
		if (find_disabled(_obj)) {
			SCML2_MODEL_INTERNAL(LEVEL1) << "Recording is forced disabled for " << scml2::objects::getBaseName(_obj) << endl;
			return false;
		}
		if (t->createRecorder(_obj, outfile)) {
			SCML2_MODEL_INTERNAL(LEVEL1) << "Adding recording for " << scml2::objects::getBaseName(_obj) << endl;
			return true;
		}
	}
	return false;
}
bool io_trace_recorder::add_recorder(sc_core::sc_object* _obj) {
	return add_recorder_t<sc_core::sc_object>(_obj, &trace_types);
}
bool io_trace_recorder::add_recorder(scml2::base::object* _obj) {
	return add_recorder_t<scml2::base::object>(_obj, scml_trace_types);
}


void io_trace_recorder::stop_recording() {
	for (unsigned int i = 0; i < trace_types.size(); i++) {
		trace_types[i]->deleteRecorders();
	}
	for (unsigned int i = 0; i < scml_trace_types->size(); i++) {
		(*scml_trace_types)[i]->deleteRecorders();
	}
}



// -------------------------------------------------------------------------------------
//
// IO_TRACE_PLAYER
//
// -------------------------------------------------------------------------------------

void io_trace_player::processTrace(const char* filename, bool do_checks) {
	m_line_nr = 0;
	m_test_result = 0;
	m_test_checks = 0;
	m_filename = filename;
	m_inputfile.open(filename);
	if (!m_inputfile.is_open()) {
		SCML2_ERROR(CONFIGURATION_ERROR) << "could not open log file for test" << endl;
		m_test_result++;
		test_finished.notify(SC_ZERO_TIME);
		return;
	}
	for (auto* i : sc_object_players ) {
	  i->player_started();
		i->enable_checks(do_checks);
	}
	for (auto* i : scml_object_players ) {
	  i->player_started();
		i->enable_checks(do_checks);
	}
	process_inputfile();
	m_launched_ev.notify();
}


template<class OBJ>
void io_trace_player::add_players(const std::vector<OBJ*>* obj_list, const std::vector<type_tracer_base_if<OBJ>*>* trace_types, std::vector<type_tracer_base_if<OBJ>*> &players) {
	for (OBJ* obj : *obj_list) {
		if (find_disabled(obj)) {
			disabled_obj.push_back(scml2::objects::getBaseName(obj));
			continue;
		}
		for (auto* t : *trace_types) {
			if (t->createPlayer(obj, m_mod, &m_finished_ev, m_mode)) {
				bool found = false;
				for (type_tracer_base_if<OBJ>* p : players ) {
					if (p == t) {
						found = true;
						break;
					}
				}
				if (!found) players.push_back(t);
				break;
			}
		}
	}
}


void io_trace_player::invoke_callback() {
	if (!m_enable) {
		return;
	}
	// create players
	add_players<sc_core::sc_object>(&(m_mod->get_child_objects()), &trace_types, sc_object_players);
	add_players<scml2::base::object>(scml_obj_to_trace, scml_trace_types, scml_object_players);

	// indicate new tracer is finalized
	io_trace::registry* inst = io_trace::registry::getInstance();
	inst->clear_tracer();
}


void io_trace_player::traceThread() {
	while(1) {
		wait(m_launched_ev);
		bool done = false;
		while (!done) {
			wait(m_finished_ev);
			done = true;
			for (auto* p : sc_object_players ) {
				if (!p->allPlayersDone()) { done = false; break; }
			}
			if (!done) continue;
			for (auto* p: scml_object_players) {
				if (!p->allPlayersDone()) { done = false; break; };
			}
		}
		SCML2_INFO(TEST_INFO) << "TEST DONE" << endl;
		for (auto* p : sc_object_players) {
			m_test_result += p->get_errors();
			m_test_checks += p->get_checks();
		}
		for (auto* p: scml_object_players) {
			m_test_result += p->get_errors();
			m_test_checks += p->get_checks();
		}
		if (m_test_result != 0) {
			SCML2_INFO(TEST_INFO) << "TEST: file trace with filename: " << m_filename << " FAILED with in total " << m_test_result << " errors out of " << m_test_checks << " checks." << endl;
		} else {
			SCML2_INFO(TEST_INFO) << "TEST: file trace with filename: " << m_filename << " PASSED with in total " << m_test_checks << " checks." << endl;
		}
		test_finished.notify(SC_ZERO_TIME);
	}
}

bool io_trace_player::find_disabled_by_name(const std::string n) {
	for (auto& i : disabled_obj) if (i == n) return true;
	return false;
}


std::string io_trace_player::trim(const std::string& str) {
	const std::string whitespace = " \t";

    size_t first = str.find_first_not_of(whitespace);
    if (std::string::npos == first) {
        return str;
    }
    size_t last = str.find_last_not_of(whitespace);
    return str.substr(first, (last - first + 1));
}


void io_trace_player::process_inputfile() {
	// align starting time for the test with chosen test spacing
	sc_core::sc_time now = sc_core::sc_time_stamp();
	if (now > SC_ZERO_TIME) {
		m_accumulated_test_time =  now + (m_time_alignment- (now%m_time_alignment));
		wait(m_accumulated_test_time- now);
	}

	std::string full_line = "";
	std::string line, tok;
	std::stringstream fline, sline;
	std::string type, objname;
	sc_core::sc_time t;
	std::vector<std::string> vec;
	while(std::getline(m_inputfile, full_line)) {
		m_line_nr++;
		// skip empty lines and comment lines
		if (full_line.length() == 0 || full_line[0] == '#') continue;
		full_line.erase(std::remove(full_line.begin(), full_line.end(), '\r'), full_line.end()); // remove carriage return chars

		// strip any end-of-line comments
		fline.str(full_line);
		fline.clear();
		std::getline(fline, line, '#');

		// split line in tokens
		sline.str(line);
		sline.clear();
		vec.clear();
		unsigned int i = 0;
		while(std::getline(sline, tok, ',')) {
			switch(i) {
			case 0:
				t = string_to_val<sc_core::sc_time>::convertVal(tok);
				break;
			case 1:
				type = tok;
				break;
			case 2:
				objname = tok;
				break;
			default:
				vec.push_back(trim(tok)); // remove leading and trailing whitespace
				break;
			}
			i++;
		}
		if (i < 4) {
			SCML2_ERROR(GENERIC_ERROR)  << "insufficient arguments: failed to process line " << m_line_nr << ": " << line << endl;
			m_test_result++;
		}
		io_player_input* cur_ip = nullptr;
		for (auto ip = m_inputs.begin(); ip != m_inputs.end(); ++ip) {
			if ((*ip)->type == type && (*ip)->objname == objname) {
				cur_ip = *ip;
				std::rotate(m_inputs.begin(), ip, ip + 1); // move to front of vector so that we get a recently used first list
				break;
			}
		}
		if (cur_ip == nullptr) {
			cur_ip = new io_player_input(type, objname);
			m_inputs.push_back(cur_ip);
		}
		io_tracer_payload* inp_line = new io_tracer_payload(m_line_nr, vec, t);
		cur_ip->lines.push_back(inp_line);
	}
	for (auto ip: m_inputs) {
		bool ok = false;
		for (auto* p : sc_object_players) {
			if (p->process(ip, m_accumulated_test_time, m_mode)) {
				ok = true;
				break;
			}
		}
		if (!ok) for (auto* p : scml_object_players) {
			if (p->process(ip, m_accumulated_test_time, m_mode)) {
				ok = true;
				break;
			}
		}
		if (!ok) {
			if (!find_disabled_by_name(ip->objname)) {
				SCML2_ERROR(GENERIC_ERROR)  << "unable to replay: failed to process input for object " << ip->objname << endl;
				m_test_result++;
			}
		}
	}
	m_inputfile.close();
}

// get the input list for a specific object (in case replay implementation could be optimized for a certain interface e.g. target sockets)
std::vector<io_tracer_payload*> *io_trace_player::get_input_list(std::string objname) {
	for (auto *p: sc_object_players) {
		std::vector<io_tracer_payload*> *i_list = p->get_input_list(objname);
		if (i_list) {
			return i_list;
		}
	}
	for (auto *p: scml_object_players) {
		std::vector<io_tracer_payload*> *i_list = p->get_input_list(objname);
		if (i_list) {
			return i_list;
		}
	}
	return nullptr;
}

// disables a player after creation (allows to get e.g. the input list but then disable all further activity from player
void io_trace_player::disable_player(std::string objname) {
	for (auto *p: sc_object_players) {
		if (p->disable_player(objname)) {
			return;
		}
	}
	for (auto *p: scml_object_players) {
		if (p->disable_player(objname)) {
			return;
		}
	}
	return;
}


// -------------------------------------------------------------------------------------
//
// IO_TRACE::REGISTRY
//
// -------------------------------------------------------------------------------------
namespace io_trace {

void registry::add(t_type_tracer* _t, scml2::base::object* _o) {
	if (!check_module()) {
		cout << "IO_TRACER WARNING : no parent module found for " << _o->get_name() << ", tracing will be enabled with first next tracer that is created" << endl;
	}
	if (m_tracer) {
		m_tracer->add_scml_object_trace_type(_t, _o);
	} else {
		if (!_t->add_tracer_to_list(m_trace_types)) {
			delete(_t);
		}
		m_trace_objects->push_back(_o);
	}
}

bool registry::set_tracer(io_tracers_base* _tracer) {
	check_module(); // check if we're still in the same module
	if (_tracer == nullptr) {
		cout << "IO_TRACER ERROR : trying to set new tracer with nullptr, ignored" << endl;
		return false;
	}
	m_tracer = _tracer;
	m_tracer->set_scml_object_trace_types(m_trace_types, m_trace_objects);
	new_lists();
	m_list_is_transferred = true;
	return true;
}

void registry::clear_tracer() { m_tracer = nullptr; }

// check for parent module, if new module clear list, return true if parent is different from previous registry activity...
bool registry::check_module() {
	const sc_core::sc_module* mod = scml2::objects::getParentModule(sc_core::sc_get_current_object());
	if (!mod) return false; // no module found
	if (m_module !=nullptr && mod != m_module) {
		// new module, clear...
		if (!m_list_is_transferred) {
			// if list wasn't transferred to module then registry is still owner of the list.. clear it..
			cleanup_lists();
		}
		new_lists();
		m_tracer = nullptr;
		m_list_is_transferred = false;
		m_module = mod;
		return true;
	} else if (m_module == nullptr) {
		m_module = mod;
	}
	return true;
}

void registry::cleanup_lists() {
	for (unsigned int i = 0; i < m_trace_types->size(); i++) {
		delete((*m_trace_types)[i]);
	}
	m_trace_types->clear();
	delete(m_trace_types);
	m_trace_objects->clear();
	delete(m_trace_objects);
}

void registry::new_lists() {
	m_trace_types = new t_trace_list;
	m_trace_objects = new (std::vector<scml2::base::object*>);

}

registry *registry::instance=nullptr;

} // namespace io_trace

// namespaces scml2 and objects
}
}




// -------------------------------------------------------------------------------------
//
// MONITOR and IPT support for TLM target port trace
//
// Not available in source code kit
//
// -------------------------------------------------------------------------------------




