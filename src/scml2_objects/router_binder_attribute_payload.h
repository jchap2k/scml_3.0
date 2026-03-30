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

#include <scml2.h>

#include <memory>

namespace scml2 { 
namespace objects { 
namespace utils {

  class binder {
    class inner_binder_base {
    public:
      inner_binder_base() {}
      virtual ~inner_binder_base() {}
      virtual void finalize(scml2::mappable_if& rhs) = 0;
    };
    template<typename T>
      class inner_binder : public inner_binder_base {
      public:
      inner_binder(T& lhs_of_bind_arg) : lhs_of_bind(lhs_of_bind_arg) {}
      void finalize(scml2::mappable_if& rhs) override {
	lhs_of_bind(rhs);
      }
      private:
      T& lhs_of_bind;
    };
  public:
    binder() {}

    binder(const binder& other) 
      : m_inner_binder(other.m_inner_binder) {}

    binder(binder& other) 
      : m_inner_binder(other.m_inner_binder) {}

    binder& operator=(const binder& other) {
      m_inner_binder = other.m_inner_binder;
      return *this;
    }

    template<typename T>
      binder(T& lhs_of_bind) 
      : m_inner_binder(std::make_shared<inner_binder<T>>(lhs_of_bind)) {
    }

    void finalize(scml2::mappable_if& rhs) {
      if (m_inner_binder) {
	m_inner_binder->finalize(rhs);
      }
    }

  private:
    std::shared_ptr<inner_binder_base> m_inner_binder;
  };

}
}
}
