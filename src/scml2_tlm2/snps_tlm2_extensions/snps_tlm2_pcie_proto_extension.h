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
 *             TLM2 FT PCIe Protocol in TLM2 Initiators and Targets. Each of
 *             these extensions model a specific transaction attribute of
 *             PCIe Protocol.
******************************************************************************/


#ifndef __SNPS_TLM2_PCIE_PROTO_EXTENSION_H__
#define __SNPS_TLM2_PCIE_PROTO_EXTENSION_H__

#include "tlm"

namespace scml2 {

typedef enum {
    PCIE_GEN_UNKNOWN,
    PCIE_GEN_1,
    PCIE_GEN_2,
    PCIE_GEN_3,
    PCIE_GEN_4,
    PCIE_GEN_5,
    PCIE_GEN_6,

} pcie_gen_type;

// TLP type (Fmt[2:0] + type[4:0])
typedef enum {
    PCIE_MRd_3DW        = 0x00,             // 000 00000 b
    PCIE_MRd_4DW        = 0x20,             // 001 00000 b

    PCIE_MRdLk_3DW      = 0x01,             // 000 00001 b      // Only Root Port / CPU can initiate. Meant for legacy end point
    PCIE_MRdLk_4DW      = 0x21,             // 001 00001 b      // Only Root Port / CPU can initiate. Meant for legacy end point

    PCIE_MWr_3DW        = 0x40,             // 010 00000 b
    PCIE_MWr_4DW        = 0x60,             // 011 00000 b

    PCIE_IORd_3DW       = 0x02,             // 000 00010 b      // Only Root Port / CPU can initiate. Meant for legacy end point
    PCIE_IOWr_3DW       = 0x42,             // 010 00010 b      // Only Root Port / CPU can initiate. Meant for legacy end point

    PCIE_CfgRd0_3DW     = 0x04,             // 000 00100 b
    PCIE_CfgWr0_3DW     = 0x44,             // 010 00100 b
    PCIE_CfgRd1_3DW     = 0x05,             // 000 00101 b
    PCIE_CfgWr1_3DW     = 0x45,             // 010 00101 b

    PCIE_TCfgRd_3DW     = 0x1B,             // 000 11011 b
    PCIE_TCfgWr_3DW     = 0x5B,             // 010 11011 b

    PCIE_DMWr_3DW       = PCIE_TCfgWr_3DW,
    PCIE_DMWr_4DW       = 0x7B,

    PCIE_Msg_4DW        = 0x30,             // 001 10RRR b      // R is routing scheme (pcie_routing_scheme)
    PCIE_MsgD_4DW       = 0x70,             // 011 10RRR b

    PCIE_Cpl_3DW        = 0x0A,             // 000 01010 b
    PCIE_CplD_3DW       = 0x4A,             // 010 01010 b
    PCIE_CplLk_3DW      = 0x0B,             // 000 01011 b
    PCIE_CplDLk_3DW     = 0x4B,             // 010 01011 b

    PCIE_FetchAdd_3DW   = 0x4C,             // 010 01100 b
    PCIE_FetchAdd_4DW   = 0x6C,             // 011 01100 b

    PCIE_Swap_3DW       = 0x4D,             // 010 01101 b
    PCIE_Swap_4DW       = 0x6D,             // 011 01101 b

    PCIE_CAS_3DW        = 0x4E,             // 010 01110 b
    PCIE_CAS_4DW        = 0x6E,             // 011 01110 b

    PCIE_LPrfx          = 0x80,             // 100 0LLLL b,     // L is local tlp prefix type (pcie_local_tlp_prefix)
    PCIE_EPrfx          = 0x90,             // 100 1EEEE b      // E is end-end tlp prefix type (pcie_end_tlp_prefix)

} pcie_tlp_type;

// DLLP type (type[7:0])
typedef enum {
   PCIE_Ack                  = 0x00,     // 00000000
   PCIE_MRInit               = 0x01,     // 00000001
   PCIE_Data_Link_Feature    = 0x02,     // 00000010
   PCIE_Nak                  = 0x10,     // 00010000
   PCIE_Vendor_Specific      = 0x30,     // 00110000
   PCIE_NOP                  = 0x31,     // 00110001

   PCIE_InitFC1_P            = 0x40,     // 01000VVV            // V is virtual channel
   PCIE_InitFC1_NP           = 0x50,     // 01010VVV
   PCIE_InitFC1_Cpl          = 0x60,     // 01100VVV
   PCIE_MRInitFC1            = 0x70,     // 01110VVV
   PCIE_InitFC2_P            = 0xC0,     // 11000VVV
   PCIE_InitFC2_NP           = 0xD0,     // 11010VVV
   PCIE_InitFC2_Cpl          = 0xE0,     // 11100VVV
   PCIE_MRInitFC2            = 0xF0,     // 11110VVV

   PCIE_UpdateFC_P           = 0x80,     // 10000VVV
   PCIE_UpdateFC_NP          = 0x90,     // 10010VVV
   PCIE_UpdateFC_Cpl         = 0xA0,     // 10100VVV
   PCIE_MRUpdateFC           = 0xB0,     // 10110VVV

   /* Convenience enums for Gen6 DLLP */
   PCIE_OPTIMIZED_UPDATE_FC  = 0x1000,
   PCIE_FLIT_MARKER          = 0x1001,
   PCIE_NOP2                 = 0x1002,

} pcie_dllp_type;

typedef enum {
    /* Common Enums */
    PCIE_TLP_PREFIX,
    PCIE_TLP_HEADER,
    PCIE_TLP_DATA,
    PCIE_TLP,
    PCIE_DLLP,
    PCIE_TLP_LCRC,
    PCIE_DLLP_LCRC,
    PCIE_TLP_ECRC,

    /* Gen3, Gen4, Gen5 enums */
    PCIE_TOKEN_STP,                     // Start of TLP
    PCIE_TOKEN_SDP,                     // Start of DLLP
    PCIE_TOKEN_IDL,                     // Logical idle
    PCIE_TOKEN_EDB,                     // End bad token to represent nullified TLP
    PCIE_TOKEN_EDS,                     // End of stream (data block) token

    PCIE_SYNC_HEADER_DATA,              // Data Sync Header (10 b), marks beginning of a data block of 128b
    PCIE_SYNC_HEADER_ORDERED_SET,       // Ordered Set sync header (01b), marks beginning of an os block of 128b
    PCIE_SKIP_ORDERED_SET,
    PCIE_SDS_ORDERED_SET,               // Start of data stream ordered set

    /* Overheads */
    PCIE_MISC_HEADER_OVERHEAD,          // Used for TLP Prefix / TLP Trailer / TLP Suffix
    PCIE_MISC_DATA_OVERHEAD,

    /* CXL enums */
    CXL_PROTOCOL_ID_IO,                 // IO Protocol ID
    CXL_PROTOCOL_ID_CM,                 // CM Protocol ID
    CXL_CRC_RESERVED,                   // Last 2 bytes are reserved in IO flit and CRC in CM flit
    CXL_IDLE_FLIT,
    CXL_CM_FLIT,                        // CXL cache mem flit

    CXL_CM_SLOT0,
    CXL_CM_SLOT1,
    CXL_CM_SLOT2,
    CXL_CM_SLOT3,

    /* Gen6 enums */
    PCIE_TLP_OHC,
    PCIE_FLIT_CRC,                      // 8B of CRC in PCIe Flit
    PCIE_FLIT_ECC,                      // 6B of ECC in PCIe Flit

} pcie_phy_transfer_type;

typedef enum {
    PCIE_VC_0,
    PCIE_VC_1,
    PCIE_VC_2,
    PCIE_VC_3,
    PCIE_VC_4,
    PCIE_VC_5,
    PCIE_VC_6,
    PCIE_VC_7,
    PCIE_VC_MAX,

} pcie_vc_type;

// Attribute [2:1]
typedef enum {
    PCIE_DEFAULT_ORDERING,               // 00 b
    PCIE_RELAXED_ORDERING,               // 01 b
    PCIE_ID_BASED_ORDERING,              // 10 b
    PCIE_RELAXED_ID_BASED_ORDERING,      // 11 b

} pcie_attr_ordering;

// Attribute [0:0]
typedef enum {
    PCIE_DEFAULT_SNOOP,
    PCIE_NO_SNOOP

} pcie_attr_snoop;

// address type [1:0]
typedef enum {
    PCIE_DEFAULT_UNTRANSLATED,           // 00 b
    PCIE_TRANSLATION_REQUEST,            // 01 b
    PCIE_TRANSLATED,                     // 10 b

} pcie_addr_type;

typedef enum {
    PCIE_CREDIT_POSTED_HEADER,           // Minimum 1 credit = 4 DW header + Digest = 5 DW // Max 128
    PCIE_CREDIT_POSTED_DATA,             // Minimum based on max_payload_size ( min_credits = max_payload_size_dw / 4) // Max 2048
    PCIE_CREDIT_NON_POSTED_HEADER,       // Minimum 1 credit = 4 DW header + Digest = 5 DW // Max 128
    PCIE_CREDIT_NON_POSTED_DATA,         // Minimum 1 credit = 4 DW data or Minumum 2 credit for agents supporting Atomics
    PCIE_CREDIT_CPL_HEADER,              // Minimum 1 credit = 3 DW header + Digest = 4 DW // Max 128
    PCIE_CREDIT_CPL_DATA,                // Minimum based on max_payload_size ( min_credits = max_payload_size_dw / 4) // Max 2048
    PCIE_CREDIT_TYPE_MAX,

} pcie_credit_type;

typedef enum {
    PCIE_ROUTED_TO_RC,
    PCIE_ROUTED_BY_ADDRESS,
    PCIE_ROUTED_BY_ID,
    PCIE_BROADCAST_FROM_RC,
    PCIE_TERMINATE_AT_RECEIVER,
    PCIE_GATHERED_AND_ROUTED_TO_RC,

} pcie_routing_scheme;

typedef enum {
    PCIE_MRIOV          = 0x0,
    PCIE_VENDPREFIXL0   = 0xE,
    PCIE_VENDPREFIXL1   = 0xF,

} pcie_local_tlp_prefix;

typedef enum {
    PCIE_TPH            = 0x0,
    PCIE_PASID          = 0x1,
    PCIE_VENDPREFIXE0   = 0xE,
    PCIE_VENDPREFIXE1   = 0xF,

} pcie_end_tlp_prefix;

typedef enum {
    SUCCESSFUL_CPL,
    UNSUPPORTED_REQ,
    CONFIG_REQ_RETRY_STATUS,
    COMPLETER_ABORT,

} pcie_cpl_status;

// Processing Hint processing_hint PH [1:0]
typedef enum {
    BIDIRECTIONAL,              // Frequest Read / Write by host & device
    REQUESTER,                  // Frequest Read / Write by device
    TARGET,                     // Frequest Read / Write by host
    TARGET_WITH_PRIORITY,       // Frequest Read / Write by host with high temporal locality

} pcie_ph;

struct pcie_tlp_prefix {
    pcie_tlp_type           type            = {PCIE_LPrfx};
    uint32_t                value           = 0;                          // TLP Prefix is always 1 DW (32 bits)

};

using pcie_tlp_prefix_vector = std::vector<pcie_tlp_prefix>;

typedef enum {
    PCIE_NO_TRAILER                         = 0x0,
    PCIE_1DW_TRAILER_ECRC                   = 0x1,
    PCIE_2DW_TRAILER                        = 0x3,
    PCIE_3DW_IDE_MAC                        = 0x5,
    PCIE_4DW_IDE_MAC_PCRC                   = 0x6,
    PCIE_5DW_TRAILER                        = 0x7,
} pcie_trailer_type;

/* PCIe Flit Mode orthogonal header content (OHC), 1DW */
typedef enum {
    PCIE_NO_OHC,
    PCIE_OHC_A1,            // A1 is must when byte enables are needed /  /
    PCIE_OHC_A2,            // A2 is must for IO req
    PCIE_OHC_A3,            // A3 is must for Cfg req
    PCIE_OHC_A4,
    PCIE_OHC_A5,
    PCIE_OHC_B,
    PCIE_OHC_C,
    PCIE_NO_OHC_E,
    PCIE_OHC_E1,            // used for end to end prefix
    PCIE_OHC_E2,
    PCIE_OHC_E4,
} pcie_ohc_type;

struct pcie_tlp_ohc {
    pcie_ohc_type           type            = {PCIE_NO_OHC};
    uint32_t                value           = 0;                          // Each OHC is 1 DW (32 bits)

};

using pcie_tlp_ohc_vector = std::vector<pcie_tlp_ohc>;

/*
 * Class to store size / transmission time information
 * Used by Port Adaptors and IPT
 * This class also specifies interface_id (if multiple interfaces are supported)
 */

class pcie_ft_common {
public:
    class timing_info {
    public:
        timing_info() = default;

