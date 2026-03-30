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

namespace scml2 {

typedef tlm::tlm_fw_transport_if<> fw_interface_type;
typedef tlm::tlm_bw_transport_if<> bw_interface_type;

enum tlm2_ft_mode {
  LT,
  FT,
  TLM2_GP
};

} // namespace scml2
