/***************************************************************************
 * Copyright 1996-2025 Synopsys, Inc.
 *
 * This Synopsys software and all associated documentation are proprietary
 * to Synopsys, Inc. and may only be used pursuant to the terms and
 * conditions of a written license agreement with Synopsys, Inc.
 * All other use, reproduction, modification, or distribution of the
 * Synopsys software or the associated documentation is strictly prohibited.
 ***************************************************************************/

#ifndef SCML2_BASE_UTILS_H_INCLUDED
#define SCML2_BASE_UTILS_H_INCLUDED

#include <scml2_base/snps_vp_dll.h>

#if defined(_WIN32) && _MSC_VER > 1700 
#define SNPS_SLS_VP_BASE
#endif

#ifndef _WIN32
#define GCC_VERSION (__GNUC__ * 10000 \
                   + __GNUC_MINOR__ * 100 \
                   + __GNUC_PATCHLEVEL__)
#if GCC_VERSION > 40702 || defined(__clang__)
#define SNPS_SLS_VP_BASE
#endif // GCC_VERSION > 40702
#endif // _WIN32

#ifdef SNPS_SLS_VP_BASE

namespace scml2 { 

template <typename T>
class obj_array;

namespace base {

  class object;

  
  inline void reset_model_if_available(scml2::base::object&) {}
  template<typename C, typename = decltype(static_cast<C*>(nullptr)->reset_model())> void reset_model_if_available(C& c)
  {
    return c.reset_model();
  }
  template<typename C> void reset_model_if_available(scml2::obj_array<C>& c)
  {
    for (unsigned i = 0; i != c.size(); ++i) {
      reset_model_if_available(c[i]);
    }
  }

}}

#endif // SNPS_SLS_VP_BASE

#endif // SCML2_BASE_UTILS_H_INCLUDED
