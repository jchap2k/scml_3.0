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
 *             TLM2 FT CXL Protocol in TLM2 Initiators and Targets. Each of
 *             these extensions model a specific transaction attribute of
 *             CXL Protocol.
******************************************************************************/


#ifndef __SNPS_TLM2_CXL_PROTO_EXTENSION_H__
#define __SNPS_TLM2_CXL_PROTO_EXTENSION_H__

#include "tlm"
#include "snps_tlm2_pcie_proto_extension.h"
#include <array>

namespace scml2 {

typedef enum {
    CXL_REVISION_1_0,
    CXL_REVISION_1_1,
    CXL_REVISION_2_0,
    CXL_REVISION_3_0,
    CXL_REVISION_3_1,
} cxl_revision_type;

/* Phy Enums */

typedef enum {
    CXL_ID_IO                       = 0xFFFF,
    CXL_ID_IO_IMPLIED_EDS           = 0xD2D2,
    CXL_ID_CACHE_MEM                = 0x5555,
    CXL_ID_CACHE_MEM_IMPLIED_EDS    = 0x8787,
    CXL_ID_NULL_FLIT                = 0x9999,
    CXL_ID_NULL_FLIT_IMPLIED_EDS    = 0x4B4B,
    CXL_ID_ALMP                     = 0xCCCC,
    CXL_ID_ALMP_IMPLIED_EDS         = 0x1E1E,

} cxl_protocol_id_type;

/* Link Layer Enums */

typedef enum {
    CXL_PROTOCOL_FLIT           = 0x0,
    CXL_CONTROL_FLIT            = 0x1,

    /* Below are convenience enums and are not defined by spec */
    CXL_CREDIT_CONTROL_FLIT     = 0x2,
    CXL_IO_PROTOCOL_FLIT        = 0x3,
    CXL_CM_PROTOCOL_FLIT        = 0x4,

} flit_type;

typedef enum {
    CXL_RETRY                   = 0x1,
    CXL_LLCRD                   = 0x0,      // Retryable (Enters LLRB)
    CXL_IDE                     = 0x2,      // Retryable (Enters LLRB)
    CXL_INIT                    = 0xC,      // Retryable (Enters LLRB)

} cxl_control_flit_type;

typedef enum {
    CXL_RETRY_IDLE              = 0x0,
    CXL_RETRY_REQ,
    CXL_RETRY_ACK,
    CXL_RETRY_FRAME,

    CXL_LLCRD_ACKNOWLEDGE,

    CXL_IDE_IDLE,
    CXL_IDE_START,
    CXL_IDE_TMAC,

    CXL_INIT_PARAM,

} cxl_control_flit_sub_type;

// CXL common

typedef enum {
    CXL_Meta_State              = 0x0,
    CXL_Extended_Meta_State     = 0x1,
    CXL_Meta_No_Op              = 0x3,

} cxl_meta_field;

typedef enum {
    CXL_Meta_State_Invalid      = 0x0,
    CXL_Meta_State_No_Op        = 0x1,
    CXL_Meta_State_Any          = 0x2,
    CXL_Meta_State_Shared       = 0x3,

} cxl_meta_value;

// CXL.mem

typedef enum {
    /* M2S Req Opcodes */
    CXL_M2S_MemInv          = 0x0,
    CXL_M2S_MemRd           = 0x1,
    CXL_M2S_MemRdData       = 0x2,
    CXL_M2S_MemRdFwd        = 0x3,
    CXL_M2S_MemWrFwd        = 0x4,
    CXL_M2S_MemRdTEE        = 0x5,
    CXL_M2S_MemRdDataTEE    = 0x6,
    CXL_M2S_MemSpecRd       = 0x8,
    CXL_M2S_MemInvNT        = 0x9,
    CXL_M2S_MemClnEvct      = 0xA,
    CXL_M2S_MemSpecRdTEE    = 0xC,
    CXL_M2S_TEUpdate        = 0xD,

    /* M2S RwD Opcodes */
    CXL_M2S_MemWr           = 0x1001,   // Actual value in spec is 0x1
    CXL_M2S_MemWrPtl        = 0x1002,   // Actual value in spec is 0x2
    CXL_M2S_BIConflict      = 0x1004,
    CXL_M2S_MemRdFill       = 0x1005,
    CXL_M2S_MemWrTEE        = 0x1009,
    CXL_M2S_MemWrPtlTEE     = 0x100A,
    CXL_M2S_MemRdFillTEE    = 0x100D,

} cxl_mem_req_opcode;

typedef enum {
    CXL_MemData             = 0x0,
    CXL_MemData_NXM         = 0x1,
    CXL_MemDataTEE          = 0x2,

} cxl_mem_data_opcode;

typedef enum {
    CXL_S2M_Cmp             = 0x0,
    CXL_S2M_Cmp_S           = 0x1,
    CXL_S2M_Cmp_E           = 0x2,
    CXL_S2M_Cmp_M           = 0x3,
    CXL_S2M_BI_ConflictAck  = 0x4,
    CXL_S2M_CmpTEE          = 0x4,

} cxl_mem_resp_opcode;

typedef enum {
    CXL_M2S_BIRspI          = 0x0,
    CXL_M2S_BIRspS          = 0x1,
    CXL_M2S_BIRspE          = 0x2,
    CXL_M2S_BIRspIBlk       = 0x4,
    CXL_M2S_BIRspSBlk       = 0x5,
    CXL_M2S_BIRspEBlk       = 0x6,

} cxl_mem_bi_resp_opcode;

typedef enum {
    CXL_S2M_BISnpCur          = 0x0,
    CXL_S2M_BISnpData         = 0x1,
    CXL_S2M_BISnpInv          = 0x2,
    CXL_S2M_BISnpCurBlk       = 0x4,
    CXL_S2M_BISnpDataBlk      = 0x5,
    CXL_S2M_BISnpInvBlk       = 0x6,

} cxl_mem_bi_snp_opcode;

typedef enum {
    CXL_S2M_Block_None      = 0x0,
    CXL_S2M_Lower_Block     = 0x1,
    CXL_S2M_Upper_Block     = 0x2,
    CXL_S2M_256B_Block      = 0x3,

} cxl_mem_bi_snp_blk;

typedef enum {
    CXL_M2S_NoSnoop         = 0x0,
    CXL_M2S_SnpData         = 0x1,
    CXL_M2S_SnpCur          = 0x2,
    CXL_M2S_SnpInv          = 0x3,

} cxl_mem_snoop_type;

typedef enum {
    CXL_Mem_Light_Load      = 0x0,
    CXL_Mem_Optimal_Load    = 0x1,
    CXL_Mem_Moderate_Load   = 0x2,
    CXL_Mem_Severe_Load     = 0x3,

} cxl_mem_dev_load;

typedef enum {
    CXL_DATA_HEADER,
    CXL_DATA_CHUNK_32B,
    CXL_DATA_CHUNK_64B,
    CXL_BYTE_ENABLE,
} cxl_data_msg_type;

// CXL.mem flit definitions

class cxl_mem_m2s_req {
public:
    bool operator == (const cxl_mem_m2s_req&) const { return false; }

