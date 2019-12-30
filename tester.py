#!/usr/bin/env python
# -*- coding: utf-8 -*-
import os

COMMAND = "./sim ./{tests_dir}/{test_dir}/cfg.txt ./{tests_dir}/{test_dir}/memin.txt ./{tests_dir}/{test_dir}/test_memout.txt ./{tests_dir}/{test_dir}/test_regout.txt ./{tests_dir}/{test_dir}/test_traceinst.txt ./{tests_dir}/{test_dir}/test_traceunit.txt"
TESTS_DIR = "Test_Files"
FILES = ["cfg.txt", "memin.txt", "memout.txt", "regout.txt", "traceinst.txt", "traceunit.txt"]

class bcolors:
	RED = '\033[91m'
	GREEN = '\033[92m'
	YELLOW = '\033[93m'
	BLUE = '\033[94m'
	ENDC = '\033[0m'

def color_print(color, string):
	print "{}[Python message] {}{}".format(color, string, bcolors.ENDC)

def compare_files(file_1, file_2):
	with open(file_1, "r") as infile_1:
		lines_1 = infile_1.readlines()
	with open(file_2, "r") as infile_2:
		lines_2 = infile_2.readlines()
	lines_1 = [l.strip().lower() for l in lines_1] # Fix file content
	lines_2 = [l.strip().lower() for l in lines_2] # Fix file content
	lines_1 += ["00000000"]*(max(len(lines_2), len(lines_1))-len(lines_1)) # Fix file length
	lines_2 += ["00000000"]*(max(len(lines_2), len(lines_1))-len(lines_2)) # Fix file length
	# Loop
	for line_num in range(len(lines_1)):
		if lines_1[line_num] != lines_2[line_num]:
			color_print(bcolors.RED, "{} At line {} found {} instead of {}".format(file_1.split("/")[-1], line_num, lines_2[line_num], lines_1[line_num]))
			return False
	return True

def main():
	list_tests = os.listdir(TESTS_DIR)
	list_tests.sort()
	for test_dir in list_tests:
		counter = 0
		for filename in os.listdir("{}/{}".format(TESTS_DIR, test_dir)):
			if filename in FILES:
				counter += 1
		if counter == len(FILES):
			loop_command = COMMAND.format(tests_dir = TESTS_DIR, test_dir = test_dir)
			color_print(bcolors.BLUE, "Execute '{}'".format(loop_command))
			os.system(loop_command)
			found_error = False
			for compare_file in FILES[2:]:
				if not os.path.isfile("{}/{}/test_{}".format(TESTS_DIR, test_dir, compare_file)):
					color_print(bcolors.YELLOW, "Output file 'test_{}' does not exist".format(compare_file))
					found_error = True
				elif not compare_files("{}/{}/{}".format(TESTS_DIR, test_dir, compare_file), "{}/{}/test_{}".format(TESTS_DIR, test_dir, compare_file)):
					color_print(bcolors.RED, "Output file {} is wrong".format(compare_file))
					found_error = True
			if not found_error:
				color_print(bcolors.GREEN, "Test file {} pass".format(test_dir))
		else:
			color_print(bcolors.YELLOW, "[Error] Folder '{}' is an invalid test directory".format(test_dir))

if __name__ == "__main__":
	main()

