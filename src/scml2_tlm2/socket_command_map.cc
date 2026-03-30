/***************************************************************************
 * Copyright 1996-2025 Synopsys, Inc.
 *
 * This Synopsys software and all associated documentation are proprietary
 * to Synopsys, Inc. and may only be used pursuant to the terms and
 * conditions of a written license agreement with Synopsys, Inc.
 * All other use, reproduction, modification, or distribution of the
 * Synopsys software or the associated documentation is strictly prohibited.
 ***************************************************************************/

#include <scml2_tlm2/snps_tlm2_utils/snps_tlm2_socket_if.h>


/******************************************************************************
Factory Method to instantiate an implementer for the snps_initiator_socket_if
interface. It takes the parent Socket pointer as argument. Called from
tlm_base_initiator_socket class to get an implementer class which will implement
the functionality of socket interface class
*******************************************************************************/
namespace snps_tlm2
{


snps_init_string_to_cmd_map :: snps_init_string_to_cmd_map()
    : mMap()
    , isMapInitialized (false)
{
    //TODO
}


str2SocketCmdMapType&
snps_init_string_to_cmd_map :: get_map ()
{
    static snps_init_string_to_cmd_map  init_object;
    init_object.initializeStr2SocketCmdMap ();
    return init_object.mMap;
}


void
snps_init_string_to_cmd_map :: initializeStr2SocketCmdMap()
{
   if (isMapInitialized == true)
   {
       return;
   }

   isMapInitialized = true;

   mMap["HANDLE_CONSTRUCTION"]      = HANDLE_CONSTRUCTION;
   mMap["BEFORE_END_OF_ELAB"]       = BEFORE_END_OF_ELAB;
   mMap["END_OF_ELAB"]              = END_OF_ELAB;
   mMap["START_OF_SIM"]             = START_OF_SIM;
   mMap["BIND_INTERFACE"]           = BIND_INTERFACE;
   mMap["BIND_TO_PEER"]             = BIND_TO_PEER;
   mMap["BIND_TO_PARENT"]           = BIND_TO_PARENT;
   mMap["BIND_TO_CHILD"]            = BIND_TO_CHILD;
   mMap["BIND_TRGT_SOCKET"]         = BIND_TRGT_SOCKET;
   mMap["GET_TRGT_IPT"]             = GET_TRGT_IPT;
   mMap["GET_TRGT_PROTOCOL"]        = GET_TRGT_PROTOCOL;
   mMap["GET_INIT_PROTOCOL"]        = GET_INIT_PROTOCOL;
   mMap["GET_TRGT_CLK"]             = GET_TRGT_CLK;
   mMap["GET_INIT_CLK"]             = GET_INIT_CLK;
   mMap["GET_TRGT_GP_MAPPER"]       = GET_TRGT_GP_MAPPER;
   mMap["GET_TRGT_GDA_IPT"]         = GET_TRGT_GDA_IPT;
   mMap["GET_INIT_GDA_IPT"]         = GET_INIT_GDA_IPT;
   mMap["GET_IF_TRGT_PORT"]         = GET_IF_TRGT_PORT;
   mMap["SET_FT_PROTOCOL"]          = SET_FT_PROTOCOL;
   mMap["SET_FT_CLOCK"]             = SET_FT_CLOCK;
   mMap["SET_CHILD_HANDLE"]         = SET_CHILD_HANDLE;
   mMap["SET_PARENT_HANDLE"]        = SET_PARENT_HANDLE;
   mMap["SET_PEER_HANDLE"]          = SET_PEER_HANDLE;
   mMap["REPLACE_IF_TRGT_PORT"]     = REPLACE_IF_TRGT_PORT;
   mMap["REPLACE_IF_INIT_PORT"]     = REPLACE_IF_INIT_PORT;
   mMap["REGISTER_STATE_OBSERVER"]  = REGISTER_STATE_OBSERVER;
   mMap["INIT_SOCKET_TYPE"]         = INIT_SOCKET_TYPE;
   mMap["GET_FW_DESTINATION"]       = GET_FW_DESTINATION;
   mMap["GET_BW_DESTINATION"]       = GET_BW_DESTINATION;
   mMap["GET_PORT_INTERFACE"]       = GET_PORT_INTERFACE;
   mMap["GET_TLM_IPT"]              = GET_TLM_IPT;
   mMap["SET_TARGET_CLK"]           = SET_TARGET_CLK;
   mMap["CHECK_MULTI_BINDING"]      = CHECK_MULTI_BINDING;
   mMap["GET_INITIATOR_CHILD"]      = GET_INITIATOR_CHILD;
   mMap["REGISTER_SIM_PROBE"]       = REGISTER_SIM_PROBE;
   mMap["REGISTER_INTERCEPTION"]    = REGISTER_INTERCEPTION;
   mMap["UNREGISTER_INTERCEPTION"]  = UNREGISTER_INTERCEPTION;
   mMap["SET_IPT_METADATA"]         = SET_IPT_METADATA;
}

}// namespace snps_tlm2
