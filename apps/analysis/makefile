CC = g++
CC_FLAGS = -O3 -Wall -D VERBOSE

ZORO = zoro

ALL: $(ZORO)
	
$(ZORO): perf_model.cpp
	$(CC) $(CC_FLAGS) -o $(ZORO) perf_model.cpp

clean:
	rm -f $(ZORO) *.out *.stat *.csv
