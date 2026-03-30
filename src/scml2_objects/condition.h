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



#include "condition_base.h"

namespace scml2 { 
namespace objects { 

class condition : public condition_base {
public:

	condition(const scml2::base::object_name& name) : condition_base(name) {}

	void finalize_local_construction() {}

private:

	bool evaluate_impl() { return main(); };
};



} // objects
} // scml2
