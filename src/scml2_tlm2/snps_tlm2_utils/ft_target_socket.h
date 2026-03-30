/***************************************************************************
 * Copyright 1996-2025 Synopsys, Inc.
 *
 * This Synopsys software and all associated documentation are proprietary
 * to Synopsys, Inc. and may only be used pursuant to the terms and
 * conditions of a written license agreement with Synopsys, Inc.
 * All other use, reproduction, modification, or distribution of the
 * Synopsys software or the associated documentation is strictly prohibited.
 ***************************************************************************/
#ifndef SCML2_FT_TARGET_SOCKET_H
#define SCML2_FT_TARGET_SOCKET_H

//#include <tlm_h/tlm_sockets/tlm_target_socket.h>
#include "tlm"
#include "scml2_tlm2/snps_tlm2_utils/ft_socket_impl_factory.h"
#if (!defined CWR_SYSTEMC && defined SYSTEMC_VERSION && SYSTEMC_VERSION <= 20221128 /*2.3.4*/)
#else
#include <typeindex>
#endif

namespace scml2{

template <unsigned int BUSWIDTH = 32,
          typename TYPES = tlm::tlm_base_protocol_types,
          int N = 1
#if !(defined SYSTEMC_VERSION & SYSTEMC_VERSION <= 20050714)
          ,sc_core::sc_port_policy POL = sc_core::SC_ONE_OR_MORE_BOUND
#endif
         >
class ft_target_socket :
	public snps_tlm2::snps_tlm2_socket_if,
    public tlm::tlm_target_socket<BUSWIDTH
									,TYPES
									,N
#if !(defined SYSTEMC_VERSION & SYSTEMC_VERSION <= 20050714)
									,POL	
#endif
									> 
{

public:

  typedef tlm::tlm_fw_transport_if<> fw_interface_type;
  typedef tlm::tlm_bw_transport_if<> bw_interface_type;

  typedef tlm::tlm_base_initiator_socket_b<BUSWIDTH,
                                      fw_interface_type,
                                      bw_interface_type
                                     > base_initiator_socket_type;
 
  typedef tlm::tlm_base_target_socket_b<BUSWIDTH,
                                      fw_interface_type,
                                      bw_interface_type
                                     > base_target_socket_type;

  typedef tlm::tlm_target_socket<BUSWIDTH, TYPES, N
#if !(defined SYSTEMC_VERSION & SYSTEMC_VERSION <= 20050714)
									,POL	
#endif
									> base_type;


  
  ft_target_socket() :
  base_type()
  ,mBwIf(NULL)
  {
   
    m_targetSocketImpl = scml2::template create_ft_socket_impl<BUSWIDTH>(this, false);
    m_targetSocketImpl->execute_command("HANDLE_CONSTRUCTION");
     
  }

  explicit ft_target_socket(const char* name) :
  base_type(name)
  ,mBwIf(NULL)

  {
    m_targetSocketImpl = scml2::template create_ft_socket_impl<BUSWIDTH>(this, false);
    m_targetSocketImpl->execute_command("HANDLE_CONSTRUCTION");
  }
     
  virtual ~ft_target_socket()
  {
      delete m_targetSocketImpl;
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

  //Binds the export to the interface.
  void bind(fw_interface_type& ifs)
  {
    m_targetSocketImpl->execute_command("BIND_INTERFACE", (void *)&ifs);	
   //  this->get_base_export().sc_core::sc_export<fw_interface_type>::bind(ifs);
  }

  void operator() (fw_interface_type& ifs)
  {
      bind(ifs);
  }
    /************************************ ****************************************
     *  function : overloaded operator ->
     *  Returns the forward interface pointer which is given by the impl class           
     ***************************************************************************/
	tlm::tlm_bw_transport_if<TYPES> * operator ->() {
		
		return mBwIf;
	}
    /************************************ ****************************************
     *  function : setForwardPointer
     * Called by the socketImpl class to set the forward pointer whenever it changes
     ***************************************************************************/
	void setForwardPointer(void* tempPtr) {

		mBwIf = static_cast<tlm::tlm_bw_transport_if<TYPES> *>(tempPtr);

	}

  //peer binding
  void bind(base_initiator_socket_type& s)
  {
	m_targetSocketImpl->execute_command("BIND_TO_PEER", (void *)&s);

  }

  void operator() (base_initiator_socket_type& s)
  {
     bind(s);
  }

  // Bind target socket to target socket (hierarchical bind)
  // - Binds both the export and the port
  // s is the reference to child module.
  void bind(base_target_socket_type& s)
  {
	m_targetSocketImpl->execute_command("BIND_TO_CHILD", (void *)&s);
  }

  void operator() (base_target_socket_type& s)
  {
    bind(s); 
  }

  unsigned int get_bus_width() const
  {
    return BUSWIDTH;
  }

  void before_end_of_elaboration() 
  {
   	 m_targetSocketImpl->execute_command("BEFORE_END_OF_ELAB");
  }	

  void end_of_elaboration() 
  {
   	 m_targetSocketImpl->execute_command("END_OF_ELAB");
  }	
	
  void start_of_simulation()
  {
	m_targetSocketImpl->execute_command("START_OF_SIM");
  }	
  //must be called before end_of_elaboration. 
  template <typename ext_type>
  void set_protocol(std::string protocol)
  {
    sc_assert(protocol.empty() == false);
    ext_type    ext;    //Dummy instance to register mapping
    (void)(ext);        // prevent unused variable warning
    (void)(protocol);        // prevent unused variable warning

	unsigned int proto = ext_type::ID;
	m_targetSocketImpl->execute_command("SET_FT_PROTOCOL", (void *)(&proto), NULL, NULL);
  }

  /****************************************************************************
  *  function : set_clock 
  *
  *  purpose  : sets clock.  
  ****************************************************************************/
  void set_clock(sc_core::sc_time clockPeriod)
  {
    sc_core::sc_time* clk = new sc_core::sc_time(clockPeriod);
	m_targetSocketImpl->execute_command("SET_FT_CLOCK", (void *)clk, NULL, NULL);
	delete clk;
  }
 
  void* execute_command(std::string cmd, void* arg1 = NULL, void* arg2 = NULL, void* arg3 = NULL) 
  {
    return m_targetSocketImpl->execute_command(cmd, arg1, arg2, arg3);
  }	
   /****************************************************************************
     *  function : socket_type
     *  purpose  : Returns the type of socket
	******************************************************************************/
  	std::string socket_type() {
		 return "ft_target_socket";
  	}		
  
protected:
  	snps_tlm2::snps_tlm2_socket_if*			m_targetSocketImpl;	
	tlm::tlm_bw_transport_if<TYPES>*		mBwIf;
}; //ft_target_socket
} //namespace scml2
#endif
