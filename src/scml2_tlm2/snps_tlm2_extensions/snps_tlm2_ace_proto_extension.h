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
 *             TLM2 FT ACE Protocol in TLM2 Initiators and Targets. Each of 
 *             these extensions model a specific transaction attribute of 
 *             ACE Protocol. 
******************************************************************************/


#ifndef __SNPS_TLM2_ACE_PROTO_EXTENSION_H__
#define __SNPS_TLM2_ACE_PROTO_EXTENSION_H__ 

#include "tlm" 
//#include "snps_tlm2_base_extension.h"

namespace scml2 { //Top Level Name space

/******************************************************************************
 * ACE Response Types
 ******************************************************************************/

/******************************************************************************
 *
 *       NOTE : FOR EACH PROTOCOL EXTENSION DEFINED HERE A CORRESPONDING SCV
 *              TYPE DECLARATION IS REQUIRED IN THE FILE
 *              snps_tlm2_ace_proto_extension_scv_types.h for enabling tracing   
 *
 ******************************************************************************/
#define   SHARED_BIT_MASK     (1 << 0)
#define   DIRTY_BIT_MASK      (1 << 1)
#define   ACE_RSP_MAX_BIT_ID  2

typedef enum {
   aceRspNotSharedClean = 0,
   aceRspNotSharedDirty = DIRTY_BIT_MASK,

   aceRspSharedClean    = SHARED_BIT_MASK,
   aceRspSharedDirty    = (DIRTY_BIT_MASK | SHARED_BIT_MASK),

   aceRspNone           = (1 << ACE_RSP_MAX_BIT_ID)

   //XXXAG: Not handling IsUnique for now
} ace_response_enum;

DECLARE_EXTENSION_DECLS (ace_response_extension, ace_response_enum, aceRspNone);
DECLARE_EXTENSION_DECLS (ace_rsp_pass_data_extension, bool, false);
DECLARE_EXTENSION_DECLS (ace_rsp_was_unique_extension, bool , false);

/*******************************************************************************
 * Additional custom extensions to ease modeling. 
 ******************************************************************************/
DECLARE_EXTENSION_DECLS(snoop_master_id,       unsigned int,        -1);

DECLARE_EXTENSION_DECLS(cache_entry_extension, void *,               0);
DECLARE_EXTENSION_DECLS(cache_attr_extension,  unsigned int,         0);
DECLARE_EXTENSION_DECLS(ace_address_extension, unsigned long long,   0);

} /* namespace scml2 */



#endif /* __SNPS_TLM2_ACE_PROTO_EXTENSION_H__  */
