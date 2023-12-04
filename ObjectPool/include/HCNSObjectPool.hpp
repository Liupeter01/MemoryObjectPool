#pragma once
#ifndef _HCNSOBJECTPOOL_H_
#define _HCNSOBJECTPOOL_H_

#include<HCNSObjectPoolAllocator.hpp>	//introduce objectpool allocator

#define OBJECTPOOL_SETTING (100)

template<typename _Tp>
class HCNSObjectPool
{
public:
	void* operator new(size_t _size);
	void operator delete(void* _ptr);
	void* operator new[](size_t _size);
	void operator delete[](void* _ptr);

	template<typename ...Args>
	static _Tp* createObject(Args &&... args);

	static void deleteObject(_Tp* obj);

protected:
	HCNSObjectPool() = default;
	virtual ~HCNSObjectPool() =default;

private:
	static HCNSObjectPoolAllocator<_Tp> &getInstance();
};

template<typename _Tp>
void* HCNSObjectPool<_Tp>::operator new(size_t _size)
{
		  return getInstance().allocObjectMemory(_size);
}

template<typename _Tp>
void HCNSObjectPool<_Tp>::operator delete(void* _ptr)
{
		  getInstance().deallocObjectMemory(reinterpret_cast< _Tp*>(_ptr));
}

template<typename _Tp>
void* HCNSObjectPool<_Tp>::operator new[](size_t _size)
{
	return operator new(_size);
}

template<typename _Tp>
void HCNSObjectPool<_Tp>::operator delete[](void* _ptr)
{
	operator delete(_ptr);
}

/*------------------------------------------------------------------------------------------------------
* create an object
* @description: use templates to forward init params to class
* @function:  _Tp *createObject(Args &&... args)
*------------------------------------------------------------------------------------------------------*/
template<typename _Tp>
template<typename ...Args>
_Tp* HCNSObjectPool<_Tp>::createObject(Args &&... args) {
		  _Tp* obj(new _Tp(args...));
		  return obj;
}

/*------------------------------------------------------------------------------------------------------
* delete an object
* @function:  _Tp *deleteObject(_Tp* obj)
*------------------------------------------------------------------------------------------------------*/
template<typename _Tp>
void  HCNSObjectPool<_Tp>::deleteObject(_Tp* obj) {
		  delete obj;
}

/*------------------------------------------------------------------------------------------------------
* create an static singleton object instance
* @description: use singleton design pattern
* @function:  static HCNSObjectPoolAllocator<_Tp> &getInstance()
*------------------------------------------------------------------------------------------------------*/
template<typename _Tp>
HCNSObjectPoolAllocator<_Tp> & HCNSObjectPool<_Tp>::getInstance()
{
	static HCNSObjectPoolAllocator<_Tp> alloc_obj(OBJECTPOOL_SETTING);
	return alloc_obj;
}

#endif