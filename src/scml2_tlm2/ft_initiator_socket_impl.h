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
// Purpose of file : Implementation class for tlm_initiator_socket
// tlm_initiator socket class forwards all calls to this class
//
*/
#ifndef __FT_INITIATOR_SOCKET_IMPL_H__
#define __FT_INITIATOR_SOCKET_IMPL_H__

#include "tlm"

//#define FT_SOCKET_DEBUG_ENABLE

namespace scml2
{

template <unsigned int BUSWIDTH>
class ft_initiator_socket_impl
    : public snps_tlm2::snps_tlm2_socket_if
    , public EnabledStateObserver
{
  public:
    typedef tlm::tlm_fw_transport_if<> fw_interface_type;
    typedef tlm::tlm_bw_transport_if<> bw_interface_type;

    typedef sc_core::sc_export<bw_interface_type> export_type;

    typedef tlm::tlm_base_target_socket_b<BUSWIDTH>    base_target_socket_type;
    typedef tlm::tlm_base_initiator_socket_b<BUSWIDTH> base_type;

    typedef snps_tlm2::snps_tlm2_socket_if    initiator_socket_if_type;
    typedef snps_tlm2::snps_tlm2_socket_if    target_socket_if_type;

    /*******************************************************************************
     *Function:  Constructor
     *******************************************************************************/
    explicit ft_initiator_socket_impl(base_type* mOwner)
        : m_owner(mOwner)
        , m_initSocket(dynamic_cast<initiator_socket_if_type*>(m_owner))
        , m_initiatorProtocol(TLM2_GP_PROTOCOL_ID)
        , m_targetProtocol(TLM2_GP_PROTOCOL_ID)
        , m_initiatorClkPeriod(0, sc_core::SC_NS)
        , m_targetClkPeriod(0, sc_core::SC_NS)
        , m_socketType("FT_INITIATOR")
        , m_portInterface(NULL)
        , m_parentPortRef( NULL )
        , m_parentHandle (NULL)
        , m_childHandle (NULL)
        , m_peerHandle(NULL)
        , m_isImplementer(false)
        , m_isScPortBound(false)
        , m_handledEndOfElab(false)

    {
        if(m_initSocket)
	        m_initSocket->setForwardPointer(m_portInterface);
    }



    /*******************************************************************************
     *Function: Destructor
     *******************************************************************************/
    ~ft_initiator_socket_impl()
    {
    }

    /******************************************************************************
     * Function     :  handle_construction
     * Description  :  Instantiates GDA IPT as it can only be done at this
     *                 time. and adds it to the sc_port
     *******************************************************************************/
    void handle_construction()
    {
    }

    /******************************************************************************
     * Function     :  handle_before_end_of_elaboration
     * Description  :  Gets the relevant Mapper factory from Registry class and
     *                 creates mapper instance
     *******************************************************************************/
    void handle_before_end_of_elaboration()
    {
    }
    /******************************************************************************
     Function:  isTracingDisabled
    *******************************************************************************/
    static bool isTracingDisabled()
    {
        return 1;
    }

    /******************************************************************************
     Function:  isGenericIptDisabled
    *******************************************************************************/
    static bool isGenericIptDisabled()
    {
        // For OSCI, a dummy IPT is instantiated.
        return 1;
    }


    /****************************************************************************
    *  function : handle_end_of_elaboration.
    *  purpose  : For the Implementer (socket connected to the inner most
    *             initiator ) only.
    *             1. This function passes the ipt pointers to the fw and bw
    *                implementation classes.
    *             2. Attaches fowrarding IPT's to other initiator sockets.
    *                These would primarily be tlm_base_initiator_sockets on
    *                the initiators parents in the Hierarchy.
    *             3. Initializes the mapper and passes it to the fw and bw
    *                interface implementers.
    *
    * *************************************************************************/
    void handle_end_of_elaboration()
    {
        m_handledEndOfElab = true;
        if ( m_isImplementer == false ||
                m_isScPortBound == false ||
                  m_owner->get_base_port().size() != 1)
        {
            return;
        }
        //get the connected interface and set it as destination
        //of the m_fwImpl
        fw_interface_type* ifs = m_owner->get_base_port().get_interface(0);
        sc_assert(ifs);
        {
	        m_portInterface =  dynamic_cast<fw_interface_type *>(ifs);
    	    m_initSocket->setForwardPointer(m_portInterface);
		}
    }

    /***************************************************************************
     * Function    : handle_start_of_simulation
     * Description : Passes clock period to Protocol checker
     ****************************************************************************/
    void handle_start_of_simulation()
    {
    }

    /******************************************************************************
     *  Function:  bind_to_peer
     *  purpose :  This function is called irrespective of the socket on which the
     *             peer binding function is called.
     *             target_socket.bind(initiator_socket) will also land up here.
     *             1. Binds port of initiator/target to export of target/intiator.
     *******************************************************************************/
    void bind_to_peer(base_target_socket_type& s)
    {
        m_owner->get_base_port().sc_core::sc_port_b<fw_interface_type>::bind(s.get_base_interface());

        //bind targets port as in base
        //s.get_base_port().bind(cwr_base_type::m_export);
        s.get_base_port().sc_core::sc_port_b<bw_interface_type>::bind(m_owner->get_base_interface());

        if (m_isScPortBound == false )
        {
            target_socket_if_type *s_cast = dynamic_cast<target_socket_if_type*>(&s);
            if (s_cast != NULL) {

                m_peerHandle = s_cast;

                initiator_socket_if_type *socketIf = dynamic_cast<initiator_socket_if_type*>(this->m_owner);
                if(socketIf != NULL)
                {
                    m_peerHandle->execute_command("SET_PEER_HANDLE",
                          socketIf);
                }
            }
            m_portInterface = dynamic_cast<fw_interface_type *>(m_owner->get_base_port().get_interface(0));
            m_initSocket->setForwardPointer(m_portInterface);
        }
        m_isScPortBound = true;

    }

    /******************************************************************************
     *  Function :  bind_to_interface
     *  purpose  :  Will be called only for the socket present on the initiator
     *              implementing the backward interface.
     *              1. set m_isImplementer = true (connected to the innermost
     *                 initaitor in the heirarchy).
     *              2. Intercept the BW interface.
     *******************************************************************************/
    void bind_to_interface (bw_interface_type& bwIf, export_type& export_ptr)
    {
        m_isImplementer = true;
        export_ptr.sc_core::sc_export<bw_interface_type>::bind(bwIf);
        //has to be different from the base_class one. As using get_base_interface
        //will result in an infinite loop here.
        //See multi_passthrough_initiator_socket's get_base_export().
    }

    /******************************************************************************
     * Function :  bind_to_parent
     * purpose  : This function is called on all the children in a hierarchy.
     *
     *             1. Binds port of initiator socket to port of parents intiator
     *                socket and export of parent's initaitor socket to export of
     *                childs initiator socket.
     *             2. Intercepts the forward IF for the implementer and stores the
     *                m_parentPortRef for completing the path at
     *                end_of_elaboration
     *******************************************************************************/
    void bind_to_parent(base_type& s)
    {
        m_owner->get_base_port().sc_core::sc_port_b<fw_interface_type>
                                                        ::bind(s.get_base_port());

        //bind export as in base.
        bw_interface_type *pIf = dynamic_cast<bw_interface_type *>(m_owner->get_base_export().get_interface());
        if(!pIf)
        {
          //Will die!
           s.get_base_export().sc_core::sc_export<bw_interface_type>::bind(m_owner->get_base_export());
        }

        s.get_base_export().sc_core::sc_export<bw_interface_type>::bind(*pIf);

        initiator_socket_if_type *s_cast       = dynamic_cast<initiator_socket_if_type*>(&s);
        initiator_socket_if_type *child_handle = dynamic_cast<initiator_socket_if_type*>(m_owner);

        if ( m_isScPortBound == false )
        {
            //set child handle in parent to this.
            m_parentHandle = s_cast;
        }

        m_isScPortBound = true;
          if((s_cast != NULL )&& (child_handle != NULL)) {
            //store the child handle in the parent
            s_cast->execute_command ("SET_CHILD_HANDLE", child_handle);
        }
    }
    /******************************************************************************
     Function:  set_clock
    *******************************************************************************/
    void set_clock (sc_core::sc_time clockPeriod)
    {
        m_initiatorClkPeriod = clockPeriod;
		handle_start_of_simulation();
    }

    /****************************************************************************
    *  function : enabledStateChanged
    *
    *  purpose  : Handles state changes in initiator and target ipts.
    *
    *****************************************************************************/
    void enabledStateChanged()
    {
    }

    /****************************************************************************
     *  function : checkInterceptActive
     *  purpose  : Checks if intercept is enabled.
     *****************************************************************************/
     bool checkInterceptActive()
     {
         return false;
     }

    /****************************************************************************
     *  function : enableIntercept
     *
     *  purpose  : Enables the fw and bw intercepts by replacing the interfaces
     *             at the respective initiator and target ports.
     *****************************************************************************/
    void enableIntercept()
    {
    }

    /**************************************************************************
     * Function :  replace_interface_init_port
     * Name     :
     * ************************************************************************/
     void replace_interface_init_port(sc_core::sc_interface* replacement_ifs)
    {
        if ( m_childHandle != NULL ){

           (m_childHandle)->execute_command("REPLACE_IF_INIT_PORT", (void*)(replacement_ifs));

       } else {

             fw_interface_type* replacement_ifs_cast = dynamic_cast<fw_interface_type*>(replacement_ifs);
             sc_assert(replacement_ifs_cast);

        if(!checkInterceptActive())  {
             /*fw_interface_type* ifs = m_owner->get_base_port().get_interface(0);
             sc_assert(ifs);*/
             //m_owner->get_base_port().replace_interface(ifs, replacement_ifs_cast);
             m_portInterface = dynamic_cast<fw_interface_type *>(replacement_ifs_cast);
             m_initSocket->setForwardPointer(m_portInterface);
            }
      }

    }

    /****************************************************************************
    *  function : replace_interface_target_port
    *  purpose  : Convenience function called by enableIntercept in
    *             implementer socket to replace the targets interface with
    *             the bw intercept.
    ***************************************************************************/
    void replace_interface_target_port(sc_core::sc_interface* ifs)
    {
        if ( m_parentHandle != NULL ) {

            m_parentHandle->execute_command ("REPLACE_IF_TRGT_PORT", (void*)ifs);

        } else if (m_peerHandle){

            m_peerHandle->execute_command   ("REPLACE_IF_TRGT_PORT", (void*)ifs);
        } else  {
            sc_assert(false);
        }
    }

    /******************************************************************************
     *  function : get_target_clock
     *
     *  purpose  : Convenience function called by start_of_simulation in
     *             implementer socket to set the target clock period
     *******************************************************************************/
    sc_core::sc_time*  get_target_clock()
    {
        void* clk = NULL;

        if( m_parentHandle != NULL )
        {
            clk = m_parentHandle->execute_command("GET_TRGT_CLK");
            return (static_cast<sc_core::sc_time*>(clk));
        } else if (m_peerHandle != NULL)
        {
            clk = m_peerHandle->execute_command  ("GET_TRGT_CLK");
            return (static_cast<sc_core::sc_time*>(clk));
        }

        return NULL;
    }


     /******************************************************************************
     Function: get_init_protocol
     *******************************************************************************/
     unsigned int* get_init_protocol() {

        if ( m_childHandle != NULL ){

           void* protocol = (m_childHandle)->execute_command("GET_INIT_PROTOCOL");
            return (static_cast<unsigned int *>(protocol));

       }
	   /*if(m_initiatorProtocol == TLM2_GP_PROTOCOL_ID) {
			m_initiatorProtocol = scml2::gp_protocol_state_extension::ID;
	    }	*/
        return(static_cast<unsigned int *>    (&m_initiatorProtocol));

     }


    /**************************************************************************
     * Function :  get_init_clock_period
     * Purpose  :
     *************************************************************************/
    sc_core::sc_time* get_init_clock()
    {
        if ( m_childHandle != NULL ){

           void* clock = (m_childHandle)->execute_command("GET_INIT_CLK");
            return (static_cast<sc_core::sc_time *>(clock));

       }
        return &m_initiatorClkPeriod;
    }

    /**************************************************************************
     * Function :  set_target_clock_period
     * Purpose  : Sets the target clock period on the innermost socket- Called
					 from peer socket when its clock period changes dynamically
     *************************************************************************/
    void set_target_clock_period(sc_core::sc_time* clk)
    {
       if ( m_childHandle != NULL ){

           (m_childHandle)->execute_command("SET_TARGET_CLK", (void*)clk);
		   return;

       }
        m_targetClkPeriod = *clk;
		handle_start_of_simulation();
    }

    /******************************************************************************
     Function:  get_target_protocol
    *******************************************************************************/
    unsigned int* get_target_protocol()
    {
        if( m_parentHandle != NULL )
        {
            void* protocol = m_parentHandle->execute_command ("GET_TRGT_PROTOCOL");
              return (static_cast<unsigned int*>(protocol));
        }
        else if (m_peerHandle != NULL)
        {
            void* protocol = m_peerHandle->execute_command ("GET_TRGT_PROTOCOL");
            return (static_cast<unsigned int*>(protocol));
        }

        return NULL;
    }
    /******************************************************************************
     Function:  check_multi_binding
    *******************************************************************************/
    void check_multi_binding(bool* is_multi_bound) {
        if (m_owner->get_base_port().size() > (int)1) {
            // this initiator is connected to multiple targets
            *is_multi_bound = true;
        } else if (m_parentHandle != nullptr) {
            m_parentHandle->execute_command("CHECK_MULTI_BINDING", static_cast<void*>(is_multi_bound));
        } else if (m_peerHandle != nullptr) {
            // check if target is connected to multiple initiators
            m_peerHandle->execute_command("CHECK_MULTI_BINDING", static_cast<void*>(is_multi_bound));
        } else {
            // No parents nor peers registered. Socket may be bound to non-compliant peer. Returning true to block interception.
            *is_multi_bound = true;
        }
    }

    /***************************************************************************
     * Function:  execute_command
     *
     * purpose  : blanket command added to handle different commands that
     *            initiator socket is supposed to handle, first argument is
     *            command type enum and other arguments are typecasted
     *            appropriately and forward to each API.
     ***************************************************************************/
    void* execute_command(std::string cmdStr, void* arg1, void* arg2, void* arg3)
    {
        unsigned int*   t_protocol;
        unsigned int*   i_protocol;
        unsigned int*   protocolId;
        sc_core::sc_time*              clk_period;
        sc_core::sc_time*              clock;
        sc_core::sc_time*              in_clk_period;


        sc_core::sc_interface*         ifs;
        sc_core::sc_interface*         fwIf;

        base_target_socket_type*       s_cast;
        base_target_socket_type*       targetSocket;

        base_type*                     parentSocket;
        bw_interface_type*             bwIf;
        export_type*                   export_ptr;
        target_socket_if_type*         target_cast;


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


        switch (cmd)
        {
            case snps_tlm2::SET_FT_PROTOCOL:
            {
                protocolId = static_cast<unsigned int*>(arg1);
                sc_assert(protocolId != NULL);
				m_initiatorProtocol = *protocolId;
		//        std::cout<< (&(m_owner->get_base_port()))->name() << " Set Protocol called in FT initiator socket "
        //              <<"with value "<<m_initiatorProtocol<<endl;
//                set_protocol(*protocol);
                break;
            }

            case snps_tlm2::SET_FT_CLOCK:
            {
                clock = static_cast<sc_core::sc_time*>(arg1);
                sc_assert(clock != NULL);
                set_clock(*clock);
                break;
            }
            case snps_tlm2::SET_TARGET_CLK:
            {
                clock = static_cast<sc_core::sc_time*>(arg1);
                sc_assert(clock != NULL);
                set_target_clock_period(clock);
                break;
            }

            case snps_tlm2::HANDLE_CONSTRUCTION:
            {
                handle_construction();
                return NULL;
            }

            case snps_tlm2::END_OF_ELAB:
            {
                handle_end_of_elaboration();
                return NULL;
            }

			case snps_tlm2::BEFORE_END_OF_ELAB:
			{
				handle_before_end_of_elaboration();
				return NULL;
			}
            case snps_tlm2::START_OF_SIM:
            {
                handle_start_of_simulation();
                return NULL;
            }

            case snps_tlm2::BIND_INTERFACE:
            {
                bwIf       = static_cast<bw_interface_type*>(arg1);
                export_ptr = static_cast<export_type*>(arg2);
                bind_to_interface (*bwIf, *export_ptr);
                return NULL;
            }

            case snps_tlm2::BIND_TO_PEER:
            {
                targetSocket = static_cast<base_target_socket_type*>(arg1);
                bind_to_peer(*targetSocket);
                return NULL;
            }

            case snps_tlm2::BIND_TO_PARENT:
            {
                parentSocket = static_cast<base_type*>(arg1);
                bind_to_parent(*parentSocket);
                return NULL;
            }

            case snps_tlm2::GET_TRGT_PROTOCOL:
            {
                t_protocol = get_target_protocol();
                return (static_cast<void*>(t_protocol));
            }

            case snps_tlm2::GET_INIT_PROTOCOL:
            {
                i_protocol = get_init_protocol();
                return (static_cast<void*>(i_protocol));
            }
            case snps_tlm2::GET_TRGT_CLK:
            {
                clk_period = get_target_clock();
                return (static_cast<void *>(clk_period));
            }

            case snps_tlm2::GET_INIT_CLK:
            {
                in_clk_period = get_init_clock();
                return (static_cast<void *>(in_clk_period));
            }

            case snps_tlm2::REPLACE_IF_TRGT_PORT:
            {
                ifs = static_cast<sc_core::sc_interface*>(arg1);
                replace_interface_target_port(ifs);
                break;
            }

            case snps_tlm2::SET_PEER_HANDLE:
            {
                target_cast = static_cast<target_socket_if_type*>(arg1);
                m_peerHandle = target_cast;
                m_isScPortBound = true;
                m_portInterface = dynamic_cast<fw_interface_type *>(m_owner->get_base_port().get_interface(0));
                m_initSocket->setForwardPointer(m_portInterface);
                break;
            }

            case snps_tlm2::BIND_TRGT_SOCKET:
            {
                s_cast = static_cast<base_target_socket_type*>(arg1);
                sc_assert(s_cast);
                bind_to_peer(*s_cast);
                break;
            }

            case snps_tlm2::REPLACE_IF_INIT_PORT:
            {
                fwIf = static_cast<sc_core::sc_interface*>(arg1);
                replace_interface_init_port(fwIf);
                break;
            }

            case snps_tlm2::SET_PARENT_HANDLE:
            {
                sc_assert (0);
                break;
            }

            case snps_tlm2::SET_CHILD_HANDLE:
            {
                m_childHandle = static_cast <initiator_socket_if_type*>(arg1);
                break;
            }

            case snps_tlm2::REGISTER_STATE_OBSERVER:
            {
                if (m_childHandle != NULL) {
                    m_childHandle->execute_command ("REGISTER_STATE_OBSERVER", arg1);
                    break;
                }
                break;
            }
            case snps_tlm2::INIT_SOCKET_TYPE:
            {
                if(m_childHandle != NULL)
                {
                    return m_childHandle->execute_command ("INIT_SOCKET_TYPE");
                    break;
                }
                else
                    return static_cast<void *>(&m_socketType);
            }
            case snps_tlm2::GET_FW_DESTINATION:
            {
                fw_interface_type * ifs =
                        m_owner->get_base_port().get_interface(0);
                return (static_cast<void*>(ifs));
            }
            case snps_tlm2::GET_BW_DESTINATION:
            {
	      return (static_cast<void*>( dynamic_cast<bw_interface_type *>(m_owner->get_base_export().get_interface())));
            }

            case snps_tlm2::GET_PORT_INTERFACE:
            {
                return (static_cast<void*>(m_portInterface));
            }
            case snps_tlm2::CHECK_MULTI_BINDING:
	    {
		sc_assert(arg1);
                check_multi_binding(reinterpret_cast<bool*>(arg1));
		break;
	    }
	    case snps_tlm2::GET_INITIATOR_CHILD:
	    {
		if (m_childHandle != NULL) { // work down the initiator hierarchy
		    return m_childHandle->execute_command("GET_INITIATOR_CHILD");
		}
		return static_cast<void*>(m_initSocket);
	    }
            default:
            {
                std::cerr << (&(m_owner->get_base_port()))->name()
                          << ":: impl Unimplemented command :: "<<cmdStr<<" passed "
                          << "as argument to execute_command().\n";
                break;
            }
        }
        return NULL;
    }

  private:

    base_type*                              m_owner;
    initiator_socket_if_type*               m_initSocket ;
    unsigned int                             m_initiatorProtocol;
    unsigned int                            m_targetProtocol;
    sc_core::sc_time                        m_initiatorClkPeriod;
    sc_core::sc_time                        m_targetClkPeriod;
    std::string                             m_socketType;
    fw_interface_type *                     m_portInterface;
    std::map<const std::string, unsigned int>  mProtocol2EnumMap;

  protected:

    sc_core::sc_port_b<fw_interface_type>*           m_parentPortRef;
    initiator_socket_if_type*               m_parentHandle;
    initiator_socket_if_type*               m_childHandle;
    target_socket_if_type*                  m_peerHandle;

    bool                                    m_isImplementer;
    bool                                    m_isScPortBound;
    bool                                    m_handledEndOfElab;
};

}
#endif
