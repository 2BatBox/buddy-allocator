#pragma once

#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>
#include <stdbool.h>

// =========================================================
// Memory layout example.
//
// __BA_RANK_MIN = 12;
// __BA_RANK_RANGE = 3;
//
// Rank(value) = ceil(log2(value));
// BucketId = Rank - RANK_MIN;
//
// |<- available memory range ->|  Rank   | BucketId  |
// ----------------------------------------------------
// |            16k             |  14     | 2         |
// |      8k     |      8k      |  13     | 1         |
// |  4k  |  4k  |  4k  |  4k   |  12     | 0         |
//
//
// =========================================================


// ====================================
// = Integral type aliases.
// ====================================
typedef uint8_t Rank_t;
typedef uint8_t BucketId_t;


// ====================================
// = Types definitions.
// ====================================
struct ChunkHeader_t;

// TODO: No padding is used since no memory alignment restrictions are specified.
struct ChunkHeader_t {
	struct ChunkHeader_t* prev;
	struct ChunkHeader_t* next;
	Rank_t rank;
	bool busy;
};

typedef struct ChunkHeader_t DListNode_t;
#include "DList.h"


// ====================================
// = Static configuration.
// ====================================
#define __BUDDY_ALLOCATOR_RANK_MIN (Rank_t)(12)
#define __BUDDY_ALLOCATOR_RANK_RANGE (Rank_t)(20)


// ====================================
// = Static constant values.
// ====================================
#define __BUDDY_ALLOCATOR_RANK_MAX (Rank_t)(__BUDDY_ALLOCATOR_RANK_MIN + __BUDDY_ALLOCATOR_RANK_RANGE)
#define __BUDDY_ALLOCATOR_CAPACITY_MAX (size_t)(SIZE_MAX - sizeof(DListNode_t))


typedef struct {
	DList_t buckets[__BUDDY_ALLOCATOR_RANK_RANGE];
	void* raw_memory_ptr;
	Rank_t raw_memory_rank;
} buddy_allocator_t;


// ====================================
// = Private methods.
// ====================================
static inline int __buddy_allocator_is_po2(const size_t value) {
	return value && ((value & (value - 1u)) == 0);
}

static inline Rank_t __buddy_allocator_rank(size_t capacity) {
	Rank_t result = 0;
	if(capacity) {
		capacity--;
		while(capacity) {
			result++;

			// According to the 1999 ISO C standard (C99), size_t is an unsigned integer type,
			// which means right bit shifting fills the MSB with zero.
			capacity >>= 1;
		}
	}
	return result;
}

static inline size_t __buddy_allocator_rank_capacity(const Rank_t rank) {
	return 1ull << rank;
}




static inline void __buddy_allocator_push_node(buddy_allocator_t* const ins, DListNode_t* const node) {
	const BucketId_t bucket = node->rank - __BUDDY_ALLOCATOR_RANK_MIN;
	node->busy = false;
	dlist_push_front(ins->buckets + bucket, node);
}




static inline DListNode_t* __buddy_allocator_pop_node(buddy_allocator_t* const ins, const Rank_t rank) {
	const BucketId_t bucket = rank - __BUDDY_ALLOCATOR_RANK_MIN;
	// TODO:
}




static inline DListNode_t* __buddy_allocator_node_addr(void* raw_ptr) {
	DListNode_t* result = NULL;
	if(raw_ptr) {
		uint8_t* const u8ptr = (uint8_t* const)raw_ptr;
		result = (DListNode_t*)(u8ptr - sizeof(DListNode_t));
	}
	return result;
}

static inline void* __buddy_allocator_raw_ptr(DListNode_t* const node) {
	uint8_t* const u8ptr = (uint8_t* const)node;
	return (void*)(u8ptr + sizeof(DListNode_t));
}

void __buddy_allocator_dump_bucket(const buddy_allocator_t* const ins, const BucketId_t bucket) {
	const DList_t* const list = ins->buckets + bucket;
	const DListNode_t* head = list->head;
	const uint8_t* const raw_mem_u8ptr = (const uint8_t* const)(ins->raw_memory_ptr);
	while(head) {
		const uint8_t* head_u8ptr = (const uint8_t*)(head);
		printf("[ Offset=%zu Rank=%u ] -> ", head_u8ptr - raw_mem_u8ptr, head->rank);
		head = head->next;
	}
	printf("\n");
}


