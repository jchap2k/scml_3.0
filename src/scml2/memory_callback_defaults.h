/***************************************************************************
 * Copyright 1996-2025 Synopsys, Inc.
 *
 * This Synopsys software and all associated documentation are proprietary
 * to Synopsys, Inc. and may only be used pursuant to the terms and
 * conditions of a written license agreement with Synopsys, Inc.
 * All other use, reproduction, modification, or distribution of the
 * Synopsys software or the associated documentation is strictly prohibited.
 ***************************************************************************/

#ifndef SCML2_MEMORY_CALLBACK_DEFAULTS_H
#define SCML2_MEMORY_CALLBACK_DEFAULTS_H

#include <scml2/memory_fast_access.h>
#include <scml2/memory_callback_base.h>
#include <scml2/memory_access_restriction_base.h>
#include <scml2/memory_debug_callback_base.h>
#include <scml2/toplevel_memory_base.h>

namespace scml2 {

template <typename DT>
class memory_defaults 
: public memory_callback_base
, public memory_debug_callback_base
, public memory_access_restriction_base
{
public:
  memory_defaults(const toplevel_memory_base &memory, memory_fast_access<DT> &fastAccess)
  : memory_callback_base()
  , memory_debug_callback_base()
  , memory_access_restriction_base()
  , mMemory(memory) 
  , mFastAccess(fastAccess) {
    static_cast<memory_callback_base *>(this)->ref();
    static_cast<memory_debug_callback_base *>(this)->ref();
    static_cast<memory_access_restriction_base *>(this)->ref();
  }

  ~memory_defaults() {
    assert(static_cast<memory_callback_base *>(this)->getRefCount() == 1);
    assert(static_cast<memory_debug_callback_base *>(this)->getRefCount() == 1);
    assert(static_cast<memory_access_restriction_base *>(this)->getRefCount() == 1);
  }

  virtual void execute(tlm::tlm_generic_payload& trans, sc_core::sc_time& t) {
    mFastAccess.transport(trans);
    if (trans.is_read()) {
        t += mMemory.get_default_read_latency();

    } else {
        assert(trans.is_write());
        t += mMemory.get_default_write_latency();
    }
  }

  virtual unsigned int execute(tlm::tlm_generic_payload& trans) {
    mFastAccess.transport_debug(trans);
    return trans.get_data_length();
  }

  virtual access_restriction_result restrict(tlm::tlm_generic_payload& /*trans*/) {
    return RESTRICT_NO_ERROR;
  }

  virtual bool has_default_restriction() const {
    return true;
  }

  virtual bool has_default_behavior() const {
    return true;
  }

  virtual bool has_never_syncing_behavior() const {
    return true;
  }

  const toplevel_memory_base &mMemory;
  memory_fast_access<DT>& mFastAccess;
};

}

#endif
