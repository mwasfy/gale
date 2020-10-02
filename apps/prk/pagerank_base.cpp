//This is the baseline implementation of the pagerank algorithm (SPMV-based)
#include "pagerank_base.h"
//using namespace aocl_utils;


int main(int argc, char **argv){

	if(argc == 7){
		tmpchar = argv[1];  //graph inputfile
		filechar = argv[2];	//kernel file
		PLATFORM_NUM_FPGA = atoi(argv[3]);
		DEVICE_NUM_FPGA = atoi(argv[4]);
		file_format = atoi(argv[5]);
		directed = atoi(argv[6]); //Directed/undirected graph
	}
	else
	{
		fprintf(stderr, "You did something wrong!\n");
		exit(1);
	}

	//filechar = "./kernel_spmv.aocx";
	//allocate the csr structure && parse the graph && allocate pagerank arrays
	parse_mem_alloc();
	printf("parse_mem_alloc() --> Done\n");

	//load the OpenCL kernel source files && initialize && create contexts and programs
	initialize();
	printf("Initialize() --> Done\n");

	//Create OpenCL kernels
	create_kernels();
	printf("create_kernels() --> Done\n");
	
	//Create device buffers
	create_buffers();
	printf("create_buffers() --> Done\n");
	
	//Execute FPGA implementation
	exe_fpga();
	printf("exe_fpga() --> Done\n");
	
	//print FPGA timing information
	print_time();
	
	//clean up 
	cleanup();

	return 0;
}


int initialize(){
	char* value;
	size_t valueSize;
	size_t src_len;

	size_t items_read;
	FILE * fp = fopen(filechar, "rb"); 
	if(!fp) { fprintf(stderr, "ERROR: unable to open '%s'\n", filechar); }
	fseek(fp, 0, SEEK_END);
	src_len = (size_t) ftell(fp);
	source = (char*)malloc(sizeof(char)*src_len); 
	if(source == NULL) { fprintf(stderr, "ERROR: malloc(%d) failed\n", src_len); }
	rewind(fp);
	items_read = fread(source, src_len, 1, fp);
	if(items_read != 1) { fprintf(stderr, "ERROR: items_read(%d) failed\n", items_read); }

	fclose(fp);
	
		// get platforms & devices
	clGetPlatformIDs(0, NULL, &platformCount);
	platforms = (cl_platform_id*) malloc(sizeof(cl_platform_id) * platformCount);
	clGetPlatformIDs(platformCount, platforms, &platformCount);
	clGetDeviceIDs(platforms[PLATFORM_NUM_FPGA], CL_DEVICE_TYPE_ALL, 0, NULL, &num_devices);
	device_list = (cl_device_id*) malloc(sizeof(cl_device_id) * num_devices);
	clGetDeviceIDs(platforms[PLATFORM_NUM_FPGA], CL_DEVICE_TYPE_ALL, num_devices, device_list, NULL);


	clGetDeviceInfo(device_list[DEVICE_NUM_FPGA], CL_DEVICE_NAME, 0, NULL, &valueSize);
	value = (char*) malloc(valueSize);
	clGetDeviceInfo(device_list[DEVICE_NUM_FPGA], CL_DEVICE_NAME, valueSize, value, NULL);
	printf("Using device: %s\n", value);
	free(value);
 
	/* Create OpenCL context */
	context = clCreateContext(NULL, 1, &device_list[DEVICE_NUM_FPGA], NULL, NULL, &err);
	if(err != CL_SUCCESS) { printf("ERROR: clCreateContext err=%d\n", err); }

	/* Create Command Queue */
	cmd_queue = clCreateCommandQueue(context, device_list[DEVICE_NUM_FPGA], 0, &err);
	if(err != CL_SUCCESS) { printf("ERROR: clCreateCommandQueue err=%d\n", err); }	

		//Create and build FPGA program
	printf("Creating and building FPGA program\n");
	prog = clCreateProgramWithBinary(context, 1, &device_list[DEVICE_NUM_FPGA], &src_len, (const unsigned char **) &source, NULL, &err);
	if(err != CL_SUCCESS) { printf("ERROR: clCreateProgramWithBinary err=%d\n", err); }	
	
	err = clBuildProgram(prog, 1, &device_list[DEVICE_NUM_FPGA], "", NULL, NULL);
	if(err != CL_SUCCESS) { printf("ERROR: clBuildProgram err=%d\n", err); }	
	
	return 0;
}


