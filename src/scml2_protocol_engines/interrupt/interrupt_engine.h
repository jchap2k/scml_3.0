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

namespace scml2 {
  class interrupt_slave_engine : public scml2::base::object
  {
  private :
    
  public:
    interrupt_slave_engine(const scml2::base::object_name& name, sc_core::sc_in<bool>& interrupt_input)
      : scml2::base::object(name)
      , active_level("active_level")
      , enabled("enabled")
      , m_interrupt_input(interrupt_input)
    {
      scml2::set_change_callback(interrupt_input, SCML2_CALLBACK(interrupt_change));
    }


    void interrupt_change() {
      if (active_level == m_interrupt_input){
	enabled = true;
	interrupt_enter();
      } else {
	enabled = false;
	interrupt_exit();
      }
    }

    //callback definitions
    SCML2_BASE_CALLBACK_IGNORE(interrupt_enter, void);
    SCML2_BASE_CALLBACK_IGNORE(interrupt_exit, void);

    //attributes
    scml2::base::const_attribute<bool> active_level;
    scml2::base::protected_value_attribute<interrupt_slave_engine, bool> enabled;

    //behaviors

  private:
    const sc_core::sc_in<bool>& m_interrupt_input;
  };

  class interrupt_master_engine : public scml2::base::object
  {
  public:
    interrupt_master_engine(const scml2::base::object_name& name, sc_core::sc_out<bool> & interrupt_output) :
      scml2::base::object(name)
      , m_output(interrupt_output)
      , m_value(false)
      , active_level("active_level")
      , enable("enable")
    {
      enable.set_setter(this, &interrupt_master_engine::set_enable);
      enable.set_getter(this, &interrupt_master_engine::get_enable);
      active_level.set_post_set_callback(this, &interrupt_master_engine::update_active_level);
    }

  protected:
    void update_active_level() {
      m_output.initialize(!active_level.get());
    }
	
    void set_enable(bool value) { 
      if  (value) {
	m_value = active_level;
      } else {
	m_value = !active_level;
      }
      if (sc_core::sc_is_running()) {
	m_output = m_value;
      }
    }
	
    bool get_enable() { return m_value == active_level; }
	
    sc_core::sc_out<bool> & m_output;
    bool m_value;

  public:
    //attributes
    scml2::base::const_attribute<bool> active_level;
    scml2::base::attribute<bool> enable;

  };

} //namespace scml2