// ====================================
// = Public methods.
// ====================================

/**
* Create a buddy allocator
* @param raw_memory Backing memory. MUST NOT be null.
* @param memory_size Backing memory size. MUST BE a power of two value.
* @return the new buddy allocator pointer or NULL in case of any errors.
*/
buddy_allocator_t* buddy_allocator_create(void* raw_memory, const size_t raw_memory_size) {
	buddy_allocator_t* result = NULL;

	// TODO: Are there any raw_memory alignment restrictions?
	if(raw_memory && __buddy_allocator_is_po2(raw_memory_size)) {
		const Rank_t rank = __buddy_allocator_rank(raw_memory_size);

		if(rank >= __BUDDY_ALLOCATOR_RANK_MIN && rank <= __BUDDY_ALLOCATOR_RANK_MAX) {
			result = malloc(sizeof(*result));

			if(result) {
				memset(result, 0, sizeof(*result));

				for(Rank_t idx = 0; idx < __BUDDY_ALLOCATOR_RANK_RANGE; ++idx) {
					dlist_init(result->buckets + idx);
				}

				result->raw_memory_ptr = raw_memory;
				result->raw_memory_rank = rank;

				DListNode_t* const node = (DListNode_t*)raw_memory;
				node->rank = rank;
				__buddy_allocator_push_node(result, node);
			}
		}
	}
	return result;
}

/**
* Destroy a buddy allocator
* @param ins The buddy allocator instance pointer. MUST NOT be null.
*/
void buddy_allocator_destroy(buddy_allocator_t* const ins) {
	if(ins->raw_memory_ptr) {
		ins->raw_memory_ptr = NULL;
		free(ins);
	}
}

/**
* Allocate memory
* @param ins The buddy allocator instance pointer. MUST NOT be null.
* @param size Size of memory to allocate
* @return pointer to the newly allocated memory , or @a NULL if out of memory
*/
void* buddy_allocator_alloc(buddy_allocator_t* const ins, size_t size) {
	void* result = NULL;
	if(size < __BUDDY_ALLOCATOR_CAPACITY_MAX) {
		size += sizeof(DListNode_t);
		Rank_t rank = __buddy_allocator_rank(size);
		if(rank <= ins->raw_memory_rank) {
			if(rank < __BUDDY_ALLOCATOR_RANK_MIN) {
				rank = __BUDDY_ALLOCATOR_RANK_MIN;
			}
			DListNode_t* const node = __buddy_allocator_pop_node(ins, rank);
			result = __buddy_allocator_raw_ptr(node);
		}
	}
	return result;
}

/**
* Deallocates a perviously allocated memory area.
* If @a ptr is @a NULL , it simply returns
* @param ins The buddy allocator instance pointer. MUST NOT be null.
* @param raw_ptr The memory area to deallocate. MUST NOT be null.
*/
void buddy_allocator_free(buddy_allocator_t* const ins, void* const raw_ptr) {
	DListNode_t* const node = __buddy_allocator_node_addr(raw_ptr);
	if(node && node->busy) {
		__buddy_allocator_push_node(ins, node);
	}
}

/**
 * @warning For debug purposes only.
 * @param ins The buddy allocator instance pointer. MUST NOT be null.
 */
void buddy_allocator_dump(const buddy_allocator_t* const ins) {
	printf("==== Buddy Allocator instance ====\n");
	printf("Struct ptr       : %p\n", ins);
	printf("Struct size      : %zu\n", sizeof(*ins));
	printf("DListNode_t size : %zu\n", sizeof(DListNode_t));
	printf("Raw mem ptr      : %p\n", ins->raw_memory_ptr);
	printf("Raw mem rank     : %u\n", ins->raw_memory_rank);
	printf("Raw mem capacity : %zu\n", __buddy_allocator_rank_capacity(ins->raw_memory_rank));

	Rank_t rank = ins->raw_memory_rank;
	while(rank >= __BUDDY_ALLOCATOR_RANK_MIN) {
		const Rank_t bucket = rank - __BUDDY_ALLOCATOR_RANK_MIN;
		const size_t size = __buddy_allocator_rank_capacity(rank);

		printf("[ Bucket=%-2u", bucket);
		printf("  Rank=%-2u", rank);
		printf("  Size=%-8zu ] : ", size);

		__buddy_allocator_dump_bucket(ins, bucket);
		rank--;
	}
}
