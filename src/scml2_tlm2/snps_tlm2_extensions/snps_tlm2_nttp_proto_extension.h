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
 * Description: This header file contains declarations for generic Ignorable 
 *            Extensions. These extensions are used to mimic a generic protocol
 *            and can be used to approximate any specific protocol with
 *            appropriate semantics for which specific extensions are not
 *            available.
 *****************************************************************************/

#ifndef __SNPS_TLM2_NTTP_PROTO_EXTENSION_H__
#define __SNPS_TLM2_NTTP_PROTO_EXTENSION_H__ 


#include "tlm" 

namespace scml2 { //Top Level Name space


// ENUMS for NTTP Protocol States

typedef enum {
	NTTP_NONE = 0, 

    NTTP_BEGIN_REQ   ,     //1
    NTTP_END_REQ     ,     //2

    NTTP_BEGIN_RESP  ,     //3
    NTTP_END_RESP    ,     //4

    BEGIN_PART_REQ   ,     //5
    END_PART_REQ     ,     //6
    ACK_PART_REQ     ,     //7

    BEGIN_PART_RESP  ,     //8
    END_PART_RESP    ,     //9
    ACK_PART_RESP    ,     //10
	
	UPDATE_REQ_SIDEBAND,  //11

    MAX_NTTP_STATES

} nttp_protocol_state_enum; 


//NTTP Extension declarations

//Timing points
DECLARE_EXTENSION_DECLS(nttp_protocol_state_extension, nttp_protocol_state_enum, NTTP_NONE); 

//Extension Declarations for Arteries NTTP protocol Attributes

//FlowId
DECLARE_EXTENSION_DECLS(nttp_flowId_extension , unsigned int, 0); 

//SeqId
DECLARE_EXTENSION_DECLS(nttp_seqId_extension , unsigned int, 0); 

//SrcId
DECLARE_EXTENSION_DECLS(nttp_srcId_extension , unsigned int, 0); 

//echo
DECLARE_EXTENSION_DECLS(nttp_echo_extension , unsigned int, 0); 

//Urgency Attribute
DECLARE_EXTENSION_DECLS(nttp_urgency_extension , unsigned int, 0); 

//User Field
DECLARE_EXTENSION_DECLS(nttp_user_extension , unsigned int, 0); 

//Security
DECLARE_EXTENSION_DECLS(nttp_security_extension , unsigned int, 0); 

//Lock
DECLARE_EXTENSION_DECLS(nttp_lock_extension , bool, false); 

//Preamble
DECLARE_EXTENSION_DECLS(nttp_preamble_extension , bool, false); 

//Pressure 
DECLARE_EXTENSION_DECLS(nttp_pressure_extension , unsigned int, 0); 

//Hurry
DECLARE_EXTENSION_DECLS(nttp_hurry_extension , unsigned int, 0); 

//num of valid bytes of data send in this partial request   
DECLARE_EXTENSION_DECLS(nttp_begin_byte_extension , unsigned int, 0); 

DECLARE_EXTENSION_DECLS(nttp_end_byte_extension , unsigned int, 0); 

} /* namespace scml2 */
#endif /* __SNPS_TLM2_NTTP_PROTO_EXTENSION_H__  */
