/***************************************************************************
 * Copyright 1996-2025 Synopsys, Inc.
 *
 * This Synopsys software and all associated documentation are proprietary
 * to Synopsys, Inc. and may only be used pursuant to the terms and
 * conditions of a written license agreement with Synopsys, Inc.
 * All other use, reproduction, modification, or distribution of the
 * Synopsys software or the associated documentation is strictly prohibited.
 ***************************************************************************/

/* Export extension related symbols */
    #ifndef SNPS_VP_STATIC
        #define SNPS_VP_STATIC
    #endif

#include <scml2_tlm2/snps_tlm2_extensions/snps_tlm2_extensions.h>
#include <scml2_tlm2/snps_tlm2_extensions/snps_tlm2_axi4_stream_proto_extension.h>

namespace scml2 {

DECLARE_EXTENSION_DEFS(gft_response_extension, gft_rsp_enum, gftOK);
DECLARE_EXTENSION_DEFS(axi_response_extension, axi_rsp_enum, axiOK);

DECLARE_EXTENSION_DEFS(gp_response_extension, tlm::tlm_response_status, tlm::TLM_INCOMPLETE_RESPONSE);
DECLARE_EXTENSION_DEFS(gp_burst_type_extension, unsigned int, 0);
DECLARE_EXTENSION_DEFS(ace_response_extension, ace_response_enum, aceRspNone);
DECLARE_EXTENSION_DEFS(ace_rsp_pass_data_extension, bool, false);
DECLARE_EXTENSION_DEFS(ace_rsp_was_unique_extension, bool , false);
DECLARE_EXTENSION_DEFS(snoop_master_id, unsigned int, -1);
DECLARE_EXTENSION_DEFS(cache_entry_extension, void *, 0);
DECLARE_EXTENSION_DEFS(cache_attr_extension, unsigned int, 0);
DECLARE_EXTENSION_DEFS(ace_address_extension, unsigned long long, 0);
DECLARE_EXTENSION_DEFS(ahb_protocol_state_extension, ahb_protocol_state_enum, AHB_NONE);
DECLARE_EXTENSION_DEFS(ahb_status_extension, ahb_status_enum, ahbOk);
DECLARE_EXTENSION_DEFS(ahb_burst_type_extension, ahb_burst_wrap_enum, ahbINCR);
DECLARE_EXTENSION_DEFS(ahb_lock_type_extension, ahb_lock_type_enum, ahbNORMAL);
DECLARE_EXTENSION_DEFS(amba_cacheable_extension, bool, false);
DECLARE_EXTENSION_DEFS(amba_bufferable_extension, bool, false);
DECLARE_EXTENSION_DEFS(amba_cache_wr_alloc_extension, bool, false);
DECLARE_EXTENSION_DEFS(amba_cache_rd_alloc_extension, bool, false);
DECLARE_EXTENSION_DEFS(amba_aw_user_signal_extension, sc_dt::sc_biguint<1024>, 0);
DECLARE_EXTENSION_DEFS(amba_ar_user_signal_extension, sc_dt::sc_biguint<1024>, 0);
DECLARE_EXTENSION_DEFS(amba_w_user_signal_extension, sc_dt::sc_biguint<1024>, 0);
DECLARE_EXTENSION_DEFS(amba_r_user_signal_extension, sc_dt::sc_biguint<1024>, 0);
DECLARE_EXTENSION_DEFS(amba_b_user_signal_extension, sc_dt::sc_biguint<1024>, 0);
DECLARE_EXTENSION_DEFS(axi_snoop_extension, axi_snoop_enum, axiInvalidSnoop);
DECLARE_EXTENSION_DEFS(axi_domain_extension, axi_domain_enum, axiNon_Shareable);
DECLARE_EXTENSION_DEFS(axi_barrier_extension, axi_barrier_enum, axiNormal);
DECLARE_EXTENSION_DEFS(axi_qos_extension, unsigned int, 0);
DECLARE_EXTENSION_DEFS(axi_region_extension, unsigned int, 0);
DECLARE_EXTENSION_DEFS(axi_wr_unique_extension, bool, false);
DECLARE_EXTENSION_DEFS(axi_burst_type_extension, axi_burst_type_enum, axiINCR);
DECLARE_EXTENSION_DEFS(axi_lock_type_extension, axi_lock_type_enum, axiNORMAL);
DECLARE_EXTENSION_DEFS(axi_access_mode_extension, axi_access_mode_enum, axiNORM);
DECLARE_EXTENSION_DEFS(axi_access_type_extension, axi_access_type_enum, axiDATA_ACCESS);

DECLARE_EXTENSION_DEFS(axi4_stream_tdest_extension, unsigned, 0);

DECLARE_EXTENSION_DEFS(chi_allow_retry_extension, bool, true);
DECLARE_EXTENSION_DEFS(chi_opcode_extension, chi_opcode_enum, ReqLCrdReturn);
DECLARE_EXTENSION_DEFS(chi_req_opcode_extension, chi_opcode_enum, ReqLCrdReturn);
DECLARE_EXTENSION_DEFS(chi_snp_opcode_extension, chi_opcode_enum, SnpLCrdReturn);
DECLARE_EXTENSION_DEFS(chi_data_opcode_extension, chi_opcode_enum, DataLCrdReturn);
DECLARE_EXTENSION_DEFS(chi_resp_opcode_extension, chi_opcode_enum, RespLCrdReturn);
DECLARE_EXTENSION_DEFS(chi_atomic_opcode_extension, chi_atomic_opcode_enum, ATOMIC_NONE);
DECLARE_EXTENSION_DEFS(chi_data_pull_extension, chi_data_pull_enum, NoRead);
DECLARE_EXTENSION_DEFS(chi_data_source_extension, chi_data_source_enum, DataSourceNotSupported);
DECLARE_EXTENSION_DEFS(chi_dbid_extension, unsigned, 0);
DECLARE_EXTENSION_DEFS(chi_do_not_data_pull_extension, bool, false);
DECLARE_EXTENSION_DEFS(chi_do_not_go_to_sd_extension, bool, false);
DECLARE_EXTENSION_DEFS(chi_endian_extension, bool, false);
DECLARE_EXTENSION_DEFS(chi_exclusive_extension, bool, false);
DECLARE_EXTENSION_DEFS(chi_exp_comp_ack_extension, bool, false);
DECLARE_EXTENSION_DEFS(chi_fwd_nid_extension, unsigned, 0);
DECLARE_EXTENSION_DEFS(chi_fwd_state_extension, chi_fwd_state_enum, FwdStateInvalid);
DECLARE_EXTENSION_DEFS(chi_fwd_txnid_extension, unsigned, 0);
DECLARE_EXTENSION_DEFS(chi_home_nid_extension, unsigned, 0);
DECLARE_EXTENSION_DEFS(chi_likely_shared_extension, bool, false);
DECLARE_EXTENSION_DEFS(chi_lpid_extension, unsigned, 0);
DECLARE_EXTENSION_DEFS(chi_memattr_early_write_ack_extension, bool, false);
DECLARE_EXTENSION_DEFS(chi_memattr_device_extension, bool, false);
DECLARE_EXTENSION_DEFS(chi_memattr_cacheable_extension, bool, false);
DECLARE_EXTENSION_DEFS(chi_memattr_allocate_hint_extension, bool, false);
DECLARE_EXTENSION_DEFS(chi_ccid_extension, unsigned, 0);
DECLARE_EXTENSION_DEFS(chi_dataid_extension, unsigned, 0);
DECLARE_EXTENSION_DEFS(chi_data_check_extension, unsigned long long, 0);
DECLARE_EXTENSION_DEFS(chi_poison_extension, unsigned, 0);
DECLARE_EXTENSION_DEFS(chi_non_secure_extension, chi_non_secure_enum, Secure);
DECLARE_EXTENSION_DEFS(chi_order_extension, chi_order_enum, NoOrdering);
DECLARE_EXTENSION_DEFS(chi_p_crd_type_extension, unsigned, 0);
DECLARE_EXTENSION_DEFS(chi_qos_extension, unsigned, 0);
DECLARE_EXTENSION_DEFS(chi_resp_extension, chi_resp_enum, RespInvalid);
DECLARE_EXTENSION_DEFS(chi_data_resp_extension, chi_resp_enum, RespInvalid);
DECLARE_EXTENSION_DEFS(chi_resp_err_extension, chi_resp_err_enum, NormalOkay);
DECLARE_EXTENSION_DEFS(chi_data_resp_err_extension, chi_resp_err_enum, NormalOkay);
DECLARE_EXTENSION_DEFS(chi_ret_to_src_extension, bool, false);
DECLARE_EXTENSION_DEFS(chi_return_nid_extension, unsigned, 0);
DECLARE_EXTENSION_DEFS(chi_rsvdc_extension, unsigned, 0);
DECLARE_EXTENSION_DEFS(chi_return_txnid_extension, unsigned, 0);
DECLARE_EXTENSION_DEFS(chi_snoop_me_extension, bool, false);
DECLARE_EXTENSION_DEFS(chi_cah_extension, bool, false);
DECLARE_EXTENSION_DEFS(chi_snp_attr_extension, chi_snp_attr_enum, NonSnoopable);
DECLARE_EXTENSION_DEFS(chi_srcid_extension, unsigned, 0);
DECLARE_EXTENSION_DEFS(chi_stash_lpid_extension, unsigned, 0);
DECLARE_EXTENSION_DEFS(chi_stash_lpid_valid_extension, bool, false);
DECLARE_EXTENSION_DEFS(chi_stash_nid_extension, unsigned, 0);
DECLARE_EXTENSION_DEFS(chi_stash_nid_valid_extension, bool, false);
DECLARE_EXTENSION_DEFS(chi_tgtid_extension, unsigned, 0);
DECLARE_EXTENSION_DEFS(chi_req_tgtid_extension, unsigned, 0);
DECLARE_EXTENSION_DEFS(chi_data_tgtid_extension, unsigned, 0);
DECLARE_EXTENSION_DEFS(chi_resp_tgtid_extension, unsigned, 0);
DECLARE_EXTENSION_DEFS(chi_txnid_extension, unsigned, 0);
DECLARE_EXTENSION_DEFS(chi_req_txnid_extension, unsigned, 0);
DECLARE_EXTENSION_DEFS(chi_data_txnid_extension, unsigned, 0);
DECLARE_EXTENSION_DEFS(chi_resp_txnid_extension, unsigned, 0);
DECLARE_EXTENSION_DEFS(chi_vmid_extension, unsigned, 0);

DECLARE_EXTENSION_DEFS(chi_deep_extension, bool, false);
DECLARE_EXTENSION_DEFS(chi_pgroup_id_extension, unsigned, 0);
DECLARE_EXTENSION_DEFS(chi_mpam_extension, unsigned, 0);
DECLARE_EXTENSION_DEFS(chi_pbha_extension, unsigned, 0);
DECLARE_EXTENSION_DEFS(chi_cbusy_extension, unsigned, 0);
DECLARE_EXTENSION_DEFS(chi_data_cbusy_extension, unsigned, 0);
DECLARE_EXTENSION_DEFS(chi_resp_cbusy_extension, unsigned, 0);
DECLARE_EXTENSION_DEFS(chi_slc_rep_hint_extension, chi_slc_rep_hint_enum, NO_HINT);
DECLARE_EXTENSION_DEFS(chi_do_dwt_extension, bool, false);
DECLARE_EXTENSION_DEFS(chi_tag_op_extension, chi_tag_op_enum, OP_INVALID);
DECLARE_EXTENSION_DEFS(chi_req_tag_op_extension, chi_tag_op_enum, OP_INVALID);
DECLARE_EXTENSION_DEFS(chi_data_tag_op_extension, chi_tag_op_enum, OP_INVALID);
DECLARE_EXTENSION_DEFS(chi_resp_tag_op_extension, chi_tag_op_enum, OP_INVALID);
DECLARE_EXTENSION_DEFS(chi_stash_group_id_extension, unsigned, 0);
DECLARE_EXTENSION_DEFS(chi_tag_extension, unsigned, 0);
DECLARE_EXTENSION_DEFS(chi_tag_update_extension, unsigned, 0);
DECLARE_EXTENSION_DEFS(chi_tag_group_id_extension, unsigned, 0);
DECLARE_EXTENSION_DEFS(chi_trace_tag_extension, bool, false);

DECLARE_EXTENSION_DEFS(axi2serial_bridge_phase_extn, axi2serial_bridge_cfg , axi2serial_bridge_cfg());
DECLARE_EXTENSION_DEFS(user_attr_extension, std::string, std::string(""));
DECLARE_EXTENSION_DEFS(trans_id_extension, unsigned int, 0);
DECLARE_EXTENSION_DEFS(wrap_addr_extension, unsigned long long, 0);
DECLARE_EXTENSION_DEFS(wrap_data_extension, bool, false);
DECLARE_EXTENSION_DEFS(burst_size_extension, unsigned int, 0);
DECLARE_EXTENSION_DEFS(beat_timing_req_extension, bool, true);
DECLARE_EXTENSION_DEFS(can_accept_data_beat_array_extension, bool, false);
DECLARE_EXTENSION_DEFS(next_trans_time_extension, sc_core::sc_time, sc_core::sc_time(0,sc_core::SC_NS));
DECLARE_EXTENSION_DEFS(next_trans_extension, tlm::tlm_generic_payload *, 0);
DECLARE_EXTENSION_DEFS(task_id_extension, unsigned, ~0U);
DECLARE_EXTENSION_DEFS(gft_burst_type_extension, gft_burst_type_enum, gftINCR);
DECLARE_EXTENSION_DEFS(gft_lock_type_extension, gft_lock_type_enum, gftNORMAL);
DECLARE_EXTENSION_DEFS(gft_access_mode_extension, gft_access_mode_enum, gftNORM);
DECLARE_EXTENSION_DEFS(gft_access_type_extension, gft_access_type_enum, gftDATA_ACCESS);
DECLARE_EXTENSION_DEFS(nttp_protocol_state_extension, nttp_protocol_state_enum, NTTP_NONE);
DECLARE_EXTENSION_DEFS(nttp_flowId_extension , unsigned int, 0);
DECLARE_EXTENSION_DEFS(nttp_seqId_extension , unsigned int, 0);
DECLARE_EXTENSION_DEFS(nttp_srcId_extension , unsigned int, 0);
DECLARE_EXTENSION_DEFS(nttp_echo_extension , unsigned int, 0);
DECLARE_EXTENSION_DEFS(nttp_urgency_extension , unsigned int, 0);
DECLARE_EXTENSION_DEFS(nttp_user_extension , unsigned int, 0);
DECLARE_EXTENSION_DEFS(nttp_security_extension , unsigned int, 0);
DECLARE_EXTENSION_DEFS(nttp_lock_extension , bool, false);
DECLARE_EXTENSION_DEFS(nttp_preamble_extension , bool, false);
DECLARE_EXTENSION_DEFS(nttp_pressure_extension , unsigned int, 0);
DECLARE_EXTENSION_DEFS(nttp_hurry_extension , unsigned int, 0);
DECLARE_EXTENSION_DEFS(nttp_begin_byte_extension , unsigned int, 0);
DECLARE_EXTENSION_DEFS(nttp_end_byte_extension , unsigned int, 0);

DECLARE_EXTENSION_DEFS(pcie_req_tlp_extension, pcie_req_tlp, pcie_req_tlp());
DECLARE_EXTENSION_DEFS(pcie_cpl_tlp_extension, pcie_cpl_tlp, pcie_cpl_tlp());
DECLARE_EXTENSION_DEFS(pcie_credit_dllp_extension, pcie_credit_dllp, pcie_credit_dllp());
DECLARE_EXTENSION_DEFS(pcie_dllp_extension, pcie_dllp, pcie_dllp());
DECLARE_EXTENSION_DEFS(pcie_phy_overhead_extension, pcie_phy_overhead, pcie_phy_overhead());
DECLARE_EXTENSION_DEFS(pcie_ts_extension, pcie_ts, pcie_ts());
DECLARE_EXTENSION_DEFS(pcie_flit_extension, pcie_flit, pcie_flit());

DECLARE_EXTENSION_DEFS(cxl_mem_m2s_req_extension, cxl_mem_m2s_req, cxl_mem_m2s_req());
DECLARE_EXTENSION_DEFS(cxl_mem_s2m_ndr_extension, cxl_mem_s2m_ndr, cxl_mem_s2m_ndr());
DECLARE_EXTENSION_DEFS(cxl_mem_s2m_drs_extension, cxl_mem_s2m_drs, cxl_mem_s2m_drs());

DECLARE_EXTENSION_DEFS(cxl_mem_m2s_bi_resp_extension, cxl_mem_m2s_bi_resp, cxl_mem_m2s_bi_resp());
DECLARE_EXTENSION_DEFS(cxl_mem_s2m_bi_snp_extension, cxl_mem_s2m_bi_snp, cxl_mem_s2m_bi_snp());

DECLARE_EXTENSION_DEFS(cxl_cache_d2h_req_extension, cxl_cache_d2h_req, cxl_cache_d2h_req());
DECLARE_EXTENSION_DEFS(cxl_cache_d2h_resp_extension, cxl_cache_d2h_resp, cxl_cache_d2h_resp());
DECLARE_EXTENSION_DEFS(cxl_cache_d2h_data_extension, cxl_cache_d2h_data, cxl_cache_d2h_data());
DECLARE_EXTENSION_DEFS(cxl_cache_h2d_req_extension, cxl_cache_h2d_req, cxl_cache_h2d_req());
DECLARE_EXTENSION_DEFS(cxl_cache_h2d_resp_extension, cxl_cache_h2d_resp, cxl_cache_h2d_resp());
DECLARE_EXTENSION_DEFS(cxl_cache_h2d_data_extension, cxl_cache_h2d_data, cxl_cache_h2d_data());

DECLARE_EXTENSION_DEFS(cxl_credit_control_flit_extension, cxl_credit_control_flit, cxl_credit_control_flit());
DECLARE_EXTENSION_DEFS(cxl_control_flit_extension, cxl_control_flit, cxl_control_flit());
DECLARE_EXTENSION_DEFS(cxl_cm_flit_extension, cxl_cm_flit, cxl_cm_flit());
DECLARE_EXTENSION_DEFS(cxl_io_flit_extension, cxl_io_flit, cxl_io_flit());


DECLARE_PROTO_STATE_EXT_DEFS(gp_protocol_state_extension, unsigned int, 0);
DECLARE_PROTO_STATE_EXT_DEFS(axi_protocol_state_extension, axi_protocol_state_enum, AXI_NONE)
DECLARE_PROTO_STATE_EXT_DEFS(chi_rnf_protocol_state_extension, chi_protocol_state_enum, CHI_NONE);
DECLARE_PROTO_STATE_EXT_DEFS(chi_rni_protocol_state_extension, chi_protocol_state_enum, CHI_NONE);
DECLARE_PROTO_STATE_EXT_DEFS(chi_sn_protocol_state_extension, chi_protocol_state_enum, CHI_NONE);
DECLARE_PROTO_STATE_EXT_DEFS(gft_protocol_state_extension, gft_protocol_state_enum, GFT_NONE);
DECLARE_PROTO_STATE_EXT_DEFS(agft_protocol_state_extension, agft_protocol_state_enum, AGFT_NONE);
DECLARE_PROTO_STATE_EXT_DEFS(axi4_stream_protocol_state_extension, axi4_stream_protocol_state_enum, AXI4_STREAM_NONE);
DECLARE_PROTO_STATE_EXT_DEFS(pcie_protocol_state_extension, pcie_protocol_state_enum, PCIE_NONE);
DECLARE_PROTO_STATE_EXT_DEFS(cxl_protocol_state_extension, cxl_protocol_state_enum, CXL_NONE);
DECLARE_PROTO_STATE_EXT_DEFS(ocp_protocol_state_extension, ocp_protocol_state_enum, OCP_NONE);

DECLARE_ARRAY_EXTENSION_DEFS(axi4_stream_tkeep_extension, unsigned char);
DECLARE_ARRAY_EXTENSION_DEFS(data_beat_avail_extension, unsigned int);
DECLARE_ARRAY_EXTENSION_DEFS(data_beat_used_extension, unsigned int);

}
