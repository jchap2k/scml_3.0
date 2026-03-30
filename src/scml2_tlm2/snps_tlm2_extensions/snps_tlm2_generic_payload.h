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
Description: Contains declaration and implmentation of ft_generic_payload
which is derived from tlm_generic_payload class and adds some additional utility
functions over it.
******************************************************************************/

#ifndef __SNPS_TLM2_GENERIC_PAYLOAD_H__
#define __SNPS_TLM2_GENERIC_PAYLOAD_H__


#include "tlm"
#include <map>
#include "scml2_tlm2/snps_tlm2_extensions/snps_tlm2_base_extension.h"


namespace scml2 {

#define ABORT_IF_NOT_FT_PAYLOAD(payload) \
{\
    if(!payload) { \
        std::cerr << __FUNCTION__ << " Error!! FT extensions can only be used on scml2::ft_generic_payload. Aborting simulation!!" << std::endl; \
        std::abort(); \
    } \
}


#define ADDnSET_EXT(payload, ext_type, attr_val ) \
{\
	ext_type* _ext001_inst_; \
    scml2::ft_generic_payload::set_extension_attr_s(payload, _ext001_inst_, attr_val, true);\
}

#define ADDnSET_TIME_EXT(payload, ext_type, array_size, attr_val ) \
{\
	ext_type* _ext001_inst_; \
	assert(array_size); \
	scml2::ft_generic_payload::add_extension_s(payload, _ext001_inst_, array_size); \
	scml2::ft_generic_payload::set_extension_attr_s(payload, _ext001_inst_, attr_val);\
}
#define SET_EXT_ATTR(payload, ext_type, attr_val) {                     \
	ext_type* _ext002_inst_;                                            \
    scml2::ft_generic_payload::set_extension_attr_s(payload, _ext002_inst_, attr_val, true);     \
}


#define GET_EXT_ATTR(payload, ext_type, attr_type, attr_inst )               \
	attr_type attr_inst ;                                                    \
	{                                                                        \
        ext_type* _ext003_inst_;                                             \
	    if (scml2::ft_generic_payload::get_extension_attr_s(payload, _ext003_inst_, attr_inst) == NULL) { \
           scml2::ft_generic_payload::get_extension_attr_mapped_s(payload, _ext003_inst_, attr_inst, true);      \
        }                                                                    \
    }

#define ADDnSET_ARRAY_EXT(payload, ext_type, array_type, array_val ,array_size) \
    array_type *array_val=NULL;\
{\
    ext_type* _ext004_inst_; \
    if(scml2::ft_generic_payload::get_extension_attr_s(payload, _ext004_inst_, array_val)==NULL){\
       scml2::ft_generic_payload::set_extension_attr_s<ext_type,array_type*>(payload, _ext004_inst_, NULL,true);\
   }\
   assert(_ext004_inst_ != NULL);\
   array_val = _ext004_inst_ -> setArray(array_size);\
}

#define SET_ARRAY_EXT_ATTR(payload, ext_type, array_type, array_val, array_size)\
    array_type *array_val=NULL;\
   { \
    ext_type* _ext005_inst_;\
        if(scml2::ft_generic_payload::get_extension_attr_s(payload, _ext005_inst_, array_val)==NULL){\
        scml2::ft_generic_payload::set_extension_attr_s<ext_type,array_type*>(payload, _ext005_inst_, NULL,true);\
   }\
   assert(_ext005_inst_ != NULL);\
   array_val = _ext005_inst_ -> setArray(array_size);\
  }

#define GET_ARRAY_EXT_ATTR(payload, ext_type, array_type, array_val , valid_entries) \
    array_type *array_val =NULL; \
    unsigned int valid_entries;\
    {\
    ext_type* _ext006_inst_; \
    if(scml2::ft_generic_payload::get_extension_attr_s(payload, _ext006_inst_, array_val)==NULL){\
        valid_entries = 0;\
    } else {\
        valid_entries = _ext006_inst_ -> mValidCount; \
      }\
    }


/******************************************************************************
Name: ft_generic_payload
Description: Declarations of ft_generic_payload class which is derived
			 from TLM Generic Payload. It adds certain utility functions
			 on top of TLM Generic Payload class, and provides user with
			 simple and easy to use interface for setting and getting protocol
			 specific attributes without worrying about managing multiple
			 extensions.
******************************************************************************/


class ft_generic_payload : public tlm::tlm_generic_payload
{

 public:
	//Constructors
	ft_generic_payload() : tlm::tlm_generic_payload(),
							gpMasterPtr(reinterpret_cast<tlm::tlm_generic_payload*>(this)) {}

	ft_generic_payload(tlm::tlm_mm_interface *mm)
					:tlm::tlm_generic_payload(mm),
					gpMasterPtr(reinterpret_cast<tlm::tlm_generic_payload*>(this)) {}


