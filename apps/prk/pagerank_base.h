#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>
#include <CL/cl.h>
#include <algorithm>
//#include "AOCLUtils/aocl_utils.h"
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

char * source;

//OpenCL runtime device information
cl_uint platformCount;
cl_platform_id* platforms;
cl_uint maxComputeUnits;




//OpenCL kernels (FPGA)
cl_kernel kernel1, kernel2, kernel3, kernel4;

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


//timers
double timer_refs = 0;
double timer_refe = 0;
double timer_h2d = 0;
double timer_d2h = 0;
double timer_k1 = 0;
double timer_k2 = 0;
double timer_k3 = 0;
double timer_k4 = 0;
double timer_overhead = 0;

//Functions
void print_vectorf(float *vector, int num);
int  initialize();
void create_kernels();
void parse_mem_alloc();
void create_buffers();
void exe_fpga();
void print_time();
void reset_timers();
void  cleanup();

