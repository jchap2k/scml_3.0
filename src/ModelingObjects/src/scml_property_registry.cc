/***************************************************************************
 * Copyright 1996-2025 Synopsys, Inc.
 *
 * This Synopsys software and all associated documentation are proprietary
 * to Synopsys, Inc. and may only be used pursuant to the terms and
 * conditions of a written license agreement with Synopsys, Inc.
 * All other use, reproduction, modification, or distribution of the
 * Synopsys software or the associated documentation is strictly prohibited.
 ***************************************************************************/
/*
 * Modifications :
 *
 * Purpose of file : Implementation file for scml_property_registry
 *
 */

#include "scml_property_registry.h"
#include "scml_property_server_if.h"
#include "scmlinc/scml_inmemory_property_server.h"

#include <sys/stat.h>
#include <cassert>
#include <ctype.h>
#include <stdlib.h>
#include <string.h>


namespace {
bool _ParseParams(const char* src, std::string& to, int limit = 100 )
{
  std::string toFind;
  bool changed = false;
  const char* from = src;

  if(limit <= 0) {
    std::cerr << "Recursion limit reached while evaluating <" << from << ">" << std::endl;
	abort();
  }
  while(*from) {
    if(from[0] == '$' && from[1] == '(')  { // if it looks like a parameter, then copy the param
      { // copy the param name to toFind
        toFind.clear();
        from += 2;
        while(*from && *from != ')') toFind += *from++; // copy the param name
        from++; // skip the ')'
      }
      
      const char * env_val = getenv(toFind.c_str());
      
      if(!env_val) {
        // Note that persist.h, GetInitVal() will return NULL if it sees an unmatched parameter.
        from -= toFind.length() + 3; // move back to '$'
        to += *from++; // just copy the unmatched parameter
      } else {
        //Check for recursive reference foo=$(foo)abc
        const char *hit = strstr(env_val, toFind.c_str());
        size_t v_len = strlen(toFind.c_str());
        if( hit && (hit > (env_val+1)) && (hit[-2] == '$') && (hit[-1] == '(') &&
            (hit[v_len]== ')') ) {
                std::cerr << "Infinite recursion detected while evaluating <" << src << ">" << std::endl
                    << "Variable <" << toFind << "> refers to itself" << std::endl;
                abort();
        }
        // now substitute the param reference for the value
        const char* sz = env_val;
        while(*sz) { to += *sz++;}
        changed = true;
      }
    } else {
      to += *from++;
    }
  }
  if(changed) {
    std::string tmp_out;
    _ParseParams(to.c_str(), tmp_out, limit - 1);
    to = tmp_out;
    return true;
  }
  return false;
}

bool replaceEnvironmentRefs(const std::string& in, std::string &out) {
    return _ParseParams(in.c_str(), out);
}
} //namespace


using std::string;
using std::cerr;
using std::endl;

static const std::string TAG_PARAMETERS                 ("parameters");
static const std::string TAG_PARAMETER                  ("parameter");
static const std::string TAG_CONSTRUCTOR_ARGUMENTS      ("constructor_arguments");
static const std::string TAG_PROTOCOL_PARAMETERS        ("protocol_parameters");
static const std::string TAG_MODULES                    ("modules");
static const std::string TAG_MODULE                     ("module");
static const std::string TAG_PORTS                      ("ports");
static const std::string TAG_PORT                       ("port");
static const std::string TAG_NAME                       ("name");
static const std::string TAG_VALUE                      ("value");

static const std::string TAG_PORT_ARRAYS                ("port_arrays");
static const std::string TAG_PORT_ARRAY                 ("port_array");

static const std::string STR_INTEGER                    ("integer");
static const std::string STR_DOUBLE                     ("double");
static const std::string STR_BOOL                       ("bool");
static const std::string STR_STRING                     ("string");
static const std::string STR_ONE                        ("1");
static const std::string STR_ZERO                       ("0");
static const std::string STR_TRUE                       ("true");
static const std::string STR_FALSE                      ("false");
static const std::string STR_EMPTY;


scml_property_registry& scml_property_registry::inst() 
{
  static scml_property_registry instance;
  return instance;
}

scml_property_registry::scml_property_registry() : 
  mOs( &cerr), 
  mCustomPropServer( 0)
{
  mSetProperties = new scml_inmemory_property_server();
}

