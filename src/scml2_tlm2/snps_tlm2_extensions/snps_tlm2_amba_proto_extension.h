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
Description: This header file contains declarations for Ignorable Extensions 
to TLM2 Generic Payload . These extensions can be used to model any protocol belonging
to AMBA protocol Family using TLM2 FT API's. 
******************************************************************************/

#ifndef __SNPS_TLM2_AMBA_PROTO_EXTENSION_H__
#define __SNPS_TLM2_AMBA_PROTO_EXTENSION_H__ 


#include "tlm" 
#include "scml2_tlm2/snps_tlm2_extensions/snps_tlm2_extension_macros.h"

namespace scml2 {

/******************************************************************************
Name: amba_cacheable_extension

Description: Cacheable extension class for extending the generic payload 
			 with Cacheable attribute
*******************************************************************************/
DECLARE_EXTENSION_DECLS(amba_cacheable_extension, bool, false);

/******************************************************************************
Name: amba_bufferable_extension

Description: Bufferable extension class for extending the generic payload 
			 with bufferable attribute
*******************************************************************************/
DECLARE_EXTENSION_DECLS(amba_bufferable_extension, bool, false);

/******************************************************************************
Name: amba_cache_wr_alloc_extension

Description: Cache Write Alloc extension class for extending the generic payload 
			 with cache write_alloc attribute
*******************************************************************************/
DECLARE_EXTENSION_DECLS(amba_cache_wr_alloc_extension, bool, false);

/******************************************************************************
Name: amba_cache_rd_alloc_extension

Description: Cache Read Alloc extension class for extending the generic payload 
			 with cache read alloc attribute
*******************************************************************************/
DECLARE_EXTENSION_DECLS(amba_cache_rd_alloc_extension, bool, false);

/******************************************************************************
Name: amba_aw_user_signal_extension

Description: Extension class for extending the generic payload 
			 with user signal for Wr Addr channel
*******************************************************************************/
DECLARE_EXTENSION_DECLS(amba_aw_user_signal_extension, sc_dt::sc_biguint<1024>, 0);

/******************************************************************************
Name: amba_ar_user_signal_extension

Description: Extension class for extending the generic payload 
			 with user signal for Read Address Channel
*******************************************************************************/
DECLARE_EXTENSION_DECLS(amba_ar_user_signal_extension, sc_dt::sc_biguint<1024>, 0);

/******************************************************************************
Name: amba_w_user_signal_extension

Description: Extension class for extending the generic payload 
			 with user signal for Write Data Channel
*******************************************************************************/
DECLARE_EXTENSION_DECLS(amba_w_user_signal_extension, sc_dt::sc_biguint<1024>, 0);

/******************************************************************************
Name: amba_r_user_signal_extension

Description: Extension class for extending the generic payload 
			 with user signal for Read Data Channel
*******************************************************************************/
DECLARE_EXTENSION_DECLS(amba_r_user_signal_extension, sc_dt::sc_biguint<1024>, 0);

/******************************************************************************
Name: amba_b_user_signal_extension

Description: Extension class for extending the generic payload 
			 with user signal for Write Resp User Signal
*******************************************************************************/
DECLARE_EXTENSION_DECLS(amba_b_user_signal_extension, sc_dt::sc_biguint<1024>, 0);
} /* namespace scml2 */
#endif /* __SNPS_TLM2_AMBA_PROTO_EXTENSION_H__  */
