#pragma once

#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>
#include <stdbool.h>

// =========================================================
// = Memory layout example.
//
// __BUDDY_ALLOCATOR_RANK_MIN = 12;
// __BUDDY_ALLOCATOR_RANK_RANGE = 3;
//
// rank(value) = ceil(log2(value));
// bucket = rank - RANK_MIN;
//
// |<-        chunks          ->| rank   | bucket |
// -------------------------------------------------
// |            16k             | 14     | 2      |
// |      8k     |      8k      | 13     | 1      |
// |  4k  |  4k  |  4k  |  4k   | 12     | 0      |
//
//
// = chunk layout
//
// | < ---- (2^rank) bytes ---- >|
//
// [ Chunk                       ]
// [ ChunkHeader_t ][ User space ]
// |                |
// Header ptr       User ptr
// =========================================================


// ====================================
// = Types definitions.
// ====================================
typedef uint8_t Rank_t;
typedef uint8_t BucketId_t;

struct ChunkHeader;
struct ChunkHeader {
	struct ChunkHeader* prev;
	struct ChunkHeader* next;
	Rank_t rank;
	bool busy;
	// TODO: No padding is used since no memory alignment restrictions are specified.
};

typedef struct ChunkHeader ChunkHdr_t;
typedef ChunkHdr_t DListNode_t;
#include "DList.h"


// ====================================
// = Static configuration.
// ====================================
#define __BUDDY_ALLOCATOR_RANK_MIN (Rank_t)(12)
#define __BUDDY_ALLOCATOR_RANK_RANGE (Rank_t)(20)

#define __BUDDY_ALLOCATOR_RANK_MAX (Rank_t)(__BUDDY_ALLOCATOR_RANK_MIN + __BUDDY_ALLOCATOR_RANK_RANGE)
#define __BUDDY_ALLOCATOR_CAPACITY_MAX (size_t)(SIZE_MAX - sizeof(ChunkHdr_t))


typedef struct {
	DList_t buckets[__BUDDY_ALLOCATOR_RANK_RANGE];
	void* raw_memory_ptr;
	Rank_t raw_memory_rank;
} BuddyAllocator_t;


// ====================================
// = Private methods.
// ====================================

/**
 * @param ins The buddy allocator instance pointer. MUST NOT be null.
 * @return The maximum chunk size that can be allocated.
 */
size_t buddy_allocator_capacity_max(const BuddyAllocator_t* const ins) {
	return (1ull << ins->raw_memory_rank) - sizeof(ChunkHdr_t);
}

/**
 * Translates a user pointer to its header pointer including NULL.
 */
static inline ChunkHdr_t* __buddy_allocator_header_ptr(void* user_ptr) {
	ChunkHdr_t* result = NULL;
	if(user_ptr) {
		uint8_t* const u8ptr = (uint8_t* const)user_ptr;
		result = (ChunkHdr_t*)(u8ptr - sizeof(ChunkHdr_t));
	}
	return result;
}

/**
 * Translates a header pointer to its user pointer including NULL.
 */
static inline void* __buddy_allocator_user_ptr(ChunkHdr_t* const chunk) {
	void* result = NULL;
	if(chunk) {
		uint8_t* const u8ptr = (uint8_t* const) chunk;
		result = (void*) (u8ptr + sizeof(ChunkHdr_t));
	}
	return result;
}

/**
 * Checks if a value is any power of two.
 */
static inline int __buddy_allocator_is_po2(const size_t value) {
	return value && ((value & (value - 1u)) == 0);
}

/**
 * Rank calculation. ceil(log2(capacity))
 */
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

/**
 * Calculates the buddy pointer.
 * May return NULL.
 */
static inline ChunkHdr_t* __buddy_allocator_buddy(
	BuddyAllocator_t* const ins, ChunkHdr_t* const chunk
                                                    ) {
	ChunkHdr_t* result = NULL;
	if(chunk->rank < ins->raw_memory_rank) {
		uint8_t* const raw_mem_u8ptr = (uint8_t* const) (ins->raw_memory_ptr);
		const uint8_t* const chunk_u8ptr = (const uint8_t* const) chunk;
		size_t offset = chunk_u8ptr - raw_mem_u8ptr;
		offset ^= 1ull << chunk->rank;
		result = (ChunkHdr_t*) (raw_mem_u8ptr + offset);
	}
	return result;
}


/**
 * Pushes a chunk to the free list.
 */
static inline void __buddy_allocator_push_chunk(BuddyAllocator_t* const ins, ChunkHdr_t* const chunk) {
	const BucketId_t bucket = chunk->rank - __BUDDY_ALLOCATOR_RANK_MIN;
	ChunkHdr_t* const buddy = __buddy_allocator_buddy(ins, chunk);

	if(buddy && !(buddy->busy) && buddy->rank == chunk->rank) {
		ChunkHdr_t* const parent = chunk < buddy ? chunk : buddy;
		dlist_remove(ins->buckets + bucket, buddy);
		parent->rank++;
		__buddy_allocator_push_chunk(ins, parent);
	} else {
		chunk->busy = false;
		dlist_push_front(ins->buckets + bucket, chunk);
	}

}

