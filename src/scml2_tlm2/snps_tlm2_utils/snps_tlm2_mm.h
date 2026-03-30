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
Delcaration and Implementation of mem_manager class.
******************************************************************************/

#ifndef __SNPS_TLM2_MM_H__
#define __SNPS_TLM2_MM_H__

#include "tlm"
#include <unordered_map>
#include <vector>

namespace scml2 {
/*******************************************************************************
Name: mem_manager

Description: Memory Manager class for doing memory management for extensions and
			 payload.This implements tlm_mm_interface.It is a templated class
			 because it can be used for memory management of transaction objects
			 as well as different payload extension objects
*******************************************************************************/

template <typename memObjType >
class mem_manager : public tlm::tlm_mm_interface {
public:
	/******************************************************************************
	 Destructor of the memory manager. Deletes the memory for all nodes in the pool
		******************************************************************************/
	~mem_manager()
	{
		for (memObjType* ptr : freeMemObjs) {
			delete ptr;
		}
	}

	/*******************************************************************************
	Name: claim_obj: Method to get pointer to an objtype which takes arg as constructor
					argument. Functionality same as
					claim(), except that constructor for extension require an
					extra constructor argument
	*******************************************************************************/
	memObjType* claim_obj(unsigned int arg=0)
	{
		memObjType* ptr = NULL;
		get_free_node(ptr);

		if(ptr == NULL){
			ptr = new memObjType(arg);
			ptr->set_mm(this);
		}
		return ptr;
	}

	/******************************************************************************
	Name: claim()
	Description: Returns pointer to an free memObjType object. If no free obj exists
				in the pool. It allocates from the heap and returns the pointer.
	******************************************************************************/
	memObjType* claim()
	{
		memObjType* ptr = NULL;
		get_free_node(ptr);
		return ptr ? ptr : new memObjType(this);
	}

	/******************************************************************************
	Name: free()
	Description: Implementation of virtual method of tlm_mm_interface, re-directs
				call to the actual implementation method.
	******************************************************************************/
	void free(tlm::tlm_generic_payload* payload)
	{
		payload->reset();
		free_obj(reinterpret_cast<memObjType *>(payload));
	}

	/******************************************************************************
	Name: free_obj()
	Description: It is called to return an allocated object to the pool,
				after its usage is completed. If there is an empty memObjNode
				which does not point to memObj pointer, free obj pointer is appended to it.
				Else a new memObjNode is created and appended to the free list.
	******************************************************************************/
	void free_obj( memObjType* memObj )
	{
		freeMemObjs.push_back(memObj);
	}

protected:
	mem_manager<memObjType>& operator =(mem_manager<memObjType>&);

	/******************************************************************************
	Name:get_free_node
	Description: Returns the first free element from free list and moves
				pointer to the next available free element
	******************************************************************************/
	void get_free_node(memObjType* & ptr)
	{
		if( !freeMemObjs.empty() ) {
			ptr = freeMemObjs.back();
			freeMemObjs.pop_back();
		}
	}

	// the pool of free nodes; remaining elements will be deleted in the destructor
	// stored as std::vector and not as std::list because std::list will create and
	// destroy each individual node on the heap, which is kind of what we want
	// to avoid with a memory manager
	std::vector<memObjType *> freeMemObjs;
};

template <typename memObjType >
class mem_manager_array : protected mem_manager< std::vector<memObjType> > {
public:
	using storage_type = std::vector<memObjType>;
	
	memObjType* claim_array(size_t num_entries)
	{
		storage_type* ptr = NULL;
		this->get_free_node(ptr);
		if(!ptr) {
			ptr = new storage_type();
		}
		ptr->resize(num_entries);
		memObjType* data = ptr->data();
		m_map[data] = ptr;

		return data;
	}

	/******************************************************************************
	Name: free()
	Description: Implementation of virtual method of tlm_mm_interface, re-directs
				call to the actual implementation method.
	******************************************************************************/
	void free_array(memObjType *data)
	{
		/* Get the storage_type container for this array */
		auto it = m_map.find(data);
		if(it != m_map.end()) {
			this->free_obj(it->second);
		}
	}
	
private:
	std::unordered_map<memObjType*, storage_type*> m_map;

};

} /* namespace scml2 */
#endif /* __SNPS_TLM2_MM_H__  */
