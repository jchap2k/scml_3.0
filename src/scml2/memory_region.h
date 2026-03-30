/***************************************************************************
 * Copyright 1996-2025 Synopsys, Inc.
 *
 * This Synopsys software and all associated documentation are proprietary
 * to Synopsys, Inc. and may only be used pursuant to the terms and
 * conditions of a written license agreement with Synopsys, Inc.
 * All other use, reproduction, modification, or distribution of the
 * Synopsys software or the associated documentation is strictly prohibited.
 ***************************************************************************/
#ifndef SCML2_MEMORY_REGION_H
#define SCML2_MEMORY_REGION_H

#include <scml2/callback_observers.h>
#include <scml2/utils.h>
#include <scml2_logging/snps_vp_dll.h>
#include <scml2/tagged_message_handler_provider.h>

#include <scml2_logging/stream.h>
#include <systemc>
#include <string>

namespace scml2 {

class memory_content_observer;
class callback_observer;
class streams;

class SNPS_VP_API memory_content_observers_api_owner
{
public:
  memory_content_observers_api_owner() {}
  virtual ~memory_content_observers_api_owner() {}

  virtual bool content_is_observable() const = 0;
  virtual bool register_content_observer(memory_content_observer* o) = 0;
  virtual bool unregister_all_content_observers() = 0;
  virtual bool unregister_content_observer(memory_content_observer* o) = 0;
};

class SNPS_VP_API callback_observers_api_owner
{
public:
  callback_observers_api_owner() {}
  virtual ~callback_observers_api_owner() {}

  virtual bool register_callback_observer(callback_observer* o) = 0;
  virtual bool unregister_all_callback_observers() = 0;
  virtual bool unregister_callback_observer(callback_observer* o) = 0;
};

class SNPS_VP_API memory_dmi_api_owner
{
public:
  memory_dmi_api_owner() {}
  virtual ~memory_dmi_api_owner() {}

  virtual void enable_dmi() = 0;
  virtual void disable_dmi() = 0;
  virtual bool is_dmi_enabled() const = 0;
};

class SNPS_VP_API memory_description_api_owner
{
public:
  memory_description_api_owner() {}
  virtual ~memory_description_api_owner() {}

  virtual void set_description(const std::string& d) = 0;
  virtual const std::string& get_description() const = 0;
};

class SNPS_VP_API memory_region_api_owner
{
public:
  memory_region_api_owner() {}
  virtual ~memory_region_api_owner() {}

  virtual unsigned long long get_offset() const = 0;
  virtual unsigned long long get_size() const = 0;
  virtual unsigned int get_width() const = 0;
 
};

class SNPS_VP_API memory_region 
  : public sc_core::sc_object
  , public tagged_message_handler_provider
  , public memory_content_observers_api_owner
  , public callback_observers_api_owner
  , public memory_dmi_api_owner
  , public memory_description_api_owner
  , public memory_region_api_owner
{
protected:
  memory_region(const std::string& name,
                unsigned long long offset,
                unsigned long long size,
                unsigned int width);

public:
  virtual ~memory_region();

  std::string get_name() const;
  unsigned long long get_offset() const override;
  unsigned long long get_size() const override;
  unsigned int get_width() const override;

  virtual void set_description(const std::string& d) override;
  virtual const std::string& get_description() const override;

  virtual unsigned int read(unsigned long long address,
                            unsigned char* data,
                            unsigned int size);
  virtual unsigned int write(unsigned long long address,
                             const unsigned char* data,
                             unsigned int size);

  virtual bool content_is_observable() const override;
  virtual bool register_content_observer(memory_content_observer* o) override;
  virtual bool unregister_all_content_observers() override;
  virtual bool unregister_content_observer(memory_content_observer* o) override;

  virtual bool register_callback_observer(callback_observer* o) override;
  virtual bool unregister_all_callback_observers() override;
  virtual bool unregister_callback_observer(callback_observer* o) override;

  virtual void enable_dmi() override;
  virtual void disable_dmi() override;
  virtual bool is_dmi_enabled() const override;

  streams& get_streams() const;

  void set_description(const char *d);

protected:
  void init_streams(streams& s);

private:
  streams* mStreams;
  const unsigned long long mOffset;
  const unsigned long long mSize;
  const unsigned int mWidth;
  std::string mDescription;

protected:
  callback_observers mCallbackObservers;

private:
  // disabled
  memory_region();
  memory_region(const memory_region&);
  memory_region& operator= (const memory_region&);
};

inline
std::string
memory_region::
get_name() const
{
  return name();
}

inline
unsigned long long
memory_region::
get_offset() const
{
  return mOffset;
}

inline
unsigned long long
memory_region::
get_size() const
{
  return mSize;
}

inline
unsigned int
memory_region::
get_width() const
{
  return mWidth;
}

inline
streams&
memory_region::
get_streams() const
{
  assert(mStreams);
  return *mStreams;
}

}

#endif
