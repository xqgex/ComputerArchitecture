#ifndef INST_QUEUE_H_
#define INST_QUEUE_H_

#include "defines.h"

InstQ* InstQ_ConstructQueue(int);
void InstQ_DestructQueue(InstQ *);
bool InstQ_Enqueue(InstQ *, command_row_t);
bool InstQ_Peek(InstQ *, command_row_t *);
bool InstQ_Pop(InstQ *);

#endif /* INST_QUEUE_H_ */
