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



#include "action_base.h"

namespace scml2 { 
namespace objects { 

class action : public action_base {
public:

	action(const scml2::base::object_name& name) : action_base(name) {}

	void finalize_local_construction() {}

private:

	void execute_impl() { main(); };
};


} // objects
} // scml2