        timing_info(pcie_phy_transfer_type _type, const sc_core::sc_time &_start_time, const sc_core::sc_time &_end_time
            , uint32_t _start_symbol, uint32_t _start_lane, uint32_t _transfer_size_bits
            , uint32_t _trf_handle = 0, bool _is_start_traced = false, bool _is_end_traced = false
        )
            : type(_type)
            , start_time(_start_time)
            , end_time(_end_time)
            , start_symbol(_start_symbol)
            , start_lane(_start_lane)
            , transfer_size_bits(_transfer_size_bits)
            , trf_handle(_trf_handle)
            , is_start_traced(_is_start_traced)
            , is_end_traced(_is_end_traced)
        {

        }

        pcie_phy_transfer_type  type                    = PCIE_TOKEN_IDL;

        /* Absolute time of start and end for type @type */
        sc_core::sc_time        start_time;
        sc_core::sc_time        end_time;

        uint32_t                start_symbol            = 0;
        uint32_t                start_lane              = 0;

        /* Data transmitted over serial lanes */
        uint32_t                transfer_size_bits      = 0;

        /* Tracing related variables */
        uint32_t                trf_handle              = 0;
        bool                    is_start_traced         = false;
        bool                    is_end_traced           = false;
    };

private:
    class pcie_metadata {
    public:
        pcie_metadata() = default;

