#include "input_output.h"

/************************************************************************/
/*	Private functions prototypes					*/
/************************************************************************/
/* Function to remove all spaces and tabs from a given string
 * Source: https://stackoverflow.com/a/1514740 */
void trim(char *, char **);
/* The function recieved a configuration line with 'unsigned short int',
 * if its a valid configuration line, the parsed value will be stored at 'output_value'
 * The function return SUCCESS (true) on sucess and FAILURE (false) otherwise */
bool validate_conf_num(char *, char *, unsigned short int *);
/* The function recieved a configuration line with 'char*',
 * if its a valid configuration line, the parsed value will be stored at 'output_value'
 * The function return SUCCESS (true) on sucess and FAILURE (false) otherwise */
bool validate_conf_str(char *, char *, char []);

#ifdef _WIN32
size_t getline(char **, size_t *, FILE *);
#endif /* _WIN32 */
/************************************************************************/
/*	Public functions declaration					*/
/************************************************************************/
/* The function check 'path' for existance and optional check for read and/or write permission
 * The function return SUCCESS (true) on sucess and FAILURE (false) otherwise */
bool check_files_permission(char* message, char* path, bool path_is_input) {
	struct stat st;
	FILE *fp;
	if (stat(path, &st) == -1) { /* Upon successful completion, these functions shall return 0. Otherwise, these functions shall return -1 and set errno to indicate the error. */
		if (path_is_input == false) {
			return SUCCESS;
		}
		if (errno == ENOENT) {
			printf("[Error] %s file '%s' does not exist\n", message, path);
		} else if (errno == EACCES) {
			printf("[Error] %s file '%s' is not accessible\n", message, path);
		}
		return FAILURE;
	}
	/* Check if path is a directory */
	if (st.st_mode & S_IFDIR) {
		printf("[Error] %s file '%s' is a directory\n", message, path);
	}
	/* Check read access. */
	if ((fp = fopen(path, "r")) == NULL) { /* Upon successful completion fopen() return a FILE pointer. Otherwise, NULL is returned and errno is set to indicate the error. */
		printf("[Error] %s file '%s' is not readable (access denied)\n", message, path);
		return FAILURE;
	}
	fclose(fp);
	/* Check write access. */
	if (path_is_input == false) {
		if ((fp = fopen(path, "w")) == NULL) { /* Upon successful completion fopen() return a FILE pointer. Otherwise, NULL is returned and errno is set to indicate the error. */
			if (errno == EACCES) {
				printf("[Error] %s file '%s' is not writable (access denied)\n", message, path);
			} else if (errno == EROFS) {
				printf("[Error] %s file '%s' is not writable (read-only filesystem)\n", message, path);
			}
			return FAILURE;
		}
		fclose(fp);
	}
	return SUCCESS;
}

/* This function used to convert int SAFELY to register enum as defined in reg_t */
/* The function must received a valid int between 0 to 15 */
reg_t int_to_reg(int input) {
	switch (input) {
		case 0: return REGISTER_F0;
		case 1: return REGISTER_F1;
		case 2: return REGISTER_F2;
		case 3: return REGISTER_F3;
		case 4: return REGISTER_F4;
		case 5: return REGISTER_F5;
		case 6: return REGISTER_F6;
		case 7: return REGISTER_F7;
		case 8: return REGISTER_F8;
		case 9: return REGISTER_F9;
		case 10: return REGISTER_F10;
		case 11: return REGISTER_F11;
		case 12: return REGISTER_F12;
		case 13: return REGISTER_F13;
		case 14: return REGISTER_F14;
		case 15: return REGISTER_F15;
	}
	return REGISTER_F0;
}

/* Load the configuration from file 'path', validate and parse it
 * The function return SUCCESS (true) on sucess and FAILURE (false) otherwise */
