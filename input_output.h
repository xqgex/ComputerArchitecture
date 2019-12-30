#ifndef INPUT_OUTPUT_H_
#define INPUT_OUTPUT_H_

#include "defines.h"

/* The function check 'path' for existance and optional check for read and/or write permission
 * The function return SUCCESS (true) on sucess and FAILURE (false) otherwise */
bool check_files_permission(char *, char *, bool);
/* This function used to convert int SAFELY to register enum as defined in reg_t */
/* The function must received a valid int between 0 to 15 */
reg_t int_to_reg(int input);
/* Load the configuration from file 'path', validate and parse it
 * The function return SUCCESS (true) on sucess and FAILURE (false) otherwise */
bool load_cfg(char *, configuration *);
/* Load the memory image from file 'path'
 * The function return SUCCESS (true) on sucess and FAILURE (false) otherwise */
bool load_memin(char *, unsigned int []);
/* This function parse a memory line into a valid command
 * The function return SUCCESS (true) on sucess and FAILURE (false) otherwise */
bool parse_opcode(unsigned int, command_row_t *);
/* Print program usage message */
void print_usage();
/* Write the memory image to file 'path'
 * The function return SUCCESS (true) on sucess and FAILURE (false) otherwise */
bool write_memout(char *, unsigned int []);
/* Write the register content to file 'path'
 * The function return SUCCESS (true) on sucess and FAILURE (false) otherwise */
bool write_regout(char *, float []);
/* Write the traceinst fields to file 'path'
 * The function return SUCCESS (true) on sucess and FAILURE (false) otherwise */
bool write_traceinst(char *, stScoreboardCPU *);
//Function that returns the number of instruction from the memory beginning until the HALT command
unsigned int get_instructionNum(unsigned int []);

#endif /* INPUT_OUTPUT_H_ */
