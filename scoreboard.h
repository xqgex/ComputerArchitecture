#ifndef SCOREBOARD_H_
#define SCOREBOARD_H_

#include "defines.h"
#include "inst_queue.h"
#include "input_output.h"

/* This function runs the pipeline
 * The function return SUCCESS (true) on sucess and FAILURE (false) otherwise */
bool scoreboard_cycle(stScoreboardCPU *, char *);
/* Destroy the scoreboard */
void scoreboard_destroy(stScoreboardCPU *, configuration *, int);
/* This function initializes the scoreboard module
 * The function return SUCCESS (true) on sucess and FAILURE (false) otherwise */
bool scoreboard_init(stScoreboardCPU *, configuration *, uint32_t *, int);

#endif /* SCOREBOARD_H_ */