        pcie_metadata(uint32_t) {

        }

        void set_mm(tlm::tlm_mm_interface *) {

        }
    };


public:
    /**
     * WARNING!!
     * Do not add / remove any variable below
     * Use class pcie_metadata for this
     */

    /* Finer level timing details for all tokens / TLPs / DLLPs / Sync headers */
    using timing_info_vector_type = std::vector<timing_info>;
    timing_info_vector_type         timing_info_vector;

    /* Type of phy trans */
    pcie_phy_transfer_type          type                    = PCIE_TOKEN_IDL;

    /* Absolute start time measured at transaction layer */
    sc_core::sc_time                tl_start_time;

    /* Absolute start time measured at data link layer */
    sc_core::sc_time                dl_start_time;

    /* Absolute start time measured at serial lanes */
    sc_core::sc_time                start_time;

    /* Absolute end time measured at serial lanes (Includes sync headers / token etc.) */
    sc_core::sc_time                end_time;

    uint32_t                        start_symbol            = 0;
    uint32_t                        start_lane              = 0;
    uint32_t                        next_symbol             = 0;
    uint32_t                        next_lane               = 0;

    /* Number of data blocks needed for transmission (i.e. number of data sync headers) */
    uint32_t                        num_data_blocks         = 0;

    /* Number of times this packet has been retried */
    uint32_t                        num_retries             = 0;

