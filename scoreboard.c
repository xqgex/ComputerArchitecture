#include "scoreboard.h"

/************************************************************************/
/*	Private functions prototypes					*/
/************************************************************************/
/* This function checks if the simulation ended and return true if so, otherwise return false */
bool scoreboard_checkExitConditions(stScoreboardCPU *, command_row_t);
/* Convert Single-precision floating-point format to int
 * The integer represent the 32-bit IEEE 754 format of the decimal value */
uint32_t scoreboard_convertFloatToInt(float);
/* Convert int to Single-precision floating-point format
 * The integer represent the 32-bit IEEE 754 format of the decimal value */
float scoreboard_convertIntToFloat(uint32_t);
/* This function performs the execution stage of the scorecoard.
 * This function is called until the execution is done */
void scoreboard_execution(stScoreboardCPU *);
/* This function fetches the next instruction from memory into the instruction Queue
 * The function return SUCCESS (true) on sucess and FAILURE (false) otherwise */
bool scoreboard_fetch(stScoreboardCPU *);
/* This function looks for an avaliabale Functional Unit based on the opcode
 * and returns the pointer to it, If there isn't avilable FU return NULL */
stFunctionalUnit* scoreboard_getAvailableFU(stScoreboardCPU *, opcode_t);
/* This function Initializes the FUs and finds the traced */
void scoreboard_initializeFUs(stScoreboardCPU *);
/* This function performs the issue stage of the scorecoard, Before issuing the command it checks if
 * there is no structural hazard (FU is available) and that there is no output dependency (WAW)
 * The function return SUCCESS (true) on sucess and FAILURE (false) otherwise */
bool scoreboard_issue(stScoreboardCPU *, command_row_t);
/* This function performs the operation of the Functional unit*/
void scoreboard_performOper(stScoreboardCPU *, stFunctionalUnit *);
/* This function print to the user the received command */
void scoreboard_printCommand(command_row_t *);
/* This function performs the Read operands stage of the scorecoard. It checks if there is no
 * data hazards (RAW) before starting the execution (first cycle of execution is done here) */
void scoreboard_readOperands(stScoreboardCPU *);
/* This function performs the Write results stage of the scorecoard.
 * It stalls until there is no WAR hazrad with previous instructions */
void scoreboard_writeResults(stScoreboardCPU *);
/* This function writes to the traceunit file in the right format */
void scoreboard_writeTracedUnit(stScoreboardCPU *, FILE *);

/************************************************************************/
/*	Public functions declaration					*/
/************************************************************************/
/* This function runs the pipeline
 * The function return SUCCESS (true) on sucess and FAILURE (false) otherwise */
bool scoreboard_cycle(stScoreboardCPU* a_pThis, char *path) {
	command_row_t currCmd;
	FILE* fp;
	fp = fopen(path, "w"); /* Upon successful completion fopen() return a FILE pointer. Otherwise, NULL is returned and errno is set to indicate the error. */
	if (fp == NULL) {
		perror("fopen");
		return FAILURE;
	}
	while (true) {
		if (a_pThis->halted != true) {
			if (scoreboard_fetch(a_pThis) == FAILURE) {
				printf("FAILURE scoreboard_fetch, currCmd.opcode=%d\n", currCmd.opcode);
				return FAILURE;
			}
		}
		if (InstQ_Peek(a_pThis->InstructionQ, &currCmd) == SUCCESS) { /* Issue the next command in queue only if it's valid */
			if (scoreboard_issue(a_pThis, currCmd) == SUCCESS) { /* If the issue was successful we pop the command from the Q */
				if (DEBUG) {
					scoreboard_printCommand(&(a_pThis->InstructionQ->head->m_Instr));
				}
				if (InstQ_Pop(a_pThis->InstructionQ) == FAILURE) {
					printf("FAILURE InstQ_Pop\n");
					return FAILURE;
				}
			}
		}
		scoreboard_readOperands(a_pThis);
		scoreboard_execution(a_pThis);
		scoreboard_writeResults(a_pThis);
		scoreboard_writeTracedUnit(a_pThis, fp);
		if (scoreboard_checkExitConditions(a_pThis, currCmd)) { /* scoreboard_checkExitConditions() return true if the simulation ended */
			break;
		} else {
			a_pThis->cycle++;
		}
	}
	// closed the traceunit file
	fclose(fp);
	return SUCCESS;
}