bool load_cfg(char* path, configuration* config) {
	FILE* fp;
	char* line = NULL;
	char* trimed_line = (char*) malloc(sizeof(char) * 4096);
	size_t len = 0;
	ssize_t read = 0;
	uint16_t readed_config = 0; /* Start as '0000 0000 0000 0000' */
	if (!trimed_line) {
		printf("[Fatal] Configuration parsing malloc failed\n");
		return EXIT_FAILURE;
	}
	fp = fopen(path, "r"); /* Upon successful completion fopen() return a FILE pointer. Otherwise, NULL is returned and errno is set to indicate the error. */
	if (fp == NULL) {
		perror("fopen");
		free(trimed_line);
		return FAILURE;
	}
	while ((read = getline(&line, &len, fp)) != -1) {
		trim(line, &trimed_line);
		if (((readed_config & 0x1) == 0) && (validate_conf_num("add_nr_units=%hu", trimed_line, &(config->add_nr_units)) == SUCCESS)) { /* add_nr_units */
			readed_config = readed_config | 0x1; /* Turn bit '0000 0000 0000 0001' on */
		} else if (((readed_config & 0x2) == 0) && (validate_conf_num("sub_nr_units=%hu", trimed_line, &(config->sub_nr_units)) == SUCCESS)) { /* sub_nr_units */
			readed_config = readed_config | 0x2; /* Turn bit '0000 0000 0000 0010' on */
		} else if (((readed_config & 0x4) == 0) && (validate_conf_num("mul_nr_units=%hu", trimed_line, &(config->mul_nr_units)) == SUCCESS)) { /* mul_nr_units */
			readed_config = readed_config | 0x4; /* Turn bit '0000 0000 0000 0100' on */
		} else if (((readed_config & 0x8) == 0) && (validate_conf_num("div_nr_units=%hu", trimed_line, &(config->div_nr_units)) == SUCCESS)) { /* div_nr_units */
			readed_config = readed_config | 0x8; /* Turn bit '0000 0000 0000 1000' on */
		} else if (((readed_config & 0x10) == 0) && (validate_conf_num("ld_nr_units=%hu", trimed_line, &(config->ld_nr_units)) == SUCCESS)) { /* ld_nr_units */
			readed_config = readed_config | 0x10; /* Turn bit '0000 0000 0001 0000' on */
		} else if (((readed_config & 0x20) == 0) && (validate_conf_num("st_nr_units=%hu", trimed_line, &(config->st_nr_units)) == SUCCESS)) { /* st_nr_units */
			readed_config = readed_config | 0x20; /* Turn bit '0000 0000 0010 0000' on */
		} else if (((readed_config & 0x40) == 0) && (validate_conf_num("add_delay=%hu", trimed_line, &(config->add_delay)) == SUCCESS)) { /* add_delay */
			readed_config = readed_config | 0x40; /* Turn bit '0000 0000 0100 0000' on */
		} else if (((readed_config & 0x80) == 0) && (validate_conf_num("sub_delay=%hu", trimed_line, &(config->sub_delay)) == SUCCESS)) { /* sub_delay */
			readed_config = readed_config | 0x80; /* Turn bit '0000 0000 1000 0000' on */
		} else if (((readed_config & 0x100) == 0) && (validate_conf_num("mul_delay=%hu", trimed_line, &(config->mul_delay)) == SUCCESS)) { /* mul_delay */
			readed_config = readed_config | 0x100; /* Turn bit '0000 0001 0000 0000' on */
		} else if (((readed_config & 0x200) == 0) && (validate_conf_num("div_delay=%hu", trimed_line, &(config->div_delay)) == SUCCESS)) { /* div_delay */
			readed_config = readed_config | 0x200; /* Turn bit '0000 0010 0000 0000' on */
		} else if (((readed_config & 0x400) == 0) && (validate_conf_num("ld_delay=%hu", trimed_line, &(config->ld_delay)) == SUCCESS)) { /* ld_delay */
			readed_config = readed_config | 0x400; /* Turn bit '0000 0100 0000 0000' on */
		} else if (((readed_config & 0x800) == 0) && (validate_conf_num("st_delay=%hu", trimed_line, &(config->st_delay)) == SUCCESS)) { /* st_delay */
			readed_config = readed_config | 0x800; /* Turn bit '0000 1000 0000 0000' on */
		} else if (((readed_config & 0x1000) == 0) && (validate_conf_str("trace_unit", trimed_line, config->trace_unit) == SUCCESS)) { /* trace_unit */
			readed_config = readed_config | 0x1000; /* Turn bit '0001 0000 0000 0000' on */
		} else if (strlen(trimed_line) > 0) {
			printf("[Error] Configuration file have invalid lines\n");
			free(line);
			free(trimed_line);
			fclose(fp);
			return FAILURE;
		}
	}
	if (line) {
		free(line);
	}
	free(trimed_line);
	fclose(fp);
	if (readed_config == 0x1FFF) { /* Found all the configuration line '0001 1111 1111 1111' */
		return SUCCESS;
	} else {
		printf("[Error] Configuration file have missing lines\n");
		return FAILURE;
	}
}

/* Load the memory image from file 'path'
 * The function return SUCCESS (true) on sucess and FAILURE (false) otherwise */
bool load_memin(char* path, unsigned int memory[]) {
	FILE* fp;
	int line_count = 0;
	unsigned int temp = 0;
	char* line = NULL;
	size_t len = 0;
	ssize_t read = 0;
	fp = fopen(path, "r"); /* Upon successful completion fopen() return a FILE pointer. Otherwise, NULL is returned and errno is set to indicate the error. */
	if (fp == NULL) {
		perror("fopen");
		return FAILURE;
	}
	while ((read = getline(&line, &len, fp)) != -1) {
		if (sscanf(line, "%x", &temp) != 1) {
			free(line);
			fclose(fp);
			return FAILURE;
		}
		memory[line_count] = temp;
		line_count++;
	}
	if (line) {
		free(line);
	}
	fclose(fp);
	return SUCCESS;
}