    /* Phy packet size (Header + Data + ECRC + LCRC) */
    uint32_t                        transfer_size_bits      = 0;

    /* Variables for handling partial TLP / DLLP in CXL.IO mode */

    /* Bits that have been transmitted so far */
    uint32_t                        transferred_size_bits   = 0;

    /* Whether STP / SDP token has been striped on serial lanes or not */
    bool                            is_stp_sdp_transmitted  = false;

    /* Whether (header + LCRC) or (header + data + LCRC) has been completely transmitted or not */
    bool                            is_header_transmitted   = false;

    /* Whether outgoing callback for this packet has been triggered or not */
    bool                            is_out_cb_triggered     = false;

    bool                            is_partial_packet       = false;

    pcie_phy_transfer_type          partial_packet_type     = PCIE_TOKEN_IDL;

    /* Transaction handle used by IPT for partial packets */
    uint32_t                        trs_handle              = 0;

    /* Interface ID used by App layer to pump packets into TL */
    uint32_t                        interface_id            = 0;

    uint64_t                        rsvd                    = 0;

    /* Additional transfer size in bytes */
    uint32_t                        header_overhead         = 0;
    uint32_t                        data_overhead           = 0;

private:
    /* Future use. Using a vector to avoid changing size of class when pcie_metadata changes */
    std::vector<pcie_metadata>      metadata;

};

// Transaction layer Packet Format
// Header -- Data if Any -- ECRC if any

// Transaction layer Packet Format for Gen6 Flit Mode
// Header First DW : Type(8b) TC(3b) OHC(5b) TS(3b) Attr(3b) Length(10b) */
// Header -- Data if Any -- ECRC if any

// Data Link Layer Packet Format
// sequence_num -- TLP -- LCRC

// Physical Layer Packet format
// Framing Token(1DW) -- Header(3DW / 4DW) -- Data if any(n*DW) -- ECRC(1DW) if any -- LCRC(1DW) -- End

class pcie_common_tlp : public pcie_ft_common {
public:
    uint32_t                sequence_num                = 0;                            // 12 bit counter

