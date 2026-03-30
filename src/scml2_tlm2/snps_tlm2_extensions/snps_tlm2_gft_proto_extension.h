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




/******************************************************************************
 *
 *       NOTE : FOR EACH PROTOCOL EXTENSION DEFINED HERE A CORRESPONDING SCV
 *              TYPE DECLARATION IS REQUIRED IN THE FILE
 *              snps_tlm2_gft_proto_extension_scv_types.h     
 *
 ******************************************************************************/
#ifndef __SNPS_TLM2_GFT_PROTO_EXTENSION_H__
#define __SNPS_TLM2_GFT_PROTO_EXTENSION_H__ 

#include "tlm" 
//#include "snps_tlm2_base_extension.h"

namespace scml2 { //Top Level Name space

// ENUMS for GFT Transaction Attribute types.The semantics are same as for CA GFT

//GFT Resp Type
typedef enum {
   gftOK      ,   //0
   gftEX_OK   ,   //1
   gftSLV_ERR ,   //2
   gftDEC_ERR ,   //3
   gftSplit   ,   //4
   gftRetry   ,    //5
} gft_rsp_enum;

//GFT Burst Type
typedef enum {
   gftFIXED     ,   //0
   gftINCR      ,   //1
   gftWRAP      ,   //2 
} gft_burst_type_enum;

//GFT Lock Type
typedef enum {
   gftNORMAL   ,   //0    
   gftEXCL     ,   //1 
   gftLOCK     ,   //2 
} gft_lock_type_enum;

//GFT Access Mode
typedef enum {
   gftNORM            ,   //0
   gftPRIV            ,   //1
   gftNORM_NON_SECURE ,   //2
   gftPRIV_NON_SECURE     //3
} gft_access_mode_enum;

//GFT Access Type
typedef enum {
   gftDATA_ACCESS,       //0 
   gftINST_ACCESS        //1
} gft_access_type_enum;




//GFT Extension declarations

//Timing points
//DECLARE_EXTENSION_DECLS(gft_protocol_state_extension, gft_protocol_state_enum, GFT_NONE); 

//Additional responses on top of TLM2 GP responses 
DECLARE_EXTENSION_DECLS(gft_response_extension, gft_rsp_enum, gftOK); 
//Additional burst types on top of TLM2 GP  
DECLARE_EXTENSION_DECLS(gft_burst_type_extension, gft_burst_type_enum, gftINCR);

//Lock attributes 
DECLARE_EXTENSION_DECLS(gft_lock_type_extension, gft_lock_type_enum, gftNORMAL);

//Security information
DECLARE_EXTENSION_DECLS(gft_access_mode_extension, gft_access_mode_enum, gftNORM);

//Instruction/data type accesses 
DECLARE_EXTENSION_DECLS(gft_access_type_extension, gft_access_type_enum, gftDATA_ACCESS);
   

//GFT Protocol States 
typedef enum {
	GFT_NONE = 0, 

    GFT_REQ          ,     //1
    GFT_GRANT        ,     //2

    WR_CMD           ,     //3
    WR_CMD_ACK       ,     //4

    WR_DATA          ,     //5
    WR_DATA_ACK      ,     //6

    WR_DATA_LAST     ,     //7
    WR_DATA_LAST_ACK ,     //8


    RD_CMD           ,     //9
    RD_CMD_ACK       ,     //10

    RD_DATA          ,     //11
    RD_DATA_ACK      ,     //12

    RD_DATA_LAST     ,     //13
    RD_DATA_LAST_ACK ,     //14

    MAX_GFT_STATES
} gft_protocol_state_enum; 

typedef enum {

	GFT_INVALID,				//0
	GFT_REQ_CHNL,	 		//1
	GFT_WR_CMD_CHNL,   		//2
	GFT_WR_DATA_CHNL, 		//3
	GFT_WR_DATA_LAST_CHNL,			//4
	GFT_WR_RESP_CHNL,			//5
	GFT_RD_CMD_CHNL,		//6
	GFT_RD_DATA_CHNL,		//7
	GFT_RD_DATA_LAST_CHNL,	//8
	MAX_GFT_CHNLS

} gft_channels_enum	;

//Declare the GFT Proto State Ext
DECLARE_PROTO_STATE_EXT_DECLS(gft_protocol_state_extension, gft_protocol_state_enum, GFT_NONE);
} /* namespace scml2 */

#endif /* __SNPS_TLM2_GFT_PROTO_EXTENSION_H__  */
