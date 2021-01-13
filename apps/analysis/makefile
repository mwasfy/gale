CC = g++
CC_FLAGS = -O3 -Wall -D VERBOSE

EXE = perf_model 
ZORO = zoro

ALL: $(EXE) $(ZORO)

$(EXE): hls_model.cpp
	$(CC) $(CC_FLAGS) -o $(EXE) hls_model.cpp
	
$(ZORO): read_results.cpp
	$(CC) $(CC_FLAGS) -o $(ZORO) read_results.cpp

clean:
	rm -f $(EXE) $(ZORO) *.out *.stat *.csv