    bool                Valid           = {false};                      //  1 bit (Set to true to pass user defined M2S req to port adaptor)
    cxl_mem_req_opcode  Opcode          = {CXL_M2S_MemRd};              //  4 bits
    cxl_meta_field      MetaField       = {CXL_Meta_No_Op};             //  2 bits
    cxl_meta_value      MetaValue       = {CXL_Meta_State_Invalid};     //  2 bits
    cxl_mem_snoop_type  SnpType         = {CXL_M2S_NoSnoop};            //  3 bits

    // Use trans.get_address() for address field
    //sc_dt::uint64     Address;                                        // 47 bits Rd, 46 bits Wr

    unsigned            Tag             = {0};                          // 16 bits
    unsigned            TC              = {0};                          // 2  bits
    bool                Poison          = {false};                      // 1  bit Wr, 0 bit Rd
    unsigned            LD_ID           = {0};                          // 4 bits
    unsigned            RSVD            = {0};                          // 6 bits

    bool                TRP             = {false};
    unsigned            SPID            = {0};
    unsigned            DPID            = {0};
    unsigned            CKID            = {0};
};

class cxl_mem_s2m_ndr {
public:
    bool operator == (const cxl_mem_s2m_ndr&) const { return false; }

    bool                Valid           = {false};
    cxl_mem_resp_opcode Opcode          = {CXL_S2M_Cmp};
    cxl_meta_field      MetaField       = {CXL_Meta_No_Op};
    cxl_meta_value      MetaValue       = {CXL_Meta_State_Invalid};

