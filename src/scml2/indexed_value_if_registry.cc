/***************************************************************************
 * Copyright 1996-2025 Synopsys, Inc.
 *
 * This Synopsys software and all associated documentation are proprietary
 * to Synopsys, Inc. and may only be used pursuant to the terms and
 * conditions of a written license agreement with Synopsys, Inc.
 * All other use, reproduction, modification, or distribution of the
 * Synopsys software or the associated documentation is strictly prohibited.
 ***************************************************************************/

#include "indexed_value_if_registry.h"

#include <algorithm>
#include <cassert>
#include <cstring>
#include <set>

#include <iostream>
#include <sstream>

#define INDEXED_VALUE_IF_VERBOSE 0

namespace scml2 {

  bool indexed_value_if_entry::operator<(const scml2::indexed_value_if_entry& rhs) const {
    if (get_name_ref()   < rhs.get_name_ref())   {return true;}
    if (get_name_ref()   > rhs.get_name_ref())   {return false;}
    if (get_start()  < rhs.get_start())  {return true;}
    if (get_start()  > rhs.get_start())  {return false;}
    if (get_size()   < rhs.get_size())   {return true;}
    if (get_size()   > rhs.get_size())   {return false;}
    if (get_offset() < rhs.get_offset()) {return true;}
    if (get_offset() > rhs.get_offset()) {return false;}
    return false;
  }
  
  bool indexed_value_if_entry::operator==(const scml2::indexed_value_if_entry& rhs) const {
    return !((*this)<rhs || rhs<(*this));
  }
  
  std::ostream& operator<<(std::ostream& os, const indexed_value_if_entry& e) {
    os << e.get_name_ref() << "(" << e.get_start() << ", " << e.get_size() << ", " << e.get_offset() << ")";
    return os;
  }
  
  indexed_value_if_entry::
  indexed_value_if_entry
  (const std::string& name, unsigned start, unsigned size, unsigned offset)
    : mName(name)
    , mStart(start)
    , mSize(size)
    , mOffset(offset)
  {
  }


  indexed_value_if_registry::
  indexed_value_if_registry()
  {
  }

  indexed_value_if_registry
  indexed_value_if_registry::
  sInstance;

  indexed_value_if_registry&
  indexed_value_if_registry::
  get_instance()
  {
    return sInstance;
  }

  void
  indexed_value_if_registry::
  get_forwardings_for_target(const std::string& name, std::vector<std::string>& entries){
#if INDEXED_VALUE_IF_VERBOSE
    std::cout << "get_forwardings_for_target(" << name << ")" << std::endl;
#endif
    std::set<std::string> names_to_process;
    std::set<std::string> result;
    names_to_process.insert(name);
    while (!names_to_process.empty()) {
      // coverity[auto_uses_copy]
      auto name_to_process = *names_to_process.begin();
      names_to_process.erase(name_to_process);
      result.insert(name_to_process);
#if INDEXED_VALUE_IF_VERBOSE
      std::cout << " adding forwarding " << name_to_process << std::endl;
#endif
      for (auto providerp : mProviders) {
	if (providerp->has_forwardings(name_to_process)) {
	  std::vector<std::string> tmp_entries;
	  providerp->get_forwardings(name_to_process, tmp_entries);
	  for (auto& te : tmp_entries) {
	    if (result.find(te) == result.end()) {
	      names_to_process.insert(te);
	    }
	  }
	}
      }
    }
    entries.insert(entries.end(), result.begin(), result.end());
  }

#if INDEXED_VALUE_IF_VERBOSE
  namespace {
    unsigned int indent_cnt = 0;
    std::string indent() {
      std::string result;
      for (unsigned i = 0; i != indent_cnt; ++i) {
	result += " ";
      }
      return result;
    }
    void down() {
    indent_cnt++;
    }
    void up() {
      indent_cnt--;
    }
  }
#endif 