void create_kernels(){
	char * kernelprk1  = "inibuffer";
	char * kernelprk2  = "inicsr";
	char * kernelprk3  = "spmv_csr_scalar_kernel";
	char * kernelprk4  = "pagerank2";

	//Create OpenCL kernels (FPGA)
	kernel1 = clCreateKernel(prog, kernelprk1, &err);  
	if(err != CL_SUCCESS) { fprintf(stderr, "ERROR: clCreateKernel() 1 => %d\n", err); }
	kernel2 = clCreateKernel(prog, kernelprk2, &err);  
	if(err != CL_SUCCESS) { fprintf(stderr, "ERROR: clCreateKernel() 2 => %d\n", err); }
	kernel3 = clCreateKernel(prog, kernelprk3, &err);  
	if(err != CL_SUCCESS) { fprintf(stderr, "ERROR: clCreateKernel() 3 => %d\n", err); }
	kernel4 = clCreateKernel(prog, kernelprk4, &err);  
	if(err != CL_SUCCESS) { fprintf(stderr, "ERROR: clCreateKernel() 4 => %d\n", err); }
}


//allocate the csr structure and parse the graph
void parse_mem_alloc(){

	csr = (csr_array *)malloc(sizeof(csr_array));
	if(!csr) fprintf(stderr, "malloc failed csr\n");

	printf("file_format = %d\n", file_format);
	//parse graph files into csr structure 
	if (file_format == 1) //metis
		csr = parseMetis(tmpchar, &num_nodes, &num_edges, directed);
	else if (file_format == 0) //dimacs9		
		csr = parseCOO(tmpchar, &num_nodes, &num_edges, 1);
	else if (file_format == 2) //matrix market
		csr = parseMM_transpose(tmpchar, &num_nodes, &num_edges, directed, 0);
	else{
		printf("File format unrecognized");
		exit(1);
	}

	//printf("DEBUG:: nodes = %d , edges = %d\n", num_nodes, num_edges);

	//allocate the pagerank array 1
	pagerank_array  = (float *)malloc(num_nodes * sizeof(float));
	if (!pagerank_array) fprintf(stderr, "malloc failed page_rank_array\n");
	//allocate the pagerank array 2
	pagerank_array2 = (float *)malloc(num_nodes * sizeof(float));
	if (!pagerank_array2) fprintf(stderr, "malloc failed page_rank_array2\n");
}


//Create device buffers
void create_buffers(){
	//FPGA buffers
	row_d = clCreateBuffer(context, CL_MEM_READ_WRITE, (num_nodes + 1) * sizeof(int), NULL, &err );
	if(err != CL_SUCCESS) { printf("ERROR: clCreateBuffer row_d (size:%d) => %d\n",  num_nodes + 1, err); }
	
	col_d = clCreateBuffer(context, CL_MEM_READ_WRITE, num_edges * sizeof(int), NULL, &err );
	if(err != CL_SUCCESS) { printf("ERROR: clCreateBuffer col_d (size:%d) => %d\n",  num_edges , err); }

	data_d = clCreateBuffer(context,CL_MEM_READ_WRITE, num_edges * sizeof(float), NULL, &err );
	if(err != CL_SUCCESS) { printf("ERROR: clCreateBuffer data_d (size:%d) => %d\n", num_edges , err); }	

	col_cnt_d = clCreateBuffer(context,CL_MEM_READ_WRITE, num_nodes * sizeof(int), NULL, &err );
	if(err != CL_SUCCESS) { printf("ERROR: clCreateBuffer col_cnt (size:%d) => %d\n", num_nodes , err); }

	pagerank_d1 = clCreateBuffer(context,CL_MEM_READ_WRITE, num_nodes * sizeof(float), NULL, &err );
	if(err != CL_SUCCESS) { printf("ERROR: clCreateBuffer vector_d1 (size:%d) => %d\n", 1 , err); }

	pagerank_d2 = clCreateBuffer(context,CL_MEM_READ_WRITE, num_nodes * sizeof(float), NULL, &err );
	if(err != CL_SUCCESS) { printf("ERROR: clCreateBuffer vector_d2 (size:%d) => %d\n", 1 , err); }
}


