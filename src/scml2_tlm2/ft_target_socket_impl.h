/***************************************************************************
 * Copyright 1996-2025 Synopsys, Inc.
 *
 * This Synopsys software and all associated documentation are proprietary
 * to Synopsys, Inc. and may only be used pursuant to the terms and
 * conditions of a written license agreement with Synopsys, Inc.
 * All other use, reproduction, modification, or distribution of the
 * Synopsys software or the associated documentation is strictly prohibited.
 ***************************************************************************/

/*
// Modifications :
//
// Purpose of file : Implementation class of tlm_target socket specialization
//                   It is called by tlm_target_socket to handle functionality.
//
//
*/
#ifndef __FT_TARGET_SOCKET_IMPL_H__
#define __FT_TARGET_SOCKET_IMPL_H__

#include <scml2_tlm2/snps_tlm2_utils/ft_initiator_socket.h>
#include "scml2_tlm2/snps_tlm2_utils/snps_tlm2_utils.h"
#include <tlm>
//#define FT_SOCKET_DEBUG_ENABLE
namespace scml2
{

template <unsigned int BUSWIDTH >
class ft_target_socket_impl
    : public snps_tlm2::snps_tlm2_socket_if
    , public EnabledStateObserver
{

public:
    typedef tlm::tlm_fw_transport_if<> fw_interface_type;
    typedef tlm::tlm_bw_transport_if<> bw_interface_type;

    typedef sc_core::sc_export<bw_interface_type> export_type;


    typedef tlm::tlm_base_initiator_socket_b<BUSWIDTH,
                                     fw_interface_type,
                                     bw_interface_type> base_initiator_socket_type;

    typedef tlm::tlm_base_target_socket_b<BUSWIDTH,
                                        fw_interface_type,
                                        bw_interface_type> base_type;


    typedef snps_tlm2::snps_tlm2_socket_if initiator_socket_if_type;

    typedef snps_tlm2::snps_tlm2_socket_if    target_socket_if_type;

    /******************************************************************************
     Function:  Constructor
     *******************************************************************************/
     explicit ft_target_socket_impl(base_type* owner)
       : m_owner(owner)
       , m_targetSocket(dynamic_cast<target_socket_if_type*>(m_owner))
       , m_portInterface(NULL)
       , m_parentHandle(NULL)
       , m_childHandle(NULL)
       , m_peerHandle(NULL)
       , m_targetProtocol(TLM2_GP_PROTOCOL_ID)
       , m_initiatorProtocol(TLM2_GP_PROTOCOL_ID)
       , m_targetClkPeriod(0,sc_core::SC_NS)
       , m_initClkPeriod(0, sc_core::SC_NS)
       , m_isImplementer(false)
       , m_isScPortBound(false)
       , m_isInterceptActive(false)
     {
        //Only required in open scml version
        if(m_targetSocket)
	        m_targetSocket->setForwardPointer(m_portInterface);

     }

    /******************************************************************************
     Function: Destructor
     *******************************************************************************/
     ~ft_target_socket_impl()
     {

     }

     /******************************************************************************
     Function:  handle_construction
     *******************************************************************************/
     void handle_construction()
     {
     }

    /******************************************************************************
    * Function:  handle_before_end_of_elaboration
    * create mapper if required
    *******************************************************************************/
    void handle_before_end_of_elaboration()
    {
    }
    /******************************************************************************
     * Function:  handle_end_of_elaboration
     *
     *******************************************************************************/
     void handle_end_of_elaboration()
     {

        //XXX TODO check if the connected socket is ft_socket.
        //if true nothing is required here.
        //return
        if(m_owner->get_base_port().get_interface(0) == NULL ||
                        m_owner->get_base_port().size() != 1) {
            //Socket is unconnected or is connected to more than one interface
            return ;
        }
        {
			//Set the target socket's forward pointer to original interface
			//only if it is not pointing to initiator socket's intercept
	        m_portInterface =  dynamic_cast<bw_interface_type *>(m_owner->get_base_port().get_interface(0));
    	    m_targetSocket->setForwardPointer(m_portInterface);
		}
        return;
     }


    /******************************************************************************
     Function: handle_start_of_simulation
     Description: Passes clock period to Protocol checker
    *******************************************************************************/
    void handle_start_of_simulation()
    {
        sc_core::sc_time* clk = get_init_clock();
        if(!clk) return;
    }

   /***************************************************************************
     Function:  bind_to_interface
    *  purpose  : Will be called only for the socket present on the target
    *             implementing the forward interface.
    *
    *             1. set m_isImplementer = true (connected to the innermost
    *                initaitor in the heirarchy).
    *             2. Initialize the intercepts for forward and backward
    *                interface.
    *             3. Intercept the BW interface.
    ***************************************************************************/
    void bind_to_interface(fw_interface_type& fwIf )
    {
        m_isImplementer = true;
        m_owner->get_base_export().sc_core::sc_export<fw_interface_type>::
                                                                    bind(fwIf);
    }


    /**************************************************************************
     * Function:  bind_to_peer
     * Purpose  : Binds to peer initiator socket
     *************************************************************************/
     void bind_to_peer(base_initiator_socket_type& s)
     {
         //bind targets port as in base
         //s.get_base_port().bind(cwr_base_type::m_export);
         s.get_base_port().sc_core::sc_port_b<fw_interface_type> ::
                                           bind(m_owner->get_base_interface());

         m_owner->get_base_port().sc_core::sc_port_b<bw_interface_type> ::
                                                  bind(s.get_base_interface());

          initiator_socket_if_type* s_cast =
                                   dynamic_cast<initiator_socket_if_type*>(&s);



         //set the peer handle for the initiator_socket as well
        m_peerHandle = s_cast;
        m_isScPortBound = true;
        m_portInterface = dynamic_cast<bw_interface_type *>(m_owner->get_base_port().get_interface(0));
        m_targetSocket->setForwardPointer(m_portInterface);
     }


     /*************************************************************************
      * Function: bind_to_child
      * Purpose : Binds to child target socket
      ************************************************************************/
     void bind_to_child(base_type& s)
     {
         sc_assert(m_isImplementer == false);

         m_owner->get_base_export().sc_core::sc_export<fw_interface_type>::
                                                   bind(s.get_base_export());

         sc_core::sc_interface *fwIf =
                                  m_owner->get_base_export().get_interface();

         fw_interface_type* myFwIf = dynamic_cast<fw_interface_type*>(fwIf);
         sc_assert(myFwIf != NULL);

         s.get_base_port().sc_core::sc_port_b<bw_interface_type> ::
                                               bind(m_owner->get_base_port());

         target_socket_if_type* s_cast = dynamic_cast<target_socket_if_type*>(&s);
         if(!s_cast) {
             return;
         }
         m_childHandle = s_cast;
         s_cast->execute_command ("SET_PARENT_HANDLE", (void*)this);
     }
     /*************************************************************************
      * Function:  set_clock
      * purpose : Sets target clock period
      * ***********************************************************************/
     void set_clock(sc_core::sc_time clockPeriod)
     {
         m_targetClkPeriod = clockPeriod;
		 handle_start_of_simulation();
		//Inform the peer initiator socket of the change in Target clock
         if(m_peerHandle)
         {
             m_peerHandle->execute_command("SET_TARGET_CLK", (void*)&m_targetClkPeriod);

         } else if(m_parentHandle)
         {
             m_parentHandle->execute_command("SET_TARGET_CLK",  (void*)&m_targetClkPeriod);
         }

     }

     /**************************************************************************
      * Function:  get_target_clock
      *
      * ***********************************************************************/
     sc_core::sc_time*  get_target_clock()
     {
         if ( m_childHandle != NULL )
         {
            void* clk = m_childHandle->execute_command("GET_TRGT_CLK");
            return (static_cast<sc_core::sc_time*>(clk));
            //return m_childHandle->get_target_clock();
         } else {
             return &m_targetClkPeriod;
         }
     }

     /*************************************************************************
      *  function : get_init_clock
      *
      *  purpose  : Convenience function called by start_of_simulation in
      *             implementer socket to set the target clock period
      *
      * ***********************************************************************/
     sc_core::sc_time*  get_init_clock()
     {
         if(m_peerHandle)
         {
             void* clk = m_peerHandle->execute_command("GET_INIT_CLK");
             return (static_cast<sc_core::sc_time*>(clk));

         } else if(m_parentHandle)
         {
            void* clk = m_parentHandle->execute_command("GET_INIT_CLK");
             return (static_cast<sc_core::sc_time*>(clk));
         }

         //sc_time  ret;
         return NULL;

     }
     /*************************************************************************
      *  function : get_init_socket_type
      *
      *  purpose  : Gets the socket type of Initiator socket
      *
      * ***********************************************************************/
     std::string*  get_init_socket_type()
     {
         if(m_peerHandle)
         {
             void* socket_type = m_peerHandle->execute_command("INIT_SOCKET_TYPE");
             return (static_cast<std::string*>(socket_type));

         } else if(m_parentHandle)
         {
            void* socket_type = m_parentHandle->execute_command("INIT_SOCKET_TYPE");
             return (static_cast<std::string*>(socket_type));
         }

         return NULL;

     }

     /**************************************************************************
      * Function:  get_target_protocol
      *
      * ************************************************************************/
     unsigned int* get_init_protocol()
     {
         if(m_peerHandle)
         {
             //return m_peerHandle->get_target_protocol();
             void* protocol = m_peerHandle->execute_command("GET_INIT_PROTOCOL");
             return (static_cast<unsigned int*>(protocol));

         } else if(m_parentHandle) {

            void* protocol = m_parentHandle->execute_command("GET_INIT_PROTOCOL");
             return (static_cast<unsigned int*>(protocol));
         }


         return NULL;
     }


     /**************************************************************************
      * Function:  replace_interface_init_port
      *
      * ************************************************************************/
     void replace_interface_init_port(sc_core::sc_interface* replace_if)
     {
         if(m_peerHandle)
         {
              m_peerHandle->execute_command("REPLACE_IF_INIT_PORT", (void*)replace_if);

         } else if(m_parentHandle) {

             m_parentHandle->execute_command("REPLACE_IF_INIT_PORT", (void*)replace_if);
         }

     }

     /*************************************************************************
      * Function:  get_target_protocol
      *
      * **************************************************************************/
     unsigned int* get_target_protocol()
     {
         if( m_childHandle != NULL )
         {
             void* protocol = m_childHandle->execute_command("GET_TRGT_PROTOCOL");
             return (static_cast<unsigned int*>(protocol));
         } else {
			 /*if(m_targetProtocol == TLM2_GP_PROTOCOL_ID){
				m_targetProtocol = scml2::gp_protocol_state_extension::ID;
			 }*/
             return &m_targetProtocol;
         }
     }

     /******************************************************************************
      * Function:  get_target_gp_mapper
      *
      * *******************************************************************************/
     /*scml2::ft2gp_sm_mapper_base* get_target_gp_mapper()
     {
         if ( m_childHandle != NULL )
         {
             return
              ( scml2::ft2gp_sm_mapper_base*)m_childHandle->execute_command("GET_TRGT_GP_MAPPER");
         } else {

             return m_fromGPMapper;
         }
         return NULL;
     }*/


    /****************************************************************************
        *  function : enabledStateChanged
        *
     *  purpose  : Handles state changes in initiator and target ipts.
     *
     *****************************************************************************/
       void enabledStateChanged() {
      }
     /******************************************************************************
     Function:  replace_interface_target_port
     *******************************************************************************/
     void replace_interface_target_port(sc_core::sc_interface* replacement_ifs) {

        if ( m_childHandle != NULL ){

           (m_childHandle)->execute_command("REPLACE_IF_TRGT_PORT", (void*)(replacement_ifs));

       } else {

             bw_interface_type* replacement_ifs_cast = dynamic_cast<bw_interface_type*>(replacement_ifs);
             sc_assert(replacement_ifs_cast);
        if(!m_isInterceptActive)  {
             m_portInterface = dynamic_cast<bw_interface_type *>(replacement_ifs_cast);
             m_targetSocket->setForwardPointer(m_portInterface);
			m_isInterceptActive = true;
        }
      }
     }
   /******************************************************************************
     Function:  execute_command
  *  purpose  : blanket command added to handle different commands that initiator socket
  *             is supposed to handle, first argument is command type enum and
                other arguments    are typecasted appropriately and forward to each API
    *******************************************************************************/

    void* execute_command(std::string cmdStr, void* arg1, void* , void* )
    {

        unsigned int*                    t_protocol, *protocol;
        sc_core::sc_time*               clk_period, *in_clk_period, *clock;

        sc_core::sc_interface*          ifs;
        fw_interface_type*              fwIf;
        base_initiator_socket_type*     initSocket;
        base_type*                      childSocket;

        snps_tlm2::socket_cmd_type_t             cmd;
        snps_tlm2::str2SocketCmdMapTypeIterator  enum_it;

       #ifdef FT_SOCKET_DEBUG_ENABLE
        std::cerr << (&(m_owner->get_base_port()))->name()
                  << ":: impl CMD "<< cmdStr << " recieved.\n";
        #endif
        enum_it = (snps_tlm2::snps_init_string_to_cmd_map::get_map()).find(cmdStr);

        if (enum_it == (snps_tlm2::snps_init_string_to_cmd_map::get_map()).end())
        {
            std::cerr <<  (&(m_owner->get_base_port()))->name()
                      << ":: impl : Unknown Command " << cmdStr
                      << " received. Exiting." << std::endl;
            exit(-1);
        }

        cmd = (snps_tlm2::socket_cmd_type_t)enum_it->second;


        switch(cmd)
        {
            case snps_tlm2::SET_FT_PROTOCOL:
            {
                protocol = static_cast<unsigned int*>(arg1);
                sc_assert(protocol != NULL);
                m_targetProtocol = *protocol;
//		        std::cout<< (&(m_owner->get_base_port()))->name() << " Set Protocol called in FT target socket "
                      //<<"with value "<<m_targetProtocol<<endl;
                break;
            }

            case snps_tlm2::SET_FT_CLOCK:
            case snps_tlm2::SET_TARGET_CLK://call from the child socket
            {
                clock = static_cast<sc_core::sc_time*>(arg1);
                sc_assert(clock != NULL);
                set_clock(*clock);
                break;
            }

            case snps_tlm2::HANDLE_CONSTRUCTION:
            {
                handle_construction();
                return NULL;
            }

            case snps_tlm2::BEFORE_END_OF_ELAB:
            {
                handle_before_end_of_elaboration();
                return NULL;
            }

            case snps_tlm2::END_OF_ELAB:
            {
                handle_end_of_elaboration();
                return NULL;
            }

            case snps_tlm2::START_OF_SIM:
            {
                handle_start_of_simulation();
                return NULL;
            }

            case snps_tlm2::BIND_TO_PEER:
            {
                initSocket = static_cast<base_initiator_socket_type*>(arg1);
                bind_to_peer(*initSocket);
                return NULL;
            }

            case snps_tlm2::BIND_TO_CHILD:
            {
                childSocket = static_cast<base_type*>(arg1);
                bind_to_child(*childSocket);
                return NULL;
            }

            case snps_tlm2::BIND_INTERFACE:
            {
                fwIf = static_cast<fw_interface_type*>(arg1);
                bind_to_interface(*fwIf);
                return NULL;
            }

            case snps_tlm2::GET_TRGT_CLK:
            {
                clk_period = get_target_clock();
                return (static_cast<void*>(clk_period));
            }
            case snps_tlm2::GET_INIT_CLK:
            {
                in_clk_period = get_init_clock();
                return (static_cast<void *>(in_clk_period));
            }

            case snps_tlm2::GET_TRGT_PROTOCOL:
            {
                t_protocol = get_target_protocol();
                return (static_cast<void*>(t_protocol));
            }


            case snps_tlm2::SET_PEER_HANDLE:
            {
                //initSocket = static_cast<base_initiator_socket_type*>(arg1);
                m_peerHandle = static_cast<initiator_socket_if_type*>(arg1);
                m_isScPortBound = true;
                m_portInterface = dynamic_cast<bw_interface_type *>(m_owner->get_base_port().get_interface(0));
                m_targetSocket->setForwardPointer(m_portInterface);
                break;
            }

            case snps_tlm2::REPLACE_IF_TRGT_PORT:
            {
                ifs = static_cast<sc_core::sc_interface*>(arg1);
                replace_interface_target_port(ifs);
                break;
            }
            case snps_tlm2::REPLACE_IF_INIT_PORT:
            {

                replace_interface_init_port(static_cast<sc_core::sc_interface*>(arg1));
                break;
            }

            case snps_tlm2::GET_IF_TRGT_PORT:
            {
                return (static_cast<void*>(m_owner->get_base_port().get_interface(0)));
            }

            case snps_tlm2::SET_PARENT_HANDLE:
            {
                m_parentHandle = static_cast <target_socket_if_type*>(arg1);
                m_isScPortBound = true;
                break;
            }
            case snps_tlm2::INIT_SOCKET_TYPE:
            {
                return static_cast <void *>(get_init_socket_type());
            }

            case snps_tlm2::SET_CHILD_HANDLE:
            {
               sc_assert (0);
               break;
            }

            case snps_tlm2::REGISTER_STATE_OBSERVER:
            {
                if (m_parentHandle != NULL) {
                    m_parentHandle->execute_command ("REGISTER_STATE_OBSERVER", arg1);
                } else if (m_peerHandle != NULL) {
                    m_peerHandle->execute_command ("REGISTER_STATE_OBSERVER", arg1);
                }
                break;
            }

            case snps_tlm2::GET_PORT_INTERFACE:
            {
                return (static_cast<void*>(m_portInterface));
            }
            case snps_tlm2::GET_BW_DESTINATION:
            {
                bw_interface_type * ifs =
                        m_owner->get_base_port().get_interface(0);
                return (static_cast<void*>(ifs));
            }
            case snps_tlm2::CHECK_MULTI_BINDING:
	    {
		sc_assert(arg1);
		bool* is_multi_bound = reinterpret_cast<bool*>(arg1);
		if (m_owner->get_base_port().size() > (int)1) {
		    *is_multi_bound = true;
		} else if (m_childHandle != nullptr) {
                    m_childHandle->execute_command("CHECK_MULTI_BINDING", static_cast<void*>(is_multi_bound));
                }
		break;
	    }
	    case snps_tlm2::GET_INITIATOR_CHILD:
	    {
		if (m_parentHandle != NULL) { // fwd to parent
		    return m_parentHandle->execute_command("GET_INITIATOR_CHILD");
		} else if (m_peerHandle != NULL) { // fwd to initiator peer
		    return m_peerHandle->execute_command("GET_INITIATOR_CHILD");
		}
		break;
	    }
            default:
            {
                std::cerr << (&(m_owner->get_base_port()))->name()
                          << ":: impl Unimplemented command ::"<<cmdStr<<" passed "
                          << "as argument to execute_command().\n";
            }

        }
        return NULL;
    }

  private:

    base_type*                                                      m_owner;
    target_socket_if_type*                                          m_targetSocket;
    bw_interface_type *                                             m_portInterface;

  protected:
    target_socket_if_type*                                         m_parentHandle;
    target_socket_if_type*                                         m_childHandle;
    initiator_socket_if_type*                                      m_peerHandle;
    unsigned int                                                   m_targetProtocol;
    unsigned int                                                   m_initiatorProtocol;
    sc_core::sc_time                                                        m_targetClkPeriod;
    sc_core::sc_time                                                        m_initClkPeriod;
    bool                                                           m_isImplementer;
    bool                                                           m_isScPortBound;
    bool                                                           m_isInterceptActive;
};

}

#endif

