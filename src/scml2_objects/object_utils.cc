/***************************************************************************
 * Copyright 1996-2025 Synopsys, Inc.
 *
 * This Synopsys software and all associated documentation are proprietary
 * to Synopsys, Inc. and may only be used pursuant to the terms and
 * conditions of a written license agreement with Synopsys, Inc.
 * All other use, reproduction, modification, or distribution of the
 * Synopsys software or the associated documentation is strictly prohibited.
 ***************************************************************************/

#include "scml2_objects/object_utils.h"


namespace scml2 { 
namespace objects { 


// -------------------------------------------------------------------------------------
// getParentModule
//
// walks object hierarchy until it reaches an sc_module, assumed to be the parent module of the current object
// -------------------------------------------------------------------------------------
sc_core::sc_module* getParentModule(sc_core::sc_object* current) {
	sc_core::sc_module* module = nullptr;
	while (current && !(module = dynamic_cast<sc_core::sc_module*>(current))) {
		current = current->get_parent_object();
	}
	return module;
}


// -------------------------------------------------------------------------------------
// getBaseName
//
// returns the leaf name of an scml object
// -------------------------------------------------------------------------------------
std::string getBaseName(scml2::base::object* _obj) {
	// split line in tokens
	std::stringstream sline(_obj->get_name());
	std::string basename;
	while(std::getline(sline, basename, '.')); // last token is basename; so keep while loop running till the end
	return basename;
}

std::string getBaseName(sc_core::sc_object* _obj) {
	return std::string(_obj->basename());
}


// namespaces scml2 and objects
}
}
