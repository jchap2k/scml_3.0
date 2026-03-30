/***************************************************************************
 * Copyright 1996-2025 Synopsys, Inc.
 *
 * This Synopsys software and all associated documentation are proprietary
 * to Synopsys, Inc. and may only be used pursuant to the terms and
 * conditions of a written license agreement with Synopsys, Inc.
 * All other use, reproduction, modification, or distribution of the
 * Synopsys software or the associated documentation is strictly prohibited.
 ***************************************************************************/
#include <scml2/status_registry.h>
#include <scml2/status.h>

#include <cassert>
#include <limits>

namespace scml2 {

status_registry&
status_registry::
get_instance()
{
  static status_registry instance;
  return instance;
}

status_registry::
status_registry()
{
}

status_registry::
~status_registry()
{
}

bool
status_registry::
register_status(status* m)
{
  assert(m);
  return mStates.insert(m).second;
}

bool
status_registry::
unregister_status(status* m)
{
  return (mStates.erase(m) != 0);
}

status*
status_registry::
find_status_by_name(const std::string& name) const
{
  auto obj = sc_core::sc_get_curr_simcontext()->find_object(name.c_str());
  return dynamic_cast<status*>(obj);
}

unsigned int
status_registry::
collect_statuses(std::vector<status*>& v) const
{
  size_t size =  mStates.size();
  assert( size < std::numeric_limits<unsigned int>::max());

  v.reserve(v.size() + size);
  for (auto s : mStates) {
    v.push_back(s);
  }

  return (unsigned int)size;
}

}