	ft_generic_payload(tlm::tlm_generic_payload* gp_base)
				: tlm::tlm_generic_payload(),
				  gpMasterPtr(gp_base) {}

	//Destructor
	~ft_generic_payload(){}

	tlm::tlm_generic_payload* get_gp_master_ptr(){ return gpMasterPtr;}

	//templated function to add a new extension to the payload
	template <typename extension_type>
		void add_extension(extension_type*& ext, unsigned int extArg = 0);

	//templated function to set attribute value in specific extension
	template <typename extension_type, typename attr_type >
		void set_extension_attr(extension_type*&  ext, attr_type attr_val,
											bool add_if_not_exist = false );

	//templated function to get attribute value from a specific extension
    template <typename extension_type, typename attr_type >
			extension_type * get_extension_attr(extension_type*& ext, attr_type& attr_val);

    template <typename extension_type, typename attr_type >
			void get_extension_attr(extension_type*& ext, attr_type& attr_val,
                                    bool get_mapped);

	template <typename extension_type>
		static void add_extension_s(tlm::tlm_generic_payload *gpMasterPtr, extension_type*& ext, unsigned int extArg = 0);

	template <typename extension_type, typename attr_type >
		static void set_extension_attr_s(tlm::tlm_generic_payload *gpMasterPtr, extension_type*&  ext, attr_type attr_val,
											bool add_if_not_exist = false );

    template <typename extension_type, typename attr_type >
        static extension_type * get_extension_attr_s(tlm::tlm_generic_payload *gpMasterPtr, extension_type*& ext, attr_type& attr_val);

    template <typename extension_type, typename attr_type >
        static void get_extension_attr_mapped_s(tlm::tlm_generic_payload *gpMasterPtr, extension_type*& ext, attr_type& attr_val,
                                            bool get_mapped);

	//decrement refrence count
	void release();


private:
	tlm::tlm_generic_payload* gpMasterPtr;

public:

	//Pure Virtual functions to be overridden by tlm2_gp class
  void acquire() {
    gpMasterPtr->acquire();
  }
  int get_ref_count() {
		return gpMasterPtr->get_ref_count();
	}
    void set_mm(tlm::tlm_mm_interface* mm) {
		gpMasterPtr->set_mm(mm);
	}
    bool has_mm() const {
		return gpMasterPtr->has_mm();
	}

    void reset() {
		gpMasterPtr->reset();
	}
    void deep_copy_from(const tlm::tlm_generic_payload & other) {
		gpMasterPtr->deep_copy_from(other);
	}
    void update_original_from(const tlm::tlm_generic_payload & other,
                          bool use_byte_enable_on_read = true) {

		gpMasterPtr->update_original_from(other, use_byte_enable_on_read);
	}

    void update_extensions_from(const tlm::tlm_generic_payload & other){
		gpMasterPtr->update_extensions_from(other);
	}
    void free_all_extensions(){
		gpMasterPtr->free_all_extensions();
	}
    bool is_read() const{
		return gpMasterPtr->is_read();
	}
    void set_read(){
		gpMasterPtr->set_read();
	}
    bool is_write() const{
		return gpMasterPtr->is_write();
	}
    void set_write(){
		gpMasterPtr->set_write();
	}
    tlm::tlm_command get_command() const{
		return gpMasterPtr->get_command();
	}
    void set_command(const tlm::tlm_command command){
		gpMasterPtr->set_command(command);
	}

    // Address related methods
    sc_dt::uint64 get_address() const{
		return gpMasterPtr->get_address();
	}
    void set_address(const sc_dt::uint64 address) {
		gpMasterPtr->set_address(address);
	}

    unsigned char* get_data_ptr() const {

		return gpMasterPtr->get_data_ptr();
	}
    void set_data_ptr(unsigned char* data) {

		gpMasterPtr->set_data_ptr(data);
	}

    unsigned int get_data_length() const {

		return gpMasterPtr->get_data_length();
	}
    void set_data_length(const unsigned int length){

		gpMasterPtr->set_data_length(length);
	}

    bool is_response_ok() const{

		return gpMasterPtr->is_response_ok();
	}
    bool is_response_error() const {

		return gpMasterPtr->is_response_error();
	}
    tlm::tlm_response_status  get_response_status() const {

		return gpMasterPtr->get_response_status();
	}
    void set_response_status(const tlm::tlm_response_status response_status){
		gpMasterPtr->set_response_status(response_status);
	}
    std::string get_response_string() const {
		return gpMasterPtr->get_response_string();
	}
    unsigned int get_streaming_width() const {
		return gpMasterPtr->get_streaming_width();
	}
    void set_streaming_width(const unsigned int streaming_width){
		gpMasterPtr->set_streaming_width(streaming_width);
	}