    unsigned            Tag             = {0};
    unsigned            LD_ID           = {0};
    cxl_mem_dev_load    DevLoad         = {CXL_Mem_Light_Load};

    unsigned            DPID            = {0};
    unsigned            RSVD            = {0};

};

class cxl_mem_s2m_drs {
public:
    bool operator == (const cxl_mem_s2m_drs&) const { return false; }

    bool                Valid           = {false};
    cxl_mem_data_opcode Opcode          = {CXL_MemData};
    cxl_meta_field      MetaField       = {CXL_Meta_No_Op};
    cxl_meta_value      MetaValue       = {CXL_Meta_State_Invalid};

    unsigned            Tag             = {0};
    bool                Poison          = {false};
    unsigned            LD_ID           = {0};
    cxl_mem_dev_load    DevLoad         = {CXL_Mem_Light_Load};
    unsigned            RSVD            = {0};

    unsigned            DPID            = {0};
    bool                TRP             = {false};
};

class cxl_mem_m2s_bi_resp {
public:
    bool operator == (const cxl_mem_m2s_bi_resp&) const { return false; }

    bool                    Valid       = {false};
    cxl_mem_bi_resp_opcode  Opcode      = {CXL_M2S_BIRspI};
    unsigned                BI_ID       = {0};
    unsigned                BITag       = {0};
    unsigned                LowAddr     = {0};
    unsigned                SPID        = {0};
    unsigned                DPID        = {0};
    unsigned                RSVD        = {0};
};

class cxl_mem_s2m_bi_snp {
public:
    bool operator == (const cxl_mem_s2m_bi_snp&) const { return false; }

    bool                    Valid       = {false};
    cxl_mem_bi_snp_opcode   Opcode      = {CXL_S2M_BISnpCur};
    unsigned                BI_ID       = {0};
    unsigned                BITag       = {0};
    cxl_mem_bi_snp_blk      Blk         = {CXL_S2M_Block_None};

