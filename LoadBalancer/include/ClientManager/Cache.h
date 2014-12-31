#ifndef CACHE_H
#define CACHE_H
/////////////////////////////////////////////////////////////////////
//  Cache.h - definitions a LRU cache and a FIFO cache.
//  ver 1.0                                                        
//  Language:      standard C++                                 
//  Platform:      Ubuntu 14.04, 32-bit                               
//  Application:   2014 Summer Project                            
//  Author:        Chunxu Tang, Syracuse University
//                 chunxutang@gmail.com
/////////////////////////////////////////////////////////////////////
/*
* File Description:
* ==================
* This file defines operations two kinds of cache: LRU cache and FIFO
* chache.
*
* Required Files:
* ===============
* Cache.h, Cache.cpp
*
* Maintenance History:
* ====================
* ver 1.0 : 18 Aug 2014
* - first release
*/

#include <iostream>
#include <list>
#include <unordered_map>


//***********************************************************************
// Cache
//
// An abstract class which has three pure virtual functions. It is inherited
// by LRU cache and FIFO cache classes.
//***********************************************************************

template<class KeyType, class ElemType>
class Cache
{
public:
	virtual ElemType getElement(KeyType key) = 0;
	virtual void putElement(KeyType key, ElemType element) = 0;
	virtual bool isCached(KeyType key) = 0;
};




//***********************************************************************
// LRUCache
//
// This class inherits from Cache, and define and implement operations of
// a LRU (Least Recently used) cache. The recently used element will be 
// move to the front of the linked list. Because C++ container unordered_map
// (implementd by hash table) has O(1) time complexity of searching an
// element, it is used combined with C++ stl list, to improve preformance.
//***********************************************************************

template <class KeyType, class ElemType>
class LRUCache : Cache<KeyType, ElemType>
{
public:
	LRUCache(size_t size);
	~LRUCache();
	ElemType getElement(KeyType key);
	void putElement(KeyType key, ElemType element);
	bool isCached(KeyType key);
	const std::list<KeyType>& getCacheList();
	const std::unordered_map<KeyType, ElemType>& getCacheMap();
private:
	void moveToFront(KeyType key);
	void removeBack();
	void insertNewElement(KeyType key, ElemType element);

	size_t cache_size_;
	std::list<KeyType> cache_list_;
	std::unordered_map<KeyType, ElemType> cache_map_;
};

//-------------------------------------------------------------------
// Constructor
//-------------------------------------------------------------------
template <class KeyType, class ElemType>
LRUCache<KeyType, ElemType>::LRUCache(size_t size)
: cache_size_(size)
{}

//-------------------------------------------------------------------
// Destructor
//-------------------------------------------------------------------
template <class KeyType, class ElemType>
LRUCache<KeyType, ElemType>::~LRUCache()
{}

//-------------------------------------------------------------------
// Put an element into the cache
//-------------------------------------------------------------------
template <class KeyType, class ElemType>
void LRUCache<KeyType, ElemType>::putElement(KeyType key, ElemType element)
{
	// The element has been cached
	if (isCached(key)) 
	{
		cache_map_.at(key) = element;
		moveToFront(key);
	}
	// The element is not be cached, and there is still some room for
	// new elements in the cache.
	else if (cache_map_.size() >= cache_size_) 
	{
		insertNewElement(key, element);
		removeBack();
	}
	// The element is not cached, and the cache has been full.
	else
	{
		insertNewElement(key, element);
	}
}

//-------------------------------------------------------------------
// Get an element from the cache
//-------------------------------------------------------------------
template <class KeyType, class ElemType>
ElemType LRUCache<KeyType, ElemType>::getElement(KeyType key)
{
	moveToFront(key);

	return cache_map_[key];
}

//-------------------------------------------------------------------
// Determine whether an element has been cached or not.
//-------------------------------------------------------------------
template <class KeyType, class ElemType>
bool LRUCache<KeyType, ElemType>::isCached(KeyType key)
{
	if (cache_map_.find(key) != cache_map_.end())
		return true;
	return false;
}

//-------------------------------------------------------------------
// Get the list of elements in the cache
//-------------------------------------------------------------------
template <class KeyType, class ElemType>
const std::list<KeyType>& LRUCache<KeyType, ElemType>::getCacheList()
{
	return cache_list_;
}

