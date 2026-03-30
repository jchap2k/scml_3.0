/***************************************************************************
 * Copyright 1996-2025 Synopsys, Inc.
 *
 * This Synopsys software and all associated documentation are proprietary
 * to Synopsys, Inc. and may only be used pursuant to the terms and
 * conditions of a written license agreement with Synopsys, Inc.
 * All other use, reproduction, modification, or distribution of the
 * Synopsys software or the associated documentation is strictly prohibited.
 ***************************************************************************/

#ifndef __SNPS_TLM2_OCP_PROTO_EXTENSION_H__
#define __SNPS_TLM2_OCP_PROTO_EXTENSION_H__

#include "tlm"

namespace scml2 {

typedef enum {
    OCP_NONE,

    OCP_CMD_THREAD_BUSY_CHANGE,
    OCP_DATA_THREAD_BUSY_CHANGE,
    OCP_RESP_THREAD_BUSY_CHANGE,

    OCP_BEGIN_RESET,
    OCP_END_RESET,

    OCP_BEGIN_INTERRUPT,
    OCP_END_INTERRUPT,

    OCP_MFLAG_CHANGE,
    OCP_SFLAG_CHANGE,

    OCP_BEGIN_ERROR,
    OCP_END_ERROR,

    OCP_TL2_TIMING_CHANGE,

    OCP_BEGIN_RD_REQ,
    OCP_END_RD_REQ,

    OCP_BEGIN_RD_DATA,
    OCP_END_RD_DATA,

    OCP_BEGIN_RD_RESP,
    OCP_END_RD_RESP,

    OCP_BEGIN_WR_REQ,
    OCP_END_WR_REQ,

    OCP_BEGIN_WR_DATA,
    OCP_END_WR_DATA,

    OCP_BEGIN_WR_RESP,
    OCP_END_WR_RESP,
} ocp_protocol_state_enum;

typedef enum {
	OCP_INVALID_CHNL,

} ocp_channels_enum;

DECLARE_PROTO_STATE_EXT_DECLS(ocp_protocol_state_extension, ocp_protocol_state_enum, OCP_NONE);

} // namespace scml2

#endif // __SNPS_TLM2_OCP_PROTO_EXTENSION_H__