    unsigned                SPID        = {0};
    unsigned                DPID        = {0};
    unsigned                RSVD        = {0};
};

// CXL.cache

typedef enum {
    CXL_D2H_RdCurr              = 0x01,
    CXL_D2H_RdOwn               = 0x02,
    CXL_D2H_RdShared            = 0x03,
    CXL_D2H_RdAny               = 0x04,
    CXL_D2H_RdOwnNoData         = 0x05,
    CXL_D2H_ItoMWr              = 0x06,
    CXL_D2H_MemWr               = 0x07,
    CXL_D2H_WrCur               = 0x07,     // WrCur = MemWr
    CXL_D2H_CLFlush             = 0x08,
    CXL_D2H_CleanEvict          = 0x09,
    CXL_D2H_DirtyEvict          = 0x0A,
    CXL_D2H_CleanEvictNoData    = 0x0B,
    CXL_D2H_WOWrInv             = 0x0C,
    CXL_D2H_WOWrInvF            = 0x0D,
    CXL_D2H_WrInv               = 0x0E,
    CXL_D2H_CacheFlushed        = 0x10,

} cxl_cache_d2h_req_opcode;

typedef enum {
    CXL_D2H_RspIHitI            = 0x04,
    CXL_D2H_RspVHitV            = 0x06,
    CXL_D2H_RspIHitSE           = 0x05,
    CXL_D2H_RspSHitSE           = 0x01,
    CXL_D2H_RspSFwdM            = 0x07,
    CXL_D2H_RspIFwdM            = 0x0F,
    CXL_D2H_RspVFwdV            = 0x16,

} cxl_cache_d2h_resp_opcode;

typedef enum {
    CXL_H2D_SnpData             = 0x1,
    CXL_H2D_SnpInv              = 0x2,
    CXL_H2D_SnpCurr             = 0x3,

} cxl_cache_h2d_req_opcode;

typedef enum {
    CXL_H2D_WritePull               = 0x1,
    CXL_H2D_GO                      = 0x4,
    CXL_H2D_GO_WritePull            = 0x5,
    CXL_H2D_ExtCmp                  = 0x6,
    CXL_H2D_GO_WritePull_Drop       = 0x8,
    CXL_H2D_Fast_GO                 = 0xC,
    CXL_H2D_Fast_GO_WritePull       = 0xD,
    CXL_H2D_GO_ERR_WritePull        = 0xF,

    CXL_H2D_GO_M,
    CXL_H2D_GO_E,
    CXL_H2D_GO_S,
    CXL_H2D_GO_I,

} cxl_cache_h2d_resp_opcode;

typedef enum {
    CXL_HOST_CACHE_MISS_TO_LOCAL_MEMORY,
    CXL_HOST_CACHE_HIT,
    CXL_HOST_CACHE_MISS_TO_REMOTE_MEMORY,

} cxl_cache_rsp_pre;

typedef enum {
    CXL_CACHE,
    CXL_MEM,
} cxl_credit_protocol;

// CXL.cache flit definitions

class cxl_cache_d2h_req {
public:
    bool operator == (const cxl_cache_d2h_req&) const { return false; }

    bool                        Valid           = {false};              //  1 bit (Set to true to pass user defined D2H req to port adaptor)
    cxl_cache_d2h_req_opcode    Opcode          = {CXL_D2H_RdCurr};     //  5 bits

    // Use trans.get_address() for address field
    //sc_dt::uint64             Address;                                // 46 bits

    unsigned                    CQID            = {0};                  // 12 bits
    bool                        NT              = {false};              //  1 bit
    unsigned                    RSVD            = {0};                  // 14 bits

    /** CXL 3.0 specific members */
    unsigned                    CacheID         = {0};                  //  4 bits in 256B flit
    unsigned                    SPID            = {0};                  // 12 bits in PBR flit
    unsigned                    DPID            = {0};                  // 12 bits in PBR flit

};                                                                      // 79b / 76b / 96b

class cxl_cache_d2h_resp {
public:
    bool operator == (const cxl_cache_d2h_resp&) const { return false; }

    bool                        Valid           = {false};              //  1 bit
    cxl_cache_d2h_resp_opcode   Opcode          = {CXL_D2H_RspIHitI};   //  5 bits

    unsigned                    UQID            = {0};                  // 12 bits
    unsigned                    RSVD            = {0};                  //  2 bits

    /** CXL 3.0 specific members */
    unsigned                    DPID            = {0};                  // 12 bits in PBR flit

};                                                                      // 20b / 24b / 36b

class cxl_cache_d2h_data {
public:
    bool operator == (const cxl_cache_d2h_data&) const { return false; }

    bool                        Valid           = {false};              //  1 bit
    unsigned                    UQID            = {0};                  // 12 bits
    bool                        ChunkValid      = {false};              //  1 bit
    bool                        Bogus           = {false};              //  1 bit
    bool                        Poison          = {false};              //  1 bit
    unsigned                    RSVD            = {0};                  //  1 bit

