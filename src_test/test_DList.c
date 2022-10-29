#include "test_environment.h"

#include <stdint.h>

struct DummyNode1;
struct DummyNode1 {
	struct DummyNode1* prev;
	struct DummyNode1* next;
	uint64_t user_data;
};

typedef struct DummyNode1 DListNode_t;

#include "../src/DList.h"

void test_dlist_dump(const DList_t* const ins);

void test_dlist_push_back_pop_back(
	DList_t* const list,
	DListNode_t* const storage,
	const size_t storage_nb
                                  ) {
	TRACE_CALL;
	assert(dlist_size(list) == 0);
	for(size_t i = 0; i < storage_nb; ++i) {
		DListNode_t* node_push = storage + i;
		dlist_push_back(list, node_push);
		DListNode_t* node_pop = NULL;
		node_pop = dlist_pop_back(list);
		assert(node_push == node_pop);
	}
	assert(dlist_size(list) == 0);

	for(size_t i = 0; i < storage_nb; ++i) {
		DListNode_t* node_push = storage + i;
		dlist_push_back(list, node_push);
	}
	assert(dlist_size(list) == storage_nb);

	for(size_t i = storage_nb - 1; i < storage_nb; --i) {
		DListNode_t* node_pop = NULL;
		node_pop = dlist_pop_back(list);
		assert(node_pop == storage + i);
	}
	assert(dlist_size(list) == 0);
}

void test_dlist_push_back_pop_front(
	DList_t* const list,
	DListNode_t* const storage,
	const size_t storage_nb) {

	TRACE_CALL;
	assert(dlist_size(list) == 0);
	for(size_t i = 0; i < storage_nb; ++i) {
		DListNode_t* node_push = storage + i;
		dlist_push_back(list, node_push);
		DListNode_t* node_pop = dlist_pop_front(list);
		assert(node_push == node_pop);
	}
	assert(dlist_size(list) == 0);

	for(size_t i = 0; i < storage_nb; ++i) {
		DListNode_t* node_push = storage + i;
		dlist_push_back(list, node_push);
	}
	assert(dlist_size(list) == storage_nb);

	for(size_t i = 0; i < storage_nb; ++i) {
		DListNode_t* node_pop = dlist_pop_front(list);
		assert(node_pop == storage + i);
	}
	assert(dlist_size(list) == 0);
}

void test_dlist_push_front_pop_back(
	DList_t* const list,
	DListNode_t* const storage,
	const size_t storage_nb) {

	TRACE_CALL;
	assert(dlist_size(list) == 0);
	for(size_t i = 0; i < storage_nb; ++i) {
		DListNode_t* node_push = storage + i;
		dlist_push_front(list, node_push);
		DListNode_t* node_pop = dlist_pop_back(list);
		assert(node_push == node_pop);
	}
	assert(dlist_size(list) == 0);

	for(size_t i = 0; i < storage_nb; ++i) {
		DListNode_t* node_push = storage + i;
		dlist_push_front(list, node_push);
	}
	assert(dlist_size(list) == storage_nb);

	for(size_t i = 0; i < storage_nb; ++i) {
		DListNode_t* node_pop = dlist_pop_back(list);
		assert(node_pop == storage + i);
	}
	assert(dlist_size(list) == 0);
}

void test_dlist_push_front_pop_front(
	DList_t* const list,
	DListNode_t* const storage,
	const size_t storage_nb
                                    ) {

	TRACE_CALL;
	assert(dlist_size(list) == 0);
	for(size_t i = 0; i < storage_nb; ++i) {
		DListNode_t* node_push = storage + i;
		dlist_push_front(list, node_push);
		DListNode_t* node_pop = dlist_pop_front(list);
		assert(node_push == node_pop);
	}
	assert(dlist_size(list) == 0);

	for(size_t i = 0; i < storage_nb; ++i) {
		DListNode_t* node_push = storage + i;
		dlist_push_front(list, node_push);
	}
	assert(dlist_size(list) == storage_nb);

	for(size_t i = storage_nb - 1; i < storage_nb; --i) {
		DListNode_t* node_pop = dlist_pop_front(list);
		assert(node_pop == storage + i);
	}
	assert(dlist_size(list) == 0);
}

void test_dlist_push_before(
	DList_t* const list,
	DListNode_t* const storage,
	const size_t storage_nb
                           ) {

	TRACE_CALL;
	assert(dlist_size(list) == 0);
	DListNode_t* node_base = storage + 0;
	dlist_push_front(list, node_base);
	assert(dlist_size(list) == 1);

	for(size_t i = 1; i < storage_nb; ++i) {
		DListNode_t* node_to_insert = storage + i;
		dlist_push_before(list, node_base, node_to_insert);
		DListNode_t* node_pop = dlist_pop_front(list);
		assert(node_to_insert == node_pop);
	}
	assert(dlist_size(list) == 1);

	for(size_t i = 1; i < storage_nb; ++i) {
		DListNode_t* node_to_insert = storage + i;
		dlist_push_before(list, node_base, node_to_insert);
	}
	assert(dlist_size(list) == storage_nb);
	dlist_remove(list, node_base);
	assert(dlist_size(list) == storage_nb - 1);

	for(size_t i = 1; i < storage_nb; ++i) {
		DListNode_t* node_pop = dlist_pop_front(list);
		assert(node_pop == storage + i);
	}
	assert(dlist_size(list) == 0);
}

