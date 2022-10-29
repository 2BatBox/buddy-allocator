#pragma once

#include <memory.h>
#include <stdlib.h>
#include <stdint.h>

// ====================================
// = Types definitions.
// ====================================

/**
 * An intrusive way implemented doubly linked list.
 *
 * The intrusive DListNode_t contract:
 * DListNode_t structure MUST BE defined before including this header file.
 * DListNode_t structure MUST HAVE the following fields:
 * DListNode_t* prev;
 * DListNode_t* next;
 */
typedef struct {
	DListNode_t* head;
	DListNode_t* tail;
} DList_t ;


// ====================================
// = Private methods.
// ====================================

static inline void __dlist_link_first(DList_t* const ins, DListNode_t* const node) {
	node->next = NULL;
	node->prev = NULL;
	ins->head = ins->tail = node;
}

static inline void __dlist_link_head(DList_t* const ins, DListNode_t* const node) {
	node->next = ins->head;
	node->prev = NULL;
	ins->head->prev = node;
	ins->head = node;
}

static inline void __dlist_link_tail(DList_t* const ins, DListNode_t* const node) {
	node->next = NULL;
	node->prev = ins->tail;
	ins->tail->next = node;
	ins->tail = node;
}

static inline DListNode_t* __dlist_unlink_last(DList_t* const ins) {
	DListNode_t* const result = ins->head;
	ins->head = ins->tail = NULL;
	return result;
}

static inline DListNode_t* __dlist_unlink_head(DList_t* const ins) {
	DListNode_t* const result = ins->head;
	ins->head = ins->head->next;
	ins->head->prev = NULL;
	return result;
}

static inline DListNode_t* __dlist_unlink_tail(DList_t* const ins) {
	DListNode_t* const result = ins->tail;
	ins->tail = ins->tail->prev;
	ins->tail->next = NULL;
	return result;
}

static inline void __dlist_link_before(DListNode_t* const before, DListNode_t* const node) {
	node->next = before;
	node->prev = before->prev;
	before->prev->next = node;
	before->prev = node;
}

static inline void __dlist_link_after(DListNode_t* const after, DListNode_t* const node) {
	node->next = after->next;
	node->prev = after;
	after->next->prev = node;
	after->next = node;
}

static inline void __dlist_unlink(DListNode_t* const node) {
	node->prev->next = node->next;
	node->next->prev = node->prev;
}


// ====================================
// = Public methods.
// ====================================

/**
 * Initialize the DList structure instance.
 * @param ins - must not be NULL.
 */
void dlist_init(DList_t* const ins) {
	if(ins) {
		memset(ins, 0, sizeof(*ins));
	}
}

/**
 * Attach a new node to the head of the list.
 * @param ins - must not be NULL.
 * @param node - must not be attached to any lists before the calling.
 */
void dlist_push_front(DList_t* const ins, DListNode_t* const node) {
	if(ins->head) {
		__dlist_link_head(ins, node);
	} else {
		__dlist_link_first(ins, node);
	}
}

/**
 * Attach a new node to the tail of the list.
 * @param ins - must not be NULL.
 * @param node - must not be attached to the list before the calling.
 */
void dlist_push_back(DList_t* const ins, DListNode_t* const node) {
	if(ins->tail) {
		__dlist_link_tail(ins, node);
	} else {
		__dlist_link_first(ins, node);
	}
}

/**
 * Detach the head node of the list.
 * @param ins - must not be NULL.
 * @return - a pointer of the detached node or NULL in case of the list is empty.
 */
DListNode_t* dlist_pop_front(DList_t* const ins) {
	DListNode_t* result = NULL;
	if(ins->head != ins->tail) {
		result = __dlist_unlink_head(ins);
	} else if(ins->head) {
		result = __dlist_unlink_last(ins);
	}
	return result;
}

/**
 * Detach the tail node of the list.
 * @param ins - must not be NULL.
 * @return - a pointer of the detached node or NULL in case of the list is empty.
 */
DListNode_t* dlist_pop_back(DList_t* const ins) {
	DListNode_t* result = NULL;
	if(ins->head != ins->tail) {
		result = __dlist_unlink_tail(ins);
	} else if(ins->head) {
		result = __dlist_unlink_last(ins);
	}
	return result;
}

/**
 * Attach @node just before node @before in the list.
 * @param ins - must not be NULL.
 * @param before - must be attached to the list.
 * @param node - must not be attached to the list before the calling.
 */
void dlist_push_before(DList_t* const ins, DListNode_t* const before, DListNode_t* const node) {
	if(before == ins->head) {
		__dlist_link_head(ins, node);
	} else {
		__dlist_link_before(before, node);
	}
}

/**
 * Attach @node just after node @after in the list.
 * @param ins - must not be NULL.
 * @param after - must be attached to the list.
 * @param node - must not be attached to the list before the calling.
 */
void dlist_push_after(DList_t* const ins, DListNode_t* const after, DListNode_t* const node) {
	if(after == ins->tail) {
		__dlist_link_tail(ins, node);
	} else {
		__dlist_link_after(after, node);
	}
}

/**
 * Detach the given node.
 * @param ins - must not be NULL.
 * @param node - must be attached to the list before the calling.
 */
void dlist_remove(DList_t* const ins, DListNode_t* const node) {
	if(ins->head) {
		if(node == ins->head) {
			dlist_pop_front(ins);
		} else if(node == ins->tail) {
			dlist_pop_back(ins);
		} else {
			__dlist_unlink(node);
		}
	}
}

/**
 * Unlink all the nodes which the list contains.
 * @param ins - must not be NULL.
 */
void dlist_reset(DList_t* const ins) {
	ins->head = NULL;
	ins->tail = NULL;
}

/**
 * Check if the list is empty.
 * @param ins - must not be NULL.
 * @return non zero value in case empty list.
 */
int dlist_empty(DList_t* const ins) {
	return (ins->head == NULL);
}

/**
 * Calculate the number of nodes the list contains.
 * Warning: the time complexity of the method is O(N), where N is a number of nodes.
 * @param ins - must not be NULL.
 */
size_t dlist_size(const DList_t* const ins) {
	const DListNode_t* head = ins->head;
	size_t result = 0;
	while(head) {
		result++;
		head = head->next;
	}
	return result;
}
