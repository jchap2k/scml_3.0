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



#include <scml2_base.h>
#include <scml2.h>


namespace scml2 { 
namespace objects { 


class action_base : public scml2::base::object {
public:

	action_base(const scml2::base::object_name& name)
		: scml2::base::object(name)
		, execute("execute", this, &action_base::execute_impl)
	{
	}

	// behaviors
	scml2::base::behavior<void> execute;

	// callback definitions
	SCML2_BASE_CALLBACK(main, void);

private:

	virtual void execute_impl() = 0;
};



} // objects
} // scml2