/* Destroy the scoreboard */
void scoreboard_destroy(stScoreboardCPU* a_pThis, configuration* a_pConfiguration, int a_NumberOfInstructions) {
	int i = 0;
	free(a_pThis->pLD_FUs);
	free(a_pThis->pST_FUs);
	free(a_pThis->pADD_FUs);
	free(a_pThis->pSUB_FUs);
	free(a_pThis->pMULT_FUs);
	free(a_pThis->pDIV_FUs);
	for (i=0; i<4; ++i) {
		free(a_pThis->instructionStatus[i]);
	}
	for (i=0; i<a_NumberOfInstructions; ++i) {
		free(a_pThis->instructionFUs[i]);
	}
	InstQ_DestructQueue(a_pThis->InstructionQ);
	free(a_pThis->instructionFUs);
	free(a_pConfiguration);
}

/* This function initializes the scoreboard module
 * The function return SUCCESS (true) on sucess and FAILURE (false) otherwise */
bool scoreboard_init(stScoreboardCPU* a_pThis, configuration* a_pConfiguration, uint32_t* a_pMemory, int a_NumberOfInstructions) {
	size_t i = 0;
	a_pThis->pConfiguration = a_pConfiguration;
	a_pThis->pMemory = a_pMemory;
	a_pThis->PC = 0;
	a_pThis->cycle = 1; /* we start from cycle No. 1 */
	a_pThis->lastIssuedPC = 0;
	a_pThis->InstructionQ = InstQ_ConstructQueue(INSTRUCTION_QUEUE_LENGTH);
	a_pThis->pTracedUnit = NULL;
	a_pThis->halted = false;
	scoreboard_initializeFUs(a_pThis);
	if (a_pThis->pTracedUnit == NULL) {
		printf("[Error] Did not found the requested traced unit\n");
		return FAILURE;
	}
	/* Allocating mem for the instruction FU strings (for trace purposes) */
	a_pThis->instructionFUs = calloc(a_NumberOfInstructions, sizeof(char*));
	for (i=0; i<(size_t)a_NumberOfInstructions; ++i) {
		a_pThis->instructionFUs[i] = calloc(MAX_LENGTH_OF_FU_NAME, sizeof(char));
	}
	if (!a_pThis->pLD_FUs || !a_pThis->pST_FUs || !a_pThis->pADD_FUs || !a_pThis->pSUB_FUs || !a_pThis->pMULT_FUs || !a_pThis->pDIV_FUs || !a_pThis->InstructionQ) {
		return FAILURE;
	}
	for (i=0; i<4; ++i) {
		a_pThis->instructionStatus[i] = calloc(a_NumberOfInstructions, sizeof(uint32_t));
		if (!a_pThis->instructionStatus[i]) {
			return FAILURE;
		}
	}
	for (i=0; i<NUM_OF_REGISTERS; ++i) {
		a_pThis->Register[i] = i;
		a_pThis->registerResultStatus[i] = NULL;
	}
	return SUCCESS;
}

/************************************************************************/
/*	Private functions declaration					*/
/************************************************************************/
/* This function checks if the simulation ended and return true if so, otherwise return false */
bool scoreboard_checkExitConditions(stScoreboardCPU* a_pThis, command_row_t a_cmd) {
	size_t i = 0;
	if ((a_cmd.opcode == OPCODE_HALT) || (a_pThis->PC == (MEMORY_SIZE - 1))) { /* If we got the HALT opcode or read all the memory */
		for (i=0; i<NUM_OF_REGISTERS; ++i) {
			if (a_pThis->registerResultStatus[i] != NULL) { /* Check for active FUs */
				return false;
			}
		}
		return true; /* Means that no FU's are active (all are done) */
	} else {
		return false;
	}
}

/* Convert Single-precision floating-point format to int
 * The integer represent the 32-bit IEEE 754 format of the decimal value */
uint32_t scoreboard_convertFloatToInt(float input) {
	unsigned char a[4];
	int i = 0;
	unsigned int res = 0;
	memcpy(a, &input, 4);
	for (i=3; 0<=i; --i) {
		res = res << 8;
		res += (int)a[i];
	}
	return (uint32_t)res; /* scoreboard_convertFloatToInt(0.085) == 1034818683; */
}

