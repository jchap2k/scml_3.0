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

#ifndef __SNPS_TLM2_BASE_EXTENSION_H__
#define __SNPS_TLM2_BASE_EXTENSION_H__

#include <scml2_tlm2/snps_tlm2_utils/snps_tlm2_mm.h>
#  ifndef SNPS_VP_API
#    define SNPS_VP_API
#    define SNPS_VP_API_TMPL_FUNC(x)
#    define SNPS_VP_API_TMPL_CLASS(x)
#  endif

namespace scml2 {

class port_adaptor_impl;

class SNPS_VP_API base_extension_if
{
  public:
   virtual void* get_attribute_generic ()=0;
};

template <typename ext_type> mem_manager<ext_type>* get_global_mm();

/******************************************************************************
* Name: base_extension
*
* Description: Base class for snps_tlm2 extension classes. This is templated
*              over extension type and attribute type of the extension
*              The constructor of this Base class, takes the default value of
*              each extension. This default value is stored in member variable
*              and can be retrieved using function "get_default_val"
******************************************************************************/
template <typename extension_type, typename attr_type>
class base_extension : public tlm::tlm_extension< extension_type >, public base_extension_if
{
 public :
    friend class scml2::port_adaptor_impl;

    /*******************************************************************************
     * Constructor of base_extension class
     ******************************************************************************/
    base_extension(const attr_type &defaultVal, unsigned int arraySize=0)
    : attr_val       (defaultVal)
    , mArraySize     (arraySize)
    , mDefaultVal    (defaultVal)
    , my_mm (NULL)
    , mValChanged    (false)
    { }

    /*******************************************************************************
     * Name: free() -> Calls Memory manager's free_obj() method to free the ext.
     ******************************************************************************/
    void        free()
    {
        if( my_mm == NULL) {
            tlm::tlm_extension<extension_type>::free();
            return;
        }
        if(mArraySize == 0) {
            attr_val = mDefaultVal;
        }
        my_mm->free_obj(reinterpret_cast<extension_type*>(this));
    }

    /*******************************************************************************
     * Name: set_attribute
     *
     * Description: Sets the attribute of extension
     ******************************************************************************/
    void        set_attribute(tlm::tlm_generic_payload* payload,
                              const attr_type                & val)
    {
        (void)payload;
        /* If memory is already allocated for array extension
        ** then we should try to reuse that memory
        ** So, returning if mArraySize is non zero
        */
        if ((attr_val == val) || mArraySize) {
            return;
        }

        attr_val = val;
        mValChanged = true;   //flag for tracking value update
    }

    /*******************************************************************************
     * Name: get_attribute
     *
     * Description: Calls the appropriate convert functions according to the state of
     *              extension and returns the attribute value after conversion.
     ******************************************************************************/
    attr_type   get_attribute(tlm::tlm_generic_payload* payload)
    {
        return get_attr_by_ref(payload);
    }

    /*******************************************************************************
     * Name: get_attr_by_ref
     *Description: Calls the appropriate convert functions according to the state of
     *              extension and returns the refrence to attribute value after conversion.
     *******************************************************************************/
    attr_type&  get_attr_by_ref(tlm::tlm_generic_payload* payload)
    {
        (void)payload;
        return attr_val;
    }

    /*******************************************************************************
     * Name: get_default_val() : Returns the default value of the extension
     ******************************************************************************/

    attr_type   get_default_val() {
        return mDefaultVal;
    }


    /*******************************************************************************
     * Name: getValChanged: Returns the value of mValChanged
     ******************************************************************************/
    bool getValChanged()
    {
        return mValChanged;
    }

    /*******************************************************************************
     * Name: setValChanged(): Sets the value of mValChanged attribute
     ******************************************************************************/
    void setValChanged(bool changed)
    {
        mValChanged = changed;
    }


 public :
    virtual void* get_attribute_generic(){
      return &(this->attr_val);
    }

    //Sets the memory manager for the extension
    void set_mm(mem_manager<extension_type>* mm)
    {
        my_mm = mm;
    }

 public :     //virtual functions of tlm_extension_base class