    /** CXL 3.0 specific members */
    unsigned                    BEP             = {0};                  //  1 bit  in 256B / PBR flit
    unsigned                    DPID            = {0};                  // 12 bits in PBR flit

};                                                                      // 17b / 24b / 36b

class cxl_cache_h2d_req {
public:
    bool operator == (const cxl_cache_h2d_req&) const { return false; }

    bool                        Valid           = {false};              //  1 bit
    cxl_cache_h2d_req_opcode    Opcode          = {CXL_H2D_SnpCurr};    //  3 bits

    // Use trans.get_address() for address field
    //sc_dt::uint64             Address;                                // 46 bits

    unsigned                    UQID            = {0};                  // 12 bits
    unsigned                    RSVD            = {0};                  //  3 bits

    /** CXL 3.0 specific members */
    unsigned                    CacheID         = {0};                  //  4 bits in 256B flit
    unsigned                    SPID            = {0};                  // 12 bits in PBR flit
    unsigned                    DPID            = {0};                  // 12 bits in PBR flit

};                                                                      // 64b / 72b / 92b

class cxl_cache_h2d_resp {
public:
    bool operator == (const cxl_cache_h2d_resp&) const { return false; }

    bool                        Valid           = {false};              //  1 bit
    cxl_cache_h2d_resp_opcode   Opcode          = {CXL_H2D_GO};         //  4 bits

    unsigned                    RspData         = {0};                  // 12 bits
    cxl_cache_rsp_pre           RSP_PRE         = {};                   //  2 bits
    unsigned                    CQID            = {0};                  // 12 bits
    unsigned                    RSVD            = {0};                  //  1 bits

    /** CXL 3.0 specific members */
    unsigned                    CacheID         = {0};                  //  4 bits in 256B flit
    unsigned                    DPID            = {0};                  // 12 bits in PBR flit

};                                                                      // 32b / 40b / 48b

class cxl_cache_h2d_data {
public:
    bool operator == (const cxl_cache_h2d_data&) const { return false; }

    bool                        Valid           = {false};              //  1 bit
    unsigned                    CQID            = {0};                  // 12 bits
    bool                        ChunkValid      = {false};              //  1 bits
    bool                        Poison          = {false};              //  1 bits
    bool                        GO_Err          = {false};              //  1 bits
    unsigned                    RSVD            = {0};                  //  8 bits

    /** CXL 3.0 specific members */
    unsigned                    CacheID         = {0};                  //  4 bits in 256B flit
    unsigned                    DPID            = {0};                  // 12 bits in PBR flit

};                                                                      // 24b / 28b / 36b

/* Control Flit structure */

using cxl_ft_common = pcie_ft_common;

class cxl_flit_timing_info {
public:
    sc_core::sc_time        protocol_id_start_time;
    sc_core::sc_time        protocol_id_end_time;
    sc_core::sc_time        flit_start_time;                 // Flit start time i.e. just after protocol ID is transmitted
    sc_core::sc_time        flit_end_time;
    sc_core::sc_time        lcrc_start_time;
    sc_core::sc_time        lcrc_end_time;
};

class cxl_common_control_flit : public cxl_ft_common, public cxl_flit_timing_info {
public:
    cxl_control_flit_type       type            = {CXL_RETRY};
    cxl_control_flit_sub_type   sub_type        = {CXL_RETRY_ACK};

    uint64_t                    rsvd_0          = {0};                  // For future use
    uint64_t                    rsvd_1          = {0};                  // For future use
};

class cxl_control_flit : public cxl_common_control_flit {
public:
    bool operator == (const cxl_control_flit&) const { return false; }

