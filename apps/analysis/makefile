CC = g++
CC_FLAGS = -O3 -Wall -D VERBOSE

EXE = perf_model

ALL: $(EXE)

$(EXE): hls_model.cpp
	$(CC) $(CC_FLAGS) -o $(EXE) hls_model.cpp

clean:
	rm -f $(EXE) *.o *.out *.stat *.csv
