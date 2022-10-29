#include <stdio.h>
#include <assert.h>

#include "BuddyAllocator.h"

void foo(size_t value) {
	printf("__buddy_allocator_rank(%zu)=%u\n", value, __buddy_allocator_rank(value));
}

int main() {
	const size_t size = 0x10000;
	void* mem = malloc(size);
	assert(mem);

	buddy_allocator_t* ba = buddy_allocator_create(mem, size);

	buddy_allocator_dump(ba);

	return 0;
}