/* Convert int to Single-precision floating-point format
 * The integer represent the 32-bit IEEE 754 format of the decimal value */
float scoreboard_convertIntToFloat(uint32_t input) {
	unsigned char a[4];
	memcpy(a, &input, 4);
	float* f=(float*)a;
	return (float)*f; /* scoreboard_convertIntToFloat(1034818683) == 0.085; */
}

/* This function performs the execution stage of the scorecoard.
 * This function is called until the execution is done */
void scoreboard_execution(stScoreboardCPU* a_pThis) {
	size_t i = 0;
	for (i=0; i<NUM_OF_REGISTERS; ++i) {
		if (a_pThis->registerResultStatus[i] != NULL) { /* Check for active FUs */
			int relatedPC = a_pThis->registerResultStatus[i]->relatedPC;
			if (a_pThis->instructionStatus[READ_REGISTERS_STAGE_IDX][relatedPC] != a_pThis->cycle) { /* Check that we are not executing in the same cycle as the read operands */
				if ((a_pThis->instructionStatus[READ_REGISTERS_STAGE_IDX][relatedPC] != 0) && a_pThis->registerResultStatus[i]->Busy && (a_pThis->registerResultStatus[i]->time_left != 0)) { /* Check the Execution conditions (and read operands stage is already done) */
					a_pThis->registerResultStatus[i]->time_left--;
					if (a_pThis->registerResultStatus[i]->time_left == 0) {
						a_pThis->instructionStatus[EXECUTE_END_STAGE_IDX][relatedPC] = a_pThis->cycle;
					}
				}
			}
		}
	}
}

/* This function fetches the next instruction from memory into the instruction Queue
 * The function return SUCCESS (true) on sucess and FAILURE (false) otherwise */
bool scoreboard_fetch(stScoreboardCPU* a_pThis) {
	command_row_t cmd;
	/* Load & parse PC instruction from memin into the Q */
	if (parse_opcode(a_pThis->pMemory[a_pThis->PC], &cmd) == FAILURE) {
		return FAILURE;
	}
	if (InstQ_Enqueue(a_pThis->InstructionQ, cmd) == SUCCESS) { /* If enqueue was successful - increase PC */
		if (cmd.opcode == OPCODE_HALT) {
			a_pThis->halted = true;
		} else if (a_pThis->PC < (MEMORY_SIZE - 1)) {
			a_pThis->PC++;
		} else {
			return FAILURE;
		}
	} else {
		return FAILURE;
	}
	return SUCCESS;
}

/* This function looks for an avaliabale Functional Unit based on the opcode
 * and returns the pointer to it, If there isn't avilable FU return NULL */
