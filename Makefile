CC = gcc
OBJS = input_output.o inst_queue.o scoreboard.o sim.o
EXEC = sim
COMP_FLAG = -std=c99 -Wall -Wextra -Werror -pedantic-errors -DNDEBUG -g3

default: $(EXEC)
$(EXEC): $(OBJS)
	$(CC) $(OBJS) -lm -o $@
sim.o: sim.c sim.h defines.h input_output.h scoreboard.h
	$(CC) $(COMP_FLAG) -c $*.c
scoreboard.o: scoreboard.c scoreboard.h defines.h inst_queue.h input_output.h
	$(CC) $(COMP_FLAG) -c $*.c
inst_queue.o: inst_queue.c inst_queue.h defines.h
	$(CC) $(COMP_FLAG) -c $*.c
input_output.o: input_output.c input_output.h defines.h
	$(CC) $(COMP_FLAG) -c $*.c
clean:
	rm -f $(OBJS) $(EXEC)
