#ifndef DEFINES_H_
#define DEFINES_H_
#ifdef _WIN32
#include <io.h>
#include "unistd.h"
#else /* Linux */
#define _GNU_SOURCE /* Define S_IFDIR and getline() for Linux systems */
#endif /* _WIN32 */
#include <errno.h>
#include <math.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>

#define DEBUG				false /* Turn on the printing */

#define CHECK_INPUT_FILE		true
#define CHECK_OUTPUT_FILE		false
#define CONFIGURATION_INT_MAX_VALUE	4096
#define CONFIGURATION_NAME_MAX_LENGTH	15
#define CONFIGURATION_STR_MAX_LENGTH	5
#define EXECUTE_END_STAGE_IDX		2
#define FAILURE				false
#define INSTRUCTION_QUEUE_LENGTH	16
#define ISSUE_STAGE_IDX			0
#define MAX_LENGTH_OF_FU_NAME		5
#define MEMORY_SIZE			4096
#define NUM_OF_REGISTERS		16
#define READ_REGISTERS_STAGE_IDX	1
#define SUCCESS				true
#define WRITE_RESULT_STAGE_IDX		3

typedef struct configuration_Name	configuration;
typedef struct command_row_t_Name	command_row_t;
typedef struct InstQ_name		InstQ;
typedef struct Node_t			NODE;
typedef struct stFunctionalUnit_Name	stFunctionalUnit;
typedef struct stScoreboardCPU_name	stScoreboardCPU;
typedef union  utInstruction_name	utInstruction;

typedef enum {
	OPCODE_LD	= 0,
	OPCODE_ST	= 1,
	OPCODE_ADD	= 2,
	OPCODE_SUB	= 3,
	OPCODE_MULT	= 4,
	OPCODE_DIV	= 5,
	OPCODE_HALT	= 6,
} opcode_t;

typedef enum {
	REGISTER_F0	= 0,
	REGISTER_F1	= 1,
	REGISTER_F2	= 2,
	REGISTER_F3	= 3,
	REGISTER_F4	= 4,
	REGISTER_F5	= 5,
	REGISTER_F6	= 6,
	REGISTER_F7	= 7,
	REGISTER_F8	= 8,
	REGISTER_F9	= 9,
	REGISTER_F10	= 10,
	REGISTER_F11	= 11,
	REGISTER_F12	= 12,
	REGISTER_F13	= 13,
	REGISTER_F14	= 14,
	REGISTER_F15	= 15,
} reg_t;

/************************************************************************/
/*	Struct's declaration						*/
/************************************************************************/
/* Configuration struct */
struct configuration_Name {
	unsigned short int add_nr_units;		/* How many ADD operation we have */
	unsigned short int sub_nr_units;		/* How many SUB operation we have */
	unsigned short int mul_nr_units;		/* How many MUL operation we have */
	unsigned short int div_nr_units;		/* How many DIV operation we have */
	unsigned short int ld_nr_units;			/* How many LD operation we have */
	unsigned short int st_nr_units;			/* How many ST operation we have */
	unsigned short int add_delay;			/* The delay for ADD operation */
	unsigned short int sub_delay;			/* The delay for SUB operation */
	unsigned short int mul_delay;			/* The delay for MUL operation */
	unsigned short int div_delay;			/* The delay for DIV operation */
	unsigned short int ld_delay;			/* The delay for LD operation */
	unsigned short int st_delay;			/* The delay for ST operation */
	char trace_unit[CONFIGURATION_STR_MAX_LENGTH+1];/* Command name for the traceunit file */
};

/* Command struct */
struct command_row_t_Name {
	opcode_t opcode;				/* Command OPCODE */
	reg_t dst;					/* Command destination register */
	reg_t src0;					/* Command source 0 register */
	reg_t src1;					/* Command source 1 register */
	unsigned short int imm;				/* Command immediate */
};

/* The HEAD of the Queue, hold the amount of node's that are in the queue */
struct InstQ_name {
	NODE* head;
	NODE* tail;
	int size;
	int limit;
};

/* A link in the queue, holds the info and point to the next Node */
struct Node_t {
	command_row_t  m_Instr;
	struct Node_t* prev;
};

struct stFunctionalUnit_Name {
	bool			Busy;
	opcode_t		m_op;
	reg_t			Fi;
	reg_t			Fj;
	reg_t			Fk;
	stFunctionalUnit*	Qj;
	stFunctionalUnit*	Qk;
	bool			Rj;
	bool			Rk;
	/* These are not part of the classic scoreboard, but used for the simlutaion */
	char			fuName[MAX_LENGTH_OF_FU_NAME];
	float			tempRegister;
	unsigned short int	tempImmidiate;
	unsigned short int	time_left;
	int			relatedPC;
};

struct stScoreboardCPU_name {
	configuration*		pConfiguration;
	uint32_t*		pMemory;
	uint32_t		PC;
	uint32_t		cycle;
	uint32_t		lastIssuedPC;
	float			Register[NUM_OF_REGISTERS];
	uint32_t*		instructionStatus[4]; /* For logging the instruction cycles */
	char**			instructionFUs;	/* For logging the instruction active FUs */
	stFunctionalUnit*	pTracedUnit; /* Pointer to the traced functional unit */
	stFunctionalUnit*	registerResultStatus[NUM_OF_REGISTERS];
	InstQ*			InstructionQ;
	stFunctionalUnit*	pLD_FUs;
	stFunctionalUnit*	pST_FUs;
	stFunctionalUnit*	pADD_FUs;
	stFunctionalUnit*	pSUB_FUs;
	stFunctionalUnit*	pMULT_FUs;
	stFunctionalUnit*	pDIV_FUs;
	bool			halted;
};

union utInstruction_name {
	uint32_t command;
	struct {
		uint32_t unusedbits : 4;
		uint32_t opcode : 4;
		uint32_t dst : 4;
		uint32_t src0 : 4;
		uint32_t src1 : 4;
		uint32_t imm : 12;
	} inner;
};

#endif /* DEFINES_H_ */
