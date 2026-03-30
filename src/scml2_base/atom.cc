/***************************************************************************
 * Copyright 1996-2025 Synopsys, Inc.
 *
 * This Synopsys software and all associated documentation are proprietary
 * to Synopsys, Inc. and may only be used pursuant to the terms and
 * conditions of a written license agreement with Synopsys, Inc.
 * All other use, reproduction, modification, or distribution of the
 * Synopsys software or the associated documentation is strictly prohibited.
 ***************************************************************************/

#include <scml2_base/atom.h>
#include <scml2_base/object_registry.h>
#include <scml2_base/object.h>

#ifdef SNPS_SLS_VP_BASE

#include <cassert>

namespace scml2 { namespace base {

    namespace {
      std::string get_full_object_name()
      {
	std::string hier_name = object_registry::get_object_registry().get_current_hierarchical_name();
	sc_core::sc_object* object = sc_core::sc_get_current_object();
	if (object != 0) {
	  return object->name() + ("." + hier_name);
	} else {
	  return hier_name;
	}
      }

      std::string get_full_object_name(const std::string& name)
      {
	object* o = object_registry::get_object_registry().get_current_object();
	if (o != 0) {
	  return o->get_name() + "." + name;
	} else {
	  sc_core::sc_object* sc_o = sc_core::sc_get_current_object();
	  if (sc_o != 0) {
	    return sc_o->name() + ("." + name);
	  } else {
	    return name;
	  }
	}
      }
    }

    atom::~atom()
    {
      if (m_parent != 0) {
	m_parent->remove_child(this);
      }
    }

    atom::atom()
      : named_base(get_full_object_name())
      , m_parent(object_registry::get_object_registry().get_previous_object())
    {
      if (m_parent != 0) {
	m_parent->add_child(this);
      }
    }

    atom::atom(const std::string& name)
      : named_base(get_full_object_name(name))
      , m_parent(object_registry::get_object_registry().get_current_object())
    {
      if (m_parent != 0) {
	m_parent->add_child(this);
      } else {
	std::cerr << "ERROR: scml2::base::atom '" << get_name() << "' is constructed without a valid scml2::base::object in existence. The printed hierarchical name can be wrong due to this. Potentially an object derived from scml2::base::object used a string instead of a 'const scml2::base::object_name&' argument as name." << std::endl;
	assert(false);
      }
    }

    object* atom::get_parent() const
    {
      return m_parent;
    }

  }}

#endif // SNPS_SLS_VP_BASE
