// -*- C++ -*-
/***************************************************************************
 * Copyright 1996-2025 Synopsys, Inc.
 *
 * This Synopsys software and all associated documentation are proprietary
 * to Synopsys, Inc. and may only be used pursuant to the terms and
 * conditions of a written license agreement with Synopsys, Inc.
 * All other use, reproduction, modification, or distribution of the
 * Synopsys software or the associated documentation is strictly prohibited.
 ***************************************************************************/

#include <scml2_tlm2/snps_tlm2_extensions/snps_tlm2_base_extension.h>
#include <map>
#include <typeindex>

namespace scml2 {
  // Motivation for the following map: DLLs on Windows do not provide
  // correct support for static variables in class/function
  // templates. There is no single instance, but one per DLL. Therefore we
  // have to load the value of a static variables from a global
  // variable. We use a map that is indexed by
  // type_index(template_type).
  typedef std::map<std::type_index, tlm::tlm_mm_interface*> tExtensionType2MM;
  tExtensionType2MM* g_extension_type_to_mm = 0;
}

tlm::tlm_mm_interface* scml2::find_global_mm(const std::type_info& info) {
  if (g_extension_type_to_mm == 0) {
    return 0;
  }
  tExtensionType2MM::iterator ptr = g_extension_type_to_mm->find(std::type_index(info));
  if (ptr == g_extension_type_to_mm->end()) {
    return 0;
  }
  return ptr->second;
}

void scml2::set_global_mm(const std::type_info& info, tlm::tlm_mm_interface* mm) {
  if (g_extension_type_to_mm == 0) {
    g_extension_type_to_mm = new tExtensionType2MM;
  }
  std::pair<tExtensionType2MM::iterator, bool> p = g_extension_type_to_mm->emplace(std::type_index(info), mm);
  assert(p.second); (void)p;
}
