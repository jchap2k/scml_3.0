/***************************************************************************
 * Copyright 1996-2025 Synopsys, Inc.
 *
 * This Synopsys software and all associated documentation are proprietary
 * to Synopsys, Inc. and may only be used pursuant to the terms and
 * conditions of a written license agreement with Synopsys, Inc.
 * All other use, reproduction, modification, or distribution of the
 * Synopsys software or the associated documentation is strictly prohibited.
 ***************************************************************************/

#include "scml2/access_payload.h"
#include "scml2/access_payload_priv.h"

#include <vector>
#include <cstdlib>
#include <iostream>

namespace scml2 {
  namespace tlm2 {
    static std::vector<tlm::tlm_generic_payload*> sPayloads;

    payload_scope_helper::payload_scope_helper(tlm::tlm_generic_payload& payload) {
      sPayloads.push_back(&payload);
    }
    payload_scope_helper::~payload_scope_helper() {
      sPayloads.pop_back();
    }
    
    tlm::tlm_generic_payload& get_current_payload() {
      if (sPayloads.empty()) {
	std::cerr << "ERROR: scml2::tlm::get_current_payload() called when no payload was active. Terminating the simulation." << std::endl;
	abort();
      }
      return *sPayloads.back();
    }
    
  };
}