    uint32_t                    retry_sequence  = {0};                  // CXL_RETRY_REQ, CXL_RETRY_ACK
    uint32_t                    num_retry       = {0};                  // CXL_RETRY_REQ, CXL_RETRY_ACK
    bool                        viral           = {false};              // CXL_RETRY_ACK
    uint32_t                    viral_ld_id     = {0};                  // CXL_RETRY_ACK

};

class cxl_cm_chnl_credits {
public:
    uint32_t                req_credit          = {0};                  // CXL Request credits
    cxl_credit_protocol     req_credit_type     = {CXL_CACHE};

    uint32_t                data_credit         = {0};                  // CXL Data credits
    cxl_credit_protocol     data_credit_type    = {CXL_CACHE};

    uint32_t                resp_credit         = {0};                  // CXL Response credits
    cxl_credit_protocol     resp_credit_type    = {CXL_CACHE};
};

class cxl_credit_control_flit : public cxl_common_control_flit {
public:
    bool operator == (const cxl_credit_control_flit&) const { return false; }

    uint32_t                full_ack            = {0};                  // Absolute number of flits that are acknowledged
    cxl_cm_chnl_credits     cm_credits;                                 // Absolute number of credits being returned to receiver

};

typedef enum {
    CXL_NONE = PCIE_NONE,                   // 0

    CXL_IO_TX_REQ_TLP,
    CXL_IO_TX_REQ_WITH_DATA_TLP,
    CXL_IO_TX_CPL_TLP,
    CXL_IO_TX_CPL_WITH_DATA_TLP,

    CXL_IO_RX_REQ_TLP,
    CXL_IO_RX_REQ_WITH_DATA_TLP,
    CXL_IO_RX_CPL_TLP,
    CXL_IO_RX_CPL_WITH_DATA_TLP,

    CXL_IO_TX_P_CREDIT_DLLP,                // Incoming Flow Control / Credit DLLPs
    CXL_IO_TX_NP_CREDIT_DLLP,
    CXL_IO_TX_CPL_CREDIT_DLLP,

    CXL_IO_RX_P_CREDIT_DLLP,                // Outgoing Flow Control / Credit DLLPs
    CXL_IO_RX_NP_CREDIT_DLLP,
    CXL_IO_RX_CPL_CREDIT_DLLP,

    CXL_IO_TX_DLLP,                         // All other DLLPs like Ack / Nack etc.
    CXL_IO_RX_DLLP,

    CXL_TX_PHY_OVERHEAD,                    // PHY overhead (STP, SDP etc.)
    CXL_RX_PHY_OVERHEAD,

    CXL_TRAINING_SET,

    CXL_IO_TX_FLIT,                         // PCIe gen6 Tx Flit
    CXL_IO_RX_FLIT,

    CXL_MEM_M2S_REQ,
    CXL_MEM_M2S_REQ_WITH_DATA,
    CXL_MEM_M2S_BI_RESP,
    CXL_MEM_S2M_RESP,
    CXL_MEM_S2M_RESP_WITH_DATA,
    CXL_MEM_S2M_BI_SNP,

    CXL_CACHE_D2H_REQ,
    CXL_CACHE_D2H_DATA,
    CXL_CACHE_D2H_RESP,

    CXL_CACHE_H2D_REQ,
    CXL_CACHE_H2D_DATA,
    CXL_CACHE_H2D_RESP,

    CXL_TX_CREDIT_CONTROL_FLIT,
    CXL_RX_CREDIT_CONTROL_FLIT,

    CXL_TX_CONTROL_FLIT,
    CXL_RX_CONTROL_FLIT,

    CXL_TX_CM_FLIT,
    CXL_RX_CM_FLIT,

    MAX_CXL_STATES,

} cxl_protocol_state_enum;

using cxl_io_trans_map = std::map<sc_core::sc_time, tlm::tlm_generic_payload*>;

/* CXL Cache Mem Slot format */
typedef enum {
    CXL_SLOT0,
    CXL_SLOT1,
    CXL_SLOT2,
    CXL_SLOT3,
    CXL_MAX_SLOTS,

} cxl_slot_type;

typedef enum {

    CXL_H2D_M2S_H0,
    CXL_H2D_M2S_H1,
    CXL_H2D_M2S_H2,
    CXL_H2D_M2S_H3,
    CXL_H2D_M2S_H4,
    CXL_H2D_M2S_H5,
    CXL_H2D_M2S_H6,
    CXL_H2D_M2S_HDR_RSVD,

    CXL_H2D_M2S_G0,
    CXL_H2D_M2S_G1,
    CXL_H2D_M2S_G2,
    CXL_H2D_M2S_G3,
    CXL_H2D_M2S_G4,
    CXL_H2D_M2S_G5,
    CXL_H2D_M2S_RSVD,
    CXL_H2D_M2S_RSVD1,

    CXL_D2H_S2M_H0,
    CXL_D2H_S2M_H1,
    CXL_D2H_S2M_H2,
    CXL_D2H_S2M_H3,
    CXL_D2H_S2M_H4,
    CXL_D2H_S2M_H5,
    CXL_D2H_S2M_H6,
    CXL_D2H_S2M_HDR_RSVD,

    CXL_D2H_S2M_G0,
    CXL_D2H_S2M_G1,
    CXL_D2H_S2M_G2,
    CXL_D2H_S2M_G3,
    CXL_D2H_S2M_G4,
    CXL_D2H_S2M_G5,
    CXL_D2H_S2M_G6,
    CXL_D2H_S2M_RSVD,

    CXL_G0_BE,                  // G0 slot being used for byte enables
    CXL_SLOT_INVALID,           // No valid data in slot

} cxl_slot_format_type;

typedef enum {
    CXL_32B_TRANSFER,
    CXL_64B_TRANSFER,

} cxl_size_type;

typedef enum {
    CXL_BE_NOT_PRESENT,
    CXL_BE_PRESENT,

} cxl_be_type;

struct cxl_slot_trans_type {
    cxl_slot_trans_type(tlm::tlm_generic_payload *_trans, cxl_protocol_state_enum _proto_state, bool _is_last = false, bool _is_last_data = false)
        : trans(_trans)
        , proto_state(_proto_state)
        , is_last(_is_last)
        , is_last_data(_is_last_data)
    {

    }

