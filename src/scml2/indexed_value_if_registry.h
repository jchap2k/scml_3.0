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

#include <scml2_logging/snps_vp_dll.h>

#include <vector>
#include <string>
#include <map>
#include <set>

namespace scml2 {

  class SNPS_VP_API indexed_value_if_entry {
    std::string mName;
    unsigned mStart;
    unsigned mSize;
    unsigned mOffset;
  public:
    indexed_value_if_entry(const std::string& name, unsigned start, unsigned size, unsigned offset);

    // NOTE: 'get_name' only kept for binary compatibility on windows, where it
    //       was exported as part of the public API and thus is present in the
    //       DLL. User code may have chosen not to inline the function and thus
    //       relies on it being there (e.g. indexed_value_decoder).
    //       https://devblogs.microsoft.com/oldnewthing/20140109-00/?p=2123
    std::string get_name() const { return mName; }
    std::string const& get_name_ref() const { return mName; }
    unsigned get_start() const { return mStart; }
    unsigned get_size() const { return mSize; }
    unsigned get_offset() const { return mOffset; }

    bool operator<(const scml2::indexed_value_if_entry& rhs) const;
    bool operator==(const scml2::indexed_value_if_entry& rhs) const;
  };

  class SNPS_VP_API indexed_value_if_registry {
    static indexed_value_if_registry sInstance;
    indexed_value_if_registry();

    std::map<std::string, std::vector<indexed_value_if_entry> > mTargetEntriesCache;
    void get_entries_for_target_slow(const std::string& name, std::vector<indexed_value_if_entry>& entries);
    void get_entries_for_target_internal(const std::string& name, std::vector<indexed_value_if_entry>& entries);
public:
    static indexed_value_if_registry& get_instance();

    void indexed_value_if_provider_state_changed();
    
    bool has_entries_for_target(const std::string& name);
    void get_entries_for_target(const std::string& name, std::vector<indexed_value_if_entry>& entries);
    void get_forwardings_for_target(const std::string& name, std::vector<std::string>& entries);
    
    class SNPS_VP_API indexed_value_if_provider {
    public:
      indexed_value_if_provider();
      virtual ~indexed_value_if_provider();

      virtual bool has_entries(const std::string& name) = 0;
      virtual void get_entries(const std::string& name, std::vector<indexed_value_if_entry>& entries) = 0;

      virtual bool has_forwardings(const std::string& name) = 0;
      virtual void get_forwardings(const std::string& name, std::vector<std::string>& entries) = 0;      

      virtual void get_unmapped_targets(std::vector<std::string>& targets) = 0;
    };

    void register_provider(indexed_value_if_provider& provider);
    void unregister_provider(indexed_value_if_provider& provider);

    class SNPS_VP_API observer {
    public:
      observer();
      virtual ~observer();
      virtual void handle_state_changed() = 0; // observers are allowed to unregister themselves from inside this call
    };

    void register_observer(observer& o); 
    void unregister_observer(observer& o); 

    bool is_unmapped_target(const std::string& name) const;
  private:
    std::vector<indexed_value_if_provider*> mProviders;
    std::vector<observer*> mObservers;
    std::set<std::string> mUnmappedTargets;
  };
  
}
