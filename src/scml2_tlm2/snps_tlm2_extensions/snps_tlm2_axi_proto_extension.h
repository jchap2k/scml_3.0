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
#ifndef __SNPS_TLM2_AXI_PROTO_EXTENSION_H__
#define __SNPS_TLM2_AXI_PROTO_EXTENSION_H__ 

#include "tlm" 
//#include "snps_tlm2_base_extension.h"

namespace scml2 { //Top Level Name space

//AXI Resp Type
typedef enum {
    axiOK      ,    //0
    axiEX_OK   ,    //1 
    axiSLV_ERR ,    //2 
    axiDEC_ERR  ,    //3
} axi_rsp_enum;

//AXI Burst Type
typedef enum {
   axiFIXED      ,  //0   
   axiINCR       ,  //1 
   axiWRAP       ,  //2 
} axi_burst_type_enum;

//AXI Lock Type
typedef enum {
    axiNORMAL     ,  //0 
    axiEXCL       ,  //1 
    axiLOCK       ,  //2 
} axi_lock_type_enum;

//AXI Access Mode
typedef enum {
   axiNORM            ,  //0 
   axiPRIV            ,  //1 
   axiNORM_NON_SECURE ,  //2 
   axiPRIV_NON_SECURE    //3
} axi_access_mode_enum;

//AXI Access Type
typedef enum {
    axiDATA_ACCESS ,    //0 
    axiINST_ACCESS      //1
} axi_access_type_enum;

//Additional responses on top of TLM2 responses
DECLARE_EXTENSION_DECLS(axi_response_extension, axi_rsp_enum, axiOK); 
//Additional AXI burst types
DECLARE_EXTENSION_DECLS(axi_burst_type_extension, axi_burst_type_enum, axiINCR);

//Lock access types
DECLARE_EXTENSION_DECLS(axi_lock_type_extension, axi_lock_type_enum, axiNORMAL);
  
//AXI secure access modes
DECLARE_EXTENSION_DECLS(axi_access_mode_extension, axi_access_mode_enum, axiNORM);

//AXI access type (instruction/data)
DECLARE_EXTENSION_DECLS(axi_access_type_extension, axi_access_type_enum, axiDATA_ACCESS);

typedef enum {
   AXI_NONE,          //0

   AWVALID,           //1
   AWREADY,           //2
            
   WVALID,            //3
   WREADY,            //4
           
   WVALID_LAST,       //5
   WREADY_LAST,       //6
          
   BVALID,            //7 
   BREADY,            //8 

   ARVALID,           //9
   ARREADY,           //10
         
   RVALID,            //11
   RREADY,            //12
        
   RVALID_LAST,       //13
   RREADY_LAST,       //14

   MAX_AXI_STATES     //15        
} axi_protocol_state_enum;

typedef enum {
	AXI_INVALID,				//0
	AXI_WR_ADD_CHNL,	 		//1
	AXI_WR_DATA_CHNL,   		//2
	AXI_WR_DATA_LAST_CHNL, 		//4
	AXI_WR_RESP_CHNL,			//5
	AXI_RD_ADD_CHNL,			//6
	AXI_RD_DATA_CHNL,			//7
	AXI_RD_DATA_LAST_CHNL,		//8
	MAX_AXI_CHNLS

} axi_channels_enum	;

//Declare the AXI Proto State Ext
DECLARE_PROTO_STATE_EXT_DECLS(axi_protocol_state_extension, axi_protocol_state_enum, AXI_NONE)
}//end of namespace scml2

#endif /* __SNPS_TLM2_AXI_PROTO_EXTENSION_H__  */



