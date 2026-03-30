/***************************************************************************
 * Copyright 1996-2025 Synopsys, Inc.
 *
 * This Synopsys software and all associated documentation are proprietary
 * to Synopsys, Inc. and may only be used pursuant to the terms and
 * conditions of a written license agreement with Synopsys, Inc.
 * All other use, reproduction, modification, or distribution of the
 * Synopsys software or the associated documentation is strictly prohibited.
 ***************************************************************************/
#include <scml2/symbolic_names.h>

#include <algorithm>
#include <iterator>
#include <cassert>
#include <limits>

namespace scml2 {

symbolic_names::
~symbolic_names()
{
}

bool
symbolic_names::
set_symbolic_name(const std::vector<unsigned char>& value,
                  const std::string& name)
{
  if (!mMaps) {
    mMaps = std::unique_ptr<BiMap>(new BiMap());
  }

  assert (mMaps->mNameValueMap.size() == mMaps->mValueNameMap.size());

  if (mMaps->mNameValueMap.emplace(name, value).second) {
    // the name was previously unknown ...
    if (mMaps->mValueNameMap.emplace(value, name).second) {
      // ... and so was the value 
      assert (mMaps->mNameValueMap.size() == mMaps->mValueNameMap.size());
      return true;
    } else {
      // ... but the value was already taken - remove the first entry again
      mMaps->mNameValueMap.erase(name);
    }
  }

  assert (mMaps->mNameValueMap.size() == mMaps->mValueNameMap.size());
  return false;
}

std::string 
symbolic_names::
get_symbolic_name(const std::vector<unsigned char>& value) const {
  if (!mMaps) {
      return std::string();
  }
  ValueNameMap::const_iterator i = mMaps->mValueNameMap.find(value);
  if (i != mMaps->mValueNameMap.end()) {
    return i->second;
  }
  return std::string();
}

unsigned int
symbolic_names::
collect_symbolic_names(SymbolicNames& symbolicNames) const
{
  if (!mMaps) {
    return 0;
  }

  assert(mMaps->mNameValueMap.size() == mMaps->mValueNameMap.size());
  std::copy(mMaps->mValueNameMap.begin(),
            mMaps->mValueNameMap.end(),
            std::back_insert_iterator<SymbolicNames>(symbolicNames));
  size_t size = mMaps->mValueNameMap.size();
  assert( size < std::numeric_limits<unsigned int>::max());
  return (unsigned int)size;
}

}
