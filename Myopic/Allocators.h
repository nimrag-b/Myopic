#pragma once

class Allocator {
public:
	Allocator(size_t block_size);

	void* allocate(size_t size) {
		

	}

private:

	struct Block {
		Block* next;
		void* mem;
	};

	struct FreeElement {
		FreeElement* next;
		size_t length;
	};

	FreeElement* free_element;
	Block* head;
};
















class MemoryPool {
public:
	MemoryPool() {

	}
	~MemoryPool() {
		while (head) {
			Block* block = head;
			head = head->next;

		}
	}

	void* alloc() {
		if (free_element) {
			void* mem = free_element;
			free_element = free_element->next_element;
			return mem;
		}

	}
	void dealloc() {

	}

private:

	struct Block {
		Block* next;
		char* mem;
	};

	struct FreeElement {
		FreeElement* next_element;
	};


	FreeElement* free_element;
	Block* head;
	size_t block_size;
	size_t num_block_elements;
};

