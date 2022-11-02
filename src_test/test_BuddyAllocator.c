#include "test_environment.h"
#include "../src/BuddyAllocator.h"

#define __TEST_BA_MEM_RANK_RANGE (Rank_t)(2)
#define __TEST_BA_MEM_RANK (Rank_t)(__TEST_BA_MEM_RANK_RANGE + __BUDDY_ALLOCATOR_RANK_MIN)
#define __TEST_BA_MEM_CAPACITY (size_t)(1ull << __TEST_BA_MEM_RANK)

void test_min(BuddyAllocator_t* ba) {
	buddy_allocator_dump(ba);
//	for(unsigned i = 0; i < 10; i++) {
//		void* raw = buddy_allocator_alloc(ba, 1);
//		buddy_allocator_dump(ba);
//		if(raw) {
//			buddy_allocator_free(ba, raw);
//		}
//	}
}


int main() {
	TRACE_CALL;
	printf("__TEST_BA_MEM_RANK_RANGE    : %u\n", __TEST_BA_MEM_RANK_RANGE);
	printf("__TEST_BA_MEM_RANK          : %u\n", __TEST_BA_MEM_RANK);
	printf("__TEST_BA_MEM_CAPACITY      : %zu\n", __TEST_BA_MEM_CAPACITY);

	void* mem = malloc(__TEST_BA_MEM_CAPACITY);
	assert(mem);

	BuddyAllocator_t* ba = buddy_allocator_create(mem, __TEST_BA_MEM_CAPACITY);
	assert(ba);

	test_min(ba);

	buddy_allocator_destroy(ba);

	return 0;
}


