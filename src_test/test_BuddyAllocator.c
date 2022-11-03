#include "test_environment.h"
#include "../src/BuddyAllocator.h"

#define __TEST_BA_MEM_RANK_RANGE (Rank_t)(5)
#define __TEST_BA_MEM_RANK (Rank_t)(__TEST_BA_MEM_RANK_RANGE + __BUDDY_ALLOCATOR_RANK_MIN)
#define __TEST_BA_MEM_CAPACITY (size_t)(1ull << __TEST_BA_MEM_RANK)
#define __TEST_BA_STORAGE_SIZE (size_t)(1ull << __TEST_BA_MEM_RANK_RANGE)
#define __TEST_BA_INTEGRITY_ITERATIONS (unsigned)999
#define __TEST_BA_VERBOSE 0

void test_integral(BuddyAllocator_t* ba) {
	TRACE_CALL;
	size_t* storage [__TEST_BA_STORAGE_SIZE];

	for(size_t i = 0; i < __TEST_BA_STORAGE_SIZE; i++) {
		storage[i] = buddy_allocator_alloc(ba, sizeof(size_t));
		*(storage[i]) = i;
		assert(storage[i]);
	}

	for(size_t i = 0; i < __TEST_BA_STORAGE_SIZE; i++) {
		assert(*(storage[i]) == i);
	}

	assert(buddy_allocator_alloc(ba, 1) == NULL);

	for(size_t i = 0; i < __TEST_BA_STORAGE_SIZE; i++) {
		buddy_allocator_free(ba, storage[i]);
	}
}


void test_capacity(BuddyAllocator_t* ba) {
	TRACE_CALL;
	const size_t capacity_max = buddy_allocator_capacity_max(ba);
	for(size_t size = 0; size <= capacity_max; size++) {
		void* raw = buddy_allocator_alloc(ba, size);
		assert(raw);
		buddy_allocator_free(ba, raw);
	}
}

void __test_integrity(BuddyAllocator_t* ba, int seed) {
	uint8_t* storage[__TEST_BA_STORAGE_SIZE];
	const size_t capacity_max = buddy_allocator_capacity_max(ba);
	assert(RAND_MAX > capacity_max);

	// Allocate and randomly fill the chunks.
	srand(seed);
	for(size_t i = 0; i < __TEST_BA_STORAGE_SIZE; ++i) {
		const size_t size = (rand() % capacity_max) + 1ull;
		storage[i] = buddy_allocator_alloc(ba, size);
		if(storage[i]) {
			for(int j = 0; j < size; ++j) {
				storage[i][j] = (uint8_t)rand();
			}
		}
	}

	// Check for any corruptions.
	srand(seed);
	for(size_t i = 0; i < __TEST_BA_STORAGE_SIZE; ++i) {
		const size_t size = (rand() % capacity_max) + 1ull;
		if(storage[i]) {
			for(int j = 0; j < size; ++j) {
				assert(storage[i][j] == (uint8_t)rand());
			}
		}
	}

	// Free all the chunks.
	for(size_t i = 0; i < __TEST_BA_STORAGE_SIZE; ++i) {
		if(storage[i]) {
			buddy_allocator_free(ba, storage[i]);
		}
	}
}

void test_integrity(BuddyAllocator_t* ba) {
	TRACE_CALL;
	for(unsigned i = 0; i < __TEST_BA_INTEGRITY_ITERATIONS; ++i) {
		__test_integrity(ba, i);
	}
}

int main() {
	TRACE_CALL;

	if(__TEST_BA_VERBOSE) {
		printf("__TEST_BA_MEM_RANK_RANGE    : %u\n", __TEST_BA_MEM_RANK_RANGE);
		printf("__TEST_BA_MEM_RANK          : %u\n", __TEST_BA_MEM_RANK);
		printf("__TEST_BA_MEM_CAPACITY      : %zu\n", __TEST_BA_MEM_CAPACITY);
	}

	void* mem = malloc(__TEST_BA_MEM_CAPACITY);
	assert(mem);

	BuddyAllocator_t* ba = buddy_allocator_create(mem, __TEST_BA_MEM_CAPACITY);
	assert(ba);

	test_integral(ba);
	test_capacity(ba);
	test_integrity(ba);

	if(__TEST_BA_VERBOSE) {
		__buddy_allocator_dump(ba);
	}

	buddy_allocator_destroy(ba);

	return 0;
}


