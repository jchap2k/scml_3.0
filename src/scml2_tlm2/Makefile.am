#############################################################################
# Copyright 1996-2025 Synopsys, Inc.                                        #
#                                                                           #
# This Synopsys software and all associated documentation are proprietary   #
# to Synopsys, Inc. and may only be used pursuant to the terms and          #
# conditions of a written license agreement with Synopsys, Inc.             #
# All other use, reproduction, modification, or distribution of the         #
# Synopsys software or the associated documentation is strictly prohibited. #
#############################################################################

noinst_LIBRARIES = libscml2_tlm2.a

H_FILES = \
	ft_initiator_socket_impl.h \
	ft_target_socket_impl.h \
	snps_tlm2_utils/EnabledStateObserver.h \
	snps_tlm2_utils/ft_initiator_socket.h \
	snps_tlm2_utils/ft_socket_impl_factory.h \
	snps_tlm2_utils/ft_target_socket.h \
	snps_tlm2_utils/snps_tlm2_mm.h \
	scml2_tlm2.h \
	snps_tlm2_utils/snps_tlm2_socket_if.h \
	snps_tlm2_utils/snps_tlm2_utils.h \
	snps_tlm2_extensions/snps_tlm2_base_extension.h \
	snps_tlm2_extensions/snps_tlm2_extensions.h \
	snps_tlm2_extensions/snps_tlm2_phase_extension.h \
	snps_tlm2_extensions/snps_tlm2_extension_macros.h \
	snps_tlm2_extensions/snps_tlm2_common_extension.h \
	snps_tlm2_extensions/snps_tlm2_amba_proto_extension.h \
	snps_tlm2_extensions/snps_tlm2_ahb_proto_extension.h \
	snps_tlm2_extensions/snps_tlm2_axi_proto_extension.h \
	snps_tlm2_extensions/snps_tlm2_axi4_proto_extension.h \
	snps_tlm2_extensions/snps_tlm2_axi4_stream_proto_extension.h \
	snps_tlm2_extensions/snps_tlm2_ace_proto_extension.h \
	snps_tlm2_extensions/snps_tlm2_ace_utility.h \
	snps_tlm2_extensions/snps_tlm2_chi_proto_extension.h \
	snps_tlm2_extensions/snps_tlm2_pcie_proto_extension.h \
	snps_tlm2_extensions/snps_tlm2_cxl_proto_extension.h \
	snps_tlm2_extensions/snps_tlm2_ocp_proto_extension.h \
	snps_tlm2_extensions/snps_tlm2_gft_proto_extension.h \
	snps_tlm2_extensions/snps_tlm2_agft_proto_extension.h \
	snps_tlm2_extensions/snps_tlm2_nttp_proto_extension.h \
	snps_tlm2_extensions/snps_tlm2_axi2serial_bridge_extension.h

noinst_HEADERS = $(H_FILES)

CXX_FILES = \
	ft_socket_impl_factory.cc \
	socket_command_map.cc \
	snps_tlm2_base_extension.cc \
	snps_tlm2_extensions_defs.cc

libscml2_tlm2_a_SOURCES = $(CXX_FILES)

INCLUDES = -I$(srcdir)/.. -I$(srcdir)/snps_tlm2_extensions -I$(srcdir)/snps_tlm2_utils $(OSCITLM_INC) $(SYSC_INC)

INCLUDE_DIR = $(prefix)/include
LIB_DIR = $(prefix)/lib-$(TARGET_ARCH)

install-data-local: scml2_tlm2.h
	@$(NORMAL_INSTALL)
	$(mkinstalldirs) $(INCLUDE_DIR)/scml2_tlm2
	$(mkinstalldirs) $(INCLUDE_DIR)/scml2_tlm2/snps_tlm2_extensions
	$(mkinstalldirs) $(INCLUDE_DIR)/scml2_tlm2/snps_tlm2_utils
	$(INSTALL_DATA) $(srcdir)/snps_tlm2_extensions/*.h $(INCLUDE_DIR)/scml2_tlm2/snps_tlm2_extensions/.; \
	$(INSTALL_DATA) $(srcdir)/snps_tlm2_utils/*.h $(INCLUDE_DIR)/scml2_tlm2/snps_tlm2_utils/.; \
	$(INSTALL_DATA) $(srcdir)/scml2_tlm2.h $(INCLUDE_DIR)/scml2_tlm2.h; \
	$(mkinstalldirs) $(LIB_DIR)
	$(INSTALL_DATA) libscml2_tlm2.a $(LIB_DIR)/libscml2_tlm2.a
	$(RANLIB) $(LIB_DIR)/libscml2_tlm2.a

uninstall-local:
	@$(NORMAL_UNINSTALL)
	rm -rf $(INCLUDE_DIR)/scml2_tlm2; \
	rm -f $(INCLUDE_DIR)/scml2_tlm2.h; \
	rm -f $(LIB_DIR)/libscml2_tlm2.a
	-rmdir $(prefix)/lib-$(TARGET_ARCH)