//-------------------------------------------------------------------
// Get the hash table of elements in the cache
//-------------------------------------------------------------------
template <class KeyType, class ElemType>
const std::unordered_map<KeyType, ElemType>& LRUCache<KeyType, ElemType>::getCacheMap()
{
	return cache_map_;
}

//-------------------------------------------------------------------
// Move a recently used element to the front of the cache list
//-------------------------------------------------------------------
template <class KeyType, class ElemType>
void LRUCache<KeyType, ElemType>::moveToFront(KeyType key)
{
	if (cache_list_.front() == key)
		return;

	cache_list_.remove(key);
	cache_list_.push_front(key);
}

//-------------------------------------------------------------------
// Remove the last element in the cache list
//-------------------------------------------------------------------
template <class KeyType, class ElemType>
void LRUCache<KeyType, ElemType>::removeBack()
{
	cache_map_.erase(cache_list_.back());
	
	cache_list_.pop_back();
}

//-------------------------------------------------------------------
// Insert a new element in the cache
//-------------------------------------------------------------------
template <class KeyType, class ElemType>
void LRUCache<KeyType, ElemType>::insertNewElement(KeyType key, ElemType element)
{
	cache_list_.push_front(key);
	cache_map_.insert({ key, element });
}




//***********************************************************************
// FIFOCache
//
// This class inherits from Cache, and define and implement operations of
// a FIFO (First in, first out) cache. The new element is inserted at the
// back of the cache, and if the cache has been full, the first element 
// will be poped out.
//***********************************************************************

template<class KeyType, class ElemType>
class FIFOCache : Cache<KeyType, ElemType>
{
public:
	FIFOCache(size_t size);
	~FIFOCache();
	ElemType getElement(KeyType key);
	void putElement(KeyType key, ElemType element);
	bool isCached(KeyType key);
	void removeFront();
	void insertNewElement(KeyType key, ElemType element);
private:
	size_t cache_size_;
	std::list<KeyType> cache_list_;
	std::unordered_map<KeyType, ElemType> cache_map_;
};

//-------------------------------------------------------------------
// Constructor
//-------------------------------------------------------------------
template<class KeyType, class ElemType>
FIFOCache<KeyType, ElemType>::FIFOCache(size_t size)
: cache_size_(size)
{}

//-------------------------------------------------------------------
// Destructor
//-------------------------------------------------------------------
template<class KeyType, class ElemType>
FIFOCache<KeyType, ElemType>::~FIFOCache()
{}

//-------------------------------------------------------------------
// Get an element from the cache
//-------------------------------------------------------------------
template<class KeyType, class ElemType>
ElemType FIFOCache<KeyType, ElemType>::getElement(KeyType key)
{
	return cache_map_[key];
}

//-------------------------------------------------------------------
// Put an element into the cache
//-------------------------------------------------------------------
template<class KeyType, class ElemType>
void FIFOCache<KeyType, ElemType>::putElement(KeyType key, ElemType element)
{
	if (isCached(key))
	{
		cache_map_.at(key) = element; // update the element
		return;
	}
	else if (cache_map_.size() < cache_size_)
	{
		insertNewElement(key, element);
	}
	else
	{
		insertNewElement(key, element);
		removeFront();
	}
}

//-------------------------------------------------------------------
// Determine whether an element has been cached or not.
//-------------------------------------------------------------------
template<class KeyType, class ElemType>
bool FIFOCache<KeyType, ElemType>::isCached(KeyType key)
{
	if (cache_map_.find(key) != cache_map_.end())
		return true;
	return false;
}

//-------------------------------------------------------------------
// Remove the first element from the cache list
//-------------------------------------------------------------------
template <class KeyType, class ElemType>
void FIFOCache<KeyType, ElemType>::removeFront()
{
	cache_map_.erase(cache_list_.front());
	cache_list_.pop_front();
}

//-------------------------------------------------------------------
// Insert a new element into the cache
//-------------------------------------------------------------------
template <class KeyType, class ElemType>
void FIFOCache<KeyType, ElemType>::insertNewElement(KeyType key, ElemType element)
{
	cache_list_.push_back(key);
	cache_map_.insert({ key, element });
}

#endif