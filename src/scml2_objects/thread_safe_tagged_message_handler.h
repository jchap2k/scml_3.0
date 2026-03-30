/***************************************************************************
 * Copyright 1996-2025 Synopsys, Inc.
 *
 * This Synopsys software and all associated documentation are proprietary
 * to Synopsys, Inc. and may only be used pursuant to the terms and
 * conditions of a written license agreement with Synopsys, Inc.
 * All other use, reproduction, modification, or distribution of the
 * Synopsys software or the associated documentation is strictly prohibited.
 ***************************************************************************/

#pragma once

#include <systemc>
#include <scml2/tagged_message_macros.h>
#include <scml2_base.h>


namespace scml2 {

  class thread_safe_tagged_message_handler : public scml2::base::object {
  public:
    thread_safe_tagged_message_handler(const scml2::base::object_name& name)
      : scml2::base::object(name)
      {}
  };

}