scml_property_registry::~scml_property_registry()
{
  delete mSetProperties;
}

bool scml_property_registry::setCustomPropertyServer( scml_property_server_if * p )
{
  mCustomPropServer = p;
  return (0 != mCustomPropServer);
}

scml_property_server_if * 
scml_property_registry::getCustomPropertyServer() const
{
  return mCustomPropServer;
}


static string constructCompleteName(const string& moduleName, const string& name) {
  string completeName;
  completeName.reserve(moduleName.size() + 1 + name.size()); // performance optimization
  completeName.append(moduleName);
  completeName.push_back('.');
  completeName.append(name);
  return completeName;
}

long long scml_property_registry::getIntProperty(PropertyType type, const string& moduleName, const string& name)
{
  (void) type;
  string completeName = constructCompleteName(moduleName, name);
  if (mSetProperties->hasIntProperty(completeName)) {
    return mSetProperties->getIntProperty(completeName);
  }
  if( 0 != mCustomPropServer) {
    if (mCustomPropServer->hasIntProperty(type, completeName) || mCustomPropServer->isFinal()) {
      return mCustomPropServer->getIntProperty(type, completeName);
    }
  } 
  return 0;
}
		   
unsigned long long scml_property_registry::getUIntProperty(PropertyType type, const string& moduleName, const string& name)
{
  (void) type;
  string completeName = constructCompleteName(moduleName, name);
  if (mSetProperties->hasUIntProperty(completeName)) {
    return mSetProperties->getUIntProperty(completeName);
  }
  if( 0 != mCustomPropServer) {
    if (mCustomPropServer->hasUIntProperty(type, completeName) || mCustomPropServer->isFinal()) {
      return mCustomPropServer->getUIntProperty(type, completeName);
    }
  }
  return 0;
}

bool scml_property_registry::getBoolProperty(PropertyType type, const string& moduleName, const string& name)
{
  (void) type;
  string completeName = constructCompleteName(moduleName, name);
  if (mSetProperties->hasBoolProperty(completeName)) {
    return mSetProperties->getBoolProperty(completeName);
  }
  if( 0 != mCustomPropServer) {
    if (mCustomPropServer->hasBoolProperty(type, completeName) || mCustomPropServer->isFinal()) {
      return mCustomPropServer->getBoolProperty(type, completeName);
    }
  } 
  return false;
}
				       
string scml_property_registry::getStringProperty(PropertyType type, const string& moduleName, const string& name)
{
  std::string tmpresult;
  (void) type;
  string completeName = constructCompleteName(moduleName, name);
  if (mSetProperties->hasStringProperty(completeName)) {
    tmpresult = mSetProperties->getStringProperty(completeName);
  } else if( (0 != mCustomPropServer) && (mCustomPropServer->hasStringProperty(type, completeName) || mCustomPropServer->isFinal())) {
    tmpresult = mCustomPropServer->getStringProperty(type, completeName);
  } else {
    tmpresult = "";
  }
  std::string result;
  replaceEnvironmentRefs(tmpresult, result);
  return result;
}
					   
double scml_property_registry::getDoubleProperty(PropertyType type, const string& moduleName, const string& name)
{
  (void) type;
  string completeName = constructCompleteName(moduleName, name);
  if (mSetProperties->hasDoubleProperty(completeName)) {
    return mSetProperties->getDoubleProperty(completeName);
  }
  if( 0 != mCustomPropServer) {
    if (mCustomPropServer->hasDoubleProperty(type, completeName) || mCustomPropServer->isFinal()) {
      return mCustomPropServer->getDoubleProperty(type, completeName);
    }
  }
  return 0.0;
}

