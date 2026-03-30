#############################################################################
# Copyright 1996-2025 Synopsys, Inc.                                        #
#                                                                           #
# This Synopsys software and all associated documentation are proprietary   #
# to Synopsys, Inc. and may only be used pursuant to the terms and          #
# conditions of a written license agreement with Synopsys, Inc.             #
# All other use, reproduction, modification, or distribution of the         #
# Synopsys software or the associated documentation is strictly prohibited. #
#############################################################################

noinst_LIBRARIES = libscml2_objects.a

H_FILES = 	action_base.h \
	action.h \
	attributeArray.h \
	attributeMerge.h \
	attributeMux.h \
	attributeSplit.h \
	command.h \
	condition_base.h \
	condition.h \
	event_base.h \
	event.h \
	io_tracer.h \
	fifo.h \
	object_utils.h \
	process_base.h \
	process.h \
	router_base.h \
	router_binder_attribute_payload.h \
	router.h \
	timer_base.h \
	timer.h \
	tlm2TargetIntercept.h \
	variable.h \
	thread_safe_tagged_message_handler.h \
	snps_vp_dll.h

PRIVATE_H_FILES = 

noinst_HEADERS = $(H_FILES)

CXX_FILES = 	io_tracer.cc \
	object_utils.cc \
	process.cc

libscml2_objects_a_SOURCES = $(H_FILES) $(PRIVATE_H_FILES) $(CXX_FILES)

INCLUDES = -I$(srcdir)/.. -I$(srcdir)/../scml2_base -I$(srcdir)/../scml2 -I$(srcdir)/../scml2_tlm2 $(OSCITLM_INC) $(SYSC_INC)

INCLUDE_DIR = $(prefix)/include
LIB_DIR = $(prefix)/lib-$(TARGET_ARCH)

install-data-local: scml2_objects.h $(noinst_HEADERS)
	@$(NORMAL_INSTALL)
	$(mkinstalldirs) $(INCLUDE_DIR)/scml2_objects
	for file in $(noinst_HEADERS); do \
	  $(INSTALL_DATA) $(srcdir)/$$file $(INCLUDE_DIR)/scml2_objects/$$file; \
	done
	$(INSTALL_DATA) $(srcdir)/scml2_objects.h $(INCLUDE_DIR)/scml2_objects.h; \
	$(mkinstalldirs) $(LIB_DIR)
	$(INSTALL_DATA) libscml2_objects.a $(LIB_DIR)/libscml2_objects.a
	$(RANLIB) $(LIB_DIR)/libscml2_objects.a

uninstall-local:
	@$(NORMAL_UNINSTALL)
	for file in $(noinst_HEADERS); do \
	  rm -f $(INCLUDE_DIR)/scml2_objects/$$file; \
	done
	-rmdir $(INCLUDE_DIR)/scml2_objects
	rm -f $(LIB_DIR)/libscml2_objects.a
	-rmdir $(prefix)/lib-$(TARGET_ARCH)

