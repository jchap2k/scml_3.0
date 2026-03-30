/***************************************************************************
 * Copyright 1996-2025 Synopsys, Inc.
 *
 * This Synopsys software and all associated documentation are proprietary
 * to Synopsys, Inc. and may only be used pursuant to the terms and
 * conditions of a written license agreement with Synopsys, Inc.
 * All other use, reproduction, modification, or distribution of the
 * Synopsys software or the associated documentation is strictly prohibited.
 ***************************************************************************/

/******************************************************************************
 * Description: This header file contains declaration and defination of base
 *               class for all snps tlm2 extension classes
******************************************************************************/

#ifndef __SNPS_TLM2_EXTENSION_MACROS_H__
#define __SNPS_TLM2_EXTENSION_MACROS_H__


//#include "snps_tlm2_base_extension.h"
//#include "scml2_tlm2/snps_tlm2_utils/snps_tlm2_utils.h"

namespace scml2 {

// On Linux builds, we need to make sure that the template instantiations
// are marked extern so the compiler does not create the associated
// weak symbols. Unfortunately, we cannot also declare the tlm_extension
// instances extern, because of the illegal combination of and extern static
// const member tlm_extension::ID and its initialization through a function
// call that is not constexpr.

#ifdef _MSC_VER
#  define SNPS_VP_API_EXTERN_TEMPLATE
#else
#  define SNPS_VP_API_EXTERN_TEMPLATE extern
#endif

#ifdef _MSC_VER
#  define INSTANTIATE_EXTENSION_TEMPLATE_INSTANCE(ext_name) template class SNPS_VP_API tlm::tlm_extension<scml2:: ext_name>;
#else
#  define INSTANTIATE_EXTENSION_TEMPLATE_INSTANCE(ext_name)
#endif

/*******************************************************************************
 * Name : DECLARE_EXTENSION
 *
 * Description : Macro provides the implmentation of constructor and destructor
 *               All the other functionality is taken care in Base class
 *
 ******************************************************************************/
#define DECLARE_EXTENSION(ext_name, attr_type, defaultVal)                    \
class ext_name:public base_extension<ext_name, attr_type> {                   \
                                                                              \
public:                                                                       \
    ext_name(unsigned int arraySize=0) :                                      \
        base_extension<ext_name, attr_type>(defaultVal, arraySize){           \
  }                                                                           \
};

#define DECLARE_EXTENSION_DECLS(ext_name, attr_type, defaultVal)              \
  class SNPS_VP_API ext_name;                                                 \
}; /* close scml2 namespace */                                                \
INSTANTIATE_EXTENSION_TEMPLATE_INSTANCE(ext_name)                             \
namespace scml2 {                                                             \
  SNPS_VP_API_EXTERN_TEMPLATE                                                 \
  template class SNPS_VP_API base_extension<ext_name, attr_type>;             \
  class SNPS_VP_API ext_name:public base_extension<ext_name, attr_type>{      \
  public:                                                                     \
     ext_name(unsigned int arraySize=0);                                      \
  };

#define DECLARE_EXTENSION_DEFS(ext_name, attr_type, defaultVal)               \
template class base_extension<ext_name, attr_type>;                           \
ext_name::ext_name(unsigned int arraySize) :                                  \
    base_extension<ext_name, attr_type>(defaultVal, arraySize) {              \
}

//response extension for peripherals - Aid in mapping to other FT response type
DECLARE_EXTENSION_DECLS(gp_response_extension, tlm::tlm_response_status, tlm::TLM_INCOMPLETE_RESPONSE);
/*******************************************************************************
 * Name : DECLARE_RESP_EXTENSION
 *
 * Description : Macro provides the implmentation of constructor and destructor
 *               All the other functionality is taken care in Base class
 *
 ******************************************************************************/
/* Extension mapper not supported in OSCI */
#define DECLARE_RESP_EXTENSION(ext_name, attr_type, defaultVal) DECLARE_EXTENSION_DECLS(ext_name, attr_type, defaultVal)

#define DECLARE_RESP_EXTENSION_DECLS(ext_name, attr_type, defaultVal)           \
  class SNPS_VP_API ext_name;                                                   \
}; /* close scml2 namespace */                                                  \
INSTANTIATE_EXTENSION_TEMPLATE_INSTANCE(ext_name)                               \
namespace scml2 {                                                               \
  template<> SNPS_VP_API void base_extension<ext_name, attr_type>::             \
  set_attribute (tlm::tlm_generic_payload* payload,                             \
                 const attr_type &val);                                                \
  /* Linux: declaring the specialized base_class as exported (default visiblity)\
     does not work here, it collides with the specialized set_attribute         \
     method. */                                                                 \
  /* SNPS_VP_API_EXTERN_TEMPLATE                                                \
     template class SNPS_VP_API base_extension<ext_name, attr_type>; */         \
  class SNPS_VP_API ext_name:public base_extension<ext_name, attr_type>{        \
  public:                                                                       \
     ext_name(unsigned int arraySize=0);                                        \
  };



/*******************************************************************************
 * Name : DECLARE_ARRAY_EXTENSION
 *
 * Description : Macro defination for declaring an array base extension class
 *                  Constructor allocates memory for the array based on the size
 *                  passed in constructor of the extension
 *                  It also implements the specialization of clone member function
 *                  of template base class.
 *
 ******************************************************************************/

#define DECLARE_ARRAY_EXTENSION(ext_name, attr_type)                            \
class ext_name:public base_array_extension<ext_name, attr_type>{                \
public:                                                                         \
 ext_name(unsigned int arraySize) :                                             \
        base_array_extension(arraySize) {                                       \
    }                                                                           \
};

#define DECLARE_ARRAY_EXTENSION_DECLS(ext_name, attr_type)                      \
  class SNPS_VP_API ext_name;                                                   \
}; /* close scml2 namespace */                                                  \
INSTANTIATE_EXTENSION_TEMPLATE_INSTANCE(ext_name)                               \
namespace scml2 {                                                               \
  SNPS_VP_API_EXTERN_TEMPLATE                                                   \
  template class SNPS_VP_API base_extension<ext_name, attr_type*>;              \
  SNPS_VP_API_EXTERN_TEMPLATE                                                   \
  template class SNPS_VP_API base_array_extension<ext_name, attr_type>;         \
  class SNPS_VP_API ext_name:public base_array_extension<ext_name, attr_type>{  \
  public:                                                                       \
     ext_name(unsigned int arraySize);                                          \
  };

#define DECLARE_ARRAY_EXTENSION_DEFS(ext_name, attr_type)                       \
template class base_extension<ext_name, attr_type*>;                            \
template class base_array_extension<ext_name, attr_type>;                       \
ext_name::ext_name(unsigned int arraySize) :                                    \
    base_array_extension(arraySize) {                                           \
}

#define REGISTER_PROTO_CHNL_INFO(ext_name)
#define DECLARE_PROTO_STATE_EXT(ext_name, attr_type, defVal )                   \
class ext_name : public base_extension<ext_name, attr_type> {                   \
   public :                                                                     \
       ext_name(unsigned int arraySz = 0) :                                     \
                    base_extension<ext_name, attr_type>(defVal, arraySz)        \
       {                                                                        \
            REGISTER_PROTO_CHNL_INFO(ext_name)                                  \
       }                                                                        \
                                                                                \
      void set_attribute (tlm::tlm_generic_payload* payload,                    \
                unsigned int val)                                               \
         {                                                                      \
            base_extension<ext_name, attr_type>::set_attribute(payload, (attr_type)val);    \
         }                                                                      \
};

#define DECLARE_PROTO_STATE_EXT_DECLS(ext_name, attr_type, defVal )             \
  class SNPS_VP_API ext_name;                                                   \
}; /* close scml2 namespace */                                                  \
INSTANTIATE_EXTENSION_TEMPLATE_INSTANCE(ext_name)                               \
namespace scml2 {                                                               \
  SNPS_VP_API_EXTERN_TEMPLATE                                                   \
  template class SNPS_VP_API base_extension<ext_name, attr_type>;               \
  class SNPS_VP_API ext_name : public base_extension<ext_name, attr_type> {     \
  public:                                                                       \
    ext_name(unsigned int arraySize=0);                                         \
    void set_attribute (tlm::tlm_generic_payload* payload, unsigned int val);   \
  };

#define DECLARE_PROTO_STATE_EXT_DEFS(ext_name, attr_type, defVal )              \
template class base_extension<ext_name, attr_type>;                             \
ext_name::ext_name(unsigned int arraySz) :                                      \
                   base_extension<ext_name, attr_type>(defVal, arraySz)         \
{                                                                               \
    REGISTER_PROTO_CHNL_INFO(ext_name)                                          \
}                                                                               \
                                                                                \
void ext_name::set_attribute (tlm::tlm_generic_payload* payload,                \
                              unsigned int val)                                 \
{                                                                               \
    base_extension<ext_name, attr_type>::set_attribute(payload, (attr_type)val);\
}


//Declare the GP Proto State Ext -- This is just a dummy extension
DECLARE_PROTO_STATE_EXT_DECLS(gp_protocol_state_extension, unsigned int, 0);

//Dummy extension to Aid in mapping GP Burst type to FT Burst Type
DECLARE_EXTENSION_DECLS(gp_burst_type_extension, unsigned int, 0);
} /* namespace scml */
#endif /* __base_extension_H__  */
