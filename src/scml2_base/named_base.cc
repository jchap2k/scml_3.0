/***************************************************************************
 * Copyright 1996-2025 Synopsys, Inc.
 *
 * This Synopsys software and all associated documentation are proprietary
 * to Synopsys, Inc. and may only be used pursuant to the terms and
 * conditions of a written license agreement with Synopsys, Inc.
 * All other use, reproduction, modification, or distribution of the
 * Synopsys software or the associated documentation is strictly prohibited.
 ***************************************************************************/

#include <scml2_base/named_base.h>

#ifdef SNPS_SLS_VP_BASE

namespace scml2 { namespace base {

    named_base::named_base(std::string name)
      : m_name(std::move(name))
    {}

    const std::string& named_base::get_name() const
    {
      return m_name;
    }

    std::string named_base::get_basename() const
    {
      return get_name().substr(get_name().rfind(".")+1);
    }

  }}

#endif // SNPS_SLS_VP_BASE