    pcie_tlp_prefix_vector  local_prefix;
    pcie_tlp_prefix_vector  end_prefix;                                                 // End Prefixes are max 4
    pcie_tlp_ohc_vector     ohc;

    pcie_tlp_type           type                        = {PCIE_MRd_4DW};
    uint32_t                traffic_class               = 0;                            // Traffic Class (TC)
    pcie_attr_ordering      ordering                    = {PCIE_DEFAULT_ORDERING};      // Attr[1:0]
    pcie_attr_snoop         snoop_attr                  = {PCIE_DEFAULT_SNOOP};
    bool                    lightweight_notification    = {false};                      // Lightweight Notification
    bool                    tlp_hint                    = {false};                      // TLP Processing Hint (TH)
    bool                    tlp_digest                  = {false};                      // TLP Digest (TD)
    bool                    error_poison                = {false};                      // Error Posioning (EP)
    pcie_addr_type          address_translation         = {PCIE_DEFAULT_UNTRANSLATED};  // AT[1:0]

    uint32_t                transaction_id              = 0;                            // transaction_id = (requester_id << 10) | tag
    uint32_t                requester_id                = 0;                            // BDF / requester_id = (transaction_id >> 10)
    uint32_t                tag                         = 0;                            // tag = (transaction_id & 0x3FF)

    uint32_t                ECRC                        = 0;                            // 32 bit, only applicable if tlp_digest is true
    uint32_t                LCRC                        = 0;                            // 32 bit
    pcie_vc_type            virtual_channel             = {PCIE_VC_0};

    // Use trans.get_data_length() for length field
    // uint32_t             length;

    // Use trans.get_byte_enable_ptr() for BE field
    // uint32_t char        BE[8];

