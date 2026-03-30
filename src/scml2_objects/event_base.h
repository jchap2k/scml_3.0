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

class event_base : public scml2::base::object {
public:

	event_base(const scml2::base::object_name& name)
		: scml2::base::object(name)
		, wait("wait", this, &event_base::wait_impl)
	{
	}

	// behaviors
	scml2::base::behavior<void> wait;

	// callback definitions
	SCML2_BASE_CALLBACK_IGNORE(pre, void);
	SCML2_BASE_CALLBACK_IGNORE(post, void);

private:

	virtual void wait_impl() = 0;
};




} // objects
} // scml2