/* This function parse a memory line into a valid command
 * The function return SUCCESS (true) on sucess and FAILURE (false) otherwise */
bool parse_opcode(unsigned int memory_line, command_row_t* command) {
	/* Bits 28-31 should be equal to zero */
	int temp = (memory_line >> 28) & 0xF;
	if (temp != 0) {
		return FAILURE;
	}
	/* Bits 24-27 represent the command OPCODE */
	temp = (memory_line >> 24) & 0xF;
	switch (temp) {
		case 0: command->opcode = OPCODE_LD;
			break;
		case 1: command->opcode = OPCODE_ST;
			break;
		case 2: command->opcode = OPCODE_ADD;
			break;
		case 3: command->opcode = OPCODE_SUB;
			break;
		case 4: command->opcode = OPCODE_MULT;
			break;
		case 5: command->opcode = OPCODE_DIV;
			break;
		case 6: command->opcode = OPCODE_HALT;
			break;
			return FAILURE;
	}
	/* Bits 20-23 represent the DST register */
	temp = (memory_line >> 20) & 0xF;
	if (temp < 0 || NUM_OF_REGISTERS <= temp) {
		return FAILURE;
	}
	command->dst = int_to_reg(temp);
	/* Bits 16-19 represent the SRC0 register */
	temp = (memory_line >> 16) & 0xF;
	if (temp < 0 || NUM_OF_REGISTERS <= temp) {
		return FAILURE;
	}
	command->src0 = int_to_reg(temp);
	/* Bits 12-15 represent the SRC1 register */
	temp = (memory_line >> 12) & 0xF;
	if (temp < 0 || NUM_OF_REGISTERS <= temp) {
		return FAILURE;
	}
	command->src1 = int_to_reg(temp);
	/* Bits 0-11 represent the IMM number */
	command->imm = (unsigned short int)(memory_line & 0xFFF);
	return SUCCESS;
}

/* Print program usage message */
void print_usage() {
	printf("Usage: sim <cfg> <memin> <memout> <regout> <traceinst> <traceunit>\n");
	printf("Simulator for floating point processor using the scoreboard algorithm.\n\n");
	printf("Mandatory arguments:\n");
	printf("  cfg                  Input file, Path to the file containing the configuration.\n");
	printf("  memin                Input file, Path to the file containing the memory image.\n");
	printf("  memout               Output file, Where to store the memory image when the program end.\n");
	printf("  regout               Output file, Where to store the registry image when the program end.\n");
	printf("  traceinst            Output file, Where to store the list of executed commands.\n");
	printf("  traceunit            Output file, Where to store the list of trace_unit busy clock cycles.\n");
}

/* Write the memory image to file 'path'
 * The function return SUCCESS (true) on sucess and FAILURE (false) otherwise */
bool write_memout(char* path, unsigned int memory[]) {
	FILE* fp;
	int line_count = 0;
	fp = fopen(path, "w"); /* Upon successful completion fopen() return a FILE pointer. Otherwise, NULL is returned and errno is set to indicate the error. */
	if (fp == NULL) {
		perror("fopen");
		return FAILURE;
	}
	for (line_count=0; line_count<MEMORY_SIZE; ++line_count) {
		fprintf(fp, "%.8x\n", memory[line_count]);
	}
	fclose(fp);
	return SUCCESS;
}

/* Write the register content to file 'path'
 * The function return SUCCESS (true) on sucess and FAILURE (false) otherwise */
bool write_regout(char* path, float registers[]) {
	FILE* fp;
	int line_count = 0;
	fp = fopen(path, "w"); /* Upon successful completion fopen() return a FILE pointer. Otherwise, NULL is returned and errno is set to indicate the error. */
	if (fp == NULL) {
		perror("fopen");
		return FAILURE;
	}
	for (line_count=0; line_count<NUM_OF_REGISTERS; ++line_count) {
		fprintf(fp, "%.6f\n", registers[line_count]);
	}
	fclose(fp);
	return SUCCESS;
}

/* Write the traceinst fields to file 'path'
 * The function return SUCCESS (true) on sucess and FAILURE (false) otherwise */
