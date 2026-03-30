/***************************************************************************
 * Copyright 1996-2025 Synopsys, Inc.
 *
 * This Synopsys software and all associated documentation are proprietary
 * to Synopsys, Inc. and may only be used pursuant to the terms and
 * conditions of a written license agreement with Synopsys, Inc.
 * All other use, reproduction, modification, or distribution of the
 * Synopsys software or the associated documentation is strictly prohibited.
 ***************************************************************************/
#include <scml2/memory_dmi.h>

#include <scml2/tagged_message.h>
#include <scml2/tagged_message_handler_provider.h>
#include <scml2_logging/severity.h>
#include <cassert>

namespace scml2 {

namespace {

template <typename T>
inline
T
next_it(T it)
{
  return ++it;
}

template <typename T>
inline
T
prev_it(T it)
{
  return --it;
}

}

memory_dmi::DmiDataEntry::
DmiDataEntry(unsigned long long startIndex,
             unsigned long long endIndex,
             bool dmiAllowed) :
  startIndex(startIndex),
  endIndex(endIndex),
  dmiAllowed(dmiAllowed)
{
}

memory_dmi::
memory_dmi(unsigned long long size, const tagged_message_handler_provider* provider) 
  : mSize(size)
  , mCleared(false)
  , mProvider(provider)
{
  clear();
}

void
memory_dmi::
clear()
{
  if (mCleared) {
    assert(mDmiData.size() == 2);
    return;
  } 
  mCleared = true;
  if (mDmiData.size() > 2) {
    // remove all but the first two entries
    mDmiData.erase(++ ++mDmiData.begin(), mDmiData.end());
  }

  // contains now at most two elements, which we try to re-use
  auto it = mDmiData.begin();
  if (it != mDmiData.end()) {
    *it++ = DmiDataEntry(0x0, mSize - 1, true);
  } else {
    mDmiData.push_back(DmiDataEntry(0x0, mSize - 1, true));
  }

  if (it != mDmiData.end()) {
    *it = DmiDataEntry(mSize, (unsigned long long)(-1), false);
  } else {
    mDmiData.push_back(DmiDataEntry(mSize, (unsigned long long)(-1), false));
  }

  assert(mDmiData.size() == 2);
}

void
memory_dmi::
disable()
{
  mCleared = false;
  if (mDmiData.empty()) {
    mDmiData.push_back(DmiDataEntry(0x0, (unsigned long long)(-1), false));
  } else {
    mDmiData.erase(++mDmiData.begin(), mDmiData.end());
    mDmiData.front() = DmiDataEntry(0x0, (unsigned long long)(-1), false);
  }
}

void
memory_dmi::
disable(unsigned long long startIndex,
        unsigned long long endIndex)
{
  mCleared = false;

  DmiDataIter it = mDmiData.begin();
  DmiDataConstIter end = mDmiData.end();

  for (; it != end; ++it) {
    if (it->startIndex <= startIndex &&
        it->endIndex >= endIndex &&
        !it->dmiAllowed) {
      // region already disabled
      return;
    }

    if (it->startIndex <= startIndex &&
        it->endIndex >= startIndex) {

      if (it->startIndex < startIndex) {
        if (it->dmiAllowed) {
          DmiDataEntry left = *it;
          left.endIndex = startIndex - 1;
          mDmiData.insert(it, left);

        } else {
          startIndex = it->startIndex;
        }

      } else {
        // merge with previous range
        if (it != mDmiData.begin()) {
          DmiData::iterator p = prev_it(it);
          if (!p->dmiAllowed) {
            startIndex = p->startIndex;
            mDmiData.erase(p);
          }
        }
      }

      for (; it->endIndex < endIndex; ) {
        it = mDmiData.erase(it);
      }
      assert(it->startIndex <= endIndex);

      if (it->endIndex > endIndex) {
        if (it->dmiAllowed) {
          DmiDataEntry right = *it;
          right.startIndex = endIndex + 1;
          mDmiData.insert(next_it(it), right);

        } else {
          endIndex = it->endIndex;
        }

      } else {
        // merge with next range
        DmiData::iterator n = next_it(it);
        if (n != mDmiData.end()) {
          if (!n->dmiAllowed) {
            endIndex = n->endIndex;
            mDmiData.erase(n);
          }
        }
      }

      *it = DmiDataEntry(startIndex, endIndex, false);
      return;
    }
  }

  SCML2_ERROR_TO(mProvider, UNDEFINED_ERROR) << "DMI range calculation failed" << std::endl;
}

const memory_dmi::DmiDataEntry&
memory_dmi::
find(unsigned long long index) const
{
  DmiDataConstIter it = mDmiData.begin();
  DmiDataConstIter end = mDmiData.end();

  for (; it != end; ++it) {
    if (it->startIndex <= index &&
        it->endIndex >= index) {
      return *it;
    }
  }
  SCML2_ERROR_TO(mProvider, UNDEFINED_ERROR) << "Index out of bounds" << std::endl;
  return *mDmiData.begin();
}

}
