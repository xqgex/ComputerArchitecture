#include "inst_queue.h"

/************************************************************************/
/*	Private functions prototypes					*/
/************************************************************************/
bool InstQ_Dequeue(InstQ *, command_row_t *);
bool InstQ_IsEmpty(InstQ *);

/************************************************************************/
/*	Public functions declaration					*/
/************************************************************************/
InstQ* InstQ_ConstructQueue(int limit) {
	InstQ* queue = (InstQ*) malloc(sizeof(InstQ));
	if (!queue) {
		return NULL;
	}
	queue->limit = limit;
	queue->size = 0;
	queue->head = NULL;
	queue->tail = NULL;
	return queue;
}

void InstQ_DestructQueue(InstQ* pQueue) {
	command_row_t Instr;
	while (!InstQ_IsEmpty(pQueue)) {
		if (InstQ_Dequeue(pQueue, &Instr) == FAILURE) {
			printf("[Error] Failed to dequeue\n");
		}
	}
	free(pQueue);
}

bool InstQ_Enqueue(InstQ* pQueue, command_row_t Instr) {
	/* Check parameter */
	if ( (pQueue == NULL) || (pQueue->size >= pQueue->limit)) {
		return FAILURE;
	}
	NODE* item = (NODE*) malloc(sizeof(NODE));
	if (!item) {
		return FAILURE;
	}
	item->m_Instr = Instr;
	item->prev = NULL;
	if (pQueue->size == 0) {
		/* If the queue is empty */
		pQueue->head = item;
		pQueue->tail = item;
	} else {
		/* Else, Add item to the end of the queue */
		pQueue->tail->prev = item;
		pQueue->tail = item;
	}
	pQueue->size++;
	return SUCCESS;
}

bool InstQ_Peek(InstQ* pQueue, command_row_t* pInstr) {
	if (InstQ_IsEmpty(pQueue)) {
		return FAILURE;
	}
	NODE* item = pQueue->head;
	*pInstr = item->m_Instr;
	return SUCCESS;
}

bool InstQ_Pop(InstQ* pQueue) {
	if (InstQ_IsEmpty(pQueue)) { /* The queue is empty or bad param */
		return FAILURE;
	}
	NODE* item = pQueue->head;
	pQueue->head = (pQueue->head)->prev;
	free(item);
	pQueue->size--;
	return SUCCESS;
}

/************************************************************************/
/*	Private functions declaration					*/
/************************************************************************/
bool InstQ_Dequeue(InstQ* pQueue, command_row_t* pInstr) {
	if (InstQ_IsEmpty(pQueue)) { /* The queue is empty or bad param */
		return FAILURE;
	}
	NODE* item = pQueue->head;
	*pInstr = item->m_Instr;
	pQueue->head = (pQueue->head)->prev;
	free(item);
	pQueue->size--;
	return SUCCESS;
}

bool InstQ_IsEmpty(InstQ* pQueue) {
	if (pQueue == NULL) {
		return false;
	}
	if (pQueue->size == 0) {
		return true;
	} else {
		return false;
	}
}