    // Use trans.get_address() for address field
    // sc_dt::uint64        address;

    /* Gen6 Flit Mode fields */
    pcie_trailer_type       trailer_type                = {PCIE_NO_TRAILER};

    bool                    uses_shared_fc              = {false};                      // TLP will consume shared credits or dedicated credits

};

// PCIe packet definitions
class pcie_req_tlp : public pcie_common_tlp {
public:
    bool operator == (const pcie_req_tlp&) const { return false; }

    // Cfg request fields
    uint32_t                bus_num         = 0;
    uint32_t                device_num      = 0;
    uint32_t                function_num    = 0;
    uint32_t                register_number = 0;

    // Msg Request Fields
    uint32_t                message_code    = 0;
    uint32_t                ECS             = 0;
    uint32_t                vendor_id       = 0;

    pcie_ph                 processing_hint = {BIDIRECTIONAL};
    uint32_t                steering_tag    = 0;

};

class pcie_cpl_tlp : public pcie_common_tlp {
public:
    bool operator == (const pcie_cpl_tlp&) const { return false; }

    uint32_t                completer_id    = 0;                          // Composed of Bus, Device and Function
    pcie_cpl_status         status          = {SUCCESSFUL_CPL};
    bool                    BCM             = {false};
    uint32_t                byte_count      = 0;                          // Remaining byte count
    uint32_t                lower_address   = 0;

};

// Flow control / DLLP definitions

class pcie_common_dllp : public pcie_ft_common {
public:
    pcie_dllp_type          type                = {PCIE_InitFC1_P};
    uint32_t                LCRC                = 0;                          // 16 bit

    // Sequence num for Ack / Nack in non-flit mode or flit sequence num in flit mode
    uint32_t                sequence_num        = 0;

    bool                    shared_fc           = {false};                    // Dedicated or Shared FC. Applicable only in flit mode

};

class pcie_credit_dllp : public pcie_common_dllp {
public:
    bool operator == (const pcie_credit_dllp&) const { return false; }

    // FC DLLP fields
    uint32_t                hdr_scale           = 0;
    uint32_t                hdr_fc              = 0;
    uint32_t                hdr_buffer_avail    = 0;
    uint32_t                data_scale          = 0;
    uint32_t                data_fc             = 0;
    uint32_t                data_buffer_avail   = 0;
    pcie_vc_type            virtual_channel     = {PCIE_VC_0};
};

class pcie_dllp : public pcie_common_dllp {
public:
    bool operator == (const pcie_dllp&) const { return false; }

    // Vendor specific DLLP fields
    uint32_t                vendor_defined  = 0;

    // Data Link Feature DLLP fields
    bool                    feature_ack     = {false};
    uint32_t                feature_support = 0;

};

class pcie_phy_overhead : public pcie_ft_common {
public:
    bool operator == (const pcie_phy_overhead&) const { return false; }

