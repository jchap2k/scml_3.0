/***************************************************************************
 * Copyright 1996-2025 Synopsys, Inc.
 *
 * This Synopsys software and all associated documentation are proprietary
 * to Synopsys, Inc. and may only be used pursuant to the terms and
 * conditions of a written license agreement with Synopsys, Inc.
 * All other use, reproduction, modification, or distribution of the
 * Synopsys software or the associated documentation is strictly prohibited.
 ***************************************************************************/

#ifndef __SNPS_TLM2_AHB_PROTO_EXTENSION_H__
#define __SNPS_TLM2_AHB_PROTO_EXTENSION_H__ 

#include "tlm" 
//#include "snps_tlm2.h"

namespace scml2 { //Top Level Name space


// ENUMS for AHB types
typedef enum {
   ahbOk    ,    //0 
   ahbError ,    //1 
   ahbRetry ,    //2 
   ahbAbort ,    //3 
   ahbSplit      //4
} ahb_status_enum;

typedef enum {
   ahbINCR          ,    //0 
   ahbWrapBurstSize ,    //1
   ahbWrapDataSize       //2
} ahb_burst_wrap_enum;


//This enum list the different protocol states for AHB Protocol, 
typedef enum {
	AHB_NONE      ,    //0 

    AHB_REQ       ,    //1 
    AHB_GRANT     ,    //2 

    AHB_ADDR      ,    //3 
    AHB_ADDR_ACK  ,     //4
    AHB_RDDATA    ,    //5
    AHB_WRDATA    ,    //6 
    AHB_EOT       ,    //7
    AHB_EOT_ACK   ,    //8
    
    AHB_SPLIT     ,    //9
    AHB_RETRY     ,    //10
    AHB_ABORT     ,     //11
    MAX_AHB_STATES
        
}ahb_protocol_state_enum;

DECLARE_EXTENSION_DECLS ( ahb_protocol_state_extension, ahb_protocol_state_enum, AHB_NONE);
#if 0
DECLARE_PROTO_STATE_EXT_DECLS (ahb_protocol_state_extension)

    //State         //GP State          	 //GP sub-qualifier          		//Engine Rsp   //ArbRequired //Throttling 
    {AHB_NONE,       0         ,        	  tlm::TLM_IGNORE_COMMAND, 				0       ,  	false	,		false}, //0

    {AHB_REQ,        0         ,        	  tlm::TLM_IGNORE_COMMAND, 			AHB_GRANT 	,	false	,		false}, //1
    {AHB_GRANT,      0         ,        	  tlm::TLM_IGNORE_COMMAND, 				0 		,	false	,		false}, //1
    {AHB_ADDR,       0         ,        	  tlm::TLM_IGNORE_COMMAND, 			AHB_ADDR_ACK,	false	,		false},//2
    {AHB_ADDR_ACK,   0         ,        	  tlm::TLM_IGNORE_COMMAND, 				0		,	false	,		false},//2

    {AHB_RDDATA,     tlm::BEGIN_REQ ,   	  tlm::TLM_READ_COMMAND,     			0       ,	false  	,		false}, //5
    {AHB_WRDATA,     tlm::BEGIN_REQ ,   	  tlm::TLM_WRITE_COMMAND,    			0       ,	false  	,		false}, //6

    {AHB_EOT,        tlm::BEGIN_RESP,   	  tlm::TLM_IGNORE_COMMAND,   			0       ,	false  	,		false}, //7
    {AHB_EOT_ACK,    tlm::END_RESP  ,   	  tlm::TLM_IGNORE_COMMAND,   			0       ,	false  	,		false},  //8
    {AHB_SPLIT,       0         ,       	  tlm::TLM_IGNORE_COMMAND, 				0		,	false	,		false},//2
    {AHB_RETRY,       0         ,       	  tlm::TLM_IGNORE_COMMAND, 				0		,	false	,		false},//2
    {AHB_ABORT,       0         ,       	  tlm::TLM_IGNORE_COMMAND, 				0		,	false	,		false},//2
    {MAX_AHB_STATES,       0         ,  	  tlm::TLM_IGNORE_COMMAND, 				0		,	false	,		false}//2

END_PROTO_STATE_EXT_DECL(ahb_protocol_state_extension, ahb_protocol_state_enum, AHB_NONE, false)
#endif
//ahb Lock Type
typedef enum {
    ahbNORMAL     ,  //0 
    ahbEXCL       ,  //1 
    ahbLOCK       ,  //2 
    ahbLOCK_ERR      //3
} ahb_lock_type_enum;

// extension for AHB status
DECLARE_EXTENSION_DECLS(ahb_status_extension, ahb_status_enum, ahbOk);

// Burst type extension for AHB
DECLARE_EXTENSION_DECLS(ahb_burst_type_extension, ahb_burst_wrap_enum, ahbINCR);

//Lock access types
DECLARE_EXTENSION_DECLS(ahb_lock_type_extension, ahb_lock_type_enum, ahbNORMAL);
  


}//namespace scml2 

#endif /* __SNPS_TLM2_AHB_PROTO_EXTENSION_H__  */
