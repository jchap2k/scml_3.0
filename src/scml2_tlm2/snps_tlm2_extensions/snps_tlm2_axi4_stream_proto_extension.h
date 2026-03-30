/***************************************************************************
 * Copyright 1996-2025 Synopsys, Inc.
 *
 * This Synopsys software and all associated documentation are proprietary
 * to Synopsys, Inc. and may only be used pursuant to the terms and
 * conditions of a written license agreement with Synopsys, Inc.
 * All other use, reproduction, modification, or distribution of the
 * Synopsys software or the associated documentation is strictly prohibited.
 ***************************************************************************/
/******************************************************************************
 *Description: This header file contains declarations for Ignorable Extensions 
 *             to TLM2 Generic Payload . These extensions can be used to model
 *             TLM2 FT AXI Protocol in TLM2 Initiators and Targets. Each of 
 *             these extensions model a specific transaction attribute of 
 *             AXI Protocol.
 *****************************************************************************/

/******************************************************************************
 *
 *       NOTE : FOR EACH PROTOCOL EXTENSION DEFINED HERE A CORRESPONDING SCV
 *              TYPE DECLARATION IS REQUIRED IN THE FILE
 *              snps_tlm2_axi_proto_extension_scv_types.h     
 *
 ******************************************************************************/
#ifndef __SNPS_TLM2_AXI4_STREAM_PROTO_EXTENSION_H__
#define __SNPS_TLM2_AXI4_STREAM_PROTO_EXTENSION_H__ 

#include "tlm" 

namespace scml2 { //Top Level Name space


typedef enum {
    AXI4_STREAM_NONE,          //0
    TVALID,
    TREADY,
    TVALID_LAST,
    TREADY_LAST,
    MAX_AXI4_STREAM_STATES     //5        
} axi4_stream_protocol_state_enum;

typedef enum {
	AXI4_STREAM_INVALID,				//0
	AXI4_STREAM_WR_DATA_CHNL,   		//1
	AXI4_STREAM_WR_DATA_LAST_CHNL, 		//2
	AXI4_STREAM_WR_RESP_CHNL 		        //3
} axi4_stream_channels_enum	;

//Array extension for TKEEP signal
DECLARE_ARRAY_EXTENSION_DECLS(axi4_stream_tkeep_extension, unsigned char);
//
//Type def for user signal extension
typedef amba_w_user_signal_extension  axi4_stream_tuser_signal_extension;
//Start of the AXI Proto Channels Info

DECLARE_EXTENSION_DECLS(axi4_stream_tdest_extension, unsigned, 0);

//Declare the AXI Proto State Ext
DECLARE_PROTO_STATE_EXT_DECLS(axi4_stream_protocol_state_extension, axi4_stream_protocol_state_enum, AXI4_STREAM_NONE)

}//end of namespace scml2

#endif /* __SNPS_TLM2_AXI_PROTO_EXTENSION_H__  */