    pcie_phy_transfer_type  type            = {PCIE_TOKEN_STP};
};

/*
 * Note : TX and RX are from Initiator perspective
 * e.g. PCIE_TX_REQ_TLP is Request packet from Initiator Port to Target Port
 *      PCIE_RX_REQ_TLP is Request packet from Target Port to Initiator Port
 */

typedef enum {
    PCIE_NONE,

    PCIE_TX_REQ_TLP,
    PCIE_TX_REQ_WITH_DATA_TLP,
    PCIE_TX_CPL_TLP,
    PCIE_TX_CPL_WITH_DATA_TLP,

    PCIE_RX_REQ_TLP,                    // 5
    PCIE_RX_REQ_WITH_DATA_TLP,
    PCIE_RX_CPL_TLP,
    PCIE_RX_CPL_WITH_DATA_TLP,

    PCIE_TX_P_CREDIT_DLLP,              // Outgoing Flow Control / Credit DLLPs
    PCIE_TX_NP_CREDIT_DLLP,             // 10
    PCIE_TX_CPL_CREDIT_DLLP,

    PCIE_RX_P_CREDIT_DLLP,              // Incoming Flow Control / Credit DLLPs
    PCIE_RX_NP_CREDIT_DLLP,
    PCIE_RX_CPL_CREDIT_DLLP,

    PCIE_TX_DLLP,                       // All other DLLPs like Ack / Nack etc.
    PCIE_RX_DLLP,                       // 16

    PCIE_TX_PHY_OVERHEAD,               // PHY overhead (STP, SDP, Sync Headers etc.)
    PCIE_RX_PHY_OVERHEAD,

    PCIE_TRAINING_SET,                  // Used for link negotiation

    PCIE_TX_FLIT,                       // PCIe Gen6 Tx Flit
    PCIE_RX_FLIT,

    MAX_PCIE_STATES,

} pcie_protocol_state_enum;

typedef enum {
    PCIE_INVALID_CHNL
} pcie_channels_enum;

DECLARE_EXTENSION_DECLS(pcie_req_tlp_extension, pcie_req_tlp, pcie_req_tlp());

DECLARE_EXTENSION_DECLS(pcie_cpl_tlp_extension, pcie_cpl_tlp, pcie_cpl_tlp());

DECLARE_EXTENSION_DECLS(pcie_credit_dllp_extension, pcie_credit_dllp, pcie_credit_dllp());

DECLARE_EXTENSION_DECLS(pcie_dllp_extension, pcie_dllp, pcie_dllp());

DECLARE_EXTENSION_DECLS(pcie_phy_overhead_extension, pcie_phy_overhead, pcie_phy_overhead());

DECLARE_PROTO_STATE_EXT_DECLS(pcie_protocol_state_extension, pcie_protocol_state_enum, PCIE_NONE);

/**
 *  PCIe Negotiation protocol extension / Training Set
 *  Used to make sure that both link partners are operating at same attributes
 *  Initiator passes this extension and target modifies it to make sure both links are symmetric
 *  Extension stores a map of attribute name and its value
 */

class pcie_ts {
public:
    bool operator == (const pcie_ts&) const { return false; }

    pcie_ts() = default;

    std::map<std::string, int*> *m_attributes       = nullptr;
    bool                        m_attribute_changed = false;
    std::string                 m_rc_adaptor_name;
    std::string                 m_ep_adaptor_name;
};

DECLARE_EXTENSION_DECLS(pcie_ts_extension, pcie_ts, pcie_ts());

typedef enum {
    PCIE_Idle_Flit = 0,
    PCIE_NOP_Flit,
    PCIE_Payload_Flit
} pcie_flit_type_enum;

typedef enum {
    PCIE_Explicit_Seq_Num_Flit = 0,
    PCIE_Ack_Flit,
    PCIE_Standard_Nak_Flit,
    PCIE_Selective_Nak_Flit,
} pcie_flit_replay_cmd_enum;

class pcie_flit {
public:
    bool operator == (const pcie_flit&) const { return false; }

    pcie_flit() = default;

    pcie_flit_type_enum                         type            = {PCIE_Payload_Flit};
    pcie_flit_replay_cmd_enum                   replay_cmd      = {PCIE_Explicit_Seq_Num_Flit};
    uint32_t                                    sequence_num    = 0;

    std::vector<tlm::tlm_generic_payload*>      dllp_trans_vec;
    std::vector<tlm::tlm_generic_payload*>      tlp_trans_vec;

    uint32_t                                    LCRC            = 0;
    uint32_t                                    ECC             = 0;

    sc_core::sc_time                            flit_start_time;
    sc_core::sc_time                            flit_end_time;
    sc_core::sc_time                            lcrc_start_time;
    sc_core::sc_time                            lcrc_end_time;
};

DECLARE_EXTENSION_DECLS(pcie_flit_extension, pcie_flit, pcie_flit());

}   // namespace scml2
#endif /* __SNPS_TLM2_PCIE_PROTO_EXTENSION_H__  */