bool scml_property_registry::hasProperty(PropertyType type, PropertyCType ctype, const std::string& scHierName, const std::string& name)
{
  (void) type;
  string completeName = constructCompleteName(scHierName, name);

  switch (ctype) {
    case SIGNED_INTEGER:
      if (mSetProperties->hasIntProperty(completeName)) {
        return true;
      }
      break;
    case UNSIGNED_INTEGER:
      if (mSetProperties->hasUIntProperty(completeName)) {
        return true;
      }
      break;
    case INTEGER:
      if (mSetProperties->hasIntProperty(completeName) || mSetProperties->hasUIntProperty(completeName)) {
        return true;
      }
      break;
    case STRING:
      if (mSetProperties->hasStringProperty(completeName)) {
        return true;
      }
      break;
    case BOOL:
      if (mSetProperties->hasBoolProperty(completeName)) {
        return true;
      }
      break;
    case DOUBLE:
      if (mSetProperties->hasDoubleProperty(completeName)) {
        return true;
      }
      break;
    default:
      assert(0);
  }

  if( 0 != mCustomPropServer) {
    switch (ctype) {
      case SIGNED_INTEGER:
        if (mCustomPropServer->hasIntProperty(type, completeName)) {
          return true;
        }
        break;
      case UNSIGNED_INTEGER:
        if (mCustomPropServer->hasUIntProperty(type, completeName)) {
          return true;
        }
        break;
      case INTEGER:
        if (mCustomPropServer->hasIntProperty(type, completeName) || mCustomPropServer->hasUIntProperty(type, completeName)) {
          return true;
        }
        break;
      case STRING:
        if (mCustomPropServer->hasStringProperty(type, completeName)) {
          return true;
        }
        break;
      case BOOL:
        if (mCustomPropServer->hasBoolProperty(type, completeName)) {
          return true;
        }
        break;
      case DOUBLE:
        if (mCustomPropServer->hasDoubleProperty(type, completeName)) {
          return true;
        }
        break;
      default:
        assert(0);
    }
    if (mCustomPropServer->isFinal()) {
      return false;
    }
  }
  return false;
}


void scml_property_registry::setOutputStream(::std::ostream* output)
{
  mOs = output;
}

std::string scml_property_registry::getPropertyXmlFilePath() const
{
  return mXmlFilePath;
}  

void scml_property_registry::setXMLFile(const string& )
{
}

void scml_property_registry::setIntProperty(PropertyType /* type */, const std::string& scHierName, const std::string& name, long long value) {
  mSetProperties->setProperty(std::string( scHierName + '.' + name), value);
}

void scml_property_registry::setUIntProperty(PropertyType /* type */, const std::string& scHierName, const std::string& name, unsigned long long value) {
  mSetProperties->setProperty(std::string( scHierName + '.' + name), value);
}

void scml_property_registry::setBoolProperty(PropertyType /* type */, const std::string& scHierName, const std::string& name, bool value) {
  mSetProperties->setProperty(std::string( scHierName + '.' + name), value);
}

void scml_property_registry::setStringProperty(PropertyType /* type */, const std::string& scHierName, const std::string& name, const std::string& value) {
  mSetProperties->setProperty(std::string( scHierName + '.' + name), value);
}

void scml_property_registry::setDoubleProperty(PropertyType /* type */, const std::string& scHierName, const std::string& name, double value) {
  mSetProperties->setProperty(std::string( scHierName + '.' + name), value);
}

std::vector<std::string> scml_property_registry::getPropertyNames(const std::string& scHierName) const
{
  std::vector<std::string> result;
  (void) scHierName;

  return result;
}

scml_property_registry::PropertyCType scml_property_registry::getPropertyType( const std::string& scHierName, const std::string& name) const
{
  (void) scHierName;
  (void) name;

  return BOOL;
}

void scml_property_registry::printPropertiesFor(const std::string& scHierName) 
{
  std::vector<std::string>  prop_names = getPropertyNames(scHierName);
  std::cout << "Property report for '" << scHierName << "'" << std::endl;
  for (auto& prop_name : prop_names) {
    std::cout << " " << prop_name << " = ";
    switch (getPropertyType(scHierName, prop_name)) {
    case BOOL:
      std::cout << getBoolProperty(MODULE, scHierName, prop_name);
      break;
    case INTEGER:
    case SIGNED_INTEGER:
    case UNSIGNED_INTEGER:
      std::cout << getIntProperty(MODULE, scHierName, prop_name);
      break;
    case STRING:
      std::cout << getStringProperty(MODULE, scHierName, prop_name);
      break;
    case DOUBLE:
      std::cout << getDoubleProperty(MODULE, scHierName, prop_name);
      break;
    }
    std::cout << std::endl;
  }
}
