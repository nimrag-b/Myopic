#include "Allocators.h"

PoolAllocator::PoolAllocator(size_t blockCount) {
	mMem = new char[blockCount * BlockSize];
	mSize = blockCount;

	clear();
}

PoolAllocator::~PoolAllocator() {
	delete[] mMem;
}


void* PoolAllocator::alloc(size_t bytes) {

	assert(bytes <= BlockSize); //cant allocate something bigger than the blocksize

	Block* b = mTop;
	assert(mTop->next != nullptr); //if this is null we have run out of blocks

	mTop = b->next;
	return b;
}

void PoolAllocator::dealloc(void* block) {
	Block* b = reinterpret_cast<Block*>(block);
	b->next = mTop;
	mTop = b;
}


void PoolAllocator::clear() {
	memset(mMem, 0, mSize * BlockSize);

	for (size_t i = 0; i < ((mSize - 1) * BlockSize); i += BlockSize)
	{
		Block* b = (Block*)&mMem[i];
		b->next = (Block*)&mMem[i + BlockSize];
	}

	Block* last = (Block*)&mMem[((mSize - 1) * BlockSize)];
	last->next = nullptr; //no blocks left

	mTop = (Block*)mMem;
}



StackAllocator::StackAllocator(size_t size) {
	mMemory = new char[size];
	mSize = size;
	mCurrent = 0;
}

StackAllocator::~StackAllocator() {
	delete[] mMemory;
}

void* StackAllocator::alloc(size_t bytes) {
	void* m = mMemory + mCurrent;
	mCurrent += bytes;
	assert(mCurrent < mSize);
	return m;
}



















DeStackAllocator::DeStackAllocator(size_t size) {
	mMemory = new char[size];
	mSize = size;
	mBottom = 0;
	mTop = size;
}

DeStackAllocator::~DeStackAllocator() {
	delete[] mMemory;
}

void* DeStackAllocator::allocBottom(size_t bytes) {
	void* m = mMemory + mBottom;
	mBottom += bytes;
	assert(mBottom < mSize);

	assert(mBottom < mTop);
	return m;
}

void* DeStackAllocator::allocTop(size_t bytes) {
	mTop -= bytes;
	void* m = mMemory + mTop;
	assert(mTop > 0);

	assert(mBottom < mTop);
	return m;
}
