#pragma once
#include <cassert>
#include <memory>
#include <cstdint>

class PoolAllocator {
public:

	constexpr static size_t BlockSize = 256;

	PoolAllocator(size_t blockCount);

	~PoolAllocator();


	//helper func for allocating type
	template<typename T>
	inline T* alloc_object() {
		return new (alloc(sizeof(T))) T(); //create with placement new
	}

	template<typename T>
	inline void dealloc_object(T* obj) {
		obj->~T(); //manually call destructor
		dealloc(obj);
	}

	void* alloc(size_t bytes);

	void dealloc(void* block);

	void clear();


private:

	struct Block {
		Block* next;
	};

	char* mMem;
	Block* mTop;
	size_t mSize;

};

class StackAllocator {
public:


	typedef size_t Marker;

	StackAllocator(size_t size);

	~StackAllocator();

	void* alloc(size_t bytes);

	Marker getMarker() {
		return mCurrent;
	}

	void freeToMarker(Marker marker) {
		mCurrent = marker;
	}

	void clear() {
		mCurrent = 0;
	}

private:
	char* mMemory;
	size_t mSize;
	Marker mCurrent;
};

class DeStackAllocator {
public:


	typedef size_t Marker;

	DeStackAllocator(size_t size);

	~DeStackAllocator();

	void* allocBottom(size_t bytes);

	void* allocTop(size_t bytes);

	void freeTopToMarker(Marker marker) {
		mTop = marker;
	}
	void freeBottomToMarker(Marker marker) {
		mBottom = marker;
	}

	void clear() {
		mBottom = 0;
		mTop = 0;
	}

private:
	char* mMemory;
	size_t mSize;
	Marker mBottom;
	Marker mTop;
};

