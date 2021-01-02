include ../../common/make.config
AOCL_COMPILE_CONFIG := $(shell aocl compile-config )
AOCL_LINK_CONFIG := $(shell aocl link-config )
CC = g++
CC_FLAGS = -O3 -Wall -D VERBOSE
PROJECT_ROOT = ../..
AOCL_UTILS = $(PROJECT_ROOT)/common/src/AOCLUtils
GRAPH_PARSER = $(PROJECT_ROOT)/graph_parser
GRAPH_DATASET = $(PROJECT_ROOT)/../dataset
HW_DIR = $(PROJECT_ROOT)/../hw/opencl/mis
MIS_BASE = mis_base
MIS_SWI = mis_swi

#Run Variables
#-----------

GRAPH_INPUT = $(GRAPH_DATASET)/eco/eco-everglades.edges
KERNEL_BASE = $(HW_DIR)/mis_base/bin/mis_kernel.aocx 
KERNEL_SWI = $(HW_DIR)/mis_swi/bin/mis_swi.aocx
KERNEL_LU2 = $(HW_DIR)/mis_lu2/bin/mis_lu2.aocx
PLATFORM_NUM_FPGA = 1
DEVICE_NUM_FPGA = 0
FILE_FORMAT = 2
DIRECTED = 0


#Compile Targets
#-----------

ALL: $(MIS_BASE) 


$(MIS_BASE): mis_base.cpp parse.o util.o
	$(CC) $(CC_FLAGS) -o $(MIS_BASE) -I$(OPENCL_INC) -I$(AOCL_UTILS)/../../inc $(foreach D,$(UTIL_INC),-I$D) -I$(GRAPH_PARSER) -L$(OPENCL_LIB) mis_base.cpp parse.o util.o -lOpenCL

$(MIS_SWI): mis_swi.cpp parse.o util.o
	$(CC) $(CC_FLAGS) -o $(MIS_SWI) -I$(OPENCL_INC) -I$(AOCL_UTILS)/../../inc $(foreach D,$(UTIL_INC),-I$D) -I$(GRAPH_PARSER) -L$(OPENCL_LIB) mis_swi.cpp parse.o util.o -lOpenCL	

#Support functions
#-----------------

parse.o:
	${CC} -c $(GRAPH_PARSER)/parse.cpp

util.o:
	${CC} -c $(GRAPH_PARSER)/util.cpp




#Run Targets
#-----------

run_base:
	./$(MIS_BASE) $(GRAPH_INPUT) $(KERNEL_BASE) $(PLATFORM_NUM_FPGA) $(DEVICE_NUM_FPGA) $(FILE_FORMAT) $(DIRECTED)
run_swi:
	./$(MIS_SWI) $(GRAPH_INPUT) $(KERNEL_SWI) $(PLATFORM_NUM_FPGA) $(DEVICE_NUM_FPGA) $(FILE_FORMAT) $(DIRECTED)
run_lu2:
	./$(MIS_SWI) $(GRAPH_INPUT) $(KERNEL_LU2) $(PLATFORM_NUM_FPGA) $(DEVICE_NUM_FPGA) $(FILE_FORMAT) $(DIRECTED)


#Clean Targets
#-------------

clean:
	rm -f $(MIS_BASE) $(MIS_SWI) *.o *.out
refresh:
	rm -f ._*

