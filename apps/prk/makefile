include ../../common/make.config
AOCL_COMPILE_CONFIG := $(shell aocl compile-config )
AOCL_LINK_CONFIG := $(shell aocl link-config )
CC = g++
CC_FLAGS = -O3 -Wall -D VERBOSE
PROJECT_ROOT = ../..
AOCL_UTILS = $(PROJECT_ROOT)/common/src/AOCLUtils
GRAPH_PARSER = $(PROJECT_ROOT)/graph_parser
GRAPH_DATASET = $(PROJECT_ROOT)/../dataset

PRK_BASE = prk_base

#Run Variables
#-----------

GRAPH_INPUT = $(GRAPH_DATASET)/prk/exmp.mtx
KERNEL = ./kernel/kernel_spmv.cl
PLATFORM_NUM = 1
DEVICE_NUM = 0
FILE_FORMAT = 2
DIRECTED = 1


#Compile Targets
#-----------

ALL: $(PRK_BASE) 


$(PRK_BASE): pagerank_base.cpp parse.o util.o opencl.o options.o
	$(CC) $(CC_FLAGS) -o $(PRK_BASE) -I$(OPENCL_INC) $(foreach D,$(UTIL_INC),-I$D) -I$(GRAPH_PARSER) -L$(OPENCL_LIB) pagerank_base.cpp parse.o util.o opencl.o options.o -lOpenCL
	

#Support functions
#-----------------

parse.o:
	${CC} -c $(GRAPH_PARSER)/parse.cpp

util.o:
	${CC} -c $(GRAPH_PARSER)/util.cpp

opencl.o:
	${CC} -c $(foreach D,$(UTIL_INC),-I$D) $(AOCL_UTILS)/opencl.cpp

options.o:
	${CC} -c $(foreach D,$(UTIL_INC),-I$D) $(AOCL_UTILS)/options.cpp




#Run Targets
#-----------

run_base:
	./$(PRK_BASE) $(GRAPH_INPUT) $(KERNEL) $(PLATFORM_NUM) $(DEVICE_NUM) $(FILE_FORMAT) $(DIRECTED) >> $(PRK_BASE).out

#Clean Targets
#-------------

clean:
	rm -f $(PRK_BASE) *.o *.out