bool write_traceinst(char* path, stScoreboardCPU* a_pThis) {
	FILE* fp;
	command_row_t cmd;
	int line_count = 0;
	fp = fopen(path, "w"); /* Upon successful completion fopen() return a FILE pointer. Otherwise, NULL is returned and errno is set to indicate the error. */
	if (fp == NULL) {
		perror("fopen");
		return FAILURE;
	}
	for (line_count=0; line_count<MEMORY_SIZE; ++line_count) {
		parse_opcode(a_pThis->pMemory[line_count], &cmd);
		if (cmd.opcode == OPCODE_HALT) {
			break;
		}
		fprintf(fp, "%08x %d %s %d %d %d %d\n",
							a_pThis->pMemory[line_count],
							line_count,
							a_pThis->instructionFUs[line_count],
							a_pThis->instructionStatus[0][line_count],
							a_pThis->instructionStatus[1][line_count],
							a_pThis->instructionStatus[2][line_count],
							a_pThis->instructionStatus[3][line_count]
			);
	}
	fclose(fp);
	return SUCCESS;
}

//Function that returns the number of instruction from the memory beginning until the HALT command
unsigned int get_instructionNum(unsigned int memory[])
{
	command_row_t currCmd;
	unsigned int i;
	for (i  = 0; i < MEMORY_SIZE; i++)
	{
		parse_opcode(memory[i], &currCmd);
		if (currCmd.opcode == OPCODE_HALT)
			break;
	}
	return i;
}

/************************************************************************/
/*	Private functions declaration					*/
/************************************************************************/
/* Function to remove all spaces and tabs from a given string
 * Code is based on: https://stackoverflow.com/a/1514740 */
void trim(char* src, char** buff) {
	if (4096 < strlen(src)) {
		(*buff)[0] = '\0';
		return;
	}
	char* current = src;
	unsigned int sizeBuff = 0, i = 0;
	current[strcspn(current, "\r\n")] = '\0'; /* Remove \r and \r\n */
	sizeBuff = strlen(current);
	if (sizeBuff < 1) {
		(*buff)[0] = '\0';
		return;
	}
	while ((*current != '\0') && (i<sizeBuff-1)) {
		if ((*current != ' ') && (*current != '\t')) {
			(*buff)[i++] = *current;
		}
		++current;
	}
	(*buff)[i] = '\0';
}
/* The function recieved a configuration line with 'unsigned short int',
 * if its a valid configuration line, the parsed value will be stored at 'output_value'
 * The function return SUCCESS (true) on sucess and FAILURE (false) otherwise */
bool validate_conf_num(char* property_name, char* input_line, unsigned short int* output_value) {
	unsigned short int extracted_value = 0;
	/* Parse the input line */
	if ((input_line == NULL) || (strlen(input_line) < 1)) {
		return FAILURE;
	}
	if (sscanf(input_line, property_name, &extracted_value) != 1) { /* sscanf() Return the number of input items successfully matched and assigned */
		return FAILURE;
	}
	/* Check max value */
	if (CONFIGURATION_INT_MAX_VALUE < extracted_value) {
		return FAILURE;
	}
	/* Save the property value */
	*output_value = extracted_value;
	return SUCCESS;
}

/* The function recieved a configuration line with 'char*',
 * if its a valid configuration line, the parsed value will be stored at 'output_value'
 * The function return SUCCESS (true) on sucess and FAILURE (false) otherwise */
bool validate_conf_str(char* property_name, char* input_line, char output_value[]) {
	char format[32] = {0};
	char extracted_value[CONFIGURATION_STR_MAX_LENGTH+1] = {0};
	/* Parse the input line */
	if (snprintf(format, sizeof(format), "%s=%%%ds", property_name, CONFIGURATION_STR_MAX_LENGTH) < 5) { /* snprintf() Upon successful return the number of characters printed */
		return FAILURE;
	}
	if (sscanf(input_line, format, extracted_value) != 1) { /* sscanf() Return the number of input items successfully matched and assigned */
		return FAILURE;
	}
	/* Save the property value */
	strncpy(output_value, extracted_value, CONFIGURATION_STR_MAX_LENGTH+1);
	return SUCCESS;
}

#ifdef _WIN32
size_t getline(char** lineptr, size_t* n, FILE* stream) {
	char* bufptr = NULL;
	char* p = bufptr;
	size_t size = 0;
	int c = 0;
	if (lineptr == NULL) {
		return -1;
	}
	if (stream == NULL) {
		return -1;
	}
	if (n == NULL) {
		return -1;
	}
	bufptr = *lineptr;
	size = *n;
	c = fgetc(stream);
	if (c == EOF) {
		return -1;
	}
	if (bufptr == NULL) {
		bufptr = malloc(128);
		if (bufptr == NULL) {
			return -1;
		}
		size = 128;
	}
	p = bufptr;
	while (c != EOF) {
		if ((p - bufptr) > (size - 1)) {
			size = size + 128;
			bufptr = realloc(bufptr, size);
			if (bufptr == NULL) {
				return -1;
			}
		}
		*p++ = c;
		if (c == '\n') {
			break;
		}
		c = fgetc(stream);
	}
	*p++ = '\0';
	*lineptr = bufptr;
	*n = size;
	return p - bufptr - 1;
}
#endif /* _WIN32 */
