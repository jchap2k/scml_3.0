/***************************************************************************
 * Copyright 1996-2025 Synopsys, Inc.
 *
 * This Synopsys software and all associated documentation are proprietary
 * to Synopsys, Inc. and may only be used pursuant to the terms and
 * conditions of a written license agreement with Synopsys, Inc.
 * All other use, reproduction, modification, or distribution of the
 * Synopsys software or the associated documentation is strictly prohibited.
 ***************************************************************************/
#ifndef __PORT_ADAPTOR_IMPL_H__
#define __PORT_ADAPTOR_IMPL_H__

#include <sys/stat.h>
#include <iomanip>
#include <queue>
#include <unordered_map>
#include <unordered_set>
#include <map>
#include <list>
#include <algorithm>
#include <tlm_utils/instance_specific_extensions.h>
#include <scml2/port_adaptor.h>
#include <scml2_tlm2/snps_tlm2_utils/snps_tlm2_mm.h>
#include <scml2/tagged_message.h>
#include <scml2_logging/stream.h>
#include <tlm_utils/peq_with_cb_and_phase.h>

#include <functional>

static inline bool get_tagged_message_handler_impl(sc_core::sc_module *m) {
    (void)m;
    return false;
}

#ifndef HEX_PREFIX
#if defined(_MSC_VER)
    #define HEX_PREFIX "0x"
#else
    #define HEX_PREFIX ""
#endif
#endif

/** Helper macros to provide consistent trace and log output for all derived port adaptors */
#define PORT_ADAPTOR_TRACE0(msg)                                        \
    if(m_trace_enabled) {                                               \
        std::cout << sc_core::sc_time_stamp() << " "                    \
            << m_sysc_module->name()                                    \
            << " : " << __FUNCTION__ << ":" << __LINE__                 \
            << " : " << msg << std::endl;                               \
    }                                                                   \
    else if(get_tagged_message_handler_impl(m_sysc_module)) {           \
        SCML2_MODEL_INTERNAL_TO(m_sysc_module, LEVEL6)                  \
            << " : " << __FUNCTION__ << ":" << __LINE__                 \
            << " : " << msg << std::endl;                               \
    }

#define PORT_ADAPTOR_TRACE(payload, delay, msg)                         \
    if(m_trace_enabled) {                                               \
        std::cout << sc_core::sc_time_stamp() << " "                    \
            << m_sysc_module->name() << " "                             \
            << " : " << (sc_core::sc_time_stamp() + delay)              \
            << " : " << __FUNCTION__ << ":" << __LINE__                 \
            << " : " << HEX_PREFIX << std::hex << payload << std::dec   \
            << " : " << msg << std::endl;                               \
    }                                                                   \
    else if(get_tagged_message_handler_impl(m_sysc_module)) {           \
        SCML2_MODEL_INTERNAL_TO(m_sysc_module, LEVEL6)                  \
            << " : " << (sc_time_stamp() + delay)                       \
            << " : " << __FUNCTION__ << ":" << __LINE__                 \
            << " : " << HEX_PREFIX << std::hex << payload << std::dec   \
            << " : " << msg << std::endl;                               \
    }

#define PORT_ADAPTOR_WARNING(msg)                                       \
    SCML2_WARNING_TO(m_sysc_module, GENERIC_WARNING) << " : " << msg << std::endl;

#define PORT_ADAPTOR_TXN_WARNING(payload, delay, msg)                   \
    SCML2_WARNING_TO(m_sysc_module, GENERIC_WARNING)                    \
    << " : " << (sc_time_stamp() + delay)                               \
    << " : " << __FUNCTION__ << ":" << __LINE__                         \
    << " : " << HEX_PREFIX << std::hex << payload << std::dec           \
    << " : " << msg << std::endl;


#define PORT_ADAPTOR_ERROR(msg)                                         \
    SCML2_ERROR_TO(m_sysc_module, GENERIC_ERROR) << " : " << msg << std::endl;

#define PORT_ADAPTOR_TXN_ERROR(payload, delay, msg)                     \
    SCML2_ERROR_TO(m_sysc_module, GENERIC_ERROR)                        \
       << " : " << (sc_time_stamp() + delay)                            \
       << " : " << HEX_PREFIX << std::hex << payload << std::dec        \
       << " : " << msg << std::endl;

#define PORT_ADAPTOR_FATAL(msg)                                         \
    PORT_ADAPTOR_ERROR(msg);                                            \
    sc_core::sc_stop();