stFunctionalUnit* scoreboard_getAvailableFU(stScoreboardCPU* a_pThis, opcode_t a_opcode) {
	size_t i = 0;
	switch (a_opcode) {
		case OPCODE_LD:
			for (i=0; i<a_pThis->pConfiguration->ld_nr_units; ++i) {
				if (a_pThis->pLD_FUs[i].Busy == false) {
					strcpy(a_pThis->instructionFUs[a_pThis->lastIssuedPC], a_pThis->pLD_FUs[i].fuName);
					a_pThis->pLD_FUs[i].time_left = a_pThis->pConfiguration->ld_delay;
					return &a_pThis->pLD_FUs[i];
				}
			}
			return NULL; /* Means no FU is available */
		case OPCODE_ST:
			for (i=0; i<a_pThis->pConfiguration->st_nr_units; ++i) {
				if (a_pThis->pST_FUs[i].Busy == false) {
					strcpy(a_pThis->instructionFUs[a_pThis->lastIssuedPC], a_pThis->pST_FUs[i].fuName);
					a_pThis->pST_FUs[i].time_left = a_pThis->pConfiguration->st_delay;
					return &a_pThis->pST_FUs[i];
				}
			}
			return NULL; /* Means no FU is available */
		case OPCODE_ADD:
			for (i=0; i<a_pThis->pConfiguration->add_nr_units; ++i) {
				if (a_pThis->pADD_FUs[i].Busy == false) {
					strcpy(a_pThis->instructionFUs[a_pThis->lastIssuedPC], a_pThis->pADD_FUs[i].fuName);
					a_pThis->pADD_FUs[i].time_left = a_pThis->pConfiguration->add_delay;
					return &a_pThis->pADD_FUs[i];
				}
			}
			return NULL; /* Means no FU is available */
		case OPCODE_SUB:
			for (i=0; i<a_pThis->pConfiguration->sub_nr_units; ++i) {
				if (a_pThis->pSUB_FUs[i].Busy == false) {
					strcpy(a_pThis->instructionFUs[a_pThis->lastIssuedPC], a_pThis->pSUB_FUs[i].fuName);
					a_pThis->pSUB_FUs[i].time_left = a_pThis->pConfiguration->sub_delay;
					return &a_pThis->pSUB_FUs[i];
				}
			}
			return NULL; /* Means no FU is available */
		case OPCODE_MULT:
			for (i=0; i<a_pThis->pConfiguration->mul_nr_units; ++i) {
				if (a_pThis->pMULT_FUs[i].Busy == false) {
					strcpy(a_pThis->instructionFUs[a_pThis->lastIssuedPC], a_pThis->pMULT_FUs[i].fuName);
					a_pThis->pMULT_FUs[i].time_left = a_pThis->pConfiguration->mul_delay;
					return &a_pThis->pMULT_FUs[i];
				}
			}
			return NULL; /* Means no FU is available */
		case OPCODE_DIV:
			for (i=0; i<a_pThis->pConfiguration->div_nr_units; ++i) {
				if (a_pThis->pDIV_FUs[i].Busy == false) {
					strcpy(a_pThis->instructionFUs[a_pThis->lastIssuedPC], a_pThis->pDIV_FUs[i].fuName);
					a_pThis->pDIV_FUs[i].time_left = a_pThis->pConfiguration->div_delay;
					return &a_pThis->pDIV_FUs[i];
				}
			}
			return NULL; /* Means no FU is available */
		default:
			return NULL;
	}
}