    tlm::tlm_generic_payload    *trans                          = {nullptr};        // Slot trans
    cxl_protocol_state_enum     proto_state                     = {CXL_NONE};       // Slot trans protocol state
    bool                        is_last                         = {false};          // Whether this trans / protocol state represents last phase of transaction
    bool                        is_last_data                    = {false};          // Whether this trans / protocol state represents last data chunk
    uint32_t                    opcode                          = {0};

    sc_core::sc_time            ingress_time;                                       // Time when packet was inserted into flit packer
};

/* Class to store contents of each CXL flit slot */
class cxl_slot {
public:
    bool                                is_valid                = {false};

    /* Slot format. See Table 50, Slot Format Encoding */
    cxl_slot_format_type                slot_format             = {CXL_SLOT_INVALID};

    /* A vector of all valid payloads in this slot */
    std::vector<cxl_slot_trans_type>    trans_vector;

    /* Timing info for quick access */
    sc_core::sc_time                    start_time;
    sc_core::sc_time                    end_time;
};

using cxl_slot_arr = std::array<cxl_slot, CXL_MAX_SLOTS>;

/* CXL Cache Mem Flit structure */
class cxl_cm_flit : public cxl_ft_common, public cxl_flit_timing_info {
public:
    bool operator == (const cxl_cm_flit&) const { return false; }

    flit_type               Type            = {CXL_PROTOCOL_FLIT};
    bool                    Ak              = {0};                          // acknowledgement of 8 flits
    cxl_be_type             BE              = {CXL_BE_NOT_PRESENT};
    cxl_size_type           Size            = {CXL_64B_TRANSFER};

    cxl_cm_chnl_credits     cm_credits;

    unsigned                RSVD            = {0};
    uint32_t                LCRC            = {0};

