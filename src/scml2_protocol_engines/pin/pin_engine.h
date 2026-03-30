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

#include <systemc>
#include <scml2_base.h>
#include <scml2.h>

namespace scml2 {

  namespace pin_engine {
    class pin_engine_base {
    public:
      pin_engine_base(scml2::base::object& obj,
		      sc_core::sc_event_finder& event)
	: detached("detached")
	, m_detached(false)
      {
	sc_core::sc_spawn_options opts;
	opts.spawn_method();
	opts.set_sensitivity(&event);
	m_update_method_handle = sc_core::sc_spawn
	  (sc_bind(&pin_engine_base::update_value, this),
	   sc_core::sc_gen_unique_name((obj.get_basename() + "_update_value").c_str()), &opts);	

	detached.set_getter(this, &pin_engine_base::get_detached);
	detached.set_setter(this, &pin_engine_base::set_detached);
      }
      virtual ~pin_engine_base() {}

      scml2::base::attribute<bool> detached;
    protected:
      virtual void update_value() = 0;
      virtual void detached_changed() = 0;

      bool get_detached() {
	return m_detached;
      }
      
      void set_detached(bool value) {
	bool old_value = m_detached;
	m_detached = value;
	if (!old_value && value) {
	  m_update_method_handle.disable();
	  detached_changed();
	} else if (old_value && !value) {
	  m_update_method_handle.enable();
	  detached_changed();
	}
      }
    private:
      bool m_detached;
      sc_core::sc_process_handle m_update_method_handle;
    };
  }
  
  template<typename DT>
  class pin_slave_engine 
    : public scml2::base::object
    , private pin_engine::pin_engine_base
  {
  public:
    pin_slave_engine(const scml2::base::object_name& name, sc_core::sc_in<DT>& pin)
      : scml2::base::object(name)
      , pin_engine::pin_engine_base(*this, pin.value_changed())
      , value("value")
      , m_pin(pin)
    {
    }

    scml2::base::protected_value_attribute<pin_slave_engine<DT>, DT> value;
    using pin_engine_base::detached;
    SCML2_BASE_CALLBACK_IGNORE(value_changed, void);
  private:
    void detached_changed() override {
      if (!detached && value != m_pin) {
	update_value();
      }
    }

    void update_value() override {
      value = m_pin;
      value_changed();
    }
    
    const sc_core::sc_in<DT>& m_pin;
  };
  
  template<typename DT>
  class pin_master_engine 
    : public scml2::base::object
    , private pin_engine::pin_engine_base
  {
  public:
    pin_master_engine(const scml2::base::object_name& name, sc_core::sc_out<DT> & pin) 
      : scml2::base::object(name)
      , pin_engine::pin_engine_base(*this, pin.value_changed())
      , value("value")
      , m_pin(pin)
      , m_value_write_delta_count(~0)
    {
      value.set_post_set_callback(this, &pin_master_engine<DT>::push_value);
    }

    scml2::base::value_attribute<DT> value;
    using pin_engine_base::detached;
    SCML2_BASE_CALLBACK_IGNORE(value_changed, void);
  private:
    bool from_pin{false};
    enum state { NO_VALUE, ATTRIBUTE_VALUE, PIN_VALUE } m_state{NO_VALUE};
    
    void detached_changed() override {
      if (detached) {
	m_state = NO_VALUE;
      } else {
	if (value != m_pin) {
	  switch (m_state) {
	  case NO_VALUE:
	    update_value();
	    break;
	  case ATTRIBUTE_VALUE:
	    m_pin = value;
	    break;
	  case PIN_VALUE:
	    assert(false);
	    break;
	  }
	}
      }
    }

    void update_value() override {
      if (sc_core::sc_delta_count() != m_value_write_delta_count) {
	from_pin = true;
	switch (m_state) {
	case NO_VALUE:
	  value = m_pin;
	  m_state = PIN_VALUE;
	  break;
	case ATTRIBUTE_VALUE:
	  if (value != m_pin) {
	    m_state = PIN_VALUE;
	    value = m_pin;
	  }
	  break;
	case PIN_VALUE:
	  if (value != m_pin) {
	    value = m_pin;
	  }
	  break;
	}
	from_pin = false;
      }
   }

    void push_value() {
      m_value_write_delta_count = sc_core::sc_delta_count();
      if (!detached) {
	if (m_pin.get_interface() != nullptr) {
	  m_pin = value;
	} else {
	  m_pin.initialize(value);
	}
	if (!from_pin) {
	  m_state = ATTRIBUTE_VALUE;
	}
	value_changed();
      }
    }
    
    sc_core::sc_out<DT> & m_pin;
    sc_dt::uint64 m_value_write_delta_count;
  };

} //namespace scml2