/* This function Initializes the FUs and finds the traced */
void scoreboard_initializeFUs(stScoreboardCPU* a_pThis) {
	size_t i = 0;
	char fuIdx[2] = { 0 }; //Index is max 2 char 
	a_pThis->pLD_FUs = calloc(a_pThis->pConfiguration->ld_nr_units, sizeof(stFunctionalUnit));
	a_pThis->pST_FUs = calloc(a_pThis->pConfiguration->st_nr_units, sizeof(stFunctionalUnit));
	a_pThis->pADD_FUs = calloc(a_pThis->pConfiguration->add_nr_units, sizeof(stFunctionalUnit));
	a_pThis->pSUB_FUs = calloc(a_pThis->pConfiguration->sub_nr_units, sizeof(stFunctionalUnit));
	a_pThis->pMULT_FUs = calloc(a_pThis->pConfiguration->mul_nr_units, sizeof(stFunctionalUnit));
	a_pThis->pDIV_FUs = calloc(a_pThis->pConfiguration->div_nr_units, sizeof(stFunctionalUnit));
	for (i=0; i<a_pThis->pConfiguration->ld_nr_units; ++i) {
			a_pThis->pLD_FUs[i].m_op = OPCODE_LD;
			strcpy(a_pThis->pLD_FUs[i].fuName, "LD");
			sprintf(fuIdx, "%ld", i);
			strcat(a_pThis->pLD_FUs[i].fuName, fuIdx);
			if (strcmp(a_pThis->pLD_FUs[i].fuName, a_pThis->pConfiguration->trace_unit) == 0) { /* strcmp() return an integer less than, equal to, or greater than zero if s1 is found */
				a_pThis->pTracedUnit = &a_pThis->pLD_FUs[i];
			}
	}
	for (i=0; i<a_pThis->pConfiguration->st_nr_units; ++i) {
			a_pThis->pST_FUs[i].m_op = OPCODE_ST;
			strcpy(a_pThis->pST_FUs[i].fuName, "ST");
			sprintf(fuIdx, "%ld", i);
			strcat(a_pThis->pST_FUs[i].fuName, fuIdx);
			if (strcmp(a_pThis->pST_FUs[i].fuName, a_pThis->pConfiguration->trace_unit) == 0) { /* strcmp() return an integer less than, equal to, or greater than zero if s1 is found */
				a_pThis->pTracedUnit = &a_pThis->pST_FUs[i];
			}
	}
	for (i=0; i<a_pThis->pConfiguration->add_nr_units; ++i) {
			a_pThis->pADD_FUs[i].m_op = OPCODE_ADD;
			strcpy(a_pThis->pADD_FUs[i].fuName, "ADD");
			sprintf(fuIdx, "%ld", i);
			strcat(a_pThis->pADD_FUs[i].fuName, fuIdx);
			if (strcmp(a_pThis->pADD_FUs[i].fuName, a_pThis->pConfiguration->trace_unit) == 0) { /* strcmp() return an integer less than, equal to, or greater than zero if s1 is found */
				a_pThis->pTracedUnit = &a_pThis->pADD_FUs[i];
			}
	}
	for (i=0; i<a_pThis->pConfiguration->sub_nr_units; ++i) {
			a_pThis->pSUB_FUs[i].m_op = OPCODE_SUB;
			strcpy(a_pThis->pSUB_FUs[i].fuName, "SUB");
			sprintf(fuIdx, "%ld", i);
			strcat(a_pThis->pSUB_FUs[i].fuName, fuIdx);
			if (strcmp(a_pThis->pSUB_FUs[i].fuName, a_pThis->pConfiguration->trace_unit) == 0) { /* strcmp() return an integer less than, equal to, or greater than zero if s1 is found */
				a_pThis->pTracedUnit = &a_pThis->pSUB_FUs[i];
			}
	}
	for (i=0; i<a_pThis->pConfiguration->mul_nr_units; ++i) {
			a_pThis->pMULT_FUs[i].m_op = OPCODE_MULT;
			strcpy(a_pThis->pMULT_FUs[i].fuName, "MUL");
			sprintf(fuIdx, "%ld", i);
			strcat(a_pThis->pMULT_FUs[i].fuName, fuIdx);
			if (strcmp(a_pThis->pMULT_FUs[i].fuName, a_pThis->pConfiguration->trace_unit) == 0) { /* strcmp() return an integer less than, equal to, or greater than zero if s1 is found */
				a_pThis->pTracedUnit = &a_pThis->pMULT_FUs[i];
			}
	}
	for (i=0; i<a_pThis->pConfiguration->div_nr_units; ++i) {
			a_pThis->pDIV_FUs[i].m_op = OPCODE_DIV;
			strcpy(a_pThis->pDIV_FUs[i].fuName, "DIV");
			sprintf(fuIdx, "%ld", i);
			strcat(a_pThis->pDIV_FUs[i].fuName, fuIdx);
			if (strcmp(a_pThis->pDIV_FUs[i].fuName, a_pThis->pConfiguration->trace_unit) == 0) { /* strcmp() return an integer less than, equal to, or greater than zero if s1 is found */
				a_pThis->pTracedUnit = &a_pThis->pDIV_FUs[i];
			}
	}
}

/* This function performs the issue stage of the scorecoard, Before issuing the command it checks if
 * there is no structural hazard (FU is available) and that there is no output dependency (WAW)
 * The function return SUCCESS (true) on sucess and FAILURE (false) otherwise */
bool scoreboard_issue(stScoreboardCPU* a_pThis, command_row_t a_cmd) {
	stFunctionalUnit* currFunctionalUnit = scoreboard_getAvailableFU(a_pThis, (opcode_t)a_cmd.opcode);
	if (a_cmd.opcode == OPCODE_HALT) {
		return FAILURE;
	}
	if (((a_pThis->registerResultStatus[a_cmd.dst] == NULL) || a_cmd.opcode == OPCODE_ST) && (currFunctionalUnit != NULL)) { /* Check the Issue conditions */
		currFunctionalUnit->Busy = true;
		currFunctionalUnit->Fi = a_cmd.dst;
		currFunctionalUnit->Fj = a_cmd.src0;
		currFunctionalUnit->Fk = a_cmd.src1;
		if (a_cmd.opcode == OPCODE_LD) { /* If it's memory related op, save the immidiate value */
			currFunctionalUnit->tempImmidiate = a_cmd.imm;
			currFunctionalUnit->Rj = true;
			currFunctionalUnit->Rk = true;
		}
		else if (a_cmd.opcode == OPCODE_ST)
		{
			currFunctionalUnit->tempImmidiate = a_cmd.imm;
			currFunctionalUnit->Qk = a_pThis->registerResultStatus[a_cmd.src1];
			currFunctionalUnit->Rk = (currFunctionalUnit->Qk == NULL);
			currFunctionalUnit->Rj = true;
		}
		else
		{
			currFunctionalUnit->Qj = a_pThis->registerResultStatus[a_cmd.src0];
			currFunctionalUnit->Qk = a_pThis->registerResultStatus[a_cmd.src1];
			currFunctionalUnit->Rj = (currFunctionalUnit->Qj == NULL);
			currFunctionalUnit->Rk = (currFunctionalUnit->Qk == NULL);
		}
		a_pThis->registerResultStatus[a_cmd.dst] = currFunctionalUnit;
		currFunctionalUnit->relatedPC = (a_pThis->lastIssuedPC++); /* We return current issued PC and increase it by one */
		a_pThis->instructionStatus[ISSUE_STAGE_IDX][currFunctionalUnit->relatedPC] = a_pThis->cycle; /* Save the issue stage cycle */

		return SUCCESS;
	} else {
		return FAILURE;
	}
}

