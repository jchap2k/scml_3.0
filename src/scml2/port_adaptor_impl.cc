/***************************************************************************
 * Copyright 1996-2025 Synopsys, Inc.
 *
 * This Synopsys software and all associated documentation are proprietary
 * to Synopsys, Inc. and may only be used pursuant to the terms and
 * conditions of a written license agreement with Synopsys, Inc.
 * All other use, reproduction, modification, or distribution of the
 * Synopsys software or the associated documentation is strictly prohibited.
 ***************************************************************************/

#include <scml2/port_adaptor_impl.h>
/* Export extension related symbols */
#define SNPS_VP_STATIC

#include <iostream>
#include <scml2/tagged_message_macros.h>

namespace scml2
{
int port_adaptor_impl::gMaxThreadsInPool = 10;
port_adaptor_impl::tWorkerThreads* port_adaptor_impl::gWorkerThreads = 0;


payload_info::payload_info(scml2::mem_manager<payload_info>*) :
  pTrans(NULL),
  m_id(0),
  m_beat_size(0),
  m_beat_count(0),
  m_beat_index(0),
  m_tlm_phase(tlm::BEGIN_REQ),
  m_lt_mode_txn(false),
  m_waiting_for_b_transport(false),
  m_waiting_for_complete(false),
  m_complete(false)
{
}

void payload_info::reset()
{
  m_outstanding_status = payload_info::OUTSTANDING_NOT_HIT;
  m_beat_index = 0;
  m_tlm_phase = tlm::BEGIN_REQ;

  for (unsigned int i = 0; i < payload_info::TRANS_DIR; i++) {
    m_num_resp_expected[i]  = 0;
    m_num_data_expected[i]  = 0;
    m_num_resp_count[i]     = 0;
    m_num_data_count[i]     = 0;
  }

  m_pending.clear();
  for (unsigned int i = 0; i < MAX_FT_CHANNELS; ++i) {
    m_channel_queued[i] = false;
  }

  m_current.m_event = scml2::CBK_EVENT_NONE;
  m_current.m_time = SC_ZERO_TIME;
  m_lt_mode_txn = false;
  m_waiting_for_b_transport = false;
  m_waiting_for_complete = false;
  m_complete = false;
}

void payload_info::acquire(port_adaptor_impl* adaptor, state_pool_type* pool, tlm::tlm_generic_payload& trans)
{
  pTrans = &trans;

  // set mem_manager if not already, and add custom extension so we can optionally
  // block until all references to this payload have been released
  if (!trans.has_mm()) {
    pTrans->set_mm(this);
    pTrans->set_auto_extension(&m_mm_ext);
  }

  pTrans->acquire();

  GET_FT_EXT_ATTR(&trans, scml2::trans_id_extension, unsigned int, trans_id);
  m_id = trans_id;

  m_beat_size = adaptor->get_burst_size(trans);
  m_beat_count = adaptor->get_burst_length(trans);
  m_pending.set_mm(pool);
  reset();
}

void payload_info::release()
{
  pTrans->release();
  pTrans = NULL;

}

bool payload_info::is_state_pending(callback_event_enum state) const
{
  for (deque_mm<state_time_pair>::node* pTemp = m_pending.m_pHead; pTemp != NULL; pTemp = pTemp->m_pNext) {
    if (pTemp->m_obj.m_event == state)
      return true;
  }
  return false;
}

unsigned int payload_info::count_pending(callback_event_enum state) const
{
  unsigned int count = 0;
  for (deque_mm<state_time_pair>::node* pTemp = m_pending.m_pHead; pTemp != NULL; pTemp = pTemp->m_pNext) {
    if (pTemp->m_obj.m_event == state)
      ++count;
  }
  return count;
}

//-----------------------------------------------------------------------------


port_adaptor_impl::port_adaptor_impl(const std::string& /*name*/)
 : m_clk_port(NULL)
 , m_clk(NULL)
 , m_clk_period(SC_ZERO_TIME)
 , m_clk_period_change_pending(false)
 , m_accessor(NULL)
 , m_use_return_path(true)
 , m_inside_api_call(0)
 , m_inside_process_state_machine(false)
 , m_nb_transport_payload(NULL)
 , m_returning_bw_state(false)
 , m_total_txns(0)
 , m_max_active(0)
 , m_current_active(0)
 , m_active_reads(0)
 , m_active_writes(0)
 // attribute default values
 , m_invoke_timing_callbacks(port_adaptor::eInvokeCbksForFT)
 , m_read_capacity(1)
 , m_write_capacity(1)
 , m_total_capacity(0)
 , m_in_cb_peq(this, &port_adaptor_impl::in_cb_peq)
 , m_pool(*this)

