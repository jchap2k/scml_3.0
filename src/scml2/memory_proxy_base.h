/***************************************************************************
 * Copyright 1996-2025 Synopsys, Inc.
 *
 * This Synopsys software and all associated documentation are proprietary
 * to Synopsys, Inc. and may only be used pursuant to the terms and
 * conditions of a written license agreement with Synopsys, Inc.
 * All other use, reproduction, modification, or distribution of the
 * Synopsys software or the associated documentation is strictly prohibited.
 ***************************************************************************/
#ifndef SCML2_MEMORY_PROXY_BASE_H
#define SCML2_MEMORY_PROXY_BASE_H

#include <scml2/proxy_base.h>
#include <functional>

namespace scml2 {

namespace proxy {
  inline bool should_exist_to_bool(bool value) {
    return value;
  }

  inline bool should_exist_to_bool(const std::function<bool()>& f) {
    return f();
  }
}
  
template <typename MEMORY_OBJECT>
class memory_proxy_base
  : public proxy_base<MEMORY_OBJECT>
  , public memory_restriction_api_owner
  , public memory_callback_api_owner
  , public memory_debug_callback_api_owner
  , public memory_dmi_api_owner
  , public memory_content_observers_api_owner
  , public callback_observers_api_owner
  , public memory_description_api_owner
  , public memory_region_api_owner
{
protected:
  using proxy_base<MEMORY_OBJECT>::m_proxied;
  using proxy_base<MEMORY_OBJECT>::m_name;
  using proxy_base<MEMORY_OBJECT>::m_parent;
  using proxy_base<MEMORY_OBJECT>::type;
public:
  typedef MEMORY_OBJECT proxied_type2;

  memory_proxy_base(const std::string& name)
    : proxy_base<MEMORY_OBJECT>(name)
  {}

  virtual ~memory_proxy_base() {}

  // memory_restriction_api_owner 
  SCML2_PROXY_FORWARD_GETTER(memory_access_restriction_base*, get_read_restriction);
  SCML2_PROXY_FORWARD_GETTER(memory_access_restriction_base*, get_write_restriction);
  SCML2_PROXY_FORWARD_SETTER(set_restriction, memory_access_restriction_base*);
  SCML2_PROXY_FORWARD_SETTER(set_read_restriction, memory_access_restriction_base*);
  SCML2_PROXY_FORWARD_SETTER(set_write_restriction, memory_access_restriction_base*);
  SCML2_PROXY_FORWARD_MUTATOR(remove_read_restriction);
  SCML2_PROXY_FORWARD_MUTATOR(remove_write_restriction);
  
  // memory_callback_api_owner
  SCML2_PROXY_FORWARD_SETTER(set_callback, memory_callback_base*);
  SCML2_PROXY_FORWARD_SETTER(set_read_callback, memory_callback_base*);
  SCML2_PROXY_FORWARD_SETTER(set_write_callback, memory_callback_base*);
  SCML2_PROXY_FORWARD_MUTATOR(remove_callback);
  SCML2_PROXY_FORWARD_MUTATOR(remove_read_callback);
  SCML2_PROXY_FORWARD_MUTATOR(remove_write_callback);
  SCML2_PROXY_FORWARD_GETTER(memory_callback_base*, get_read_callback);
  SCML2_PROXY_FORWARD_GETTER(memory_callback_base*, get_write_callback);

  // memory_debug_callback_api_owner
  SCML2_PROXY_FORWARD_SETTER(set_debug_callback, memory_debug_callback_base*);
  SCML2_PROXY_FORWARD_SETTER(set_debug_read_callback, memory_debug_callback_base*);
  SCML2_PROXY_FORWARD_SETTER(set_debug_write_callback, memory_debug_callback_base*);
  SCML2_PROXY_FORWARD_MUTATOR(remove_debug_callback);
  SCML2_PROXY_FORWARD_MUTATOR(remove_debug_read_callback);
  SCML2_PROXY_FORWARD_MUTATOR(remove_debug_write_callback);
  SCML2_PROXY_FORWARD_GETTER(memory_debug_callback_base*, get_debug_read_callback);
  SCML2_PROXY_FORWARD_GETTER(memory_debug_callback_base*, get_debug_write_callback);  


  // memory_dmi_api_owner
  SCML2_PROXY_FORWARD_MUTATOR(enable_dmi);
  SCML2_PROXY_FORWARD_MUTATOR(disable_dmi);
  SCML2_PROXY_FORWARD_GETTER(bool, is_dmi_enabled);

  // memory_content_observers_api_owner
  SCML2_PROXY_FORWARD_GETTER(bool, content_is_observable);
  SCML2_PROXY_FORWARD_SETTER(register_content_observer, memory_content_observer*);
  SCML2_PROXY_FORWARD_MUTATOR(unregister_all_content_observers);
  SCML2_PROXY_FORWARD_SETTER(unregister_content_observer, memory_content_observer*);

  // callback_observers_api_owner
  SCML2_PROXY_FORWARD_SETTER(register_callback_observer, callback_observer*);
  SCML2_PROXY_FORWARD_SETTER(unregister_callback_observer, callback_observer*);
  SCML2_PROXY_FORWARD_MUTATOR(unregister_all_callback_observers);

  // memory_description_api_owner
  SCML2_PROXY_FORWARD_SETTER(set_description, const std::string&);
  SCML2_PROXY_FORWARD_GETTER(const std::string&, get_description);

  // memory_region_api_owner
  SCML2_PROXY_FORWARD_GETTER(unsigned long long, get_offset);
  SCML2_PROXY_FORWARD_GETTER(unsigned long long, get_size);
  SCML2_PROXY_FORWARD_GETTER(unsigned int, get_width);
  
private:
  memory_proxy_base(const memory_proxy_base<MEMORY_OBJECT>&);
};

}

#endif
