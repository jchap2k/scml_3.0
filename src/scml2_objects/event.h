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



#include "event_base.h"

namespace scml2 { 
namespace objects { 

// coverity[dtor_in_derived]
class event : public event_base, public sc_core::sc_event {
public:

	event(const scml2::base::object_name& name) : event_base(name), sc_core::sc_event() {}
	void finalize_local_construction() {}

private:
	void wait_impl() { pre(); sc_core::wait(*this); post(); };
};



} // objects
} // scml2