  namespace {
    void filter_duplicates_and_contained_entries(std::vector<indexed_value_if_entry>& entries) {
      std::sort(entries.begin(), entries.end());
      entries.erase(std::unique(entries.begin(), entries.end()), entries.end());
      std::vector<indexed_value_if_entry> result;
      while (!entries.empty()) {
	// coverity[auto_uses_copy]
	auto last = entries.back();
	auto begini = std::lower_bound(entries.begin(), entries.end(), last, [](const indexed_value_if_entry& lhs, const indexed_value_if_entry& rhs){return lhs.get_name_ref() < rhs.get_name_ref();});
	auto endi = entries.end();
	assert(begini != endi);
	for (auto i = begini; i != endi; ++i) {
	  bool fitsinother = false;
	  for (auto j = begini; j != endi; ++j) {
	    if (i != j) {
	      // coverity[auto_uses_copy]
	      auto lhs =*i;
	      // coverity[auto_uses_copy]
	      auto rhs =*j;
	      fitsinother |= (lhs.get_offset() >= rhs.get_offset() && 
			      lhs.get_offset()+lhs.get_size()<=rhs.get_offset()+rhs.get_size() && 
			      lhs.get_offset()-rhs.get_offset() == lhs.get_start()-rhs.get_start());
	      if (fitsinother) {
#if INDEXED_VALUE_IF_VERBOSE
		std::cout << "get_entries_for_target_slow: " << lhs << " fits in " << rhs << std::endl;
#endif
		break;
	      }
	    }
	  }
	  if (!fitsinother) {
	    result.push_back(*i);
	  }
	}
	entries.erase(begini, endi);
      }
      entries = result;
    }

    bool is_not_unmapped_entry(const indexed_value_if_entry& entry, const std::set<std::string>& unmapped_names) {
      return unmapped_names.count(entry.get_name_ref()) == 0;
    }
  }

  void
  indexed_value_if_registry::
  get_entries_for_target_slow
  (const std::string& name, std::vector<indexed_value_if_entry>& entries)
  {
#if INDEXED_VALUE_IF_VERBOSE
    down();
    std::cout << indent() << "get_entries_for_target_slow(" << name << ")" << std::endl;
#endif
    std::vector<indexed_value_if_entry> entries_to_be_processed;
    for (auto providerp : mProviders) {
      if (providerp->has_entries(name)) {
	providerp->get_entries(name, entries_to_be_processed);
      }
    }
    while (!entries_to_be_processed.empty()) {
      indexed_value_if_entry entry = entries_to_be_processed.back(); entries_to_be_processed.pop_back();
#if INDEXED_VALUE_IF_VERBOSE
      std::cout << indent() << "processing entry: " << entry << std::endl;
#endif
      std::vector<indexed_value_if_entry> local_entries;
      get_entries_for_target_internal(entry.get_name_ref(), local_entries);
      bool local_entry_added = false;
      for (auto& e : local_entries) {
#if INDEXED_VALUE_IF_VERBOSE
	std::cout << indent() << "processing local local entry " << e << std::endl;
#endif
	if ((e.get_name_ref() == entry.get_name_ref() || e.get_name_ref() == name ||
	     e.get_start()              >= (entry.get_size() + entry.get_offset())) ||
	    (e.get_size()+e.get_start() <  (entry.get_offset() + 1))) {
	  // no overlap
#if INDEXED_VALUE_IF_VERBOSE
	  std::cout << indent() << "no overlap" << e << std::endl;
#endif
	} else {
	  local_entry_added = true;
	  unsigned correction = (e.get_start()>e.get_offset()+entry.get_offset()) ? (e.get_start()-e.get_offset()-entry.get_offset()) : 0u;
	  unsigned new_start = entry.get_start()+correction;
	  unsigned new_size = std::min(entry.get_size(), e.get_size()+e.get_start()-entry.get_offset() - correction);
	  unsigned new_offset = e.get_offset() + entry.get_offset() - e.get_start() + correction;
	  entries.emplace_back(e.get_name_ref(), new_start, new_size, new_offset);
#if INDEXED_VALUE_IF_VERBOSE
	  std::cout << indent() << "overlap, adding " << entries.back() << std::endl;
#endif
	}
      }
      if (!local_entry_added && is_not_unmapped_entry(entry, mUnmappedTargets)) {
#if INDEXED_VALUE_IF_VERBOSE
	std::cout << indent() << "entry is not unmapped target, so adding" << std::endl;
#endif
	entries.insert(entries.end(), entry);
      } else {
#if INDEXED_VALUE_IF_VERBOSE
	std::cout << indent() << "entry is unmapped target, so NOT adding" << std::endl;
#endif
      }
    }
    filter_duplicates_and_contained_entries(entries);

#if INDEXED_VALUE_IF_VERBOSE
    std::cout << indent() << "resulting entries entries for " << name << std::endl;
    for (auto& e : entries) {
      std::cout << indent() << e << std::endl;
    }
    std::cout << indent() << "end of entries for " << name << std::endl;
    up();
#endif
  }

  bool
  indexed_value_if_registry::
  has_entries_for_target
  (const std::string& name)
  {
    auto dot = name.rfind('.');
    const auto parent_name_size = dot == std::string::npos ? name.size() : dot;
    for (auto providerp : mProviders) {
      std::vector<indexed_value_if_entry> entries;
      if (providerp->has_entries(name)) {
	providerp->get_entries(name, entries);
	for (auto& e : entries) {
          const std::string& entry_parent_name = e.get_name_ref();
          dot = entry_parent_name.rfind('.');
          const auto entry_parent_name_size = dot == std::string::npos ? entry_parent_name.size() : dot;
          if (parent_name_size != entry_parent_name_size ||
              memcmp(name.data(), entry_parent_name.data(), parent_name_size) != 0) {
            return true;
          }
        }
      }
    }
    return false;
  }

