/***************************************************************************
 * Copyright 1996-2025 Synopsys, Inc.
 *
 * This Synopsys software and all associated documentation are proprietary
 * to Synopsys, Inc. and may only be used pursuant to the terms and
 * conditions of a written license agreement with Synopsys, Inc.
 * All other use, reproduction, modification, or distribution of the
 * Synopsys software or the associated documentation is strictly prohibited.
 ***************************************************************************/

#ifndef __SNPS_TLM2_AGFT_PROTO_EXTENSION_H__
#define __SNPS_TLM2_AGFT_PROTO_EXTENSION_H__

namespace scml2 {

//GFT Protocol States
typedef enum {
	AGFT_NONE = 0,

    AGFT_WR_CMD,
    AGFT_WR_DATA,
    AGFT_WR_DATA_LAST,

    AGFT_WR_CMD_POSTED,
    AGFT_WR_DATA_POSTED,
    AGFT_WR_DATA_LAST_POSTED,

    AGFT_WR_RESP,

    AGFT_RD_CMD,
    AGFT_RD_DATA,
    AGFT_RD_DATA_LAST,

    /* AGFT Credits */
    AGFT_WR_CMD_CREDIT,
    AGFT_WR_DATA_CREDIT,
    AGFT_WR_RESP_CREDIT,

    AGFT_RD_CMD_CREDIT,
    AGFT_RD_DATA_CREDIT,

    MAX_AGFT_STATES
} agft_protocol_state_enum;

typedef enum {
    AGFT_INVALID_CHNL
} agft_channels_enum;

DECLARE_PROTO_STATE_EXT_DECLS(agft_protocol_state_extension, agft_protocol_state_enum, AGFT_NONE);

} // namespace scml2

#endif // __SNPS_TLM2_AGFT_PROTO_EXTENSION_H__
