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
 *             TLM2 FT CHI Protocol in TLM2 Initiators and Targets. Each of
 *             these extensions model a specific transaction attribute of
 *             CHI Protocol.
******************************************************************************/


#ifndef __SNPS_TLM2_CHI_PROTO_EXTENSION_H__
#define __SNPS_TLM2_CHI_PROTO_EXTENSION_H__

#include "tlm"

namespace scml2 {

/* AllowRetry[0] REQ */
DECLARE_EXTENSION_DECLS(chi_allow_retry_extension, bool, true);

typedef enum {
    /* REQ Channel */
    ReqLCrdReturn,
    ReadShared,
    ReadClean,
    ReadOnce,
    ReadNoSnp,
    PCrdReturn,
    ReadUnique,
    CleanShared,
    CleanInvalid,
    MakeInvalid,
    CleanUnique,
    MakeUnique,
    Evict,
    EOBarrier,
    ECBarrier,
    DVMOp,
    WriteEvictFull,
    WriteCleanPtl,
    WriteCleanFull,
    WriteUniquePtl,
    WriteUniqueFull,
    WriteBackPtl,
    WriteBackFull,
    WriteNoSnpPtl,
    WriteNoSnpFull,
    WriteUniqueFullStash,
    WriteUniquePtlStash,
    StashOnceShared,
    StashOnceUnique,
    ReadOnceCleanInvalid,
    ReadOnceMakeInvalid,
    ReadNotSharedDirty,
    CleanSharedPersist,
    AtomicStore,
    AtomicLoad,
    AtomicSwap,
    AtomicCompare,
    PrefetchTgt,

    /* RSP Channel */
    RespLCrdReturn,
    SnpResp,
    CompAck,
    RetryAck,
    Comp,
    CompDBIDResp,
    DBIDResp,
    PCrdGrant,
    ReadReceipt,
    SnpRespFwded,

    /* SNP Channel */
    SnpLCrdReturn,
    SnpShared,
    SnpClean,
    SnpOnce,
    SnpNotSharedDirty,
    SnpUniqueStash,
    SnpMakeInvalidStash,
    SnpUnique,
    SnpCleanShared,
    SnpCleanInvalid,
    SnpMakeInvalid,
    SnpStashUnique,
    SnpStashShared,
    SnpDVMOp,

    SnpSharedFwd,
    SnpCleanFwd,
    SnpOnceFwd,
    SnpNotSharedDirtyFwd,
    SnpUniqueFwd,

    /* DATA Channel */
    DataLCrdReturn,
    SnpRespData,
    CopyBackWrData,
    NonCopyBackWrData,
    CompData,
    SnpRespDataPtl,
    SnpRespDataFwded,
    WriteDataCancel,

    /* CHI Issue C opcodes */

    /* RSP Channel */
    RespSepData,

    /* DATA Channel */
    DataSepResp,
    NCBWrDataCompAck,

    /* REQ Channel */
    ReadNoSnpSep,

    /* CHI Issue D opcodes */

    /* REQ Channel */
    CleanSharedPersistSep,

    /* RSP Channel */
    Persist,
    CompPersist,

    /* CHI Issue E opcodes */

    /* REQ Channel */
    MakeReadUnique,
    WriteEvictOrEvict,
    WriteUniqueZero,
    WriteNoSnpZero,
    StashOnceSepShared,
    StashOnceSepUnique,
    ReadPreferUnique,
    WriteNoSnpFullCleanSh,
    WriteNoSnpFullCleanInv,
    WriteNoSnpFullCleanShPerSep,
    WriteUniqueFullCleanSh,
    WriteUniqueFullCleanShPerSep,
    WriteBackFullCleanSh,
    WriteBackFullCleanInv,
    WriteBackFullCleanShPerSep,
    WriteCleanFullCleanSh,
    WriteCleanFullCleanShPerSep,
    WriteNoSnpPtlCleanSh,
    WriteNoSnpPtlCleanInv,
    WriteNoSnpPtlCleanShPerSep,
    WriteUniquePtlCleanSh,
    WriteUniquePtlCleanShPerSep,

    /* SNP Channel */
    SnpQuery,
    SnpPreferUnique,
    SnpPreferUniqueFwd,

    /* RSP Channel */
    TagMatch,
    DBIDRespOrd,
    StashDone,
    CompStashDone,
    CompCMO,
    Reserved
} chi_opcode_enum;

/* Opcode REQ,RSP,SNP,DATA */
DECLARE_EXTENSION_DECLS(chi_opcode_extension, chi_opcode_enum, ReqLCrdReturn);    // Deprecated. DO NOT USE

DECLARE_EXTENSION_DECLS(chi_req_opcode_extension, chi_opcode_enum, ReqLCrdReturn);

DECLARE_EXTENSION_DECLS(chi_snp_opcode_extension, chi_opcode_enum, SnpLCrdReturn);

DECLARE_EXTENSION_DECLS(chi_data_opcode_extension, chi_opcode_enum, DataLCrdReturn);

DECLARE_EXTENSION_DECLS(chi_resp_opcode_extension, chi_opcode_enum, RespLCrdReturn);

typedef enum {
    ADD,
    CLR,
    EOR,
    SET,
    SMAX,
    SMIN,
    UMAX,
    UMIN,
    ATOMIC_NONE
} chi_atomic_opcode_enum;

DECLARE_EXTENSION_DECLS(chi_atomic_opcode_extension, chi_atomic_opcode_enum, ATOMIC_NONE);

typedef enum {
    NoRead,
    Read
} chi_data_pull_enum;

/* DataPull[2:0] RSP, DATA */
DECLARE_EXTENSION_DECLS(chi_data_pull_extension, chi_data_pull_enum, NoRead);

typedef enum {
    DataSourceNotSupported,
    PrefetchTgtUseful,
    PrefetchTgtNotUseful
} chi_data_source_enum;

/* DataSource[2:0] DATA */
DECLARE_EXTENSION_DECLS(chi_data_source_extension, chi_data_source_enum, DataSourceNotSupported);

/* DBID[7:0] RSP, DATA */
DECLARE_EXTENSION_DECLS(chi_dbid_extension, unsigned, 0);

/* DoNotDataPull[0] SNP */
DECLARE_EXTENSION_DECLS(chi_do_not_data_pull_extension, bool, false);

/* DoNotGoToSD[0] SNP */
DECLARE_EXTENSION_DECLS(chi_do_not_go_to_sd_extension, bool, false);

/* Endian[0] REQ */
DECLARE_EXTENSION_DECLS(chi_endian_extension, bool, false);

/* Exclusive[0] REQ */
DECLARE_EXTENSION_DECLS(chi_exclusive_extension, bool, false);

/* ExpCompAck[0] REQ */
DECLARE_EXTENSION_DECLS(chi_exp_comp_ack_extension, bool, false);

/* FwdNID SNP */
DECLARE_EXTENSION_DECLS(chi_fwd_nid_extension, unsigned, 0);

typedef enum {
    FwdStateInvalid,
    FwdStateSharedClean,
    FwdStateUniqueClean,
    FwdStateUniqueDirty_PassDirty,
    FwdStateSharedDirty_PassDirty
} chi_fwd_state_enum;

/* FwdState[2:0] RSP, DATA */
DECLARE_EXTENSION_DECLS(chi_fwd_state_extension, chi_fwd_state_enum, FwdStateInvalid);

/* FwdTxnId[7:0] SNP */
DECLARE_EXTENSION_DECLS(chi_fwd_txnid_extension, unsigned, 0);

/* HomeNid[0] DATA */
DECLARE_EXTENSION_DECLS(chi_home_nid_extension, unsigned, 0);

/* LikelyShared[0] REQ */
DECLARE_EXTENSION_DECLS(chi_likely_shared_extension, bool, false);

/* LPID[4:0] REQ */
DECLARE_EXTENSION_DECLS(chi_lpid_extension, unsigned, 0);

/* MemAttr[3:0] REQ */
DECLARE_EXTENSION_DECLS(chi_memattr_early_write_ack_extension, bool, false);

DECLARE_EXTENSION_DECLS(chi_memattr_device_extension, bool, false);

DECLARE_EXTENSION_DECLS(chi_memattr_cacheable_extension, bool, false);

DECLARE_EXTENSION_DECLS(chi_memattr_allocate_hint_extension, bool, false);

/* CCID[1:0] DATA */
DECLARE_EXTENSION_DECLS(chi_ccid_extension, unsigned, 0);

/* DataID[1:0] DATA */
DECLARE_EXTENSION_DECLS(chi_dataid_extension, unsigned, 0);

/* DataCheck[Data_Check-1:0] DATA */
DECLARE_EXTENSION_DECLS(chi_data_check_extension, unsigned long long, 0);

/* Poison[Data_Posion-1:0] DATA */
DECLARE_EXTENSION_DECLS(chi_poison_extension, unsigned, 0);

typedef enum {
    Secure,
    NonSecure,
    Root,
    Realm,
} chi_non_secure_enum;

/* NS[0] REQ, SNP */
DECLARE_EXTENSION_DECLS(chi_non_secure_extension, chi_non_secure_enum, Secure);

typedef enum {
    NoOrdering,
    RequestAccepted,
    RequestOrderOrWriteObservation,
    EndPointOrder
} chi_order_enum;

/* Order[1:0] REQ */
DECLARE_EXTENSION_DECLS(chi_order_extension, chi_order_enum, NoOrdering);

/* PCrdType[3:0] REQ, RSP */
DECLARE_EXTENSION_DECLS(chi_p_crd_type_extension, unsigned, 0);

/* QoS[3:0] REQ, SNP, RSP, DATA */
DECLARE_EXTENSION_DECLS(chi_qos_extension, unsigned, 0);

typedef enum {
    RespInvalid,
    RespSharedClean,
    RespUniqueClean_UniqueDirty,
    RespSharedDirty,
    RespInvalid_PassDirty,
    RespSharedClean_PassDirty,
    RespUniqueClean_PassDirty,
    RespSharedDirty_PassDirty,
    RespUniqueClean,
    RespUniqueDirty,
    RespUniqueDirty_PassDirty
} chi_resp_enum;

/* Resp[2:0] RSP */
DECLARE_EXTENSION_DECLS(chi_resp_extension, chi_resp_enum, RespInvalid);

/* Resp[2:0] DATA */
DECLARE_EXTENSION_DECLS(chi_data_resp_extension, chi_resp_enum, RespInvalid);

typedef enum {
    NormalOkay,
    ExclusiveOkay,
    DataError,
    NonDataError
} chi_resp_err_enum;

/* RespErr[1:0] RSP */
DECLARE_EXTENSION_DECLS(chi_resp_err_extension, chi_resp_err_enum, NormalOkay);

/* RespErr[1:0] DATA */
DECLARE_EXTENSION_DECLS(chi_data_resp_err_extension, chi_resp_err_enum, NormalOkay);

/* RetToSrc[0] SNP */
DECLARE_EXTENSION_DECLS(chi_ret_to_src_extension, bool, false);

/* ReturnNID REQ */
DECLARE_EXTENSION_DECLS(chi_return_nid_extension, unsigned, 0);

/* RSVDC REQ, DATA */
DECLARE_EXTENSION_DECLS(chi_rsvdc_extension, unsigned, 0);

/* ReturnTxnID[7:0] REQ */
DECLARE_EXTENSION_DECLS(chi_return_txnid_extension, unsigned, 0);

/* SnoopMe[0] REQ */
DECLARE_EXTENSION_DECLS(chi_snoop_me_extension, bool, false);

/* CAH[0] REQ / DAT */
DECLARE_EXTENSION_DECLS(chi_cah_extension, bool, false);

typedef enum {
    NonSnoopable,
    Snoopable
} chi_snp_attr_enum;

/* SnpAttr[0] REQ */
DECLARE_EXTENSION_DECLS(chi_snp_attr_extension, chi_snp_attr_enum, NonSnoopable);

/* SrcID REQ, SNP, RSP, DATA */
DECLARE_EXTENSION_DECLS(chi_srcid_extension, unsigned, 0);

/* StashLPID[4:0] REQ, SNP */
DECLARE_EXTENSION_DECLS(chi_stash_lpid_extension, unsigned, 0);

/* StashLPIDValid[0] REQ, SNP */
DECLARE_EXTENSION_DECLS(chi_stash_lpid_valid_extension, bool, false);

/* StashNID REQ */
DECLARE_EXTENSION_DECLS(chi_stash_nid_extension, unsigned, 0);

/* StashNIDValid[0] REQ */
DECLARE_EXTENSION_DECLS(chi_stash_nid_valid_extension, bool, false);

/* TgtID REQ, RSP, DATA */
DECLARE_EXTENSION_DECLS(chi_tgtid_extension, unsigned, 0);    // Deprecated. DO NOT USE

DECLARE_EXTENSION_DECLS(chi_req_tgtid_extension, unsigned, 0);

DECLARE_EXTENSION_DECLS(chi_data_tgtid_extension, unsigned, 0);

DECLARE_EXTENSION_DECLS(chi_resp_tgtid_extension, unsigned, 0);

/* TxnID[7:0] REQ, SNP, RSP, DATA */
DECLARE_EXTENSION_DECLS(chi_txnid_extension, unsigned, 0);    // Deprecated. DO NOT USE
DECLARE_EXTENSION_DECLS(chi_req_txnid_extension, unsigned, 0);
DECLARE_EXTENSION_DECLS(chi_data_txnid_extension, unsigned, 0);
DECLARE_EXTENSION_DECLS(chi_resp_txnid_extension, unsigned, 0);

/* VMIDExt[7:0] SNP */
DECLARE_EXTENSION_DECLS(chi_vmid_extension, unsigned, 0);

/* Deep[0] REQ Issue D */
DECLARE_EXTENSION_DECLS(chi_deep_extension, bool, false);

/* PGroupID[4:0] REQ / RSP Issue D,  PGroupID[7:0] REQ / RSP Issue E */
DECLARE_EXTENSION_DECLS(chi_pgroup_id_extension, unsigned, 0);

/* MPAM [10:0] REQ / SNP Issue D */
DECLARE_EXTENSION_DECLS(chi_mpam_extension, unsigned, 0);

/* PBHA [3:0] REQ / SNP / DAT Issue F */
DECLARE_EXTENSION_DECLS(chi_pbha_extension, unsigned, 0);

/* CBusy [2:0] RSP, DAT Issue D */
DECLARE_EXTENSION_DECLS(chi_cbusy_extension, unsigned, 0);          // Deprecated. DO NOT USE
DECLARE_EXTENSION_DECLS(chi_resp_cbusy_extension, unsigned, 0);
DECLARE_EXTENSION_DECLS(chi_data_cbusy_extension, unsigned, 0);

typedef enum {
    NO_HINT,
    MOST_LIKELY,
    MORE_LIKELY,
    SOMEWHAT_LIKELY,
    LEAST_LIKELY
} chi_slc_rep_hint_enum;

/* SLCRepHint [6:0] REQ Issue E */
DECLARE_EXTENSION_DECLS(chi_slc_rep_hint_extension, chi_slc_rep_hint_enum, NO_HINT);

/* DoDWT [0:0] REQ Issue E */
DECLARE_EXTENSION_DECLS(chi_do_dwt_extension, bool, false);

/* StashGroupID [7:0] REQ / RSP Issue E */
DECLARE_EXTENSION_DECLS(chi_stash_group_id_extension, unsigned, 0);

/* TagGroupID [7:0] REQ / RSP Issue E */
DECLARE_EXTENSION_DECLS(chi_tag_group_id_extension, unsigned, 0);

typedef enum {
    OP_INVALID,
    OP_TRANSFER,
    OP_UPDATE,
    OP_MATCH_OR_FETCH
} chi_tag_op_enum;

/* TagOp [1:0] REQ / DAT / RSP Issue E */
DECLARE_EXTENSION_DECLS(chi_tag_op_extension, chi_tag_op_enum, OP_INVALID);        // Deprecated. DO NOT USE
DECLARE_EXTENSION_DECLS(chi_req_tag_op_extension, chi_tag_op_enum, OP_INVALID);
DECLARE_EXTENSION_DECLS(chi_data_tag_op_extension, chi_tag_op_enum, OP_INVALID);
DECLARE_EXTENSION_DECLS(chi_resp_tag_op_extension, chi_tag_op_enum, OP_INVALID);

/* Tag [DW/32-1:0] DAT Issue E */
DECLARE_EXTENSION_DECLS(chi_tag_extension, unsigned, 0);

/* TU [DW/128-1:0] DAT Issue E */
DECLARE_EXTENSION_DECLS(chi_tag_update_extension, unsigned, 0);

/* TraceTag [0:0] REQ, SNP, RSP, DAT */
DECLARE_EXTENSION_DECLS(chi_trace_tag_extension, bool, false);

typedef enum {
    CHI_NONE,
    CHI_REQ,
    CHI_CRESP,
    CHI_RDATA,
    CHI_WDATA,
    CHI_SNP,
    CHI_SRESP,

    CHI_REQ_LCREDIT,
    CHI_CRESP_LCREDIT,
    CHI_RDATA_LCREDIT,
    CHI_WDATA_LCREDIT,
    CHI_SNP_LCREDIT,
    CHI_SRESP_LCREDIT,

    // Reserved Enums, for future use
    CHI_RD_REQ,
    CHI_WR_REQ,
    CHI_RD_CRESP,
    CHI_WR_CRESP,
    CHI_RD_RESP,
    CHI_WR_RESP,
    CHI_SNP_DATA,
    CHI_PREFETCH,
    CHI_PCREDIT_GRANT,
    CHI_PCREDIT_RETURN

} chi_protocol_state_enum;

typedef enum {
    CHI_INVALID_CHNL
} chi_channels_enum;


DECLARE_PROTO_STATE_EXT_DECLS(chi_rnf_protocol_state_extension, chi_protocol_state_enum, CHI_NONE);
DECLARE_PROTO_STATE_EXT_DECLS(chi_rni_protocol_state_extension, chi_protocol_state_enum, CHI_NONE);   // Deprecated. DO NOT USE
DECLARE_PROTO_STATE_EXT_DECLS(chi_sn_protocol_state_extension, chi_protocol_state_enum, CHI_NONE);    // Deprecated. DO NOT USE

}
#endif /* __SNPS_TLM2_CHI_PROTO_EXTENSION_H__  */
