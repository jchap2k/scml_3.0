/***************************************************************************
 * Copyright 1996-2025 Synopsys, Inc.
 *
 * This Synopsys software and all associated documentation are proprietary
 * to Synopsys, Inc. and may only be used pursuant to the terms and
 * conditions of a written license agreement with Synopsys, Inc.
 * All other use, reproduction, modification, or distribution of the
 * Synopsys software or the associated documentation is strictly prohibited.
 ***************************************************************************/
#ifndef SCML2_FALLBACK_VALUE_H
#define SCML2_FALLBACK_VALUE_H

#include <functional>
#include <utility>

namespace scml2 {

  template<typename DT>
  class fallback_value 
  {
  public:
    using has_backing_value_t  = std::function<bool()>;
    using get_backing_value_t  = std::function<DT()>;
    using no_value_available_t = std::function<DT()>;
    
    fallback_value(has_backing_value_t has_backing_value, get_backing_value_t get_backing_value, no_value_available_t no_value_available);
    void set_fallback_value(DT value);
    DT get_value() const;
  private:
    has_backing_value_t m_has_backing_value;
    get_backing_value_t m_get_backing_value;
    no_value_available_t m_no_value_available;
    DT m_fallback_value{};
    bool m_fallback_valid{false};
  };
  
  template<typename DT>
  fallback_value<DT>::
  fallback_value
  (has_backing_value_t has_backing_value, 
   get_backing_value_t get_backing_value, 
   no_value_available_t no_value_available)
    : m_has_backing_value(std::move(has_backing_value))
    , m_get_backing_value(std::move(get_backing_value))
    , m_no_value_available(std::move(no_value_available))
  {
  }
  
  template<typename DT>
  void
  fallback_value<DT>::
  set_fallback_value
  (DT value)
  {
    m_fallback_value = std::move(value);
    m_fallback_valid = true;
  }
  
  template<typename DT>
  DT
  fallback_value<DT>::
  get_value
  () const
  {
    if (m_has_backing_value()) {
      return m_get_backing_value();
    } else if (m_fallback_valid) {
      return m_fallback_value;
    } else {
      return m_no_value_available();
    }
  }
  
}

#endif // SCML2_FALLBACK_VALUE_H