    unsigned char* get_byte_enable_ptr() const {
		return gpMasterPtr->get_byte_enable_ptr();
	}
    void  set_byte_enable_ptr(unsigned char* byte_enable){
		return gpMasterPtr->set_byte_enable_ptr(byte_enable);
	}
    unsigned int get_byte_enable_length() const{
		return gpMasterPtr->get_byte_enable_length();
	}
    void set_byte_enable_length(const unsigned int length){
		gpMasterPtr->set_byte_enable_length(length);
	}
    void set_dmi_allowed(bool dmi_allowed) {

		gpMasterPtr->set_dmi_allowed(dmi_allowed);
	}
    bool is_dmi_allowed() const {
		return gpMasterPtr->is_dmi_allowed();
	}
    template <typename T> T* set_extension(T* ext) {
		return gpMasterPtr->set_extension(ext);
	}

    tlm::tlm_extension_base* set_extension(unsigned int index,
       		                      tlm::tlm_extension_base* ext){

		return gpMasterPtr->set_extension(index, ext);
	}
    template <typename T> T* set_auto_extension(T* ext){

		return gpMasterPtr->set_auto_extension(ext);
	}
    tlm::tlm_extension_base* set_auto_extension(unsigned int index,
              	                    tlm::tlm_extension_base* ext){

		return gpMasterPtr->set_auto_extension(index, ext);
	}
    template <typename T> void get_extension(T*& ext) const {

		return gpMasterPtr->get_extension(ext);
	}
    template <typename T> T* get_extension() const {

		return gpMasterPtr->get_extension<T>();
	}
    tlm::tlm_extension_base* get_extension(unsigned int index) const {

		return gpMasterPtr->get_extension(index);
	}
    template <typename T> void clear_extension(const T* ext){

		gpMasterPtr->clear_extension(ext);
	}
    template <typename T> void clear_extension() {

		gpMasterPtr->clear_extension<T>();
	}
    template <typename T> void release_extension(T* ext) {
		gpMasterPtr->release_extension(ext);
	}
    template <typename T> void release_extension() {
		gpMasterPtr->release_extension<T>();
	}
    void resize_extensions() {
		gpMasterPtr->resize_extensions();
	}
	//overloaded typecast operator
	operator tlm::tlm_generic_payload* () {
		return gpMasterPtr;
	}
	/*operator tlm_generic_payload () {
		return *gpMasterPtr;
	}*/
};
//Global Map to store the snps_tlm2_gp pointer corresponding to tlm2_gp pointer
typedef std::map<tlm::tlm_generic_payload*, scml2::ft_generic_payload*> gpPtrMapT;
static gpPtrMapT gp2snps_tlm2_map;

//Global Inline function to create a proxy for tlm2_gp object
inline ft_generic_payload* get_payload(tlm::tlm_generic_payload* tmp){

	ft_generic_payload* payload =
			dynamic_cast<ft_generic_payload*>(tmp);

	if(payload == NULL) {

		gpPtrMapT::iterator mapIter;
		mapIter = gp2snps_tlm2_map.find(tmp);

		if(mapIter == gp2snps_tlm2_map.end()){

			payload = new ft_generic_payload(tmp);
			gp2snps_tlm2_map[tmp] = payload;
			std::cout<<"Allocating proxy \n";
		} else {

		 	payload  = (*mapIter).second;
		}
	}
	return payload;
}
//global inline function to delete the proxy object
inline void free_payload(ft_generic_payload* payload){

  //we need to delete this if there was a separate
  //instance of ft_generic_payload created

  if(payload->get_gp_master_ptr() !=
      reinterpret_cast<tlm::tlm_generic_payload*>(payload)){

    gpPtrMapT::iterator mapIter;
    mapIter = gp2snps_tlm2_map.find(payload->get_gp_master_ptr());

    if(mapIter != gp2snps_tlm2_map.end()){
      gp2snps_tlm2_map.erase(mapIter);
      std::cout<<"Erasing proxy \n";
    }
    std::cout<<"Deleting proxy \n";
    delete payload;
  }
}


template <typename extension_type>
void ft_generic_payload::
add_extension_s(tlm::tlm_generic_payload *gpMasterPtr, extension_type*& tlm2Ext, unsigned int extArg)
{
    //Get The memory manager for this extension type
    mem_manager<extension_type>* mm = const_cast<mem_manager<extension_type>* >((extension_type::get_mm()));

	if(!mm){
		std::cerr<<"Memory Manager for extension type "<<
				typeid(extension_type).name()<<" does not exist \n";
		std::cerr<<"Add Memory manager in TLM2ExtMapper::tlmExtMemManager \n";
		exit(1);
	}

	//Invoke memory manager to get pointer to extension instance
	tlm2Ext  = mm->claim_obj(extArg);
	gpMasterPtr->set_extension(tlm2Ext);
}