/* This function performs the operation of the Functional unit */
void scoreboard_performOper(stScoreboardCPU* a_pThis, stFunctionalUnit* pFU) {
	switch (pFU->m_op) {
		case OPCODE_LD: pFU->tempRegister = scoreboard_convertIntToFloat(a_pThis->pMemory[pFU->tempImmidiate]); /* Convert int to Single-precision floating-point format */
				break;
		case OPCODE_ST: a_pThis->pMemory[pFU->tempImmidiate] = scoreboard_convertFloatToInt(a_pThis->Register[pFU->Fk]); /* Convert Single-precision floating-point format to int */
				break;
		case OPCODE_ADD:pFU->tempRegister = a_pThis->Register[pFU->Fj] + a_pThis->Register[pFU->Fk];
				break;
		case OPCODE_SUB:pFU->tempRegister = a_pThis->Register[pFU->Fj] - a_pThis->Register[pFU->Fk];
				break;
		case OPCODE_MULT:pFU->tempRegister = a_pThis->Register[pFU->Fj] * a_pThis->Register[pFU->Fk];
				break;
		case OPCODE_DIV:pFU->tempRegister = a_pThis->Register[pFU->Fj] / a_pThis->Register[pFU->Fk];
				break;
		default:
			break;
	}
}

/* This function print to the user the received command */
void scoreboard_printCommand(command_row_t* command) {
	switch (command->opcode) {
		case OPCODE_LD: printf("ld\tF[%d] = MEM[%d]\n", command->dst, command->imm);
			break;
		case OPCODE_ST: printf("st\tMEM[%d] = F[%d]\n", command->imm, command->src1);
			break;
		case OPCODE_ADD: printf("add.d\tF[%d] = F[%d] + F[%d]\n", command->dst, command->src0, command->src1);
			break;
		case OPCODE_SUB: printf("sub.d\tF[%d] = F[%d] - F[%d]\n", command->dst, command->src0, command->src1);
			break;
		case OPCODE_MULT: printf("mult.d\tF[%d] = F[%d] * F[%d]\n", command->dst, command->src0, command->src1);
			break;
		case OPCODE_DIV: printf("dev.d\tF[%d] = F[%d] / F[%d]\n", command->dst, command->src0, command->src1);
			break;
		case OPCODE_HALT: printf("halt\tExit simulator\n");
			break;
	}
}

/* This function performs the Read operands stage of the scorecoard. It checks if there is no
 * data hazards (RAW) before starting the execution (first cycle of execution is done here) */
void scoreboard_readOperands(stScoreboardCPU* a_pThis) {
	int relatedPC = 0;
	size_t i = 0;
	for (i=0; i<NUM_OF_REGISTERS; ++i) {
		if (a_pThis->registerResultStatus[i] != NULL) { /* Check for active FUs */
			stFunctionalUnit *currFU = a_pThis->registerResultStatus[i];
			relatedPC = currFU->relatedPC;
			if (a_pThis->instructionStatus[ISSUE_STAGE_IDX][relatedPC] != a_pThis->cycle) { /* Check that we are not reading the operands in the same cycle as the issue cycle */
				if (currFU->Busy && currFU->Rj && currFU->Rk) { /* Check the ReadOperands conditions */
					currFU->Rj = false;
					currFU->Rk = false;
					currFU->time_left--;
					scoreboard_performOper(a_pThis, currFU);
					a_pThis->instructionStatus[READ_REGISTERS_STAGE_IDX][relatedPC] = a_pThis->cycle;
				}
			}
		}
	}
}

