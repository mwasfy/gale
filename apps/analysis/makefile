CC = g++
CC_FLAGS = -O3 -Wall -D VERBOSE

EXE = learn

ALL: $(EXE)

$(EXE): learn.cpp
	$(CC) $(CC_FLAGS) -o $(EXE) learn.cpp

clean:
	rm -f $(EXE) *.o *.out *.stat *.csv