template <typename extension_type, typename attr_type >
void ft_generic_payload::
set_extension_attr_s(tlm::tlm_generic_payload *gpMasterPtr, extension_type*&  tlm2Ext, attr_type attr_val,
                                        bool add_if_not_exist)
{
    //Call the Base class API to get the extension.
	gpMasterPtr->get_extension(tlm2Ext);
	if(tlm2Ext != NULL) {
		tlm2Ext->set_attribute(gpMasterPtr, attr_val);
		return;
	}
    // Extension doesn't exist. Add it always
    add_extension_s(gpMasterPtr, tlm2Ext);
	//Set the attribute of the extension
	tlm2Ext->set_attribute(gpMasterPtr, attr_val);
}

template <typename extension_type, typename attr_type >
extension_type * ft_generic_payload::
get_extension_attr_s(tlm::tlm_generic_payload *gpMasterPtr, extension_type*& tlm2Ext, attr_type& attr_val)
{
    gpMasterPtr->get_extension(tlm2Ext);

	if(tlm2Ext) {
		attr_val = tlm2Ext->get_attribute(gpMasterPtr);
		return tlm2Ext;

	}
	return tlm2Ext;
}

template <typename extension_type, typename attr_type >
void ft_generic_payload::
get_extension_attr_mapped_s(tlm::tlm_generic_payload *gpMasterPtr, extension_type*& tlm2Ext, attr_type& attr_val,
                                        bool get_mapped)
{
    (void)get_mapped;
    if (get_extension_attr_s(gpMasterPtr, tlm2Ext, attr_val) != NULL) {
       return;
    }
	//return default value
	extension_type myExt;
	attr_val = myExt.get_default_val();
}

/*******************************************************************************
Name: add_extension
Description: creates and adds the extension to the payload
			 Also takes constructor argument for the extension, whose default
			 value is set to 0
*******************************************************************************/
template <typename extension_type >
void ft_generic_payload::
add_extension( extension_type*& tlm2Ext, unsigned int extArg)
{
	add_extension_s(this->gpMasterPtr, tlm2Ext, extArg);
}
/*******************************************************************************
Name: set_extension_attr
Description: Takes the type of the extension and the attribute value as arguments.
			 Looks in the payload if the extension type exist, then it sets the
			 attribute value to the extension's attribute. Else checks the value
			 of argument "add_if_not_exist". If it is true, it instantiates the
			 extension, and add it to the payload. If false, it invokes Mapper
			 to find a relevant mapped extension. Then sets the attr value in the
			 mapped extension.
*******************************************************************************/
template <typename extension_type, typename attr_type >
 void ft_generic_payload::set_extension_attr(extension_type*& tlm2Ext,
							attr_type attr_val, bool add_if_not_exist  ){

	set_extension_attr_s(this->gpMasterPtr, tlm2Ext, attr_val, add_if_not_exist);
}
/*******************************************************************************
Name: get_extension_attr
Description: Takes the type of the extension and returns the attribute value of
		 	 the extension attribute. Looks in the payload if the extension type
			 exist, then it returns the attribute value. If not it invokes the
			 mapper to get the mapped extension . If a mapping exists, it returns
			 attribute value of the mapped extension, after applying the convert
			 function. If not default value of the attribute is returned.
*******************************************************************************/
template <typename extension_type, typename attr_type >
 extension_type * ft_generic_payload::get_extension_attr(extension_type*& tlm2Ext,
														attr_type& attr_val){

	return get_extension_attr_s(this->gpMasterPtr, tlm2Ext, attr_val);
}
/*******************************************************************************
Name: get_extension_attr
Description: Takes the type of the extension and returns the attribute value of
		 	 the extension attribute. Looks in the payload if the extension type
			 exist, then it returns the attribute value. If not it invokes the
			 mapper to get the mapped extension . If a mapping exists, it returns
			 attribute value of the mapped extension, after applying the convert
			 function. If not default value of the attribute is returned.
*******************************************************************************/
template <typename extension_type, typename attr_type >
void ft_generic_payload::get_extension_attr ( extension_type*& tlm2Ext,
										   attr_type& attr_val, bool get_mapped)
{
    get_extension_attr_mapped_s(this->gpMasterPtr, tlm2Ext, attr_val, get_mapped);
}

/*******************************************************************************
Name: release
Description:  Decrement Refcount and if refCount is zero , calls free
*******************************************************************************/

inline void ft_generic_payload::release(){

	 if(get_ref_count() == 1){
#ifdef RESET_EXTENSIONS
		gpMasterPtr->free_all_extensions();
#endif
	 }
	gpMasterPtr->release();
	if(get_ref_count() == 0)
		free_payload(this);
}

} /* namespace scml2 */
#endif /* __SNPS_TLM2_GENERIC_PAYLOAD_H__  */
