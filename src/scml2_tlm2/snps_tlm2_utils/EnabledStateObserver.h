/***************************************************************************
 * Copyright 1996-2025 Synopsys, Inc.
 *
 * This Synopsys software and all associated documentation are proprietary
 * to Synopsys, Inc. and may only be used pursuant to the terms and
 * conditions of a written license agreement with Synopsys, Inc.
 * All other use, reproduction, modification, or distribution of the
 * Synopsys software or the associated documentation is strictly prohibited.
 ***************************************************************************/

#ifndef __OSCI_TLM2_ENABLED_STATE_OBSERVER_H
#define __OSCI_TLM2_ENABLED_STATE_OBSERVER_H

class EnabledStateObserver
{
 public:
  virtual ~EnabledStateObserver() {}
  virtual void enabledStateChanged() = 0;
};

#endif