 , m_has_callbacks(false)
 , m_protocol_id(0)
{
  static bool trace_enabled = !!std::getenv("SNPS_PORT_ADAPTOR_TRACE_ALL");
  m_trace_enabled = trace_enabled;

  // link attributes
  m_attributes["trace_enabled"] = (int*)&m_trace_enabled;
  m_attributes["read_capacity"] = (int*)&m_read_capacity;
  m_attributes["write_capacity"] = (int*)&m_write_capacity;
  m_attributes["total_capacity"] = (int*)&m_total_capacity;
  m_attributes["use_q_for_outstanding"] = (int*)&m_use_q_for_outstanding;
  m_attributes["active_reads"] = NULL;
  m_attributes["active_writes"] = NULL;
  m_attributes["pending_reads"] = NULL;
  m_attributes["pending_writes"] = NULL;
  m_attributes["invoke_timing_cbks"] = (int*)&m_invoke_timing_callbacks;
  m_attributes["consume_annotated_time"] = (int*)&m_consume_annotated_time;
  m_attributes["support_unique_id_generation"] = (int*)&m_support_unique_id_generation;
  m_attributes["assert_on_pending_trans"] = (int*)&m_assert_on_pending_trans;
  m_attributes["try_dmi_for_b_transport"] = (int*)&m_try_dmi_for_b_transport;

  static bool assert_on_pending_trans = !!std::getenv("SNPS_PORT_ADAPTOR_ASSERT_ON_PENDING_TRANS");
  m_assert_on_pending_trans = assert_on_pending_trans;

  static bool use_ispex = !!std::getenv("SNPS_PORT_ADAPTOR_USE_ISPEX");
  m_use_ispex = use_ispex;
  if(m_use_ispex) {
    m_accessor = new tlm_utils::instance_specific_extension_accessor;
  }

  for (int i = 0; i < MAX_CBK_EVENT_ID_E; i++) {
    m_eventId2ProtoState[i] = scml2::INVALID_PROTOCOL_STATE;
    m_callbacks[i] = NULL;
  }
  m_eventId2ProtoState[AFTER_TRANS_END] = 0;

  {
    sc_core::sc_spawn_options opts;
    opts.spawn_method();
    opts.set_sensitivity(&m_process_state_machine_event);
    opts.dont_initialize();
    sc_core::sc_spawn(sc_bind(&port_adaptor_impl::process_state_machine, this), sc_core::sc_gen_unique_name("process_state_machine"), &opts);
  }

}

port_adaptor_impl::~port_adaptor_impl()
{
  delete m_accessor;
}

void port_adaptor_impl::my_mem_manager::free(tlm::tlm_generic_payload *trans) {
  scml2::mem_manager<scml2::ft_generic_payload>::free(trans);
}

void port_adaptor_impl::my_mem_manager::notify_observers(tlm::tlm_generic_payload &trans) {
  /* Notify all observers that transaction object is freed now (m_ref_cnt is 0) */
  for(auto it = m_observers.begin(); it != m_observers.end(); ++it) {
    auto &func = *it;
    (*func)(trans);
  }
}

std::string
port_adaptor_impl::event_name(callback_event_enum event_id) const
{
  /* ToDo create separate event name for agft posted enums */
  switch (event_id) {
    case CBK_EVENT_NONE:        return "EVENT_NONE";
    case WR_ADDR_START:         return "WR_ADDR_START";
    case WR_ADDR_END:           return "WR_ADDR_END";
    case WR_DATA_START:         return "WR_DATA_START";
    case WR_DATA_END:           return "WR_DATA_END";
    case WR_DATA_LAST_START:    return "WR_DATA_LAST_START";
    case WR_DATA_LAST_END:      return "WR_DATA_LAST_END";
    case WR_RSP_START:          return "WR_RSP_START";
    case WR_RSP_END:            return "WR_RSP_END";
    case RD_ADDR_START:         return "RD_ADDR_START";
    case RD_ADDR_END:           return "RD_ADDR_END";
    case RD_DATA_START:         return "RD_DATA_START";
    case RD_DATA_END:           return "RD_DATA_END";
    case RD_DATA_LAST_START:    return "RD_DATA_LAST_START";
    case RD_DATA_LAST_END:      return "RD_DATA_LAST_END";
    default:
    {
      std::ostringstream os;
      os << "Unknown(" << event_id << ")";
      return os.str();
    }
  }
}

std::string
port_adaptor_impl::state_name(scml2::payload_state state) const
{
  std::ostringstream os;
  os << "Unmapped(" << event_name(state.m_event) << ")";
  return os.str();
}


void
port_adaptor_impl::before_end_of_elaboration()
{
  init_outstanding_q();

  // enumerate ft_channels
  assert(m_channels.size() <= MAX_FT_CHANNELS);
  for (size_t i = 0; i < m_channels.size(); ++i) {
    m_channels[i]->m_chan_id = (unsigned int)i;
  }

}


void
port_adaptor_impl::set_attribute(const std::string& name, int value)
{
  attribute_map::iterator itr = m_attributes.find(name);
  if (itr == m_attributes.end()) {
    PORT_ADAPTOR_ERROR("set_attribute : unknown attribute name " << name);
  }
  else if (itr->second) {
      *(itr->second) = value;
  }
  // ignore registered NULL attributes
}

void
port_adaptor_impl::execute_command(const std::string& cmd, void* arg1, void* /*arg2*/, void* /*arg3*/)
{
    typedef enum {
        REGISTER_TRANS_FREE_OBSERVER,
        REINITIALIZE_OUTSTANDING_Q
    } cmd_type;

    static std::unordered_map<std::string, cmd_type> s_cmd_map = {
        {"REGISTER_TRANS_FREE_OBSERVER",  REGISTER_TRANS_FREE_OBSERVER},
        {"REINITIALIZE_OUTSTANDING_Q",    REINITIALIZE_OUTSTANDING_Q},
    };

    auto it = s_cmd_map.find(cmd);
    if(it == s_cmd_map.end()) {
        return;
    }

    cmd_type c = it->second;

    switch(c) {
        case REGISTER_TRANS_FREE_OBSERVER:
            if(arg1) {
                m_pool.m_observers.insert((reinterpret_cast<trans_free_observer_func*>(arg1)));
            }
        break;

        case REINITIALIZE_OUTSTANDING_Q:
            init_outstanding_q();
        break;

        default:
        break;
    }
}

int
port_adaptor_impl::get_attribute(const std::string& name)
{
  attribute_map::const_iterator itr = m_attributes.find(name);
  if (itr == m_attributes.end()) {
    PORT_ADAPTOR_ERROR("get_attribute : unknown attribute name " << name);
  }
  else if (itr->second) {
    return *(itr->second);
  }
  else {
    // check for virtual attributes
    if (name.compare("active_reads") == 0) {
      return (int)m_active_reads;
    }
    else if (name.compare("active_writes") == 0) {
      return (int)m_active_writes;
    }
  }
  return 0;
}

bool
port_adaptor_impl::set_timing_callback(callback_event_enum event_id, timing_callback_base* cb)
{
  assert((unsigned int)event_id < MAX_CBK_EVENT_ID_E);

  if ((event_id == CBK_EVENT_NONE) || ((unsigned int)event_id >= MAX_CBK_EVENT_ID_E)) {
    PORT_ADAPTOR_ERROR("Cannot set timing callback: invalid event: " << event_name(event_id));
    return false;
  }

  unsigned int proto_state = m_eventId2ProtoState[event_id];

  if (proto_state == scml2::INVALID_PROTOCOL_STATE)  {
    PORT_ADAPTOR_WARNING("Cannot set timing callback: no protocol state maps to event " << event_name(event_id) << " for this port adaptor");
    return false;
  }

  m_callbacks[event_id] = cb;

  // keep track if any callbacks have been registered
  if (cb) {
    m_has_callbacks = true;
  }
  else {
    // callback has been cleared, check if any still remain
    bool has_callbacks = false;
    for (auto cbk : m_callbacks) {
      if (cbk) {
        has_callbacks = true;
      }
    }
    m_has_callbacks = has_callbacks;
  }

  return true;
}


scml2::callback_event_enum
port_adaptor_impl::get_event_id(unsigned int protocol_state) const
{
  /*if (protocol_state < m_protoState2EventId.size()) {
    return m_protoState2EventId[protocol_state];
  }
  else */{
    // fall back to slow search if 1-1 mapping not possible
    for (int i = WR_ADDR_START; i < MAX_CBK_EVENT_ID_E; ++i) {
      if (m_eventId2ProtoState[i] == protocol_state) {
        return (scml2::callback_event_enum)i;
      }
    }
    return MAX_CBK_EVENT_ID_E;
  }
}

bool
port_adaptor_impl::has_callback(scml2::callback_event_enum event_id) const
{
  return ((event_id < MAX_CBK_EVENT_ID_E) && (m_callbacks[event_id] != NULL));
}

void
port_adaptor_impl::invoke_fwd_timing_callback(payload_info* p_info,
                                              payload_state state,
                                              sc_core::sc_time& t)
{
  // invoke timing callback for forwarded phases; user cannot change protocol
  // state but may want to log something or adjust timing
  tlm::tlm_sync_enum return_status = invoke_timing_callback(p_info, state, t, true);
  if (return_status != tlm::TLM_UPDATED) {
    PORT_ADAPTOR_ERROR("callback for " << event_name(state.m_event) << " did not return tlm::TLM_UPDATED");
    assert(false);
  }
}

tlm::tlm_sync_enum
port_adaptor_impl::invoke_timing_callback(payload_info* p_info,
                                          payload_state state,
                                          sc_core::sc_time& t,
                                          bool is_fwd_cb)
{
  if (!can_invoke_timing_callbacks(p_info->m_lt_mode_txn)) {
    return tlm::TLM_UPDATED;
  }


  if ((state.m_event >= MAX_CBK_EVENT_ID_E) || (m_callbacks[state.m_event] == NULL)) {
    PORT_ADAPTOR_TRACE(p_info->pTrans, t, event_name(state.m_event) << " : default callback");
    return tlm::TLM_UPDATED;
  }
  else {
    PORT_ADAPTOR_TRACE(p_info->pTrans, t, event_name(state.m_event));
    /* coverity[leaked_storage] */
    SET_FT_EXT_ATTR(p_info->pTrans, scml2::beat_index_extension, p_info->m_beat_index);

    tlm::tlm_generic_payload* pTrans = p_info->pTrans;


    if(m_consume_annotated_time && t != sc_core::SC_ZERO_TIME && is_fwd_cb == false) {
      /* Consume annotated time before triggering incoming callback */
      tlm::tlm_phase phase = tlm::tlm_phase_enum((unsigned)state.m_event);
      pTrans->acquire();
      m_in_cb_peq.notify(*pTrans, phase, t);
      return tlm::TLM_ACCEPTED;
    }

    const sc_time orig_t(t);
    tlm::tlm_sync_enum status = m_callbacks[state.m_event]->execute(*(pTrans), t);
    if (t != orig_t) {          // re-align clock if user model has altered it
      align_to_clock_edge(t);
    }

    return status;
  }
}

void
port_adaptor_impl::in_cb_peq(tlm::tlm_generic_payload &trans, const tlm::tlm_phase &phase) {
    payload_info* p_info = nullptr;
    get_payload_info(p_info, &trans);
    sc_core::sc_time t;
    tlm::tlm_sync_enum status = m_callbacks[(unsigned)phase]->execute(trans, t);
    if (t != sc_core::SC_ZERO_TIME) {          // re-align clock if user model has altered it
      align_to_clock_edge(t);
    }
    if (p_info && p_info->pTrans == &trans && status != tlm::TLM_ACCEPTED) {
      advance_state(p_info, t);
    }
    else if(!p_info) {
        clear_payload_info(p_info, &trans);
    }
    trans.release();
}

unsigned int
port_adaptor_impl::get_burst_size(tlm::tlm_generic_payload& trans) const
{
  GET_FT_EXT_ATTR((&trans), scml2::burst_size_extension, unsigned int, burst_size);
  unsigned bus_width_bytes = get_bus_width() / 8;
  if(burst_size > bus_width_bytes) {
      PORT_ADAPTOR_WARNING("burst_size:" << burst_size << " for trans:" << &trans << " is greater than bus width:" << bus_width_bytes << ". Resetting to " << bus_width_bytes);
      SET_FT_EXT_ATTR((&trans), scml2::burst_size_extension, bus_width_bytes);
      return bus_width_bytes;
  }
  else if(burst_size == 0) {
      SET_FT_EXT_ATTR((&trans), scml2::burst_size_extension, bus_width_bytes);
  }
  return (burst_size == 0) ? bus_width_bytes : burst_size;
}

unsigned int
port_adaptor_impl::get_burst_length(tlm::tlm_generic_payload& trans) const
{
  unsigned int burst_size = get_burst_size(trans);

  if (is_fixed_burst(trans)) {
    // fixed burst so can only transfer burst_size-alignment_offset bytes at a time
    burst_size = burst_size - (trans.get_address() % burst_size);
    return (trans.get_data_length() + burst_size - 1) / burst_size;
  }
  else if (supports_unaligned_beats()) {
    return (trans.get_data_length() + burst_size - 1) / burst_size;
  }
  else {
    // unaligned accesses cause extra beats
    return ((trans.get_address() % burst_size) + trans.get_data_length() + burst_size - 1) / burst_size;
  }
}

unsigned int
port_adaptor_impl::get_trans_id(tlm::tlm_generic_payload& trans) const
{
  payload_info* p_info = NULL;
  const_cast<port_adaptor_impl*>(this)->get_payload_info(p_info, &trans);
  return p_info ? p_info->m_id : 0x0;
}

//-----------------------------------------------------------------------------

sc_time
port_adaptor_impl::adjust_request_time(payload_info* p_info, const sc_time& t)
{
  if(m_use_q_for_outstanding) {
    if(p_info->m_outstanding_status == payload_info::OUTSTANDING_NA) {
      /* Outstanding logic not applicable to this trans */
      return t;
    }

    p_info->m_outstanding_status = payload_info::OUTSTANDING_NOT_HIT;
    auto *q = get_outstanding_q(p_info->pTrans->get_command());
    if(!q) {
      return t;
    }
    else if(q->empty()) {
      /* No outstanding entries available */
      p_info->m_outstanding_status = payload_info::OUTSTANDING_HIT;
      return t;
    }

    /* Adjust annotated time as per the first entry available */
    uint64_t entry_time = q->front();
    q->pop();
    uint64_t abs_time = sc_core::sc_time_stamp().value() + t.value();
    if(abs_time <= entry_time) {
      /* New outstanding entry is available at a future time. Adjust t accordingly */
      return sc_core::sc_time::from_value(t.value() + entry_time - abs_time);
    }
    return t;
  }

  /* Use old logic for maintaining outstanding requests */
  std::vector<sc_time>* p_complete_times;
  unsigned int capacity, active;

  if (m_total_capacity) {
    capacity = m_total_capacity;
    active = m_active_reads + m_active_writes;
    p_complete_times = &m_read_complete_times;
  }
  else if (p_info->pTrans->is_write()) {
    capacity = m_write_capacity;
    active = m_active_writes;
    p_complete_times = &m_write_complete_times;
  }
  else {
    capacity = m_read_capacity;
    active = m_active_reads;
    p_complete_times = &m_read_complete_times;
  }

  unsigned int remaining_capacity = capacity - active;
  if ((capacity == (unsigned int)-1) || (remaining_capacity == 0)) {
    // no need to adjust request time; capacity is unlimited, or address channel is locked
    return t;
  }

  sc_time delay(t);
  delay += sc_time_stamp();

  // sorted vector of txn complete times: drop any that are older than this request
  p_complete_times->erase(p_complete_times->begin(), std::lower_bound(p_complete_times->begin(), p_complete_times->end(), delay));

  // if vector size is still >= remaining_capacity, adjust request time to be complete[n] where n = (size - remaining_capacity)
  if (!p_complete_times->empty() && p_complete_times->size() >= remaining_capacity) {
    delay = (*p_complete_times)[p_complete_times->size() - remaining_capacity];
    PORT_ADAPTOR_TRACE(p_info->pTrans, t, "request time adjusted to: " << delay);
  }

  delay -= sc_time_stamp();

  return delay;
}

//-----------------------------------------------------------------------------

void
port_adaptor_impl::on_txn_complete(payload_info* p_info, const sc_time& t)
{
  end_watchdog_transaction(*p_info, t);
  p_info->m_complete = true;
  bool was_outstanding_hit = false;
  if (p_info->pTrans->is_write()) {
    was_outstanding_hit = !can_accept_write();
    --m_active_writes;
  }
  else if(p_info->pTrans->is_read()) {
    was_outstanding_hit = !can_accept_read();
    --m_active_reads;
  }

  if(m_use_q_for_outstanding) {
    if(p_info->m_outstanding_status != payload_info::OUTSTANDING_NA) {
      tlm::tlm_command cmd = p_info->pTrans->get_command();
      auto *q = get_outstanding_q(cmd);
      /* 1 outstanding tracker is now available. Add this entry in q */
      push_to_outstanding_q(q, sc_core::sc_time_stamp().value() + t.value());
    }
    PORT_ADAPTOR_TRACE(p_info->pTrans, t, "txn complete : reads: " << m_active_reads << ", writes:" << m_active_writes << ", req locked:" << m_req_channel.is_locked()
      << "( R:" << m_outstanding_q[OUTSTANDING_TYPE_RD].size()
      << " W:" << m_outstanding_q[OUTSTANDING_TYPE_WR].size()
      << " T:" << m_outstanding_q[OUTSTANDING_TYPE_TOTAL].size()
      << " )"
    );
  }
  else {
    PORT_ADAPTOR_TRACE(p_info->pTrans, t, "txn complete : reads: " << m_active_reads << ", writes:" << m_active_writes << ", req locked:" << m_req_channel.is_locked());
    // Legacy outstanding logic
    // check if another read or write can now be issued
    if (m_req_channel.is_locked() && was_outstanding_hit &&
        can_accept_read()) {
      m_req_channel.unlock(t);
      PORT_ADAPTOR_TRACE(p_info->pTrans, t, "unlock request channel. free_time:" << m_req_channel.free_time());
    }
    if (m_write_req_channel.is_locked() && was_outstanding_hit &&
        can_accept_write()) {
      m_write_req_channel.unlock(t);
      PORT_ADAPTOR_TRACE(p_info->pTrans, t, "unlock write request channel. free_time:" << m_write_req_channel.free_time());
    }


    // Keep track of transaction complete times if capacity is limited. Need to do this
    // to ensure annotated time of future transactions respects capacity
    unsigned int capacity = m_total_capacity ? m_total_capacity : (p_info->pTrans->is_write() ? m_write_capacity : m_read_capacity);
    if (capacity != (unsigned int)-1) {
      sc_time complete_time(t);
      complete_time += sc_time_stamp();

      std::vector<sc_time>& complete_times = (m_total_capacity || !p_info->pTrans->is_write()) ? m_read_complete_times : m_write_complete_times;

      if (m_total_capacity) {
        // read and write share capacity, so need to keep complete_times vector sorted
        std::vector<sc_time>::iterator it = std::lower_bound(complete_times.begin(), complete_times.end(), complete_time);
        complete_times.insert(it, complete_time);
      }
      else {
        complete_times.push_back(complete_time);
      }

      // only need to keep the last N complete times
      if (complete_times.size() > capacity) {
        complete_times.erase(complete_times.begin(), complete_times.end() - capacity);
      }
    }
  }

  if (p_info->m_waiting_for_complete) {
    m_txn_complete_event.notify(t);
  }

  sc_core::sc_time end_time = t;
  invoke_timing_callback(p_info, payload_state(AFTER_TRANS_END), end_time);
}


//-----------------------------------------------------------------------------

std::string
port_adaptor_impl::byte_array_to_str(unsigned char* p_data, unsigned int length) const
{
  if (p_data) {
    std::ostringstream os;
    for (unsigned int i = 0; i < length; ++i) {
      os << " " << std::setw(2) << std::setfill('0') << hex << (int)(p_data[i]);
    }
    return os.str();
  }
  else {
    return "NULL";
  }
}

std::string
port_adaptor_impl::cycles_array_to_str(unsigned int* p_data, unsigned int length) const
{
  if (p_data) {
    std::ostringstream os;
    for (unsigned int i = 0; i < length; ++i) {
      os << " " << p_data[i];
    }
    return os.str();
  }
  else {
    return "NULL";
  }
}

//-----------------------------------------------------------------------------

void
port_adaptor_impl::worker_thread::do_work_loop() {
  while (true) {
    sc_core::wait(mWakeup);
    assert(mAdaptor!=0 && mPayloadInfo!=0);
    // This is ok.
    // coverity[var_deref_model]
    mAdaptor->nb2b_thread(mEventId, mPayloadInfo);
    mAdaptor = 0;
    mPayloadInfo = 0;
  }
}

void
port_adaptor_impl::worker_thread::single_shot_do_work() {
  sc_core::wait(mWakeup);
  assert(mAdaptor!=0 && mPayloadInfo!=0);
  mAdaptor->nb2b_thread(mEventId, mPayloadInfo);
  delete this;
}

port_adaptor_impl::worker_thread*
port_adaptor_impl::spawn_worker_thread(payload_info* p_info, scml2::callback_event_enum event_id) {
  if (gWorkerThreads == 0) {
    gWorkerThreads = new tWorkerThreads;
  }

  worker_thread* t = NULL;
  for (tWorkerThreads::iterator i = gWorkerThreads->begin(); i != gWorkerThreads->end(); ++i) {
    if ((*i)->mAdaptor == 0) {
      t = *i;
      break;
    }
  }

  if (!t) {
    if ((int)gWorkerThreads->size() < gMaxThreadsInPool) {
      t = new worker_thread;
      sc_core::sc_spawn(sc_bind(&worker_thread::do_work_loop, t), sc_core::sc_gen_unique_name("sc_worker_thread"));
      gWorkerThreads->push_back(t);
    }
    else {
      t = new worker_thread;
      sc_core::sc_spawn(sc_bind(&worker_thread::single_shot_do_work, t), sc_core::sc_gen_unique_name("sc_worker_thread"));
    }
  }

  t->mAdaptor = this;
  t->mEventId = event_id;
  t->mPayloadInfo = p_info;

  return t;
}

//-----------------------------------------------------------------------------

bool
port_adaptor_impl::update_transaction(tlm::tlm_generic_payload& trans, sc_time& t)
{
  payload_info* p_info = NULL;
  get_payload_info(p_info, &trans);
  if (!p_info)
  {
    PORT_ADAPTOR_TXN_FATAL((&trans), t, "update_transaction called for payload that is not in progress");
    return false;
  }

  scml2::payload_state state = get_payload_state(p_info);
  if (state.m_event == scml2::CBK_EVENT_NONE) {
    PORT_ADAPTOR_TXN_FATAL(&trans, t, "update_transaction invoked with invalid protocol state: " << state_name(state));
    return false;
  }
  else {
    return update_transaction(trans, t, state.m_event);
  }
}

bool
port_adaptor_impl::update_transaction(tlm::tlm_generic_payload& trans, sc_time& t, scml2::callback_event_enum event_id)
{
  // track that we are inside an API call => port adaptor must not invoke any callbacks on the user model until after
  // we return from this method
  ++m_inside_api_call;

  payload_info* p_info = NULL;
  get_payload_info(p_info, &trans);
  if (!p_info)
  {
    PORT_ADAPTOR_TXN_FATAL((&trans), t, "update_transaction with event " << event_name(event_id) << " called for payload that is not in progress");
    return false;
  }

  PORT_ADAPTOR_TRACE((&trans), t, event_name(p_info->m_current.m_event) << " => " << event_name(event_id));

  sc_time delay(t);   // pass copy to push_state so original value is not changed
  align_to_clock_edge(delay);
  push_state(p_info, delay, payload_state(event_id));

  --m_inside_api_call;
  return true;
}

//-----------------------------------------------------------------------------

/** State machine processor. The intention here is to minimise the number of
  * sync points, while ensuring that timing and fwd/bwd path callbacks are
  * invoked in the correct context (ie. not nested). The state machine may be
  * kicked directly (from inside nb_transport_fw or API call) or by an sc_event.
  */

void
port_adaptor_impl::process_state_machine()
{
  if (m_inside_process_state_machine) {
    return;
  }

  m_inside_process_state_machine = true;

  sc_time t(SC_ZERO_TIME);

  // loop over each channel until no more states can be processed
  bool states_processed;
  do {
    states_processed = false;
    for (std::vector<ft_channel*>::iterator itr = m_channels.begin(); itr != m_channels.end(); ++itr) {
      if ((*itr)->m_queue.empty())
        continue;

      // Advance the state of this payload as far as possible. Loop will break when
      // the payload runs out of new states, or when the state machine needs to sync
      // before continuing.
      payload_info* p_info = (*itr)->m_queue.front();
      while (!p_info->m_pending.empty()) {

        scml2::payload_state next_state = p_info->m_pending.front();

        // Check if we are able to process this state yet

        // must be able to capture the channel of interest to process this state
        ft_channel* pChannel = get_channel_for_event(next_state);

        if(m_use_q_for_outstanding && p_info->m_outstanding_status == payload_info::OUTSTANDING_HIT) {
          // outstanding was hit when this trans was queued in port adaptor (in adjust_request_time API)
          // Check if outstanding is available now
          sc_core::sc_time &req_abs_time = p_info->m_pending.front().m_time;
          sc_core::sc_time delay;
          if(req_abs_time > sc_core::sc_time_stamp()) {
            sc_core::sc_time t = req_abs_time - sc_core::sc_time_stamp();
            delay = adjust_request_time(p_info, t) - t;
          }
          else {
            delay = adjust_request_time(p_info, sc_core::SC_ZERO_TIME);
          }
          if(p_info->m_outstanding_status == payload_info::OUTSTANDING_HIT) {
            // Outstanding still hit. Can't process this req
            break;
            //pChannel->lock();
          }
          else {
            /* Outstanding available. Adjust request time. Unlock channel if allowed */
            if(pChannel->is_locked()
            ) {
              pChannel->unlock(delay);
            }
            if(!pChannel->is_locked()) {
              /* This request can now be sent at the time when outstanding was made available */
              req_abs_time = (req_abs_time > sc_core::sc_time_stamp() ? req_abs_time : sc_core::sc_time_stamp()) + delay;
            }
          }
        }

        if (!pChannel->can_be_captured(p_info)) {
          // no need to schedule state machine in this case
          PORT_ADAPTOR_TRACE(p_info->pTrans, get_latest_time(SC_ZERO_TIME, p_info->m_current.m_time),
            "cannot process " << event_name(next_state.m_event) << ", cannot capture channel"
            << " captured:" << pChannel->is_captured() << " locked:" << pChannel->is_locked()
            << " front:" << (void*)(pChannel->get_front() ? pChannel->get_front()->pTrans : NULL)
            << " credits:" << pChannel->num_credits()
            << " outstanding_status:" << p_info->get_outstanding_status_str(p_info->m_outstanding_status) << " ("
            << " R:" << m_outstanding_q[OUTSTANDING_TYPE_RD].size()
            << " W:" << m_outstanding_q[OUTSTANDING_TYPE_WR].size()
            << " T:" << m_outstanding_q[OUTSTANDING_TYPE_TOTAL].size()
            << " )"
          );
          break;
        }

        // must be valid state transition (might be waiting for another event first)
        if (!is_valid_state_transition(p_info, next_state)) {
            if(m_protocol_id == scml2::axi4_stream_protocol_state_extension::ID) {
                PORT_ADAPTOR_TXN_ERROR(p_info->pTrans, get_latest_time(SC_ZERO_TIME, p_info->m_current.m_time),
                  "cannot process " << event_name(p_info->m_current.m_event) << " => " << event_name(next_state.m_event) << ", invalid state transition");
            }
            else {
                PORT_ADAPTOR_TXN_WARNING(p_info->pTrans, get_latest_time(SC_ZERO_TIME, p_info->m_current.m_time),
                  "cannot process " << event_name(p_info->m_current.m_event) << " => " << event_name(next_state.m_event) << ", invalid state transition");
            }
          break;
        }

        // Check if we need to switch context before processing this state

        // timing callback registered for this state, and we are already inside a
        // port adaptor API call (update_transaction etc)
        if (m_inside_api_call && has_callback(next_state.m_event)) {
          PORT_ADAPTOR_TRACE(p_info->pTrans, get_latest_time(SC_ZERO_TIME, p_info->m_current.m_time),
            "cannot process " << event_name(next_state.m_event) << ", inside callback, schedule next delta cycle");
          m_process_state_machine_event.notify();
          break;
        }

        // inside nb_transport, and already returning a state for that payload,
        // or trying to send state for a different payload
        if (m_nb_transport_payload) {
          if (!m_use_return_path && send_state_to_socket(p_info, next_state)) {
            m_process_state_machine_event.notify();
            break;
          }

          if (((m_nb_transport_payload == p_info) && m_returning_bw_state) ||
              ((m_nb_transport_payload != p_info) && send_state_to_socket(p_info, next_state))) {
            PORT_ADAPTOR_TRACE(p_info->pTrans, get_latest_time(SC_ZERO_TIME, p_info->m_current.m_time),
              "cannot process " << event_name(next_state.m_event) << ", already returning nb state, schedule next delta cycle");
            m_process_state_machine_event.notify();
            break;
          }
        }

        // Determine local time at which we can process this state
        t = get_latest_time(SC_ZERO_TIME, p_info->m_current.m_time);     // TODO: give get_latest_time API a more meaningful name
        t = get_latest_time(t, p_info->m_pending.front().m_time);
        t = get_latest_time(t, pChannel->free_time());

        // OK - we are good to process this state now

        states_processed = true;
        pChannel->capture();
        p_info->m_current = p_info->m_pending.front();
        p_info->m_pending.pop_front();

        process_payload(p_info, t);
      }

      // cleanup completed payloads
      if (p_info->m_complete && (p_info != m_nb_transport_payload)) {
        {
          release_payload_info(p_info);
        }
      }
    }
  }
  while (states_processed);

  m_inside_process_state_machine = false;
}

} // namespace scml2
