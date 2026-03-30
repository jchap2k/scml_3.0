/***************************************************************************
 * Copyright 1996-2025 Synopsys, Inc.
 *
 * This Synopsys software and all associated documentation are proprietary
 * to Synopsys, Inc. and may only be used pursuant to the terms and
 * conditions of a written license agreement with Synopsys, Inc.
 * All other use, reproduction, modification, or distribution of the
 * Synopsys software or the associated documentation is strictly prohibited.
 ***************************************************************************/
#ifndef SCML2_FT_INITIATOR_SOCKET_H
#define SCML2_FT_INITIATOR_SOCKET_H


#include "tlm"
//#include <tlm_h/tlm_sockets/tlm_initiator_socket.h>
#include <scml2_tlm2/snps_tlm2_utils/ft_socket_impl_factory.h>
#if (!defined CWR_SYSTEMC && defined SYSTEMC_VERSION && SYSTEMC_VERSION <= 20221128 /*2.3.4*/)
#include <sysc/utils/sc_typeindex.h>
#else
#include <typeindex>
#endif

namespace scml2
{
template <unsigned int BUSWIDTH = 32,
          typename TYPES = tlm::tlm_base_protocol_types,
          int N = 1
#if !(defined SYSTEMC_VERSION & SYSTEMC_VERSION <= 20050714)
          ,sc_core::sc_port_policy POL = sc_core::SC_ONE_OR_MORE_BOUND
#endif
         >
class ft_initiator_socket
	 :public tlm::tlm_base_initiator_socket<BUSWIDTH
                                    ,tlm::tlm_fw_transport_if<>
									,tlm::tlm_bw_transport_if<>
                                    ,N
#if !(defined SYSTEMC_VERSION & SYSTEMC_VERSION <= 20050714)
                                    ,POL    
#endif
                                    > 
		,public snps_tlm2::snps_tlm2_socket_if
{
  public:
    typedef tlm::tlm_fw_transport_if<> fw_interface_type;
    typedef tlm::tlm_bw_transport_if<> bw_interface_type;

    typedef tlm::tlm_base_initiator_socket<BUSWIDTH, 
                                     fw_interface_type, bw_interface_type, N
#if !(defined SYSTEMC_VERSION & SYSTEMC_VERSION <= 20050714)
                                      , POL    
#endif
                                    > base_type;
     

    typedef sc_core::sc_export<bw_interface_type> export_type;
    
    typedef tlm::tlm_base_target_socket_b<BUSWIDTH,
                                   fw_interface_type,
                                   bw_interface_type> base_target_socket_type;

   
    typedef tlm::tlm_base_initiator_socket_b<BUSWIDTH,
                                   fw_interface_type,
                                   bw_interface_type> base_initiator_socket_type;

   
    
    /****************************************************************************
     *  function    : Constructor.
     *  description : intsanitiates docket implementaion class.
     *                call execute command for the contruction of the socket.
     * *************************************************************************/
    ft_initiator_socket()
       : base_type()
		 ,mFwIf(0)	
    {
        m_socketImpl = scml2::template create_ft_socket_impl<BUSWIDTH>(this, true);
        m_socketImpl->execute_command("HANDLE_CONSTRUCTION");
    }

    /****************************************************************************
     *  function : Constructor.
     * *************************************************************************/
    explicit ft_initiator_socket(const char* name)
         : base_type(name)
		   ,mFwIf(0)	
    {
         m_socketImpl = scml2::template create_ft_socket_impl<BUSWIDTH>(this, true);
         m_socketImpl->execute_command("HANDLE_CONSTRUCTION");
    }
 
    virtual ~ft_initiator_socket()
    {
        delete m_socketImpl;
    }

    // implementation of abstact interface method (from tlm_base_socket_if)
#if (!defined CWR_SYSTEMC && defined SYSTEMC_VERSION && SYSTEMC_VERSION <= 20221128 /*2.3.4*/)
    virtual sc_core::sc_type_index get_protocol_types() const
#else
    virtual std::type_index get_protocol_types() const
#endif
    {
        return typeid(TYPES);
    }

    void before_end_of_elaboration()
    {
		m_socketImpl->execute_command("BEFORE_END_OF_ELAB");
	}	   
    /****************************************************************************
     *  function : end_of_elaboration.
     *  purpose  : For the Implementer (socket connected to the inner most
     *             initiator ) only.
     *             1. This function passes the ipt pointers to the fw and bw 
     *                implementation classes.
     *             2. Attaches fowrarding IPT's to other initiator sockets.
     *                These would primarily be tlm_base_initiator_sockets on 
     *                the initiators parents in the Hierarchy.
     *             3. Initializes the mapper and passes it to the fw and bw 
     *                interface implementers.
     ***************************************************************************/
    void end_of_elaboration()
    {
        m_socketImpl->execute_command("END_OF_ELAB");
    }

    /****************************************************************************
     *  function : start_of_simulation()  
     *                                    
     *  purpose  : to set clock for Protocol checker
     ************************************ ***************************************/
    void start_of_simulation()            
    {                                     
        m_socketImpl->execute_command("START_OF_SIM");    
    }                                     
                                          
    /************************************ ****************************************
     *  function : bind to an interface 
     *  purpose  : Will be called only for the socket present on the initiator
     *             implementing the backward interface.
     ***************************************************************************/
    void bind(bw_interface_type& ifs)
    {
        m_socketImpl->execute_command("BIND_INTERFACE", 
                (void*)(&ifs), 
                (void*)(&(this->m_export))
        );
    }

    /************************************ ****************************************
     *  function : overloaded operator ()
     *             
     ***************************************************************************/
    void operator() (bw_interface_type& ifs)
    {
        bind(ifs);
    }

    /************************************ ****************************************
     *  function : overloaded operator ->
     *  Returns the forward interface pointer which is given by the impl class           
     ***************************************************************************/
	tlm::tlm_fw_transport_if<TYPES> * operator ->() {
		
		return mFwIf;
	}
	
    /************************************ ****************************************
     *  function : setForwardPointer
     * Called by the socketImpl class to set the forward pointer whenever it changes
     ***************************************************************************/
	void setForwardPointer(void* tempPtr) {

		mFwIf = static_cast<tlm::tlm_fw_transport_if<TYPES> *>(tempPtr);

	}
    /****************************************************************************
     *  function : bind to a peer 
     *  purpose  : This function is called irrespective of the socket on which the
     *             peer binding function is called.
     *             target_socket.bind(initiator_socket) will also land up here.
     *             
     *             1. Binds port of initiator/target to export of target/intiator.
     ****************************************************************************/
    void bind (base_target_socket_type& s)
    {
        m_socketImpl->execute_command("BIND_TO_PEER", (void *)&s);
    }
  
    //operator()
    void operator() (base_target_socket_type& s)
    {
        bind(s);
    }
  
    /****************************************************************************
     *  function : bind to a parent.s is the reference to parent module.
     *  purpose  : This function is called on all the children in a hierarchy.
     *             1. Binds port of initiator socket to port of parents intiator
     *                socket and export of parent's initaitor socket to export of
     *                childs initiator socket.
     *             2. Intercepts the forward IF for the implementer and stores the 
     *                m_parentPortRef for completing the path at
     *  
     ****************************************************************************/
    void bind(base_initiator_socket_type& s)
    {
        m_socketImpl->execute_command("BIND_TO_PARENT", (void*)&s);
    }

    //operator ()
    void operator() (base_initiator_socket_type& s)
    {
        bind(s);
    }


    /****************************************************************************
     *  function : execute_command 
     *  purpose  : Implementation of the snps_tlm2::snps_tlm2_socket_if
     *             This function is used to delegate the commands to the socket 
     *             implememntation class.
     ****************************************************************************/
    void* execute_command ( std::string cmdStr, 
                            void*       arg1  = NULL, 
                            void*       arg2  = NULL, 
                            void*       arg3  = NULL) 
    {
        return m_socketImpl->execute_command (cmdStr, arg1, arg2, arg3);
    }    
    
    /****************************************************************************
     *  function : set_protocol. 
     *  purpose  : sets protocol. Will be called only for the socket connected to
     *             the innermost initiator (which is also the initiator
     *             implementing the bw interface)
     *  
     *             Must be called before end_of_elaboration 
     ****************************************************************************/
    template <typename ext_type>
    void set_protocol(std::string protocol)
    {
        (void)(protocol);        // prevent unused variable warning
        sc_assert(protocol.empty() == false);
		unsigned int proto = ext_type::ID;
        m_socketImpl->execute_command("SET_FT_PROTOCOL", (void* )(&proto), NULL, NULL );
    }

    /****************************************************************************
    *  function : set_clock 
    *  purpose  : sets clock. Will be called only for the socket connected to
    *             the innermost initiator (which is also the initiator
    *             implementing the bw interface)
    *             Must be called from end_of_elaboration 
    ****************************************************************************/
    void set_clock(sc_core::sc_time clockPeriod)
    {
        sc_core::sc_time* clk = new sc_core::sc_time(clockPeriod);
        m_socketImpl->execute_command("SET_FT_CLOCK", (void* )clk, NULL, NULL );
        delete clk;
    }

  protected:
    snps_tlm2::snps_tlm2_socket_if*            m_socketImpl;    
	tlm::tlm_fw_transport_if<TYPES> *		   mFwIf;

}; //ft_initiator_socket

 
} //namespace scml2
#endif