    /*******************************************************************************
     * Name: clone(): Implementation of pure virtual function clone
     *******************************************************************************/
    tlm::tlm_extension_base* clone () const
    {
        mem_manager<extension_type>* mm = const_cast<mem_manager<extension_type>* >((extension_type::get_mm()));
        if(!mm){
            std::cerr<<"Memory Manager for extension type "<<
                    typeid(extension_type).name()<<" does not exist \n";
            std::cerr<<"Add Memory manager in payload_ext_converter::tlmExtMemManager \n";
            exit(1);
        }

        //Invoke memory manager to get pointer to extension instance
        extension_type *t      = mm->claim_obj(0);
        t->attr_val            = this->attr_val;
        return t;
    }

    /*******************************************************************************
     * Name: copy_from(): Implementation of pure virtual function copy_from
     ******************************************************************************/
    void copy_from (const tlm::tlm_extension_base &ext)
    {
        attr_val = static_cast< extension_type const &>(ext).attr_val;
    }

  public:  //Data members
    attr_type attr_val;   //store the actual value

    static const mem_manager<extension_type> *get_mm() {
        static const mem_manager<extension_type>* mm = get_global_mm<extension_type>();
        return mm;
    }

 protected:
    unsigned int mArraySize;

 private:  //Data members
    attr_type mDefaultVal;
    mem_manager< extension_type>*     my_mm;
    bool mValChanged;
};

SNPS_VP_API void set_global_mm(const std::type_info& ext_type_info, tlm::tlm_mm_interface*);

SNPS_VP_API tlm::tlm_mm_interface* find_global_mm(const std::type_info& ext_type_info);

template <typename ext_type>
mem_manager<ext_type>* get_global_mm()
{
    const std::type_info& e_info = typeid(ext_type);
    mem_manager<ext_type>* mm = 0;
    tlm::tlm_mm_interface* mm_base = find_global_mm(e_info);
    if (mm_base != 0) {
        mm = dynamic_cast<mem_manager<ext_type>*>(mm_base);
    } else {
        mm = new mem_manager<ext_type>;
        set_global_mm(e_info, mm);
    }
    return mm;
}

template <typename ext_type, typename attr_type>
class base_array_extension : public base_extension<ext_type, attr_type*>
{
public:
    unsigned int mValidCount;

    base_array_extension(unsigned int arraySize)
    : base_extension<ext_type, attr_type*>(NULL, arraySize)
    {
        this->attr_val = NULL;
        if (arraySize != 0) {
            this->attr_val = new attr_type[arraySize];
        }
        this->mArraySize = arraySize;
        this->mValidCount = arraySize;
    }

    ~base_array_extension()
    {
        delete [] this->attr_val;
    }

    void getArray(attr_type*& array, unsigned int& validCount)
    {
        validCount = this->mValidCount;
        array = this->attr_val;
    }

    attr_type* setArray(unsigned int size)
    {
        if (size > this->mArraySize) {
            /*We need to reallocate*/
            this->mArraySize = size;
            attr_type *temp = this->attr_val;
            this->attr_val = new attr_type[this->mArraySize];
            for (unsigned int i = 0; i < this->mValidCount; i++) {
                this->attr_val[i] = temp[i];
            }
            delete [] temp;
        }
        this->mValidCount = size;
        return this->attr_val;
    }

    tlm::tlm_extension_base* clone () const
    {
        ext_type * t = new ext_type(this->mArraySize);
        for (unsigned int i = 0; i < this->mValidCount; i++) {
   	        t->attr_val[i] = this->attr_val[i];
        }
        t->mArraySize = this->mArraySize;
        t->mValidCount = this->mValidCount;
        return t;
    }

    void copy_from (const tlm::tlm_extension_base &ext)
    {
        ext_type const &realExt = static_cast<ext_type const &>(ext);
        mValidCount = realExt.mValidCount;
        if (this->mValidCount > this->mArraySize) {
            /*We need to reallocate*/
            this->mArraySize = this->mValidCount;
            delete [] this->attr_val;
            this->attr_val = new attr_type[this->mArraySize];
        }

        for (unsigned int i = 0; i < this->mValidCount; i++) {
            this->attr_val[i] = realExt.attr_val[i];
        }
    }
};

} /* namespace scml */
#endif /* __base_extension_H__  */
