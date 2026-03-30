/***************************************************************************
 * Copyright 1996-2025 Synopsys, Inc.
 *
 * This Synopsys software and all associated documentation are proprietary
 * to Synopsys, Inc. and may only be used pursuant to the terms and
 * conditions of a written license agreement with Synopsys, Inc.
 * All other use, reproduction, modification, or distribution of the
 * Synopsys software or the associated documentation is strictly prohibited.
 ***************************************************************************/
#ifndef SCML2_SIMPLE_INITIATOR_SOCKET_H
#define SCML2_SIMPLE_INITIATOR_SOCKET_H

#include <scml2_tlm2/snps_tlm2_utils/ft_initiator_socket.h>
#include <tlm_utils/simple_initiator_socket.h>
#include <sstream>

namespace scml2{

template <typename MODULE,
          unsigned int BUSWIDTH = 32,
          typename TYPES = tlm::tlm_base_protocol_types>
class simple_initiator_socket :
  public scml2::ft_initiator_socket<BUSWIDTH, TYPES>
{
public:
  typedef typename TYPES::tlm_payload_type              transaction_type;
  typedef typename TYPES::tlm_phase_type                phase_type;
  typedef tlm::tlm_sync_enum                            sync_enum_type;
  typedef tlm::tlm_fw_transport_if<TYPES>               fw_interface_type;
  typedef tlm::tlm_bw_transport_if<TYPES>               bw_interface_type;
  typedef scml2::ft_initiator_socket<BUSWIDTH, TYPES>    base_type;

public:
  simple_initiator_socket() :
    base_type(sc_core::sc_gen_unique_name("simple_initiator_socket")),
    m_process(this->name())
  {
    base_type::bind(m_process);
  }

  explicit simple_initiator_socket(const char* n) :
    base_type(n),
    m_process(this->name())
  {
    base_type::bind(m_process);
  }

  void register_nb_transport_bw(MODULE* mod,
                                sync_enum_type (MODULE::*cb)(transaction_type&,
                                                             phase_type&,
                                                             sc_core::sc_time&))
  {
    m_process.set_transport_ptr(mod, cb);
  }

  void register_invalidate_direct_mem_ptr(MODULE* mod,
                                          void (MODULE::*cb)(sc_dt::uint64, sc_dt::uint64))
  {
    m_process.set_invalidate_direct_mem_ptr(mod, cb);
  }

private:
  class process : public tlm::tlm_bw_transport_if<TYPES>
  {
  public:
    typedef sync_enum_type (MODULE::*TransportPtr)(transaction_type&,
                                                   phase_type&,
                                                   sc_core::sc_time&);
    typedef void (MODULE::*InvalidateDirectMemPtr)(sc_dt::uint64,
                                                   sc_dt::uint64);
      
    process(const std::string& name) :
      m_name(name),
      m_mod(0),
      m_transport_ptr(0),
      m_invalidate_direct_mem_ptr(0)
    {
    }
  
    void set_transport_ptr(MODULE* mod, TransportPtr p)
    {
      if (m_transport_ptr) {
        std::stringstream s;
        s << m_name << ": non-blocking callback allready registered";
        SC_REPORT_WARNING_MODELER_ONLY("/OSCI_TLM-2/simple_socket",s.str().c_str());
      } else {
        assert(!m_mod || m_mod == mod);
        m_mod = mod;
        m_transport_ptr = p;
      }
    }

    void set_invalidate_direct_mem_ptr(MODULE* mod, InvalidateDirectMemPtr p)
    {
      if (m_invalidate_direct_mem_ptr) {
        std::stringstream s;
        s << m_name << ": invalidate DMI callback allready registered";
        SC_REPORT_WARNING_MODELER_ONLY("/OSCI_TLM-2/simple_socket",s.str().c_str());
      } else {
        assert(!m_mod || m_mod == mod);
        m_mod = mod;
        m_invalidate_direct_mem_ptr = p;
      }
    }

    sync_enum_type nb_transport_bw(transaction_type& trans, phase_type& phase, sc_core::sc_time& t)
    {
      if (m_transport_ptr) {
        // forward call
        assert(m_mod);
        return (m_mod->*m_transport_ptr)(trans, phase, t);

      } else {
        std::stringstream s;
        s << m_name << ": no transport callback registered";
        SC_REPORT_ERROR("/OSCI_TLM-2/simple_socket",s.str().c_str());
      }
      return tlm::TLM_ACCEPTED;   ///< unreachable code
    }

    void invalidate_direct_mem_ptr(sc_dt::uint64 start_range,
                                   sc_dt::uint64 end_range)
    {
      if (m_invalidate_direct_mem_ptr) {
        // forward call
        assert(m_mod);
        (m_mod->*m_invalidate_direct_mem_ptr)(start_range, end_range);
      }
    }

  private:
    const std::string m_name;
    MODULE* m_mod;
    TransportPtr m_transport_ptr;
    InvalidateDirectMemPtr m_invalidate_direct_mem_ptr;
  };

private:
  process m_process;
};
 
} //namespace scml2
#endif