//Execute the FPGA implementation
void exe_fpga(){
	
	//set up OpenCL work dimensions
	int block_size = 64;
	int global_size = (num_nodes%block_size == 0)? num_nodes: (num_nodes/block_size + 1) * block_size;

	size_t local_work[3]   =  { block_size,  1, 1};
	size_t global_work[3]  =  { global_size, 1, 1}; 


	//copy data to device side buffers
	timer_refs=gettime();

	err = clEnqueueWriteBuffer(cmd_queue, 
	row_d, 
	1, 
	0, 
	(num_nodes + 1) * sizeof(int), 
	csr->row_array, 
	0, 
	0, 
	0);

	if(err != CL_SUCCESS) { fprintf(stderr, "ERROR: clEnqueueWriteBuffer row_d (size:%d) => %d\n", num_nodes, err); }

	err = clEnqueueWriteBuffer(cmd_queue, 
	col_d, 
	1, 
	0, 
	num_edges * sizeof(int), 
	csr->col_array, 
	0, 
	0, 
	0);

	if(err != CL_SUCCESS) { fprintf(stderr, "ERROR: clEnqueueWriteBuffer col_d (size:%d) => %d\n", num_nodes, err); }

	err = clEnqueueWriteBuffer(cmd_queue, 
	col_cnt_d, 
	1, 
	0, 
	num_nodes * sizeof(int), 
	csr->col_cnt, 
	0, 
	0, 
	0);

	if(err != CL_SUCCESS) { fprintf(stderr, "ERROR: clEnqueueWriteBuffer col_cnt_d (size:%d) => %d\n", num_nodes, err); }

	timer_refe=gettime();
	timer_h2d = timer_refe-timer_refs;

	//kernel 1
	clSetKernelArg(kernel1, 0, sizeof(void *), (void*) &pagerank_d1);
	clSetKernelArg(kernel1, 1, sizeof(void *), (void*) &pagerank_d2);
	clSetKernelArg(kernel1, 2, sizeof(cl_int), (void*) &num_nodes);

	//launch the initialization kernel
	timer_refs = gettime();

	err = clEnqueueNDRangeKernel(cmd_queue, 
	kernel1, 
	1, 
	NULL, 
	global_work,
	local_work, 
	0, 
	0, 
	0);
	if(err != CL_SUCCESS) { printf("ERROR: kernel1  clEnqueueNDRangeKernel()=>%d failed\n", err); }
	clFinish(cmd_queue);
	timer_refe = gettime();
	timer_k1 += (timer_refe-timer_refs);	

	//kernel 2
	clSetKernelArg(kernel2, 0, sizeof(void *), (void*) &row_d);
	clSetKernelArg(kernel2, 1, sizeof(void *), (void*) &col_d);
	clSetKernelArg(kernel2, 2, sizeof(void *), (void*) &data_d);
	clSetKernelArg(kernel2, 3, sizeof(void *), (void*) &col_cnt_d);
	clSetKernelArg(kernel2, 4, sizeof(cl_int), (void*) &num_nodes);
	clSetKernelArg(kernel2, 5, sizeof(cl_int), (void*) &num_edges);

	timer_refs = gettime();

	err = clEnqueueNDRangeKernel(cmd_queue, 
	kernel2, 
	1, 
	NULL, 
	global_work, 
	local_work, 
	0, 
	0, 
	0);

	clFinish(cmd_queue);
	timer_refe = gettime();
	timer_k2 += (timer_refe-timer_refs);

	if(err != CL_SUCCESS) { printf("ERROR: kernel2  clEnqueueNDRangeKernel()=>%d failed\n", err); }


	//kernel 3
	clSetKernelArg(kernel3, 0, sizeof(cl_int), (void*) &num_nodes);
	clSetKernelArg(kernel3, 1, sizeof(void *), (void*) &row_d);
	clSetKernelArg(kernel3, 2, sizeof(void *), (void*) &col_d);
	clSetKernelArg(kernel3, 3, sizeof(void *), (void*) &data_d);
	clSetKernelArg(kernel3, 4, sizeof(void *), (void*) &pagerank_d1);
	clSetKernelArg(kernel3, 5, sizeof(void *), (void*) &pagerank_d2);

	//kernel 4
	clSetKernelArg(kernel4, 0, sizeof(void *), (void*) &pagerank_d1);
	clSetKernelArg(kernel4, 1, sizeof(void *), (void*) &pagerank_d2);
	clSetKernelArg(kernel4, 2, sizeof(cl_int), (void*) &num_nodes);

	//Run PageRank for ITER iterations
	for(int i = 0; i < ITER ; i++){
		//kernel 3 launch
		timer_refs = gettime();
		
		err = clEnqueueNDRangeKernel(cmd_queue, 
		kernel3, 
		1, 
		NULL, 
		global_work, 
		local_work, 
		0, 
		0, 
		0);

		clFinish(cmd_queue);
		timer_refe = gettime();
		timer_k3 += (timer_refe-timer_refs);

		if(err != CL_SUCCESS) { fprintf(stderr, "ERROR: kernel3  clEnqueueNDRangeKernel()=>%d failed\n", err); }

		//kernel 4 launch
		timer_refs = gettime();
		//launch the pagerank update kernel
		err = clEnqueueNDRangeKernel(cmd_queue, 
		kernel4, 
		1, 
		NULL, 
		global_work, 
		local_work, 
		0, 
		0, 
		0);
		
		clFinish(cmd_queue);
		timer_refe = gettime();
		timer_k4 += (timer_refe-timer_refs);

		if(err != CL_SUCCESS) { fprintf(stderr, "ERROR: kernel4  clEnqueueNDRangeKernel()=>%d failed\n", err); }
	}

	//Read back the results: PageRank array 
	timer_refs = gettime();
	
	err = clEnqueueReadBuffer(cmd_queue, 
	pagerank_d1, 
	1, 
	0, 
	num_nodes * sizeof(float), 
	pagerank_array, 
	0, 
	0, 
	0);
	
	clFinish(cmd_queue);
	timer_refe = gettime();
	timer_d2h += (timer_refe-timer_refs);							  
	if(err != CL_SUCCESS) { fprintf(stderr, "ERROR: clEnqueueReadBuffer()=>%d failed\n", err); }
}