#define PORT_ADAPTOR_TXN_FATAL(payload, delay, msg)                     \
    PORT_ADAPTOR_TXN_ERROR(payload, delay, msg)                         \
    sc_core::sc_stop();

// TODO: don't generate any code for release builds (leaving in for now for improved error checking)
#ifndef NDEBUG
#define DEBUG_CHECK(condition, payload, t, msg)                         \
  if (!(condition)) {                                                   \
    PORT_ADAPTOR_TXN_ERROR((payload), t, msg);                          \
    abort();                                                            \
  }
#else
#define DEBUG_CHECK(condition, trans, t, msg)
#endif

namespace scml2
{

enum {
  INVALID_PROTOCOL_STATE = -1U,
  MAX_FT_CHANNELS = 8  // adjust as needed for new protocols
};

inline bool payload_supports_ft_extensions(tlm::tlm_generic_payload* p) {
    return dynamic_cast<scml2::ft_generic_payload*>(p) != nullptr;
}

// wrap macros to avoid accessing FT extensions on payloads that do not support it
#define GET_FT_EXT_ATTR(payload, ext_type, attr_type, attr_inst) \
  attr_type attr_inst = 0;                                       \
  if (payload_supports_ft_extensions(payload)) {                 \
    GET_EXT_ATTR(payload, ext_type, attr_type, tmp##attr_inst);  \
    attr_inst = tmp##attr_inst;                                  \
  }
#define SET_FT_EXT_ATTR(payload, ext_type, attr_val) \
  if (payload_supports_ft_extensions(payload)) {     \
    SET_EXT_ATTR(payload, ext_type, attr_val);       \
  }

class port_adaptor_impl;

struct payload_state
{
  callback_event_enum m_event;
  unsigned int m_sideband_data;
  payload_state(callback_event_enum event_id=scml2::CBK_EVENT_NONE) : m_event(event_id), m_sideband_data(-1U) {}
  payload_state(callback_event_enum event_id, unsigned int sideband_data) : m_event(event_id), m_sideband_data(sideband_data) {}
};

/** Time tracking for a pending state */
struct state_time_pair : payload_state
{
  sc_time m_time;
  state_time_pair() : payload_state(CBK_EVENT_NONE), m_time(SC_ZERO_TIME) { }
  state_time_pair(payload_state state, const sc_time& t) : payload_state(state), m_time(t) { }
  state_time_pair(callback_event_enum event_id, const sc_time& t) : payload_state(event_id), m_time(t) { }
  state_time_pair(callback_event_enum event_id, unsigned int sideband_data, const sc_time& t) : payload_state(event_id, sideband_data), m_time(t) { }
};

/** Double-ended queue that uses a mem_manager to allocate internal storage */
template<typename ObjType>
class deque_mm
{
public:
  struct node {
  node(scml2::mem_manager<node>*) : m_pNext(NULL), m_obj() {}

    node* m_pNext;
    ObjType m_obj;
  };

public:
  typedef scml2::mem_manager<node> pool_type;

  deque_mm(pool_type *mm=NULL) : m_pHead(NULL), m_pTail(NULL), m_pNodePool(mm), m_count(0) { }

  ~deque_mm()
  {
    clear();
  }

  void set_mm(pool_type *mm) {
    m_pNodePool = mm;
  }

  bool empty() const
  {
    return (m_pHead == NULL);
  }

  size_t length() const
  {
    return m_count;
 }

  void clear()
  {
    while (m_pHead != NULL) {
      pop_front();
    }
    m_count = 0;
  }

  ObjType& front()
  {
    assert(m_pHead != NULL);
    return m_pHead->m_obj;
  }

  const ObjType& front() const
  {
    assert(m_pHead != NULL);
    return m_pHead->m_obj;
  }

  void push_front(ObjType obj)
  {
    node* pTemp = m_pNodePool->claim();
    pTemp->m_obj = obj;
    pTemp->m_pNext = m_pHead;
    m_pHead = pTemp;
    if (m_pTail == NULL) {
      m_pTail = pTemp;
    }
    ++m_count;
  }

  void pop_front()
  {
    assert(m_pHead != NULL);
    node* pTemp = m_pHead;
    m_pHead = m_pHead->m_pNext;
    if (m_pTail == pTemp) {
      m_pTail = NULL;
    }
    m_pNodePool->free_obj(pTemp);
    assert(m_count > 0);
    --m_count;
  }

  void push_back(ObjType obj)
  {
    node* pTemp = m_pNodePool->claim();
    pTemp->m_pNext = NULL;

    if (m_pHead == NULL) {
      m_pHead = m_pTail = pTemp;
    }
    else {
      m_pTail->m_pNext = pTemp;
      m_pTail = pTemp;
    }
    pTemp->m_obj = obj;
    ++m_count;
  }

public:
  node *m_pHead, *m_pTail;
  pool_type *m_pNodePool;
  size_t m_count;
};

/** Structure to track a payload */
class payload_info : public tlm_utils::instance_specific_extension<payload_info>, public tlm::tlm_mm_interface
{
  // define custom extension used to notify an event when it is released (ie. when transaction ref_count reaches 0)
  struct mm_end_event_ext : public tlm::tlm_extension<mm_end_event_ext>
  {
    tlm::tlm_extension_base* clone() const override { return NULL; }
    void free() override {
      done.notify();
    }
    void copy_from(tlm::tlm_extension_base const &) override {}
    sc_core::sc_event done;
  };

public:
  typedef enum {
    OUTSTANDING_NOT_HIT,
    OUTSTANDING_HIT,
    OUTSTANDING_NA,         // Outstanding not applicable
  } outstanding_status_t;

  typedef scml2::deque_mm<state_time_pair>::pool_type state_pool_type;

  tlm::tlm_generic_payload* pTrans;
  mm_end_event_ext m_mm_ext;

  unsigned int m_id;                                        // transaction ID
  unsigned int m_beat_size;                                 // size of each beat in bytes
  unsigned int m_beat_count;                                // total number of beats in burst
  unsigned int m_beat_index;                                // current beat index

  outstanding_status_t m_outstanding_status = {OUTSTANDING_NOT_HIT};  // Outstanding hit or not

  typedef enum {
    TRANS_DIR_TX,
    TRANS_DIR_RX,
    TRANS_DIR,
  } trans_dir_e;
  unsigned int m_num_data_expected[TRANS_DIR]  = {0};       // number of data expected for the trans
  unsigned int m_num_data_count[TRANS_DIR]     = {0};       // number of data processed for the trans
  unsigned int m_num_resp_expected[TRANS_DIR]  = {0};       // number of responses expected for the trans
  unsigned int m_num_resp_count[TRANS_DIR]     = {0};       // number of responses processed for the trans

  tlm::tlm_phase m_tlm_phase;   // TLM2_GP only, store tlm phase since there is no protocol_extension in the payload


  state_time_pair m_current;
  scml2::deque_mm<state_time_pair> m_pending;
  bool m_channel_queued[MAX_FT_CHANNELS];

  bool m_lt_mode_txn;
  bool m_waiting_for_b_transport;
  bool m_waiting_for_complete;
  bool m_complete;

  payload_info(scml2::mem_manager<payload_info>* mm=NULL);

  ~payload_info()
  {
    if (pTrans) {
      release();
    }
  }

  /** tlm_mm_interface */
  ///@{
  void free(tlm::tlm_generic_payload* trans) override {
    // release extensions, which will cause mm_end_event_ext to notify event
    trans->reset();
    trans->set_mm(0);
  }

  void wait_for_zero_ref_count(tlm::tlm_generic_payload& trans) const {
    // wait for transaction reference count to reach 0 only if this payload
    // is managed by us
    if (trans.get_ref_count() && (trans.get_extension<mm_end_event_ext>() == &m_mm_ext)) {
      sc_core::wait(m_mm_ext.done);
    }
  }
  ///@}

  void reset();
  void acquire(port_adaptor_impl* adaptor, state_pool_type* pool, tlm::tlm_generic_payload& trans);
  void release();

  bool is_state_pending(callback_event_enum state) const;
  unsigned int count_pending(callback_event_enum state) const;

  static const std::string& get_outstanding_status_str(outstanding_status_t t) {
    static std::string s[] = {
      "not_hit",
      "hit",
      "na",
    };
    return s[t];
  }
};

/** Helper class to manage concurrent access to an FT channel. */
class ft_channel
{
public:
	ft_channel(unsigned *interleaving_supported = NULL)
		: m_free_time(SC_ZERO_TIME),
			m_captured(false),
			m_locked(false),
			m_chan_id(0),
			m_interleaving_supported(interleaving_supported),
			m_last_credit_time(SC_ZERO_TIME),
			m_credits(0),
			m_is_credit_based_channel(false),
			m_queue(&m_pool)
	{
	}

	size_t pending_count() const
	{
		size_t pending = m_queue.length();
		if (pending && is_captured_by(m_queue.front()))
			--pending;
		return pending;
	}

	void set_is_credit_based_channel(bool value) {
		m_is_credit_based_channel = value;
	}

	bool is_credit_available(void) const {
		return (m_is_credit_based_channel ? (m_credits != 0) : true);
	}

	/** Return true if the channel can be captured by this payload */
	bool can_be_captured(payload_info* p_info) const
	{
		return (m_captured || (!m_locked && !m_queue.empty())) && m_queue.front() == p_info;
	}

	/** Return true if the channel is currently captured by this payload */
	bool is_captured_by(payload_info* p_info) const
	{
		return m_captured && m_queue.front() == p_info;
	}

	const sc_time& free_time() const { return m_free_time; }

	/** Request access to the channel, returns true if channel is available. */
	bool request(payload_info* p_info, bool push_front = false)
	{
		if (is_captured_by(p_info)) {
			return true;
		}
		else if (!p_info->m_channel_queued[m_chan_id]) {
			// add to pending list, then check if we can process immediately
			p_info->m_channel_queued[m_chan_id] = true;
			if(push_front) {
				m_queue.push_front(p_info);
			}
			else {
				m_queue.push_back(p_info);
			}

			if (!m_locked && !m_captured && m_queue.front() == p_info) {
				return true;
			}
		}

		return false;
	}

    payload_info* get_front() const {
        if(m_queue.empty()) {
            return NULL;
        }
        else {
            return m_queue.front();
        }
    }

	bool is_front(payload_info* p_info) const {
		return !m_queue.empty() && (m_queue.front() == p_info);
	}

	bool is_captured(void) const {
		return m_captured;
	}

	void capture()
	{
		assert(m_captured || !m_locked);    // must be already captured or not locked
		m_captured = true;
	}

	/** Release the channel, it will be recaptured by the next pending request (if any) */
	void release(const sc_time& t, bool keep_channel=false)
	{
		assert(m_captured);
		assert(!m_queue.empty());

		m_free_time = std::max(m_free_time, sc_time_stamp() + t);

		if (!keep_channel) {
			// release channel
			m_captured = false;
			m_queue.front()->m_channel_queued[m_chan_id] = false;
			m_queue.pop_front();
		}
		else if(!is_credit_available()) {
			m_captured = false;
		}
	}

	/** Returns true if the channel is locked and cannot be captured by
	 *  a request. */
	bool is_locked() const { return m_locked; }

	/** Lock this channel from being captured by any new request */
	void lock()
	{
		m_locked = true;
	}

	/* Allow channel to be captured once more */
	void unlock(const sc_time& t)
	{
		if(is_credit_available()) {
			m_locked = false;
		}
		if (!m_captured) {
			m_free_time = std::max(m_free_time, sc_time_stamp() + t);
		}
	}

	/* Credit handling */
	unsigned int num_credits(void) const {
		return m_credits;
	}

	void consume_credit()
	{
		if (m_credits > 0) {
			if (--m_credits == 0) {
				// no credits remain for this channel => lock it until at least one is returned
				lock();
			}
		}
	}

	bool add_credit(const sc_time& t)
	{
		m_last_credit_time = sc_time_stamp() + t;
		if (++m_credits == 1) {
			// unlock the channel if we were on zero credits before
			unlock(t);
			return true;
		}
		else {
			return false;
		}
	}

private:
	sc_time m_free_time;                                // absolute time at which channel last became free
	bool m_captured;                                    // channel captured state
	bool m_locked;                                      // channel locked state

public:
	unsigned int m_chan_id;                             // unique ID for this channel (automatically assigned)

	/* Ordering related data structures */
	unsigned *m_interleaving_supported;
	std::unordered_map< unsigned, std::queue<payload_info*> > m_txn_id_map;          // trans id queues for each incoming address (Currently being used in data channels only)

	sc_time m_last_credit_time;                         // absolute time of last credit rx/tx
	unsigned int m_credits;                             // current number of credits
	bool m_is_credit_based_channel;                     // whether we need to use m_credits or not

	scml2::deque_mm<payload_info*>::pool_type m_pool;   // queue uses pool, so we need to declare and construct them in this order
	scml2::deque_mm<payload_info*> m_queue;             // queue of pending requests for this channel, assumed to be inserted in time order
};

//-----------------------------------------------------------------------------

class port_adaptor_impl
{

using trans_free_observer_func = std::function<void(tlm::tlm_generic_payload &)>;
protected:
  typedef std::map<std::string, int*> attribute_map;
public:
  port_adaptor_impl(const std::string& name);
  virtual ~port_adaptor_impl();

  virtual sc_core::sc_module* get_sc_module() = 0;

  virtual std::string get_name() const = 0;
  virtual unsigned int get_bus_width() const = 0;
  unsigned int get_burst_size(tlm::tlm_generic_payload& trans) const;
  virtual unsigned int get_burst_length(tlm::tlm_generic_payload& trans) const;
  unsigned int get_trans_id(tlm::tlm_generic_payload& trans) const;

  void set_attribute(const std::string& name, int value);
  int get_attribute(const std::string& name);

  void execute_command(const std::string& cmd, void* arg1, void* arg2, void* arg3);


  virtual void before_end_of_elaboration();

  scml2::callback_event_enum get_event_id(unsigned int protocol_state) const;

  /** API to register a callback for the specified event in the transaction life cycle */
  bool set_timing_callback(callback_event_enum event_id, timing_callback_base* cb);

  /** Convenience function to return if there is any callback registered for a particular event */
  bool has_callback(scml2::callback_event_enum event_id) const;

  bool update_transaction(tlm::tlm_generic_payload& trans, sc_time& t);
  bool update_transaction(tlm::tlm_generic_payload& trans, sc_time& t, scml2::callback_event_enum event_id);


protected:


  void read_port_properties(const std::string &port_name)
  {
    (void)port_name;
  }

  static inline sc_time get_latest_time(const sc_time& rel_time, const sc_time& abs_time)
  {
    sc_time now = sc_time_stamp();
    return std::max(now + rel_time, abs_time) - now;
  }

  inline void align_to_clock_edge(sc_time& delay) const
  {
  }

  payload_info* alloc_payload_info(tlm::tlm_generic_payload& trans)
  {
    // allocate payload_info extension the first time we see this payload
    payload_info* p_info = NULL;
    if(m_use_ispex) {
      (*m_accessor)(trans).get_extension(p_info);
    }
    else {
      p_info = m_p_info_map[&trans];
    }

    if (!p_info) {
      //++m_total_txns;
      ++m_current_active;
      //if (m_current_active > m_max_active) {
      //  m_max_active = m_current_active;
      //}
      p_info = m_info_pool.claim();
      if(m_use_ispex) {
        (*m_accessor)(trans).set_extension(p_info);
      }
      else {
        m_p_info_map[&trans] = p_info;
      }
    }
    return p_info;
  }

  void release_payload_info(payload_info* p_info)
  {
    --m_current_active;
    clear_payload_info(p_info, p_info->pTrans);
    p_info->release();
    m_info_pool.free_obj(p_info);
  }

  void clear_payload_info(payload_info* p_info, tlm::tlm_generic_payload* trans)
  {
    (void)p_info;
    if(m_use_ispex) {
      (*m_accessor)(*trans).clear_extension(p_info);
    }
    else {
      auto it = m_p_info_map.find(trans);
      if(it != m_p_info_map.end()) {
          m_p_info_map.erase(it);
      }
    }
  }

  void set_payload_info(payload_info* p_info, tlm::tlm_generic_payload* trans)
  {
    if(m_use_ispex) {
      (*m_accessor)(*trans).set_extension(p_info);
    }
    else {
      m_p_info_map[trans] = p_info;
    }
  }

  void get_payload_info(payload_info*& p_info, tlm::tlm_generic_payload* trans)
  {
    if(m_use_ispex) {
      (*m_accessor)(*trans).get_extension(p_info);
    }
    else {
      p_info = m_p_info_map[trans];
    }
  }

  std::string event_name(callback_event_enum event_id) const;
  virtual std::string state_name(payload_state state) const;

  bool can_invoke_timing_callbacks(bool is_lt_txn) const
  {

    return ((m_invoke_timing_callbacks == port_adaptor::eAlwaysInvokeCbks) ||
            (!is_lt_txn && (m_invoke_timing_callbacks == port_adaptor::eInvokeCbksForFT)));
  }

  virtual bool is_fixed_burst(tlm::tlm_generic_payload&) const { return false; }
  virtual bool supports_unaligned_beats() const { return false; }

  virtual void invoke_fwd_timing_callback(payload_info* p_info,
                                          payload_state state,
                                          sc_time &t);

  // invocation API to handle callback lookup and invocation
  virtual tlm::tlm_sync_enum invoke_timing_callback(payload_info* p_info,
                                                    payload_state state,
                                                    sc_time &t,
                                                    bool is_fwd_cb = false);

  void in_cb_peq(tlm::tlm_generic_payload &trans, const tlm::tlm_phase &phase);

  // helper method to check clock period and generate error message if clock stopped
  inline void check_clk_period(std::string func)
  {
    if (m_clk && !m_clk->running()) {
      PORT_ADAPTOR_ERROR(func << " : bound clock " << m_clk->name() << " is not running");
      assert(false);
    }
  }


  virtual sc_time adjust_request_time(payload_info* p_info, const sc_time& t);


  virtual void on_txn_complete(payload_info* p_info, const sc_time& t);

  /** tracing helpers */
  ///@{
  std::string byte_array_to_str(unsigned char* p_data, unsigned int length) const;
  std::string cycles_array_to_str(unsigned int* p_data, unsigned int length) const;
  ///@}

  bool can_accept_read() const
  {
    if(m_use_q_for_outstanding) {
      return true;
    }

    if (m_total_capacity > 0) {
      return ((m_active_reads + m_active_writes) < m_total_capacity);
    }
    else {
     return (m_active_reads < m_read_capacity);
    }
  }

  bool can_accept_write() const
  {
    if(m_use_q_for_outstanding) {
      return true;
    }

    if (m_total_capacity > 0) {
      return ((m_active_reads + m_active_writes) < m_total_capacity);
    }
    else {
      return (m_active_writes < m_write_capacity);
    }
  }

  virtual void process_state_machine();

  /** Protocol specific implementation */
  ///@{
  virtual bool data_beat_array_supported(payload_info*) const { return false; }
  virtual scml2::payload_state get_payload_state(payload_info* p_info) const = 0;
  virtual void set_protocol_state(tlm::tlm_generic_payload& trans, scml2::payload_state state) = 0;

  virtual bool is_valid_state_transition(payload_info* p_info, scml2::payload_state state) const = 0;
  virtual bool send_state_to_socket(payload_info* p_info, scml2::payload_state state) const = 0;
  virtual ft_channel* get_channel_for_event(payload_state state) = 0;

  virtual void push_state(payload_info* p_info, sc_time& t, scml2::payload_state state) = 0;
  virtual void advance_state(payload_info* p_info, sc_time& t) = 0;
  virtual void process_payload(payload_info* p_info, sc_time& t) = 0;
  ///@}

  /** Watchdog related APIs */
  inline void begin_watchdog_transaction(payload_info &p_info, const sc_core::sc_time &t) {
    (void)p_info;
    (void)t;
  }

  inline void end_watchdog_transaction(payload_info &p_info, const sc_core::sc_time &t) {
    (void)p_info;
    (void)t;
  }

  virtual std::string get_socket_name() const = 0;

  typedef enum {
    OUTSTANDING_TYPE_RD,        // Uses m_read_capacity
    OUTSTANDING_TYPE_WR,        // Uses m_write_capacity
    OUTSTANDING_TYPE_TOTAL,     // Uses m_total_capacity
    OUTSTANDING_TYPE_MAX,
  } outstanding_type;

  void init_outstanding_q(void) {
    static char *p = std::getenv("SNPS_PORT_ADAPTOR_DONT_USE_Q_FOR_OT");
    if(p) {
      m_use_q_for_outstanding = 0;
    }

    if(!m_use_q_for_outstanding) {
      return;
    }

    m_outstanding_applicable[OUTSTANDING_TYPE_TOTAL] = false;
    m_outstanding_applicable[OUTSTANDING_TYPE_RD]    = false;
    m_outstanding_applicable[OUTSTANDING_TYPE_WR]    = false;

    if(m_protocol_id == TLM2_GP_PROTOCOL_ID) {
      m_outstanding_applicable[OUTSTANDING_TYPE_TOTAL] = true;
    }
    static const int max_allowed_q_entries = 65536;
    unsigned i = 0;

    if(m_outstanding_applicable[OUTSTANDING_TYPE_TOTAL]) {
      if(m_total_capacity > max_allowed_q_entries) {
        /* Too many entries. Revert to old outstanding scheme */
        m_use_q_for_outstanding = 0;
        return;
      }
      auto *q = get_outstanding_q(tlm::TLM_READ_COMMAND);
      if(q && m_total_capacity) {
        std::queue<uint64_t>().swap(*q);
        for(i = 0; i < m_total_capacity; i++) {
          q->push(m_clk_period.value()*i);
        }
        m_last_outstanding_entry[OUTSTANDING_TYPE_TOTAL] = m_clk_period.value()*(i-1);
        m_capacity[OUTSTANDING_TYPE_TOTAL] = m_total_capacity;
      }
    }

    if(m_outstanding_applicable[OUTSTANDING_TYPE_RD]) {
      if(m_read_capacity > max_allowed_q_entries) {
        /* Too many entries. Revert to old outstanding scheme */
        m_use_q_for_outstanding = 0;
        return;
      }
      auto *rq = get_outstanding_q(tlm::TLM_READ_COMMAND);
      if(rq && m_read_capacity) {
        std::queue<uint64_t>().swap(*rq);
        for(i = 0; i < m_read_capacity; i++) {
          rq->push(m_clk_period.value()*i);
        }
        m_last_outstanding_entry[OUTSTANDING_TYPE_RD] = m_clk_period.value()*(i-1);
        m_capacity[OUTSTANDING_TYPE_RD] = m_read_capacity;
      }
    }

    if(m_outstanding_applicable[OUTSTANDING_TYPE_WR]) {
      if(m_write_capacity > max_allowed_q_entries) {
        /* Too many entries. Revert to old outstanding scheme */
        m_use_q_for_outstanding = 0;
        return;
      }
      auto *wq = get_outstanding_q(tlm::TLM_WRITE_COMMAND);
      if(wq && m_write_capacity) {
        std::queue<uint64_t>().swap(*wq);
        for(i = 0; i < m_write_capacity; i++) {
          wq->push(m_clk_period.value()*i);
        }
        m_last_outstanding_entry[OUTSTANDING_TYPE_WR] = m_clk_period.value()*(i-1);
        m_capacity[OUTSTANDING_TYPE_WR] = m_write_capacity;
      }
    }
  }

  inline std::queue<uint64_t>* get_outstanding_q(tlm::tlm_command cmd) {
    if(cmd == tlm::TLM_IGNORE_COMMAND) {
      return nullptr;
    }
    if(m_outstanding_applicable[OUTSTANDING_TYPE_TOTAL]) {
      return &m_outstanding_q[OUTSTANDING_TYPE_TOTAL];
    }

    if(cmd == tlm::TLM_READ_COMMAND) {
      return (m_outstanding_applicable[OUTSTANDING_TYPE_RD] ? &m_outstanding_q[OUTSTANDING_TYPE_RD] : nullptr);
    }
    else {
      return (m_outstanding_applicable[OUTSTANDING_TYPE_WR] ? &m_outstanding_q[OUTSTANDING_TYPE_WR] : nullptr);
    }
  }

  inline std::queue<uint64_t>* get_outstanding_q(tlm::tlm_command cmd) const {
    return (const_cast<port_adaptor_impl*>(this))->get_outstanding_q(cmd);
  }

  inline void push_to_outstanding_q(std::queue<uint64_t> *q, uint64_t entry) {
    if(!q) {
      return;
    }
    outstanding_type t;
    if(q == &m_outstanding_q[OUTSTANDING_TYPE_RD]) {
      t = OUTSTANDING_TYPE_RD;
    }
    else if(q == &m_outstanding_q[OUTSTANDING_TYPE_WR]) {
      t = OUTSTANDING_TYPE_WR;
    }
    else {
      t = OUTSTANDING_TYPE_TOTAL;
    }
    if(entry <= m_last_outstanding_entry[t]) {
      entry = m_last_outstanding_entry[t] + m_clk_period.value();
    }

    if(q->size() < m_capacity[t]) {
      q->push(entry);
      m_last_outstanding_entry[t] = entry;
    }
    else {
        PORT_ADAPTOR_WARNING("Outstanding overflow for q:" << t << " capacity:" << m_capacity[t]);
    }
  }

protected:
  unsigned int                m_trace_enabled = 0;
  unsigned int                m_eventId2ProtoState[MAX_CBK_EVENT_ID_E]; // map callback_event_enum to protocol specific state
  std::vector<callback_event_enum> m_protoState2EventId;
  attribute_map               m_attributes;
  sc_in<bool>*                m_clk_port;
  scml_clock_if*              m_clk;
  sc_time                     m_clk_period;
  bool                        m_clk_period_change_pending;
  sc_event                    m_clk_change_sync;

  tlm_utils::instance_specific_extension_accessor *m_accessor;
  std::unordered_map<tlm::tlm_generic_payload*, payload_info*> m_p_info_map;
  scml2::mem_manager<payload_info> m_info_pool;                 // pool for payload_info extensions
  payload_info::state_pool_type m_pending_state_pool;           // pool for payload pending states (shared by all payloads)

  sc_event m_process_state_machine_event;
  sc_event m_txn_complete_event;

  ft_channel m_req_channel;
  ft_channel m_write_req_channel;

  std::vector<ft_channel*> m_channels;

  bool m_use_return_path;
  unsigned int m_inside_api_call;
  bool m_inside_process_state_machine;
  payload_info* m_nb_transport_payload;
  bool m_returning_bw_state;
  sc_time m_returning_bw_time;

  unsigned int m_total_txns;
  unsigned int m_max_active;
  unsigned int m_current_active;

  unsigned int m_active_reads;
  unsigned int m_active_writes;
  std::vector<sc_time> m_read_complete_times;
  std::vector<sc_time> m_write_complete_times;

  std::vector<payload_info*> m_data_order;    // payload ordering for data

  // attributes
  port_adaptor::EInvokeTimingCallbacks m_invoke_timing_callbacks;
  unsigned int m_read_capacity;
  unsigned int m_write_capacity;
  unsigned int m_total_capacity;       // used instead of read/write capacity if non-zero

  /* Use instance specific extensions or std::maps for book-keeping */
  unsigned int m_use_ispex = {false};

  /* If true, try using dmi first, instead of b_transport as an optimization */
  unsigned int m_try_dmi_for_b_transport = {true};

  // Variables for outstanding support

  /* ToDo Move outstanding logic / variables in a new class / structure */

  /* If 1, use below queues to maintain annotated time for outstanding trans. If 0, then use legacy impl */
  unsigned int m_use_q_for_outstanding = 1;

  /* Queues that store the next time when a request can be sent */
  std::queue<uint64_t> m_outstanding_q[OUTSTANDING_TYPE_MAX];

  /* Time when last outstanding entry is available */
  uint64_t m_last_outstanding_entry[OUTSTANDING_TYPE_MAX] = {0};

  /* Whether outstanding logic needs to be applied or not */
  bool m_outstanding_applicable[OUTSTANDING_TYPE_MAX] = {false};

  unsigned m_capacity[OUTSTANDING_TYPE_MAX]           = {0};

  /* If 1, consume annotated time when triggering incoming callbacks */
  unsigned int m_consume_annotated_time  = 0;
  tlm_utils::peq_with_cb_and_phase<port_adaptor_impl> m_in_cb_peq;

  sc_core::sc_module *m_sysc_module = nullptr;
  unsigned int m_support_unique_id_generation = 0;
  unsigned int m_assert_on_pending_trans = 0;

  class my_mem_manager : public scml2::mem_manager<scml2::ft_generic_payload> {
  public:
    my_mem_manager(port_adaptor_impl &parent)
    :  m_parent(parent)
    {

    }

    void free(tlm::tlm_generic_payload *trans);

    void notify_observers(tlm::tlm_generic_payload &trans);

    port_adaptor_impl &m_parent;

    std::unordered_set<trans_free_observer_func *> m_observers;
  };
  my_mem_manager m_pool;

public:
  static void set_max_threads_in_pool(int max_threads_in_pool) { gMaxThreadsInPool = max_threads_in_pool; }
  static int get_max_threads_in_pool() { return gMaxThreadsInPool; }

protected:
  virtual void nb2b_thread(scml2::callback_event_enum event_id, payload_info* p_info) = 0;

  // threaded worker class to perform some task after switching context; invokes nb2b_thread on the adaptor
  class worker_thread {
  public:
    sc_core::sc_event          mWakeup;
    port_adaptor_impl*         mAdaptor;
    scml2::callback_event_enum mEventId;
    payload_info*              mPayloadInfo;

    worker_thread() : mAdaptor(0), mEventId(scml2::MAX_CBK_EVENT_ID_E), mPayloadInfo(0) { }
    void do_work_loop();
    void single_shot_do_work();
  };

  worker_thread* spawn_worker_thread(payload_info* p_info, callback_event_enum event_id);

  static int gMaxThreadsInPool;
  typedef std::list<worker_thread*> tWorkerThreads;
  static tWorkerThreads* gWorkerThreads;

  timing_callback_base*                         m_callbacks[MAX_CBK_EVENT_ID_E];
  bool                                          m_has_callbacks;
  unsigned                                      m_protocol_id;
  bool                                          m_is_initiator_adaptor          = {false};

};

} // namespace scml2

#endif  //__PORT_ADAPTOR_IMPL_H__
