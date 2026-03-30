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
to TLM2 Generic Payload. These are generic extensions which can be used in 
implementation of any protocol in TLM2 Initiators and Targets
******************************************************************************/




/******************************************************************************
 *
 *       NOTE : FOR EACH PROTOCOL EXTENSION DEFINED HERE A CORRESPONDING SCV
 *              TYPE DECLARATION IS REQUIRED IN THE FILE
 *              snps_tlm2_common_extension_scv_types.h     
 *
 ******************************************************************************/

#ifndef __SNPS_TLM2_COMMON_EXTENSION_H__
#define __SNPS_TLM2_COMMON_EXTENSION_H__ 

#include <string>
#include "tlm" 
#include "scml2_tlm2/snps_tlm2_extensions/snps_tlm2_extension_macros.h"

namespace scml2 {

/******************************************************************************
Name: trans_id_extension

Description: Transaction Id extension class for extending the generic payload 
			 with transaction ID
*******************************************************************************/
DECLARE_EXTENSION_DECLS(trans_id_extension, unsigned int, 0);

/******************************************************************************
Name: wrap_addr_extension

Description: Wrap addr extension class for extending the generic payload 
			 with Wrap Start Address of wrap burst
*******************************************************************************/
DECLARE_EXTENSION_DECLS(wrap_addr_extension, unsigned long long, 0);

/******************************************************************************
Name: wrap_data_extension

Description: Wrap Data extension class for allowing target to indicate to the 
			 initiator that it supports wrap burst. Target sets attribute
			 wrap data present if it is sending the wrap data 
			
*******************************************************************************/
DECLARE_EXTENSION_DECLS(wrap_data_extension, bool, false);

/******************************************************************************
Name: burst_size_extension

Description:  Burst Size extension allows master to float narrow burst transfers
			 Default value 0 implies that burst size is equal to bus width.
			
*******************************************************************************/

DECLARE_EXTENSION_DECLS(burst_size_extension, unsigned int , 0);

/******************************************************************************
Name: beat_timing_req_extension

Description:Extension to allow reciever of data to indicate whether he is 
			interested in beat level timing of data from the sender. Default is 
			true. Receiver can set it to false to indiicate he is not interested
			in beat level timing and sender can give the complete data in one go.   
			
*******************************************************************************/

DECLARE_EXTENSION_DECLS(beat_timing_req_extension, bool, true);

/******************************************************************************
Name: data_beat_avail_extension

Description: Extension class for extending the generic payload 
			 with data available timing array. This will allow the sender of the 
			 data to specify the cycle count at which each beat of the 
			 data would be available. The cycle count is relative to a base clock 
			 time, which is the time at which the transaction starts. 
*******************************************************************************/
DECLARE_ARRAY_EXTENSION_DECLS(data_beat_avail_extension, unsigned int);

/******************************************************************************
Name: data_beat_used_extension

Description: Extension class for extending the generic payload 
			 with data consumed timing array.This will allow the reciever of the
             data to specify the cycle count at which each beat of the
             data would be consumed. The cycle count is relative to a base clock
             time, which is the time at which it recieves the first beat of the
			 transaction.
*******************************************************************************/
DECLARE_ARRAY_EXTENSION_DECLS(data_beat_used_extension, unsigned int);
 
/******************************************************************************
Name: can_accept_data_beat_array_extension

Description: Extension class for extending the generic payload 
             with can_accept_data_beat_array_extension extension which will be
             used for initial handshaking during address phase between
             Initiators and Tragets to check if the reciever of the 
             data supports data_beat_array . Initiator of data will use
             data beat array only if the target of the data support
             data beat array.
*******************************************************************************/
DECLARE_EXTENSION_DECLS(can_accept_data_beat_array_extension, bool, false);

/******************************************************************************
Name: next_trans_time_extension

Description: Extension class for extending the generic payload 
			 with next transaction start time.This can be used to piggyback a 
             new transaction onto a previous transaction.

             This is to be used only in context of the FT bus/ FT infra 
             components like the mapper/Bus Converters. 

             The bus based on its configured protocol looks for this extension
             at a specific point in the transaction state machine and if found 
             uses the extension to start a new transaction on its initiator interface.

*******************************************************************************/
DECLARE_EXTENSION_DECLS(next_trans_time_extension, sc_core::sc_time, sc_core::sc_time(0,sc_core::SC_NS));

/******************************************************************************
Name: next_trans_extension

Description: Extension class for extending the generic payload 
			 with next transaction start time. This can be used to piggyback a 
             new transaction onto a previous transaction.

             This is to be used only in context of the FT bus/ FT infra 
             components like the mapper/Bus Converters. 

             The bus based on its configured protocol looks for this extension
             at a specific point in the transaction state machine and if found 
             uses the extension to start a new transaction on its initiator interface.

             The time of the new transaction is taken from next_trans_time_extension

             This extension must be set to zero after it is looked at and used. 
*******************************************************************************/
DECLARE_EXTENSION_DECLS(next_trans_extension, tlm::tlm_generic_payload *, 0);

DECLARE_EXTENSION_DECLS(task_id_extension, unsigned, ~0U);

/******************************************************************************
Name: user_attr_extension

Description: Extension class for extending the generic payload 
			 with any string type user attribute . This can be used to pass any
             string in the payload.

*******************************************************************************/
DECLARE_EXTENSION_DECLS(user_attr_extension, std::string, std::string(""));

} /* namespace scml2 */
#endif /* __SNPS_TLM2_COMMON_EXTENSION_H__  */
