valgrind --tool=memcheck --leak-check=full --show-leak-kinds=all --dsymutil=yes -v --log-file="valgrind.log" ./sim ./Test_Files/Test_2/cfg.txt ./Test_Files/Test_2/memin.txt ./Test_Files/Test_2/test_memout.txt ./Test_Files/Test_2/test_regout.txt ./Test_Files/Test_2/test_traceinst.txt ./Test_Files/Test_2/test_traceunit.txt