/**
 * Pops a chunk from the free list.
 * May returns NULL.
 */
static inline ChunkHdr_t* __buddy_allocator_pop_chunk(BuddyAllocator_t* const ins, const Rank_t rank) {
	ChunkHdr_t* result = NULL;
	if(rank >= __BUDDY_ALLOCATOR_RANK_MIN && rank <= ins->raw_memory_rank) {
		const BucketId_t bucket = rank - __BUDDY_ALLOCATOR_RANK_MIN;
		DList_t* const list = ins->buckets + bucket;

		if(dlist_empty(list)) {

			result = __buddy_allocator_pop_chunk(ins, (Rank_t) (rank + 1u));
			if(result) {
				result->rank = rank;

				ChunkHdr_t* const buddy = __buddy_allocator_buddy(ins, result);
				if(buddy) {
					buddy->rank = rank;
					buddy->busy = false;
					dlist_push_front(ins->buckets + bucket, buddy);
				}

			}

		} else {
			result = dlist_pop_front(list);
			result->busy = true;
		}

	}
	return result;
}

/**
 * @warning For debug purposes only.
 */
void __buddy_allocator_dump_chunk(const BuddyAllocator_t* const ins, const ChunkHdr_t* chunk) {
	const uint8_t* const raw_mem_u8ptr = (const uint8_t* const)(ins->raw_memory_ptr);
	const uint8_t* head_u8ptr = (const uint8_t*)(chunk);
	printf("[ Offset=%zu Rank=%u Busy=%d] -> ", head_u8ptr - raw_mem_u8ptr, chunk->rank, chunk->busy);
}

void __buddy_allocator_dump_bucket(const BuddyAllocator_t* const ins, const BucketId_t bucket) {
	const DList_t* const list = ins->buckets + bucket;
	const ChunkHdr_t* head = list->head;
	while(head) {
		__buddy_allocator_dump_chunk(ins, head);
		head = head->next;
	}
	printf("\n");
}

/**
 * @warning For debug purposes only.
 * @param ins The buddy allocator instance pointer. MUST NOT be null.
 */
void __buddy_allocator_dump(const BuddyAllocator_t* const ins) {
	printf("==== Buddy Allocator instance ====\n");
	printf("Struct ptr            : %p\n", ins);
	printf("BuddyAllocator_t size : %zu\n", sizeof(*ins));
	printf("ChunkHeader_t size    : %zu\n", sizeof(ChunkHdr_t));
	printf("Raw mem ptr           : %p\n", ins->raw_memory_ptr);
	printf("Raw mem rank          : %u\n", ins->raw_memory_rank);
	printf("Max capacity          : %zu\n", buddy_allocator_capacity_max(ins));

	Rank_t rank = ins->raw_memory_rank;
	while(rank >= __BUDDY_ALLOCATOR_RANK_MIN) {
		const Rank_t bucket = rank - __BUDDY_ALLOCATOR_RANK_MIN;
		const size_t size = 1ull << rank;

		printf("[ Bucket=%-2u", bucket);
		printf("  Rank=%-2u", rank);
		printf("  Size=%-8zu ] : ", size);

		__buddy_allocator_dump_bucket(ins, bucket);
		rank--;
	}
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
BuddyAllocator_t* buddy_allocator_create(void* raw_memory, const size_t raw_memory_size) {
	BuddyAllocator_t* result = NULL;

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

				ChunkHdr_t* const chunk = (ChunkHdr_t*)raw_memory;
				chunk->rank = rank;
				__buddy_allocator_push_chunk(result, chunk);
			}
		}
	}
	return result;
}

/**
* Destroy a buddy allocator
* @param ins The buddy allocator instance pointer. MUST NOT be null.
*/
void buddy_allocator_destroy(BuddyAllocator_t* const ins) {
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
void* buddy_allocator_alloc(BuddyAllocator_t* const ins, size_t size) {
	void* result = NULL;
	if(size < __BUDDY_ALLOCATOR_CAPACITY_MAX) {
		size += sizeof(ChunkHdr_t);
		Rank_t rank = __buddy_allocator_rank(size);
		if(rank <= ins->raw_memory_rank) {
			if(rank < __BUDDY_ALLOCATOR_RANK_MIN) {
				rank = __BUDDY_ALLOCATOR_RANK_MIN;
			}
			ChunkHdr_t* const chunk = __buddy_allocator_pop_chunk(ins, rank);
			result = __buddy_allocator_user_ptr(chunk);
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
void buddy_allocator_free(BuddyAllocator_t* const ins, void* const raw_ptr) {
	ChunkHdr_t* const chunk = __buddy_allocator_header_ptr(raw_ptr);
	if(chunk && chunk->busy) {
		__buddy_allocator_push_chunk(ins, chunk);
	}
}
