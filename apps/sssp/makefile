include ../../common/make.config
AOCL_COMPILE_CONFIG := $(shell aocl compile-config )
AOCL_LINK_CONFIG := $(shell aocl link-config )
CC = g++
CC_FLAGS = -O3 -Wall -D VERBOSE
PROJECT_ROOT = ../..
AOCL_UTILS = $(PROJECT_ROOT)/common/src/AOCLUtils
GRAPH_PARSER = $(PROJECT_ROOT)/graph_parser
GRAPH_DATASET = $(PROJECT_ROOT)/../dataset
HW_DIR = $(PROJECT_ROOT)/../hw/opencl/sssp
SSSP_BASE = sssp_base
SSSP_SWI = sssp_swi

#Run Variables
#-----------

GRAPH_INPUT = $(GRAPH_DATASET)/road-asia-osm.mtx
KERNEL_BASE = ../../../hw/opencl/sssp/sssp_base/bin/kernel_csr.aocx 
KERNEL_SWI = ../../../hw/opencl/sssp/sssp_swi/bin/sssp_swi.aocx
KERNEL_LU2 = ../../../hw/opencl/sssp/sssp_lu2/bin/sssp_lu2.aocx
PLATFORM_NUM_FPGA = 1
DEVICE_NUM_FPGA = 0
FILE_FORMAT = 2
DIRECTED = 1


#Compile Targets
#-----------

ALL: $(SSSP_BASE) $(SSSP_SWI) 


$(SSSP_BASE): sssp_base.cpp parse.o util.o
	$(CC) $(CC_FLAGS) -o $(SSSP_BASE) -I$(OPENCL_INC) -I$(AOCL_UTILS)/../../inc $(foreach D,$(UTIL_INC),-I$D) -I$(GRAPH_PARSER) -L$(OPENCL_LIB) sssp_base.cpp parse.o util.o -lOpenCL

$(SSSP_SWI): sssp_swi.cpp parse.o util.o
	$(CC) $(CC_FLAGS) -o $(SSSP_SWI) -I$(OPENCL_INC) -I$(AOCL_UTILS)/../../inc $(foreach D,$(UTIL_INC),-I$D) -I$(GRAPH_PARSER) -L$(OPENCL_LIB) sssp_swi.cpp parse.o util.o -lOpenCL	

#Support functions
#-----------------

parse.o:
	${CC} -c $(GRAPH_PARSER)/parse.cpp

util.o:
	${CC} -c $(GRAPH_PARSER)/util.cpp




#Run Targets
#-----------

run_base:
	./$(SSSP_BASE) $(GRAPH_INPUT) $(KERNEL_BASE) $(PLATFORM_NUM_FPGA) $(DEVICE_NUM_FPGA) $(FILE_FORMAT) $(DIRECTED)
run_swi:
	./$(SSSP_SWI) $(GRAPH_INPUT) $(KERNEL_SWI) $(PLATFORM_NUM_FPGA) $(DEVICE_NUM_FPGA) $(FILE_FORMAT) $(DIRECTED)
run_lu2:
	./$(SSSP_SWI) $(GRAPH_INPUT) $(KERNEL_LU2) $(PLATFORM_NUM_FPGA) $(DEVICE_NUM_FPGA) $(FILE_FORMAT) $(DIRECTED)
#Clean Targets
#-------------

clean:
	rm -f $(SSSP_BASE) $(SSSP_SWI) *.o *.out
refresh:
	rm -f ._*