  void
  indexed_value_if_registry::
  indexed_value_if_provider_state_changed()
  {
    mTargetEntriesCache.clear();
    auto observersCopy = mObservers;
    for (auto o : observersCopy) {
      o->handle_state_changed();
    }
  }
  
  std::string get_parent_name(const std::string& input) {
    return input.substr(0, input.rfind('.'));
  }

  void
  indexed_value_if_registry::
  get_entries_for_target_internal
  (const std::string& name, std::vector<indexed_value_if_entry>& entries)
  {
    auto cachedEntryPair = mTargetEntriesCache.find(name);
    if (cachedEntryPair == mTargetEntriesCache.end()) {
      cachedEntryPair = mTargetEntriesCache.insert({name, {}}).first;
      get_entries_for_target_slow(name, cachedEntryPair->second);
    }
    entries.insert(entries.end(), cachedEntryPair->second.begin(), cachedEntryPair->second.end());
  }

  bool entry_not_relevant_for_target(const indexed_value_if_entry& entry, const std::string& target_name) {
    std::string entry_name = entry.get_name_ref();
    std::string entry_parent_name = get_parent_name(entry_name);
    std::string::size_type pos = target_name.find(entry_parent_name + ".");
    bool target_is_in_entry_parent = pos == 0;
    bool target_parent_is_entry_parent = target_name.find('.', entry_parent_name.size()+2) == std::string::npos;
    if (target_parent_is_entry_parent) {
      bool target_and_entry_are_synonym = (entry_name.find(target_name + "[") == 0) || (target_name.find(entry_name + "[") == 0);
      return target_and_entry_are_synonym;
    }
    return target_is_in_entry_parent;
  }

  void
  indexed_value_if_registry::
  get_entries_for_target
  (const std::string& name, std::vector<indexed_value_if_entry>& entries)
  {
    std::vector<indexed_value_if_entry> tmpentries;
    get_entries_for_target_internal(name, tmpentries);
    for (auto& e : tmpentries) {
      if (!entry_not_relevant_for_target(e, name)) {
	entries.push_back(e);
      } else {
#if INDEXED_VALUE_IF_VERBOSE
	std::cout << "!!! filtering entry " << e << " for name " << name << std::endl;
#endif
      }
    }
  }

  void
  indexed_value_if_registry::
  register_provider
  (indexed_value_if_provider& provider)
  {
    mProviders.push_back(&provider);
    std::vector<std::string> unmapped_targets;
    provider.get_unmapped_targets(unmapped_targets);
    for (auto& unmapped_target : unmapped_targets) {
      mUnmappedTargets.insert(unmapped_target);
    }
    indexed_value_if_provider_state_changed();
  }

  void
  indexed_value_if_registry::
  unregister_provider
  (indexed_value_if_provider& provider)
  {
    std::vector<std::string> unmapped_targets;
    provider.get_unmapped_targets(unmapped_targets);
    for (auto& unmapped_target : unmapped_targets) {
      mUnmappedTargets.erase(unmapped_target);
    }
    auto posi = std::find(mProviders.begin(), mProviders.end(), &provider);
    assert(posi != mProviders.end());
    if (posi != mProviders.end()) {
      mProviders.erase(posi);
    }
    indexed_value_if_provider_state_changed();
  }
 
  indexed_value_if_registry::
  indexed_value_if_provider::
  indexed_value_if_provider()
  {
  }
  
  indexed_value_if_registry::
  indexed_value_if_provider::
  ~indexed_value_if_provider()
  {
  }

  indexed_value_if_registry::
  observer::
  observer()
  {
  }
  
  indexed_value_if_registry::
  observer::
  ~observer()
  {
  }
  
  void
  indexed_value_if_registry::
  register_observer(observer& o)
  {
    mObservers.push_back(&o);
  }
  
  void
  indexed_value_if_registry::
  unregister_observer(observer& o)
  {
    auto posi = std::find(mObservers.begin(), mObservers.end(), &o);
    assert(posi != mObservers.end());
    if (posi != mObservers.end()) {
      mObservers.erase(posi);
    }    
  }

  bool 
  indexed_value_if_registry::
  is_unmapped_target(const std::string& name) const
  {
    return mUnmappedTargets.count(name) != 0;
  }

}