/* This function performs the Write results stage of the scorecoard.
 * It stalls until there is no WAR hazrad with previous instructions */
void scoreboard_writeResults(stScoreboardCPU* a_pThis) {
	bool isReadyToWriteResult = true;
	int relatedPC = 0;
	size_t i = 0, j = 0;
	for (i=0; i<NUM_OF_REGISTERS; ++i) {
		if (a_pThis->registerResultStatus[i] != NULL) { /* Check for active FUs */
			stFunctionalUnit* currFU = a_pThis->registerResultStatus[i];
			relatedPC = currFU->relatedPC;
			if (a_pThis->instructionStatus[EXECUTE_END_STAGE_IDX][relatedPC] != a_pThis->cycle) { /* Check that we are not writing back in the same cycle as the executuin */
				if (currFU->Busy && (currFU->time_left == 0)) { /* Check that execution finished */
					isReadyToWriteResult = true;
					for (j=0; j<NUM_OF_REGISTERS; ++j) {
						if ((i != j) && (((currFU->Fi == currFU->Fj) && currFU->Rj) ||((currFU->Fi == currFU->Fk) && currFU->Rk))) { /* Check the WriteResults conditions */
							isReadyToWriteResult = false;
							break;
						}
					}
					if (isReadyToWriteResult) { /* Perform the WriteResults and update all the other FUs who wait for the data to be written */
						for (j=0; j<NUM_OF_REGISTERS; ++j) {
							if (a_pThis->registerResultStatus[j] != NULL) {
								if (currFU == a_pThis->registerResultStatus[j]->Qj) {
									a_pThis->registerResultStatus[j]->Rj = true;
									a_pThis->registerResultStatus[j]->Qj = NULL;
								}
								if (currFU == a_pThis->registerResultStatus[j]->Qk) {
									a_pThis->registerResultStatus[j]->Rk = true;
									a_pThis->registerResultStatus[j]->Qk = NULL;
								}
							}
						}
						if (currFU->m_op != OPCODE_ST) { /* If it is store command we are not writing back to register */
							a_pThis->Register[currFU->Fi] = currFU->tempRegister;
						}
						currFU->Busy = false;
						a_pThis->registerResultStatus[i] = NULL;
						a_pThis->instructionStatus[WRITE_RESULT_STAGE_IDX][relatedPC] = a_pThis->cycle;
					}
				}
			}
		}
	}
}

/* this function writes to the traceunit file in the right format */
void scoreboard_writeTracedUnit( stScoreboardCPU* a_pThis, FILE* fp) {
	if (a_pThis->pTracedUnit->Busy == true) {
		char nameFi[4] = {0};
		char nameFj[4] = {0};
		char nameFk[4] = {0};
		char nameQj[MAX_LENGTH_OF_FU_NAME] = {0};
		char nameQk[MAX_LENGTH_OF_FU_NAME] = {0};
		char nameRj[4] = {0};
		char nameRk[4] = {0};
		sprintf(nameFi, "F%d", (uint8_t)a_pThis->pTracedUnit->Fi);
		sprintf(nameFj, "F%d", (uint8_t)a_pThis->pTracedUnit->Fj);
		sprintf(nameFk, "F%d", (uint8_t)a_pThis->pTracedUnit->Fk);
		if (a_pThis->pTracedUnit->Qj == NULL) {
			strcpy(nameQj, "-");
		} else {
			sprintf(nameQj, "%s", a_pThis->pTracedUnit->Qj->fuName);
		}
		if (a_pThis->pTracedUnit->Qk == NULL) {
			strcpy(nameQk, "-");
		} else {
			sprintf(nameQk, "%s", a_pThis->pTracedUnit->Qk->fuName);
		}
		(a_pThis->pTracedUnit->Rj)? strcpy(nameRj, "Yes"): strcpy(nameRj, "No");
		(a_pThis->pTracedUnit->Rk) ? strcpy(nameRk, "Yes") : strcpy(nameRk, "No");
		fprintf(fp, "%d %s %s %s %s %s %s %s %s\n", a_pThis->cycle, a_pThis->pTracedUnit->fuName, nameFi, nameFj, nameFk, nameQj, nameQk, nameRj, nameRk);
	}
}
