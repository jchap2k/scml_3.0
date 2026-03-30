/***************************************************************************
 * Copyright 1996-2025 Synopsys, Inc.
 *
 * This Synopsys software and all associated documentation are proprietary
 * to Synopsys, Inc. and may only be used pursuant to the terms and
 * conditions of a written license agreement with Synopsys, Inc.
 * All other use, reproduction, modification, or distribution of the
 * Synopsys software or the associated documentation is strictly prohibited.
 ***************************************************************************/
#ifndef SCML2_ACCESS_PAYLOAD_PRIV_H
#define SCML2_ACCESS_PAYLOAD_PRIV_H

#include <scml2_logging/snps_vp_dll.h>

namespace tlm {
  class tlm_generic_payload;
}

namespace scml2 {
  namespace tlm2 {
    class payload_scope_helper {
    public:
      payload_scope_helper(tlm::tlm_generic_payload& payload);
      ~payload_scope_helper();
    };
  };
}

#endif
