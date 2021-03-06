include ../../common/make.config
AOCL_COMPILE_CONFIG := $(shell aocl compile-config )
AOCL_LINK_CONFIG := $(shell aocl link-config )
CC = g++
CC_FLAGS = -O3 -Wall -D VERBOSE
PROJECT_ROOT = ../..
AOCL_UTILS = $(PROJECT_ROOT)/common/src/AOCLUtils
GRAPH_PARSER = $(PROJECT_ROOT)/graph_parser
GRAPH_DATASET = $(PROJECT_ROOT)/../dataset
HW_DIR = $(PROJECT_ROOT)/../hw/opencl/prk
PRK_BASE = prk_base
PRK_SWI = prk_swi

#Run Variables
#-----------

GRAPH_INPUT = $(GRAPH_DATASET)/web-google.mtx
KERNEL_BASE = ../../../hw/opencl/prk/spmv_base/bin/kernel_spmv.aocx 
KERNEL_SWI = ../../../hw/opencl/prk/spmv_swi/bin/prk_swi.aocx
KERNEL_LU2 = ../../../hw/opencl/prk/prk_lu2/bin/prk_lu2.aocx
PLATFORM_NUM_FPGA = 1
DEVICE_NUM_FPGA = 0
FILE_FORMAT = 2
DIRECTED = 1

#Compile Targets
#-----------

ALL: $(PRK_BASE) $(PRK_SWI) 


$(PRK_BASE): pagerank_base.cpp parse.o util.o
	$(CC) $(CC_FLAGS) -o $(PRK_BASE) -I$(OPENCL_INC) -I$(AOCL_UTILS)/../../inc $(foreach D,$(UTIL_INC),-I$D) -I$(GRAPH_PARSER) -L$(OPENCL_LIB) pagerank_base.cpp parse.o util.o -lOpenCL

$(PRK_SWI): pagerank_swi.cpp parse.o util.o
	$(CC) $(CC_FLAGS) -o $(PRK_SWI) -I$(OPENCL_INC) -I$(AOCL_UTILS)/../../inc $(foreach D,$(UTIL_INC),-I$D) -I$(GRAPH_PARSER) -L$(OPENCL_LIB) pagerank_swi.cpp parse.o util.o -lOpenCL	

#Support functions
#-----------------

parse.o:
	${CC} -c $(GRAPH_PARSER)/parse.cpp

util.o:
	${CC} -c $(GRAPH_PARSER)/util.cpp




#Run Targets
#-----------

run_base:
	./$(PRK_BASE) $(GRAPH_INPUT) $(KERNEL_BASE) $(PLATFORM_NUM_FPGA) $(DEVICE_NUM_FPGA) $(FILE_FORMAT) $(DIRECTED)
run_swi:
	./$(PRK_SWI) $(GRAPH_INPUT) $(KERNEL_SWI) $(PLATFORM_NUM_FPGA) $(DEVICE_NUM_FPGA) $(FILE_FORMAT) $(DIRECTED)

run_lu2:
	./$(PRK_SWI) $(GRAPH_INPUT) $(KERNEL_LU2) $(PLATFORM_NUM_FPGA) $(DEVICE_NUM_FPGA) $(FILE_FORMAT) $(DIRECTED)


#Clean Targets
#-------------

clean:
	rm -f $(PRK_BASE) $(PRK_SWI) i*.o *.out
refresh:
	rm -f ._*

