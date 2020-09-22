#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>
#include <CL/cl.h>
#include <algorithm>
#include "AOCLUtils/aocl_utils.h"
#include "parse.h"
#include "util.h"

#define BIGNUM  99999999
#define ITER 20

//OpenCL runtime variables (FPGA)
static cl_context	    context;
static cl_command_queue cmd_queue;
static cl_device_type   device_type;
static cl_device_id   * device_list;
static cl_uint           num_devices;
cl_program prog;
int PLATFORM_NUM_FPGA;
int DEVICE_NUM_FPGA;


//OpenCL runtime variables (CPU)
static cl_context	    context2;
static cl_command_queue cmd_queue2;
static cl_device_id   * device_list2;
static cl_uint           num_devices2;
cl_program prog2;
int PLATFORM_NUM_CPU;
int DEVICE_NUM_CPU;


//OpenCL runtime device information
cl_uint platformCount;
cl_platform_id* platforms;
cl_uint maxComputeUnits;




//OpenCL kernels (FPGA)
cl_kernel kernel1, kernel2, kernel3, kernel4;


//OpenCL kernels (CPU)
cl_kernel kernel1_cpu, kernel2_cpu, kernel3_cpu, kernel4_cpu;


//Pagerank variables
csr_array *csr;
float *pagerank_array;
float *pagerank_array2;
char *tmpchar;	
char *filechar;
bool directed = 1;
int num_nodes; 
int num_edges;
int file_format = 1;
cl_int err = 0;

//device buffers
cl_mem row_d;
cl_mem col_d;
cl_mem data_d;
cl_mem col_cnt_d;
cl_mem pagerank_d1;
cl_mem pagerank_d2;

cl_mem row_d2;
cl_mem col_d2;
cl_mem data_d2;
cl_mem col_cnt_d2;
cl_mem pagerank_d12;
cl_mem pagerank_d22;

//timers
double timer_refs, timer_refe, timer_h2d, timer_d2h, timer_k1, timer_k2, timer_k3,timer_k4, timer_overhead;
timer_refs = 0;
timer_refe = 0;
timer_h2d = 0;
timer_d2h = 0;
timer_k1 = 0;
timer_k2 = 0;
timer_k3 = 0;
timer_k4 = 0;
timer_overhead = 0;

//Functions
void print_vectorf(float *vector, int num);
int  initialize();
void create_kernels();
void parse_mem_alloc();
void create_programs();
void create_buffers();
void exe_fpga();
void exe_cpu();
void print_time();
void  cleanup();

