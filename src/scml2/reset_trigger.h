/***************************************************************************
 * Copyright 1996-2025 Synopsys, Inc.
 *
 * This Synopsys software and all associated documentation are proprietary
 * to Synopsys, Inc. and may only be used pursuant to the terms and
 * conditions of a written license agreement with Synopsys, Inc.
 * All other use, reproduction, modification, or distribution of the
 * Synopsys software or the associated documentation is strictly prohibited.
 ***************************************************************************/
#ifndef SCML2_RESET_TRIGGER_H
#define SCML2_RESET_TRIGGER_H

#include <scml2_logging/snps_vp_dll.h>
#include <systemc.h>

namespace scml2 {

  template<class MODULE_TYPE, class FUNCPTR_TYPE>
  inline
  void register_reset_trigger(const std::string& name,
			      MODULE_TYPE* mod, FUNCPTR_TYPE func, const std::string& /*cbName*/,
			      const std::string& description="")
  {
    (void)name;
    (void)mod;
    (void)func;
  }

}
  
#endif