void test_dlist_push_after(
	DList_t* const list,
	DListNode_t* const storage,
	const size_t storage_nb
                          ) {

	TRACE_CALL;
	assert(dlist_size(list) == 0);
	DListNode_t* node_base = storage + 0;
	dlist_push_front(list, node_base);
	assert(dlist_size(list) == 1);

	for(size_t i = 1; i < storage_nb; ++i) {
		DListNode_t* node_to_insert = storage + i;
		dlist_push_after(list, node_base, node_to_insert);
		DListNode_t* node_pop = dlist_pop_back(list);
		assert(node_to_insert == node_pop);
	}
	assert(dlist_size(list) == 1);

	for(size_t i = 1; i < storage_nb; ++i) {
		DListNode_t* node_to_insert = storage + i;
		dlist_push_after(list, node_base, node_to_insert);
	}
	assert(dlist_size(list) == storage_nb);
	dlist_remove(list, node_base);
	assert(dlist_size(list) == storage_nb - 1);

	for(size_t i = 1; i < storage_nb; ++i) {
		DListNode_t* node_pop = dlist_pop_back(list);
		assert(node_pop == storage + i);
	}
	assert(dlist_size(list) == 0);
}

void test_dlist_remove(
	DList_t* const list,
	DListNode_t* const storage,
	const size_t storage_nb
                      ) {

	TRACE_CALL;
	assert(dlist_size(list) == 0);
	for(size_t i = 0; i < storage_nb; ++i) {
		DListNode_t* node_push = storage + i;
		dlist_push_front(list, node_push);
		dlist_remove(list, node_push);
	}
	assert(dlist_size(list) == 0);

	for(size_t i = 0; i < storage_nb; ++i) {
		DListNode_t* node_push = storage + i;
		dlist_push_front(list, node_push);
	}
	assert(dlist_size(list) == storage_nb);

	for(size_t i = storage_nb - 1; i < storage_nb; --i) {
		dlist_remove(list, storage + i);
	}
	assert(dlist_size(list) == 0);

	for(size_t i = 0; i < storage_nb; ++i) {
		DListNode_t* node_push = storage + i;
		dlist_push_front(list, node_push);
	}
	assert(dlist_size(list) == storage_nb);

	for(size_t i = storage_nb - 1; i < storage_nb; --i) {
		if(i % 2 == 0) {
			dlist_remove(list, storage + i);
		}
	}
	for(size_t i = storage_nb - 1; i < storage_nb; --i) {
		if(i % 2 != 0) {
			dlist_remove(list, storage + i);
		}
	}
	assert(dlist_size(list) == 0);
}

void test_dlist_reset(
	DList_t* const list,
	DListNode_t* const storage,
	const size_t storage_nb
                     ) {

	TRACE_CALL;
	assert(dlist_size(list) == 0);

	for(size_t i = 0; i < storage_nb; ++i) {
		DListNode_t* node_push = storage + i;
		dlist_push_front(list, node_push);
	}
	assert(dlist_size(list) == storage_nb);

	dlist_reset(list);
	assert(dlist_size(list) == 0);
}

void test_dlist_dump(const DList_t* const ins) {
	printf("<DList> has %zu elements \n", dlist_size(ins));
	DListNode_t* head = ins->head;
	while(head) {
		printf(" -> [%zu]", head->user_data);
		head = head->next;
	}
	printf("\n");
}


int main() {
	TRACE_CALL;
	static const unsigned STORAGE_SIZE = 16;
	DListNode_t storage[STORAGE_SIZE];
	DList_t list;

	dlist_init(&list);
	test_dlist_push_back_pop_back(&list, storage, STORAGE_SIZE);
	test_dlist_push_back_pop_front(&list, storage, STORAGE_SIZE);
	test_dlist_push_front_pop_back(&list, storage, STORAGE_SIZE);
	test_dlist_push_front_pop_front(&list, storage, STORAGE_SIZE);
	test_dlist_push_before(&list, storage, STORAGE_SIZE);
	test_dlist_push_after(&list, storage, STORAGE_SIZE);
	test_dlist_remove(&list, storage, STORAGE_SIZE);
	test_dlist_reset(&list, storage, STORAGE_SIZE);
	return 0;
}
