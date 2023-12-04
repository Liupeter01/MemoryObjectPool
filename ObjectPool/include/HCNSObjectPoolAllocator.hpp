#pragma once
#ifndef _HCNSOBJECTPOOLBASE_
#define _HCNSOBJECTPOOLBASE_

#include<HCNSMemoryAllocator.hpp>

/*
 *detour global memory allocation and deallocation functions
 */
void* operator new(size_t _size);
void operator delete(void* _ptr);
void* operator new[](size_t _size);
void operator delete[](void* _ptr);

#if _WIN32
#pragma comment(lib,"HCNSMemoryPool.lib")
#endif

/*------------------------------------------------------------------------------------------------------
* HCNSNodeDescriptor
* 4B memory alignment
*------------------------------------------------------------------------------------------------------*/
#pragma pack(push)
#pragma pack(4)
struct NodeDescriptor
{
		  NodeDescriptor* pNext;						//next Node pointer
		  uint32_t m_ref;								//reference counter
		  bool m_status;							    //is this node still exist in memory?
};
#pragma pack(pop)

template<typename _Tp>
class HCNSObjectPoolAllocator
{
public:
		  HCNSObjectPoolAllocator(std::size_t _objectAmount);
		  virtual ~HCNSObjectPoolAllocator();
		  _Tp* allocObjectMemory(size_t _size);
		  void deallocObjectMemory(_Tp* _ptr);

private:
		  void initObjectPool(std::size_t _objectAmount);
		  NodeDescriptor* getUnAmbigousHeaderValue();

private:
	  	  uint32_t _invisibleBlockSize = sizeof(NodeDescriptor);			//The size of NodeDescriptor structure
		  uint32_t _allocateMemSpace = sizeof(_Tp);							//return the size of _Tp(sizeof(_Tp))

		  uint32_t _blockTotalSize = sizeof(NodeDescriptor) + sizeof(_Tp); 	//The total size of sizeof(NodeDescriptor) + sizeof(_Tp)
          uint32_t _objectpoolTotalSize = 0;
		  uint32_t _objectAmount = 0;

		  NodeDescriptor* 	m_pHead;						//the head of the node
		  void* 			m_pBuffer;			  			//the head of memory
		  bool 				m_initStatus;					//objectPool init status
		  std::mutex 		m_mutex;						//the mutex
};

template<typename _Tp>
HCNSObjectPoolAllocator<_Tp>::HCNSObjectPoolAllocator(std::size_t _objectAmount)
{
	this->_objectAmount = _objectAmount;
	this->_objectpoolTotalSize = this->_objectAmount * this->_blockTotalSize;
	this->initObjectPool(_objectAmount);
}

template<typename _Tp>
HCNSObjectPoolAllocator<_Tp>::~HCNSObjectPoolAllocator()
{
		  ::delete[]this->m_pBuffer;
}


