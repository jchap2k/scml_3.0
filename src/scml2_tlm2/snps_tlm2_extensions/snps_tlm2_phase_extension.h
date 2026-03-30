/***************************************************************************
 * Copyright 1996-2025 Synopsys, Inc.
 *
 * This Synopsys software and all associated documentation are proprietary
 * to Synopsys, Inc. and may only be used pursuant to the terms and
 * conditions of a written license agreement with Synopsys, Inc.
 * All other use, reproduction, modification, or distribution of the
 * Synopsys software or the associated documentation is strictly prohibited.
 ***************************************************************************/

#ifndef __SNPS_TLM2_EXTENSION_H__
#define __SNPS_TLM2_EXTENSION_H__

#include "tlm"

namespace scml2 {

//Declare a new "FT_TIMING" phase to model Additional Timing Points of Different Protocols
DECLARE_EXTENDED_PHASE(FT_TIMING);

#ifndef _MSC_VER
static void _ft_timing_(void) __attribute__ ((unused));
static void _ft_timing_(void) {
    (void)FT_TIMING;
}
#endif
} /* namespace scml2*/

#endif /* __SNPS_TLM2_EXTENSION_H__  */

