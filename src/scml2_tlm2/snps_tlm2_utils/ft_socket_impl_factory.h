/***************************************************************************
 * Copyright 1996-2025 Synopsys, Inc.
 *
 * This Synopsys software and all associated documentation are proprietary
 * to Synopsys, Inc. and may only be used pursuant to the terms and
 * conditions of a written license agreement with Synopsys, Inc.
 * All other use, reproduction, modification, or distribution of the
 * Synopsys software or the associated documentation is strictly prohibited.
 ***************************************************************************/
#ifndef __XXFT_SOCKET_IMPL_FACTORY_H__
#define __XXFT_SOCKET_IMPL_FACTORY_H__


#include "snps_tlm2_socket_if.h"
#include "EnabledStateObserver.h"
#include "scml2_logging/snps_vp_dll.h"
/******************************************************************************
Factory Method to instantiate an implementer for the snps_initiator_socket_if
interface. It takes the parent Socket pointer as argument. Called from 
tlm_base_initiator_socket class to get an implementer class which will implement
the functionality of socket interface class
*******************************************************************************/
namespace scml2 
{
template <unsigned int BUSWIDTH>
snps_tlm2::snps_tlm2_socket_if* 
create_ft_socket_impl (sc_core::sc_object* parentHandle, bool isInitSocket);			
}
#endif //__XXFT_SOCKET_IMPL_FACTORY_H__
