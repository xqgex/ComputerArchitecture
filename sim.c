#include "sim.h"

int main(int argc, char** argv) {
	/* Check input arguments */
	if (argc != 7) {
		print_usage();
		return EXIT_FAILURE;
	}
	if ((check_files_permission("cfg",	argv[1], CHECK_INPUT_FILE) == FAILURE) |
	    (check_files_permission("memin",	argv[2], CHECK_INPUT_FILE) == FAILURE) |
	    (check_files_permission("memout",	argv[3], CHECK_OUTPUT_FILE) == FAILURE) |
	    (check_files_permission("regout",	argv[4], CHECK_OUTPUT_FILE) == FAILURE) |
	    (check_files_permission("traceinst",argv[5], CHECK_OUTPUT_FILE) == FAILURE) |
	    (check_files_permission("traceunit",argv[6], CHECK_OUTPUT_FILE) == FAILURE)) {
		return EXIT_FAILURE;
	}
	configuration* config = (configuration*) malloc(sizeof(configuration));
	if (!config) {
		printf("[Fatal] Configuration malloc failed\n");
		free(config);
		return EXIT_FAILURE;
	}
	if (load_cfg(argv[1], config) == FAILURE) {
		printf("[Fatal] Parsing configuration file failed\n");
		free(config);
		return EXIT_FAILURE;
	}
	unsigned int memory[MEMORY_SIZE] = {0};
	if (load_memin(argv[2], memory) == FAILURE) {
		printf("[Fatal] Parsing memin file failed\n");
		free(config);
		return EXIT_FAILURE;
	}

	unsigned int instructionNum = get_instructionNum(memory);
	/* Start the program */
	stScoreboardCPU scoreboardCPU;
	if (scoreboard_init(&scoreboardCPU, config, memory, instructionNum) == FAILURE) {
		return EXIT_FAILURE;
	}
	if (scoreboard_cycle(&scoreboardCPU, argv[6]) == FAILURE) {
		printf("[Fatal] Scoreboard algorithm cycles failed\n");
		scoreboard_destroy(&scoreboardCPU, config, MEMORY_SIZE);
		return EXIT_FAILURE;
	}
	/* Save the output */
	if (write_memout(argv[3], memory) == FAILURE) {
		printf("[Fatal] Saving memout file failed\n");
		scoreboard_destroy(&scoreboardCPU, config, MEMORY_SIZE);
		return EXIT_FAILURE;
	}
	if (write_regout(argv[4], scoreboardCPU.Register) == FAILURE) {
		printf("[Fatal] Saving memout file failed\n");
		scoreboard_destroy(&scoreboardCPU, config, MEMORY_SIZE);
		return EXIT_FAILURE;
	}
	if (write_traceinst(argv[5], &scoreboardCPU) == FAILURE) {
		printf("[Fatal] Saving traceinst file failed\n");
		scoreboard_destroy(&scoreboardCPU, config, MEMORY_SIZE);
		return EXIT_FAILURE;
	}
	/* Cleanup and exit gracefully */
	scoreboard_destroy(&scoreboardCPU, config, instructionNum);
	return EXIT_SUCCESS; 
}
