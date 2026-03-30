/***************************************************************************
 * Copyright 1996-2025 Synopsys, Inc.
 *
 * This Synopsys software and all associated documentation are proprietary
 * to Synopsys, Inc. and may only be used pursuant to the terms and
 * conditions of a written license agreement with Synopsys, Inc.
 * All other use, reproduction, modification, or distribution of the
 * Synopsys software or the associated documentation is strictly prohibited.
 ***************************************************************************/
#ifndef __SNPS_TLM2_SOCKET_IF__
#define __SNPS_TLM2_SOCKET_IF__
/******************************************************************************
* This class defines the interface that is implemented by all the sockets
* as well as their impl classes.
* ***************************************************************************/
#include <string>
#include <systemc>
#include <map>

#  ifndef SNPS_VP_API
#    define SNPS_VP_API
#    define SNPS_VP_API_TMPL_FUNC(x)
#    define SNPS_VP_API_TMPL_CLASS(x)
#  endif
#  define SC_REPORT_WARNING_MODELER_ONLY SC_REPORT_WARNING
namespace snps_tlm2
{

/******************************************************************************
 * Name     : snps_tlm2_socket_if
 * Purpose  : Interface used for intersocket as well as socket to socket
 *            implementation communication
 * *****************************************************************************/
class snps_tlm2_socket_if
{
  public:
	virtual ~snps_tlm2_socket_if() {}
    virtual void* execute_command (std::string   cmd_str,
                                   void* arg1  = NULL,
                                   void* arg2  = NULL,
                                   void* arg3  = NULL) = 0;
	virtual void setForwardPointer(void* ) {}
};//end snps_tlm2_socket_if


//convenience enumeration
typedef enum cmd_type_
{
    HANDLE_CONSTRUCTION,
    END_OF_ELAB,
    BEFORE_END_OF_ELAB,
    START_OF_SIM,
    BIND_INTERFACE,
    BIND_TO_PEER,
    BIND_TO_PARENT,
    BIND_TO_CHILD,
    BIND_TRGT_SOCKET,
    GET_TRGT_IPT,
    GET_TRGT_PROTOCOL,
    GET_INIT_PROTOCOL,
    GET_TRGT_CLK,
    GET_INIT_CLK,
    GET_TRGT_GP_MAPPER,
    GET_TRGT_GDA_IPT,
    GET_INIT_GDA_IPT,
    GET_IF_TRGT_PORT,
    SET_FT_PROTOCOL,
    SET_FT_CLOCK,
    SET_CHILD_HANDLE,
    SET_PARENT_HANDLE,
    SET_PEER_HANDLE,
    REPLACE_IF_TRGT_PORT,
    REPLACE_IF_INIT_PORT,
    REGISTER_STATE_OBSERVER,
    INIT_SOCKET_TYPE,
    GET_FW_DESTINATION,
    GET_BW_DESTINATION,
    GET_PORT_INTERFACE,
    GET_TLM_IPT,
    SET_TARGET_CLK,
    CHECK_MULTI_BINDING,
    GET_INITIATOR_CHILD,
    REGISTER_SIM_PROBE,
    REGISTER_INTERCEPTION,
    UNREGISTER_INTERCEPTION,
    SET_IPT_METADATA,
    MAX_SOCKET_COMMAND_TYPE,
} socket_cmd_type_t;

typedef std::map<std::string, unsigned int>           str2SocketCmdMapType;
typedef std::map<std::string, unsigned int>::iterator str2SocketCmdMapTypeIterator;
#define TLM2_GP_PROTOCOL_ID 0xffffffff

/******************************************************************************
 * Name     : snps_init_string_to_cmd_map
 * Purpose  : singleton class used to initialize the string to socket
 *            command enum map.
 *****************************************************************************/
class SNPS_VP_API snps_init_string_to_cmd_map
{
  public:
    static str2SocketCmdMapType& get_map();
  private:
    snps_init_string_to_cmd_map ();
    void initializeStr2SocketCmdMap();
  private:
    str2SocketCmdMapType       mMap;
    bool isMapInitialized;
};

//static str2SocketCmdMapType&   gCmdStrToEnumMap = snps_init_string_to_cmd_map::get_map();
}//end namespace snps_tlm2

#endif //__SNPS_TLM2_SOCKET_IF__
