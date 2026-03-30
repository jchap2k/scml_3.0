/***************************************************************************
 * Copyright 1996-2025 Synopsys, Inc.
 *
 * This Synopsys software and all associated documentation are proprietary
 * to Synopsys, Inc. and may only be used pursuant to the terms and
 * conditions of a written license agreement with Synopsys, Inc.
 * All other use, reproduction, modification, or distribution of the
 * Synopsys software or the associated documentation is strictly prohibited.
 ***************************************************************************/
#include "scml2_tlm2/snps_tlm2_utils/ft_socket_impl_factory.h"
#include "ft_initiator_socket_impl.h"
#include "ft_target_socket_impl.h"


/******************************************************************************
Factory Method to instantiate an implementer for the snps_tlm2::snps_tlm2_socket_if
interface. It takes the parent Socket pointer as argument. Called from 
tlm_base_initiator_socket class to get an implementer class which will implement
the functionality of socket interface class
*******************************************************************************/
namespace scml2 {

template <unsigned int BUSWIDTH>
snps_tlm2::snps_tlm2_socket_if * 
create_ft_socket_impl (sc_core::sc_object* parentHandle, bool isInitSocket)		
{
    if (isInitSocket == true) 
    {
        tlm::tlm_base_initiator_socket_b<BUSWIDTH>* parentCast = 
            dynamic_cast<tlm::tlm_base_initiator_socket_b<BUSWIDTH>* >(parentHandle);
        sc_assert (parentCast);
        return (new  ft_initiator_socket_impl <BUSWIDTH> (parentCast));
    } 
    else 
    {
        tlm::tlm_base_target_socket_b<BUSWIDTH>* parentCast = 
            dynamic_cast<tlm::tlm_base_target_socket_b<BUSWIDTH>* >(parentHandle);
        sc_assert (parentCast);
        return (new  ft_target_socket_impl <BUSWIDTH> (parentCast));
    }
}

//explictly instantiate all the templates
template SNPS_VP_API snps_tlm2::snps_tlm2_socket_if* create_ft_socket_impl<1>    (sc_core::sc_object* parentHandle, bool isInitSocket);
template SNPS_VP_API snps_tlm2::snps_tlm2_socket_if* create_ft_socket_impl<2>    (sc_core::sc_object* parentHandle, bool isInitSocket);
template SNPS_VP_API snps_tlm2::snps_tlm2_socket_if* create_ft_socket_impl<4>    (sc_core::sc_object* parentHandle, bool isInitSocket);
template SNPS_VP_API snps_tlm2::snps_tlm2_socket_if* create_ft_socket_impl<8>    (sc_core::sc_object* parentHandle, bool isInitSocket);
template SNPS_VP_API snps_tlm2::snps_tlm2_socket_if* create_ft_socket_impl<12>   (sc_core::sc_object* parentHandle, bool isInitSocket);     //   8 + 4
template SNPS_VP_API snps_tlm2::snps_tlm2_socket_if* create_ft_socket_impl<16>   (sc_core::sc_object* parentHandle, bool isInitSocket);
template SNPS_VP_API snps_tlm2::snps_tlm2_socket_if* create_ft_socket_impl<24>   (sc_core::sc_object* parentHandle, bool isInitSocket);     //  16 + 8
template SNPS_VP_API snps_tlm2::snps_tlm2_socket_if* create_ft_socket_impl<32>   (sc_core::sc_object* parentHandle, bool isInitSocket);
template SNPS_VP_API snps_tlm2::snps_tlm2_socket_if* create_ft_socket_impl<48>   (sc_core::sc_object* parentHandle, bool isInitSocket);     //  32 + 16
template SNPS_VP_API snps_tlm2::snps_tlm2_socket_if* create_ft_socket_impl<64>   (sc_core::sc_object* parentHandle, bool isInitSocket);
template SNPS_VP_API snps_tlm2::snps_tlm2_socket_if* create_ft_socket_impl<128>  (sc_core::sc_object* parentHandle, bool isInitSocket);
template SNPS_VP_API snps_tlm2::snps_tlm2_socket_if* create_ft_socket_impl<192>  (sc_core::sc_object* parentHandle, bool isInitSocket);     // 128 + 64
template SNPS_VP_API snps_tlm2::snps_tlm2_socket_if* create_ft_socket_impl<256>  (sc_core::sc_object* parentHandle, bool isInitSocket);
template SNPS_VP_API snps_tlm2::snps_tlm2_socket_if* create_ft_socket_impl<384>  (sc_core::sc_object* parentHandle, bool isInitSocket);     // 256 + 128
template SNPS_VP_API snps_tlm2::snps_tlm2_socket_if* create_ft_socket_impl<512>  (sc_core::sc_object* parentHandle, bool isInitSocket);
template SNPS_VP_API snps_tlm2::snps_tlm2_socket_if* create_ft_socket_impl<1024> (sc_core::sc_object* parentHandle, bool isInitSocket);
template SNPS_VP_API snps_tlm2::snps_tlm2_socket_if* create_ft_socket_impl<2048> (sc_core::sc_object* parentHandle, bool isInitSocket);
template SNPS_VP_API snps_tlm2::snps_tlm2_socket_if* create_ft_socket_impl<4096> (sc_core::sc_object* parentHandle, bool isInitSocket);

}// namespace scml2
