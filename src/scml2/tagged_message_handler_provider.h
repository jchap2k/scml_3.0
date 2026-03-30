/***************************************************************************
 * Copyright 1996-2025 Synopsys, Inc.
 *
 * This Synopsys software and all associated documentation are proprietary
 * to Synopsys, Inc. and may only be used pursuant to the terms and
 * conditions of a written license agreement with Synopsys, Inc.
 * All other use, reproduction, modification, or distribution of the
 * Synopsys software or the associated documentation is strictly prohibited.
 ***************************************************************************/
#ifndef SCML2_TAGGED_MESSAGE_HANDLER_PROVIDER_INCLUDED
#define SCML2_TAGGED_MESSAGE_HANDLER_PROVIDER_INCLUDED

#include <scml2_logging/snps_vp_dll.h>

#include <systemc>


namespace scml2 {

  class SNPS_VP_API tagged_message_handler_provider {
  public:
    tagged_message_handler_provider(const sc_core::sc_object* o)
      : m_sc_object(o)
    {}
    virtual ~tagged_message_handler_provider() {}


  private:
    
    
    mutable const sc_core::sc_object* m_sc_object;
  };

  
}

#endif 