    cxl_protocol_id_type    protocol_id     = {CXL_ID_CACHE_MEM};
    cxl_slot_arr            cxl_slots;                                      // Contents of 4 CXL flit slots
};

class cxl_io_flit : public cxl_flit_timing_info {
public:
    bool operator == (const cxl_io_flit&) const { return false; }
};

typedef enum {
    CXL_INVALID_CHNL,

    CXL_CHNL_M2S_REQ,
    CXL_CHNL_M2S_RWD,
    CXL_CHNL_M2S_BIRSP,
    CXL_CHNL_S2M_NDR,
    CXL_CHNL_S2M_DRS,
    CXL_CHNL_S2M_BISNP,

    CXL_CHNL_H2D_REQ,
    CXL_CHNL_H2D_DATA,
    CXL_CHNL_H2D_RESP,
    CXL_CHNL_D2H_REQ,
    CXL_CHNL_D2H_DATA,
    CXL_CHNL_D2H_RESP,

} cxl_channels_enum;

using cxl_io_req_tlp_extension      = pcie_req_tlp_extension;
using cxl_io_cpl_tlp_extension      = pcie_cpl_tlp_extension;
using cxl_io_credit_dllp_extension  = pcie_credit_dllp_extension;
using cxl_io_dllp_extension         = pcie_dllp_extension;

DECLARE_EXTENSION_DECLS(cxl_mem_m2s_req_extension, cxl_mem_m2s_req, cxl_mem_m2s_req());
DECLARE_EXTENSION_DECLS(cxl_mem_s2m_ndr_extension, cxl_mem_s2m_ndr, cxl_mem_s2m_ndr());
DECLARE_EXTENSION_DECLS(cxl_mem_s2m_drs_extension, cxl_mem_s2m_drs, cxl_mem_s2m_drs());

DECLARE_EXTENSION_DECLS(cxl_mem_m2s_bi_resp_extension, cxl_mem_m2s_bi_resp, cxl_mem_m2s_bi_resp());
DECLARE_EXTENSION_DECLS(cxl_mem_s2m_bi_snp_extension, cxl_mem_s2m_bi_snp, cxl_mem_s2m_bi_snp());

DECLARE_EXTENSION_DECLS(cxl_cache_d2h_req_extension, cxl_cache_d2h_req, cxl_cache_d2h_req());
DECLARE_EXTENSION_DECLS(cxl_cache_d2h_resp_extension, cxl_cache_d2h_resp, cxl_cache_d2h_resp());
DECLARE_EXTENSION_DECLS(cxl_cache_d2h_data_extension, cxl_cache_d2h_data, cxl_cache_d2h_data());
DECLARE_EXTENSION_DECLS(cxl_cache_h2d_req_extension, cxl_cache_h2d_req, cxl_cache_h2d_req());
DECLARE_EXTENSION_DECLS(cxl_cache_h2d_resp_extension, cxl_cache_h2d_resp, cxl_cache_h2d_resp());
DECLARE_EXTENSION_DECLS(cxl_cache_h2d_data_extension, cxl_cache_h2d_data, cxl_cache_h2d_data());

DECLARE_EXTENSION_DECLS(cxl_credit_control_flit_extension, cxl_credit_control_flit, cxl_credit_control_flit());
DECLARE_EXTENSION_DECLS(cxl_control_flit_extension, cxl_control_flit, cxl_control_flit());

DECLARE_EXTENSION_DECLS(cxl_cm_flit_extension, cxl_cm_flit, cxl_cm_flit());
DECLARE_EXTENSION_DECLS(cxl_io_flit_extension, cxl_io_flit, cxl_io_flit());

DECLARE_PROTO_STATE_EXT_DECLS(cxl_protocol_state_extension, cxl_protocol_state_enum, CXL_NONE);

}   // namespace scml2

#endif /* __SNPS_TLM2_CXL_PROTO_EXTENSION_H__  */
