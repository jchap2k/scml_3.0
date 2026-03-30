#############################################################################
# Copyright 1996-2025 Synopsys, Inc.                                        #
#                                                                           #
# This Synopsys software and all associated documentation are proprietary   #
# to Synopsys, Inc. and may only be used pursuant to the terms and          #
# conditions of a written license agreement with Synopsys, Inc.             #
# All other use, reproduction, modification, or distribution of the         #
# Synopsys software or the associated documentation is strictly prohibited. #
#############################################################################


CLOCK_H_FILES = 	clock/clock_engine.h
RESET_H_FILES = 	reset/reset_engine.h
PIN_H_FILES = 	pin/pin_engine.h
INTERRUPT_H_FILES = 	interrupt/interrupt_engine.h
FT_INITIATOR_H_FILES = 	tlm2_ft_initiator_port/tlm2_ft_initiator_port_pe.h \
	tlm2_ft_initiator_port/tlm2_ft_initiator_port_peBase.h
FT_TARGET_H_FILES = 	tlm2_ft_target_port/tlm2_ft_target_port_peBase.h \
	tlm2_ft_target_port/tlm2_ft_target_port_pe.h
FT_COMMON_H_FILES = 	tlm2_ft_common/tlm2_ft_mode.h \
	tlm2_ft_common/tlm2_intercept_base.h

noinst_HEADERS = $(CLOCK_H_FILES) \
  $(RESET_H_FILES) \
  $(PIN_H_FILES) \
  $(INTERRUPT_H_FILES) \
  $(FT_INITIATOR_H_FILES) \
  $(FT_TARGET_H_FILES) \
  $(FT_COMMON_H_FILES)

INCLUDE_DIR = $(prefix)/include

PE_INCLUDE_DIRS = clock reset pin interrupt tlm2_ft_initiator_port tlm2_ft_target_port tlm2_ft_common

install-data-local: $(noinst_HEADERS)
	@$(NORMAL_INSTALL)
	$(mkinstalldirs) $(INCLUDE_DIR)/scml2_protocol_engines
	for file in $(PE_INCLUDE_DIRS); do \
	  $(mkinstalldirs) $(INCLUDE_DIR)/scml2_protocol_engines/$$file/include; \
	done
	for file in $(CLOCK_H_FILES); do \
	  $(INSTALL_DATA) $(srcdir)/$$file $(INCLUDE_DIR)/scml2_protocol_engines/clock/include/; \
	done
	for file in $(RESET_H_FILES); do \
	  $(INSTALL_DATA) $(srcdir)/$$file $(INCLUDE_DIR)/scml2_protocol_engines/reset/include/; \
	done
	for file in $(PIN_H_FILES); do \
	  $(INSTALL_DATA) $(srcdir)/$$file $(INCLUDE_DIR)/scml2_protocol_engines/pin/include/; \
	done
	for file in $(INTERRUPT_H_FILES); do \
	  $(INSTALL_DATA) $(srcdir)/$$file $(INCLUDE_DIR)/scml2_protocol_engines/interrupt/include/; \
	done
	for file in $(FT_INITIATOR_H_FILES); do \
	  $(INSTALL_DATA) $(srcdir)/$$file $(INCLUDE_DIR)/scml2_protocol_engines/tlm2_ft_initiator_port/include/; \
	done
	for file in $(FT_TARGET_H_FILES); do \
	  $(INSTALL_DATA) $(srcdir)/$$file $(INCLUDE_DIR)/scml2_protocol_engines/tlm2_ft_target_port/include/; \
	done
	for file in $(FT_COMMON_H_FILES); do \
	  $(INSTALL_DATA) $(srcdir)/$$file $(INCLUDE_DIR)/scml2_protocol_engines/tlm2_ft_common/include/; \
	done

uninstall-local:
	@$(NORMAL_UNINSTALL)
	-rmdir $(INCLUDE_DIR)/scml2_protocol_engines

