/***************************************************************************
 * Copyright 1996-2025 Synopsys, Inc.
 *
 * This Synopsys software and all associated documentation are proprietary
 * to Synopsys, Inc. and may only be used pursuant to the terms and
 * conditions of a written license agreement with Synopsys, Inc.
 * All other use, reproduction, modification, or distribution of the
 * Synopsys software or the associated documentation is strictly prohibited.
 ***************************************************************************/
#include <scml2/bitfield_disallow_write_access_callback.h>
#ifdef SNPS_VP_TAGGED_MESSAGES
#include <scml2/tagged_message.h>
#endif
#include <scml2/bitfield.h>

namespace scml2 {

template <typename DT>
inline
bool
bitfield_disallow_write_access_callback<DT>::
write(const DT&, const DT&, sc_core::sc_time&)
{
#ifdef SNPS_VP_TAGGED_MESSAGES
  if (mReturnError) {
    SCML2_MODEL_INTERNAL_TO(&mOwner.get_register(), LEVEL6) 
      << "write access denied on " << mOwner.name() << std::endl;
  } else {
    SCML2_MODEL_INTERNAL_TO(&mOwner.get_register(), LEVEL6) 
      << "write access ignored on " << mOwner.name() << std::endl;
  }
#endif

  return !mReturnError;
}

template <typename DT>
inline
bool
bitfield_disallow_write_access_callback<DT>::
has_never_syncing_behavior() const
{
  return true;
}

 #ifdef _WIN32
#define SCML2_INSTANTIATE_TEMPLATE(type)				\
  template class SNPS_VP_API bitfield_disallow_write_access_callback<type>;
#else
#define SCML2_INSTANTIATE_TEMPLATE(type)				\
  template class bitfield_disallow_write_access_callback<type>;
#endif
SCML2_FOR_EACH_DT(SCML2_INSTANTIATE_TEMPLATE)
#undef SCML2_INSTANTIATE_TEMPLATE

}