/*------------------------------------------------------------------------------------------------------
* start to init memory pool
* @description: according to _Tp to calculate memory size
* @function:  void initObjectPool(std::size_t _objectAmount)
* @param: [IN] std::size_t _objectAmount
*------------------------------------------------------------------------------------------------------*/
template<typename _Tp>
void HCNSObjectPoolAllocator<_Tp>::initObjectPool(std::size_t _objectAmount)
{
	//calculate and allocate memory size by using memory pool
	this->m_pBuffer = ::new char[_objectAmount * this->_blockTotalSize];

	//do the data type cast
	this->m_pHead = reinterpret_cast<NodeDescriptor*>(this->m_pBuffer);
	this->m_pHead->m_ref = 0;
	this->m_pHead->pNext = nullptr;
	this->m_pHead->m_status = true;;

	NodeDescriptor* prev = this->m_pHead;
	NodeDescriptor* pnext = reinterpret_cast<NodeDescriptor*>(
			reinterpret_cast<char*>(this->m_pBuffer) + this->_blockTotalSize
			);
	NodeDescriptor* pend = reinterpret_cast<NodeDescriptor*>(
			reinterpret_cast<char*>(this->m_pBuffer) + this->_objectpoolTotalSize
			);

	/*-----------------------------------------------------------------------------------*
		*		  		  		/\  |------NodeDescriptor---------|------------_Tp------------
		*		  		  		|   |---sizeof(NodeDescriptor)---|------sizeof(_Tp)-------
		*		  		  		|   |---sizeof(NodeDescriptor)---|------sizeof(_Tp)-------
		*	   _ObjPoolSize     |   |---sizeof(NodeDescriptor)---|------sizeof(_Tp)-------
		*		 				|   |---sizeof(NodeDescriptor)---|------sizeof(_Tp)-------
		* 					    |   |---sizeof(NodeDescriptor)---|------sizeof(_Tp)-------
		* 					    \/  |---sizeof(NodeDescriptor)---|------sizeof(_Tp)-------
		*------------------------------------------------------------------------------------
		*------------------------------------------------------------------------------------
		* in order to access to the next memoryblock, the offset of (char)pointer
		* should be the size of NodeDesciptor+_Tp
		*/

		/* traveral the pool and setup linklist connection!*/
	for (; pnext < pend; pnext = reinterpret_cast<NodeDescriptor*>(reinterpret_cast<char*>(pnext) + this->_blockTotalSize)){
		pnext->m_ref = 0;
		pnext->m_status = true;
		pnext->pNext = nullptr;

		prev->pNext = pnext;		//get next block
		prev = pnext;
	}
}
/*------------------------------------------------------------------------------------------------------
* alloc objects' memory according to it's size
* @description: according to object size and return an pointer which points to _Tp
* @function:  _Tp* allocObjectMemory(size_t _size)
* @param: [IN] size_t _size
* @retvalue: _Tp*
*------------------------------------------------------------------------------------------------------*/
template<typename _Tp>
_Tp* HCNSObjectPoolAllocator<_Tp>::allocObjectMemory(size_t _size)
{
		  NodeDescriptor* pAllocMem(nullptr);
		  /*
		  * if pool hasn't been inited and allocated with memory
		  */
		  if (!this->m_initStatus || this->m_pBuffer== nullptr) {
					this->initObjectPool(100);
		  }

		  /*
		  * there is no MemoryBlock header because of the pool is run out of free memory ,therefore
		  * create a temporary memory allocation
		  * Add a mutex lock in order to avoid multithreading problem
		  */
		  if (nullptr == this->getUnAmbigousHeaderValue()) {

					/*
					* memorypool needs to manage all the allocated memory,therefore
					* we need an additional descriptor size (sizeof(MemoryBlock)) add up to the totoal size
					*/
					//auto additional_size = _size + sizeof(NodeDescriptor);
					NodeDescriptor* pAdd = reinterpret_cast<NodeDescriptor*>(
							  ::new char[this->_blockTotalSize]
							  );

					pAdd->m_ref = 1;
					pAdd->m_status = false;
					pAdd->pNext = nullptr;

					pAllocMem = pAdd;								  //

		  }
		  else {
					std::lock_guard<std::mutex> _lckg(this->m_mutex);
					pAllocMem = this->m_pHead;										  //get empty memory block
					this->m_pHead = this->m_pHead->pNext;	  //move header to the next memory block
					pAllocMem->m_ref = 1;											  //set this memory region reference time
		  }
		  /* !!! jump the memoryblock scale !!!*/
		  return reinterpret_cast<_Tp*>(++pAllocMem);
}

/*------------------------------------------------------------------------------------------------------
* free object's memory space
* @function:  void deallocObjectMemory( _Tp* _ptr)
* @param:[IN]  _Tp* _ptr
*------------------------------------------------------------------------------------------------------*/
template<typename _Tp>
void HCNSObjectPoolAllocator<_Tp>::deallocObjectMemory(_Tp* _ptr)
{
		  /*calculate pointer offset*/
		  NodeDescriptor* _pMemInfo = reinterpret_cast<NodeDescriptor*>(
					reinterpret_cast<char*>(_ptr) - sizeof(NodeDescriptor*)
					);

		  /*
		  * is the memory block inside the object pool?
		  * if so, recycle this memory block(just like insert into a linklist from the head)
		  */
		  if (_pMemInfo->m_status) {
					/* see is there any other references to this object , the number should lower than 2*/
					if ((--_pMemInfo->m_ref)) {				// reference still exist, memory pool can not recycle this memory block
							  return;
					}
					std::lock_guard<std::mutex> _lckg(this->m_mutex);
					_pMemInfo->pNext = this->m_pHead;
					this->m_pHead = _pMemInfo;
		  }
		  else { /*outside memory pool*/
					delete[]_pMemInfo;
		  }
}

/*------------------------------------------------------------------------------------------------------
* get unambigous m_pHead value (meanwhile, mutex lock should be activated )
* @function:  NodeDescriptor* getUnAmbigousHeaderValue()
*------------------------------------------------------------------------------------------------------*/
template<typename _Tp>
NodeDescriptor* HCNSObjectPoolAllocator<_Tp>::getUnAmbigousHeaderValue()
{
	std::lock_guard<std::mutex> _lckg(this->m_mutex);
	return this->m_pHead;
}
#endif