void reset_timers()
{
	timer_refs = 0;
	timer_refe = 0;
	timer_h2d = 0;
	timer_d2h = 0;
	timer_k1 = 0;
	timer_k2 = 0;
	timer_k3 = 0;
	timer_k4 = 0;
	timer_overhead = 0;
}

//print FPGA timing information
void print_time(){
	printf("################################################################\n");
	printf("################################################################\n");
	printf("H2D: Data write = %lf ms\n",timer_h2d*1000);
	printf("Kernel1: inibuffer = %lf ms\n",timer_k1*1000);
	printf("Kernel2: inicsr = %lf ms\n",timer_k2*1000);
	printf("Kernel3: spmv_csr_scalar_kernel = %lf ms\n",timer_k3*1000);
	printf("Kernel4: pagerank2 = %lf ms\n",timer_k4*1000);
	printf("D2H: Data read = %lf ms\n",timer_d2h*1000);
	printf("Overhead: workload dist. overhead = %lf ms\n",timer_overhead*1000);

	double timer_total = 0;
	double timer_total_overhead = 0;
	timer_total = timer_h2d + timer_k1 + timer_k2 + timer_k3 + timer_k4 + timer_d2h;
	timer_total_overhead = timer_total + timer_overhead;

	printf("################################################################\n");
	printf("################################################################\n");
	printf("Total time: %lf ms\n",timer_total*1000);
	printf("Total time (Overhead): %lf ms\n",timer_total_overhead*1000);
	printf("################################################################\n");
	printf("################################################################\n");
}



void print_vectorf(float *vector, int num){

	FILE * fp = fopen("result.out", "w"); 
	if(!fp) { printf("ERROR: unable to open result.txt\n");}

	for(int i = 0; i < num; i++){
	fprintf(fp, "%f\n", vector[i]);
	}

	fclose(fp);

}



void cleanup(){
	// release resources
	
	if( kernel1)  clReleaseKernel( kernel1 );
	if( kernel2)  clReleaseKernel( kernel2 );
	if( kernel3)  clReleaseKernel( kernel3 );
	if( kernel4)  clReleaseKernel( kernel4 );
	if( prog) clReleaseProgram( prog );
	if( cmd_queue ) clReleaseCommandQueue( cmd_queue );
	if( context ) clReleaseContext( context );
	free(device_list);
	free(platforms);

	//clean up the host side arrays
	free(pagerank_array);
	free(pagerank_array2);
	csr->freeArrays();
	free(csr);
	
	free(source);

	//clean up the OpenCL buffers
	clReleaseMemObject(row_d);
	clReleaseMemObject(col_d);
	clReleaseMemObject(data_d);
	clReleaseMemObject(col_cnt_d);
	clReleaseMemObject(pagerank_d1);
	clReleaseMemObject(pagerank_d2);

}

