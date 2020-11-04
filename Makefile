CC =  gcc # Path to compiler
L_FLAGS = -lrt -lpthread 
#C_FLAGS = -g

all: pt pt_c pt_d pt_e edfrt
.PHONY: all

# Project compilation
pt: periodicTask.c
	$(CC) $< -o $@ $(C_FLAGS) $(L_FLAGS)

pt_c: periodicTask_c.c
	$(CC) $< -o $@ $(C_FLAGS) $(L_FLAGS)

pt_d: periodicTask_d.c
	$(CC) $< -o $@ $(C_FLAGS) $(L_FLAGS)

pt_e: periodicTask_e.c
	$(CC) $< -o $@ $(C_FLAGS) $(L_FLAGS)

edfrt: schedDeadlineTask.c
	$(CC) $< -o $@ $(C_FLAGS) $(L_FLAGS)
	
.PHONY: clean 

clean:
	rm -f *.c~ 
	rm -f *.o
	rm pt
	rm pt_c
	rm pt_d
	rm edfrt

# Some notes
# $@ represents the left side of the ":"
# $^ represents the right side of the ":"
# $< represents the first item in the dependency list   

