/***************************************************************************
 * Copyright 1996-2025 Synopsys, Inc.
 *
 * This Synopsys software and all associated documentation are proprietary
 * to Synopsys, Inc. and may only be used pursuant to the terms and
 * conditions of a written license agreement with Synopsys, Inc.
 * All other use, reproduction, modification, or distribution of the
 * Synopsys software or the associated documentation is strictly prohibited.
 ***************************************************************************/

#ifndef __SNPS_TLM2_AXI2SERIAL_BRIDGE_EXTENSION_H__
#define __SNPS_TLM2_AXI2SERIAL_BRIDGE_EXTENSION_H__

#include "tlm"
#include <functional>

namespace scml2 {

  class axi2serial_bridge_cfg {
    public:
      bool operator == (const axi2serial_bridge_cfg&) const {return false;}
      
    public:
      bool is_valid() {
          return tx_data_transferred_ack && trans && interface_cbk;
      }
    
      /* Whether remote write ack is needed or not */
      bool                      *tx_data_transferred_ack        = {nullptr};
      
      /* Payload that needs remote write ack notification */
      tlm::tlm_generic_payload  *trans                          = {nullptr};
      
      /* Function to be triggered when remote write data response is received */
      std::function<void(tlm::tlm_generic_payload &, sc_core::sc_time &, int)> interface_cbk = {nullptr};
      
};

DECLARE_EXTENSION_DECLS(axi2serial_bridge_phase_extn, axi2serial_bridge_cfg , axi2serial_bridge_cfg());

}

#endif /* __SNPS_TLM2_AXI2SERIAL_BRIDGE_EXTENSION_H__  */
