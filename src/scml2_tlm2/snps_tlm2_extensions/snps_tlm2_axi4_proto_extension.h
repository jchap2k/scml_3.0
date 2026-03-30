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
 *             TLM2 FT AXI4 Protocol in TLM2 Initiators and Targets. Each of 
 *             these extensions model a specific transaction attribute of 
 *             AXI4 Protocol. 
******************************************************************************/


#ifndef __SNPS_TLM2_AXI4_PROTO_EXTENSION_H__
#define __SNPS_TLM2_AXI4_PROTO_EXTENSION_H__ 

#include "tlm" 
//#include "snps_tlm2_base_extension.h"

namespace scml2 { //Top Level Name space

/******************************************************************************
AXI4 extensions still carry the 'axi' naming since most of the extensions are
compatible with axi3 and also phases remain the same.

Also the ACE extensions that align with the AXI channels are added to this 
AXI4 definition to highilight that their lifetime is associated with AXI phases 
*******************************************************************************/

/******************************************************************************
 *
 *       NOTE : FOR EACH PROTOCOL EXTENSION DEFINED HERE A CORRESPONDING SCV
 *              TYPE DECLARATION IS REQUIRED IN THE FILE
 *              snps_tlm2_axi4_proto_extension_scv_types.h for enabling tracing   
 *
 ******************************************************************************/

// AXI RD_SNOOP Type
typedef enum {
    axiInvalidSnoop     ,   //0                                              

	axiReadNoSnoop 		,	//1 
	axiReadOnce	    	,   //2 
	axiReadShared		, 	//3
	axiReadClean		,	//4 
	axiReadNotSharedDirty, 	//5
	axiReadUnique		, 	//6
	axiCleanUnique		, 	//7
	axiMakeUnique		, 	//8
	axiCleanShared		, 	//9
	axiCleanInvalid		, 	//10
	axiMakeInvalid		, 	//11
	axiDVM_Complete		, 	//12
	axiDVM_Message	    ,	//13

	axiWriteNoSnoop		,	//14                                             
	axiWriteUnique		,	//15
	axiWriteLineUnique	,	//16
	axiWriteClean		,	//17 
	axiWriteBack		,	//18 
	axiEvict			,   //19
	axiWriteEvict		,   //20

    axiMaxSnoopTypes        //21
} axi_snoop_enum;

// AXI DOMAIN Type
typedef enum {
	axiNon_Shareable	,	//0 
	axiInner_Shareable	,	//1 
	axiOuter_Shareable	,	//2 
	axiSystem			   //3
} axi_domain_enum;

// AXI BARRIER Type
typedef enum {
	axiNormal		,	//0 
	axiMemory		, 	//1
	axiIgnore		, 	//2
	axiSynchronizing	//3
} axi_barrier_enum;


// adding enums as extensions plus define extension types
DECLARE_EXTENSION_DECLS(axi_snoop_extension,  axi_snoop_enum, axiInvalidSnoop);
DECLARE_EXTENSION_DECLS(axi_domain_extension, axi_domain_enum, axiNon_Shareable);
DECLARE_EXTENSION_DECLS(axi_barrier_extension, axi_barrier_enum, axiNormal);


/******************************************************************************
AXI4 QoS extension, is a user defined 4 bit QoS identifier
*******************************************************************************/
DECLARE_EXTENSION_DECLS(axi_qos_extension, unsigned int, 0);


/******************************************************************************
AXI4 REGION extension, is a user defined 4 bit QoS identifier
*******************************************************************************/
DECLARE_EXTENSION_DECLS(axi_region_extension, unsigned int, 0);

/******************************************************************************
AXI4 AWUNIQUE extension, is a user defined bool type extension
*******************************************************************************/
DECLARE_EXTENSION_DECLS(axi_wr_unique_extension, bool, false);


} /* namespace scml2 */


//adding utility to convert FTACE to CA ACE signals.
#include "snps_tlm2_ace_utility.h"

#endif /* __SNPS_TLM2_AXI4_PROTO_EXTENSION_H__  */
