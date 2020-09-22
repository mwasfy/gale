//This is the baseline implementation of the pagerank algorithm (SPMV-based)
#include "pagerank_base.h"
using namespace aocl_utils;


int main(int argc, char **argv){

	if(argc == 8){
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

	//allocate the csr structure && parse the graph && allocate pagerank arrays
	parse_mem_alloc();

	//load the OpenCL kernel source files && initialize && create contexts and programs
	create_programs();

	//Create OpenCL kernels
	create_kernels();
	
	//Create device buffers
	create_buffers();
	
	//Execute FPGA implementation
	exe_fpga();
	
	//print FPGA timing information
	print_time();

	//clean up 
	cleanup();

	return 0;
}


int initialize(){

	int i, j;
	char* value;
	size_t valueSize;


	// get all platforms
	clGetPlatformIDs(0, NULL, &platformCount);
	platforms = (cl_platform_id*) malloc(sizeof(cl_platform_id) * platformCount);
	clGetPlatformIDs(platformCount, platforms, NULL);


	clGetDeviceIDs(platforms[PLATFORM_NUM_FPGA], CL_DEVICE_TYPE_ALL, 0, NULL, &num_devices);
	device_list = (cl_device_id*) malloc(sizeof(cl_device_id) * num_devices);
	clGetDeviceIDs(platforms[PLATFORM_NUM_FPGA], CL_DEVICE_TYPE_ALL, num_devices, device_list, NULL);

	clGetDeviceInfo(device_list[DEVICE_NUM_FPGA], CL_DEVICE_NAME, 0, NULL, &valueSize);
	value = (char*) malloc(valueSize);
	clGetDeviceInfo(device_list[DEVICE_NUM_FPGA], CL_DEVICE_NAME, valueSize, value, NULL);
	printf("Using device: %s\n", value);
	free(value);

	cl_context_properties ctxprop[] = { CL_CONTEXT_PLATFORM, (cl_context_properties)platforms[PLATFORM_NUM_FPGA], 0};
	context = clCreateContextFromType( ctxprop, CL_DEVICE_TYPE_ALL, NULL, NULL, NULL );
	if( !context ) { printf("ERROR: clCreateContextFromType(%s) failed\n", use_gpu ? "GPU" : "CPU"); return -1; }

	// create command queue for the first device
	cmd_queue = clCreateCommandQueue( context, device_list[DEVICE_NUM_FPGA], 0, NULL );
	if( !cmd_queue ) { printf("ERROR: clCreateCommandQueue() failed\n"); return -1; }
	///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////////////////////////////////////////////

/* 	clGetDeviceIDs(platforms[2], CL_DEVICE_TYPE_ALL, 0, NULL, &num_devices2);
	device_list2 = (cl_device_id*) malloc(sizeof(cl_device_id) * num_devices2);
	clGetDeviceIDs(platforms[2], CL_DEVICE_TYPE_ALL, num_devices2, device_list2, NULL);

	clGetDeviceInfo(device_list2[0], CL_DEVICE_NAME, 0, NULL, &valueSize);
	value = (char*) malloc(valueSize);
	clGetDeviceInfo(device_list2[0], CL_DEVICE_NAME, valueSize, value, NULL);
	printf("Using secondary device: %s\n", value);
	free(value);

	cl_context_properties ctxprop2[] = { CL_CONTEXT_PLATFORM, (cl_context_properties)platforms[2], 0};
	context2 = clCreateContextFromType( ctxprop2, CL_DEVICE_TYPE_ALL, NULL, NULL, NULL );
	//if( !context2 ) { printf("ERROR: clCreateContextFromType(%s) failed\n", use_gpu ? "GPU" : "CPU"); return -1; }

	// create command queue for the first device
	cmd_queue2 = clCreateCommandQueue( context2, device_list2[0], 0, NULL );
	if( !cmd_queue2 ) { printf("ERROR: clCreateCommandQueue2() failed\n"); return -1; } */

	return 0;
}


void create_kernels(){
	char * kernelprk1  = "inibuffer";
	char * kernelprk2  = "inicsr";
	char * kernelprk3  = "spmv_csr_scalar_kernel";
	char * kernelprk4  = "pagerank2";

	//Create OpenCL kernels (FPGA)
	kernel1 = clCreateKernel(prog, kernelprk1, &err);  
	if(err != CL_SUCCESS) { fprintf(stderr, "ERROR: clCreateKernel() 1 => %d\n", err); return -1; }
	kernel2 = clCreateKernel(prog, kernelprk2, &err);  
	if(err != CL_SUCCESS) { fprintf(stderr, "ERROR: clCreateKernel() 2 => %d\n", err); return -1; }
	kernel3 = clCreateKernel(prog, kernelprk3, &err);  
	if(err != CL_SUCCESS) { fprintf(stderr, "ERROR: clCreateKernel() 3 => %d\n", err); return -1; }
	kernel4 = clCreateKernel(prog, kernelprk4, &err);  
	if(err != CL_SUCCESS) { fprintf(stderr, "ERROR: clCreateKernel() 4 => %d\n", err); return -1; }
	
	//Create OpenCL kernels (CPU)
	kernel1_cpu = clCreateKernel(prog2, kernelprk1, &err);  
	if(err != CL_SUCCESS) { fprintf(stderr, "ERROR: clCreateKernel() 1 => %d\n", err); return -1; }
	kernel2_cpu = clCreateKernel(prog2, kernelprk2, &err);  
	if(err != CL_SUCCESS) { fprintf(stderr, "ERROR: clCreateKernel() 2 => %d\n", err); return -1; }
	kernel3_cpu = clCreateKernel(prog2, kernelprk3, &err);  
	if(err != CL_SUCCESS) { fprintf(stderr, "ERROR: clCreateKernel() 3 => %d\n", err); return -1; }
	kernel4_cpu = clCreateKernel(prog2, kernelprk4, &err);  
	if(err != CL_SUCCESS) { fprintf(stderr, "ERROR: clCreateKernel() 4 => %d\n", err); return -1; }
}


//allocate the csr structure and parse the graph
void parse_mem_alloc(){
	csr = (csr_array *)malloc(sizeof(csr_array));
	if(!csr) fprintf(stderr, "malloc failed csr\n");

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


//load the OpenCL kernel source files && initialize && create contexts and programs
void create_programs(){
	int sourcesize = 1024*1024;
	char * source = (char *)calloc(sourcesize, sizeof(char)); 
	if(!source) { fprintf(stderr, "ERROR: calloc(%d) failed\n", sourcesize); return -1; }

	FILE * fp = fopen(filechar, "rb"); 
	if(!fp) { fprintf(stderr, "ERROR: unable to open '%s'\n", filechar); return -1; }
	fread(source + strlen(source), sourcesize, 1, fp);
	fclose(fp);

	// OpenCL initialization
	if(initialize()) return -1;


	//Create and build CPU program
	printf("Creating and building CPU program\n");
	const char * slist2[2] = { source, 0 };
	prog2 = clCreateProgramWithSource(context2, 1, slist2, NULL, &err);
	if(err != CL_SUCCESS) { fprintf(stderr, "ERROR: clCreateProgramWithSource2() => %d\n", err); return -1; }
	err = clBuildProgram(prog2, num_devices2, device_list2, "", NULL, NULL);
	{
	static char log2[65536]; 
	memset(log2, 0, sizeof(log2));
	clGetProgramBuildInfo(prog2, device_list2[1], CL_PROGRAM_BUILD_LOG, sizeof(log2)-1, log2, NULL);
	if(err || strstr(log2,"warning:") || strstr(log2, "error:")) printf("<<<<\n%s\n>>>>\n", log2);
	}
	if(err != CL_SUCCESS) { fprintf(stderr, "ERROR: clBuildProgram2() => %d\n", err); return -1; }


	//Create and build FPGA program
	printf("Creating and building FPGA program\n");
	std::string binary_file = getBoardBinaryFile("spmv_kernel", device_list[DEVICE_NUM_FPGA]);
	printf("Using AOCX: %s\n", binary_file.c_str());
	prog = createProgramFromBinary(context, binary_file.c_str(), &device_list[DEVICE_NUM_FPGA], 1);
	cl_int status;
	// Build the program that was just created.
	status = clBuildProgram(prog, 0, NULL, "", NULL, NULL);
	checkError(status, "Failed to build program");
	printf("Finished creating and building programs\n");
}


//Create device buffers
void create_buffers(){
	//FPGA buffers
	row_d = clCreateBuffer(context, CL_MEM_READ_WRITE, (num_nodes + 1) * sizeof(int), NULL, &err );
	if(err != CL_SUCCESS) { printf("ERROR: clCreateBuffer row_d (size:%d) => %d\n",  num_nodes + 1, err); return -1;}
	
	col_d = clCreateBuffer(context, CL_MEM_READ_WRITE, num_edges * sizeof(int), NULL, &err );
	if(err != CL_SUCCESS) { printf("ERROR: clCreateBuffer col_d (size:%d) => %d\n",  num_edges , err); return -1;}

	data_d = clCreateBuffer(context,CL_MEM_READ_WRITE, num_edges * sizeof(float), NULL, &err );
	if(err != CL_SUCCESS) { printf("ERROR: clCreateBuffer data_d (size:%d) => %d\n", num_edges , err); return -1;}	

	col_cnt_d = clCreateBuffer(context,CL_MEM_READ_WRITE, num_nodes * sizeof(int), NULL, &err );
	if(err != CL_SUCCESS) { printf("ERROR: clCreateBuffer col_cnt (size:%d) => %d\n", num_nodes , err); return -1;}

	pagerank_d1 = clCreateBuffer(context,CL_MEM_READ_WRITE, num_nodes * sizeof(float), NULL, &err );
	if(err != CL_SUCCESS) { printf("ERROR: clCreateBuffer vector_d1 (size:%d) => %d\n", 1 , err); return -1;}

	pagerank_d2 = clCreateBuffer(context,CL_MEM_READ_WRITE, num_nodes * sizeof(float), NULL, &err );
	if(err != CL_SUCCESS) { printf("ERROR: clCreateBuffer vector_d2 (size:%d) => %d\n", 1 , err); return -1;}
	
	//CPU buffers
	row_d2 = clCreateBuffer(context2, CL_MEM_READ_WRITE, (num_nodes + 1) * sizeof(int), NULL, &err );
	if(err != CL_SUCCESS) { printf("ERROR: clCreateBuffer row_d (size:%d) => %d\n",  num_nodes + 1, err); return -1;}
	
	col_d2 = clCreateBuffer(context2, CL_MEM_READ_WRITE, num_edges * sizeof(int), NULL, &err );
	if(err != CL_SUCCESS) { printf("ERROR: clCreateBuffer col_d (size:%d) => %d\n",  num_edges , err); return -1;}

	data_d2 = clCreateBuffer(context2,CL_MEM_READ_WRITE, num_edges * sizeof(float), NULL, &err );
	if(err != CL_SUCCESS) { printf("ERROR: clCreateBuffer data_d (size:%d) => %d\n", num_edges , err); return -1;}
	
	col_cnt_d2 = clCreateBuffer(context2,CL_MEM_READ_WRITE, num_nodes * sizeof(int), NULL, &err );
	if(err != CL_SUCCESS) { printf("ERROR: clCreateBuffer col_cnt (size:%d) => %d\n", num_nodes , err); return -1;}	

	pagerank_d12 = clCreateBuffer(context2,CL_MEM_READ_WRITE, num_nodes * sizeof(float), NULL, &err );
	if(err != CL_SUCCESS) { printf("ERROR: clCreateBuffer vector_d1 (size:%d) => %d\n", 1 , err); return -1;}

	pagerank_d22 = clCreateBuffer(context2,CL_MEM_READ_WRITE, num_nodes * sizeof(float), NULL, &err );
	if(err != CL_SUCCESS) { printf("ERROR: clCreateBuffer vector_d2 (size:%d) => %d\n", 1 , err); return -1;}
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

	if(err != CL_SUCCESS) { fprintf(stderr, "ERROR: clEnqueueWriteBuffer row_d (size:%d) => %d\n", num_nodes, err); return -1; }

	err = clEnqueueWriteBuffer(cmd_queue, 
	col_d, 
	1, 
	0, 
	num_edges * sizeof(int), 
	csr->col_array, 
	0, 
	0, 
	0);

	if(err != CL_SUCCESS) { fprintf(stderr, "ERROR: clEnqueueWriteBuffer col_d (size:%d) => %d\n", num_nodes, err); return -1; }

	err = clEnqueueWriteBuffer(cmd_queue, 
	col_cnt_d, 
	1, 
	0, 
	num_nodes * sizeof(int), 
	csr->col_cnt, 
	0, 
	0, 
	0);

	if(err != CL_SUCCESS) { fprintf(stderr, "ERROR: clEnqueueWriteBuffer col_cnt_d (size:%d) => %d\n", num_nodes, err); return -1; }

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
	clFinish(cmd_queue);
	timer_refe = gettime();
	timer_k1 += (timer_refe-timer_refs);	

	if(err != CL_SUCCESS) { printf("ERROR: kernel1  clEnqueueNDRangeKernel()=>%d failed\n", err); return -1; }

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

	if(err != CL_SUCCESS) { printf("ERROR: kernel2  clEnqueueNDRangeKernel()=>%d failed\n", err); return -1; }


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

		if(err != CL_SUCCESS) { fprintf(stderr, "ERROR: kernel3  clEnqueueNDRangeKernel()=>%d failed\n", err); return -1; }

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

		if(err != CL_SUCCESS) { fprintf(stderr, "ERROR: kernel4  clEnqueueNDRangeKernel()=>%d failed\n", err); return -1; }
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
	if(err != CL_SUCCESS) { fprintf(stderr, "ERROR: clEnqueueReadBuffer()=>%d failed\n", err); return -1; }
}


//print FPGA timing information
void print_time(){
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
	if( cmd_queue ) clReleaseCommandQueue( cmd_queue );
	if( context ) clReleaseContext( context );
	if( device_list ) delete device_list;

	if( cmd_queue2 ) clReleaseCommandQueue( cmd_queue2 );
	if( context2 ) clReleaseContext( context2 );
	if( device_list2 ) delete device_list2;

	free(platforms);

	//clean up the host side arrays
	free(pagerank_array);
	free(pagerank_array2);
	csr->freeArrays();
	free(csr);

	//clean up the OpenCL buffers
	clReleaseMemObject(row_d);
	clReleaseMemObject(col_d);
	clReleaseMemObject(data_d);
	clReleaseMemObject(col_cnt_d);
	clReleaseMemObject(pagerank_d1);
	clReleaseMemObject(pagerank_d2);

	clReleaseMemObject(row_d2);
	clReleaseMemObject(col_d2);
	clReleaseMemObject(data_d2);
	clReleaseMemObject(col_cnt_d2);
	clReleaseMemObject(pagerank_d12);
	clReleaseMemObject(pagerank_d22);

	// reset all variables
	cmd_queue = 0;
	context = 0;
	device_list = 0;
	num_devices = 0;
	device_type = 0;

	cmd_queue2 = 0;
	context2 = 0;
	device_list2 = 0;
	num_devices2 = 0;
}

/* 	
timer_refs = gettime();
sleep(1);
timer_refe = gettime();
printf("TESTING wait time = %lf ms\n",(timer_refe-timer_refs)*1000); */

