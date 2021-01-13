//This is the SWI implementation of the sssp algorithm
#include "sssp.h"

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

    //allocate the cost array
    cost_array = (int *)malloc(num_nodes * sizeof(int));
    if(!cost_array) fprintf(stderr, "malloc failed cost_array\n");
    //set the cost array to zero
    for(int i = 0; i < num_nodes; i++) cost_array[i] = 0;
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
    char * kernelsssp1  = "vector_init";
    char * kernelsssp2  = "spmv_min_dot_plus_kernel";
    char * kernelsssp3  = "vector_assign";
    char * kernelsssp4  = "vector_diff";

	//Create OpenCL kernels (FPGA)
	kernel1 = clCreateKernel(prog, kernelsssp1, &err);  
	if(err != CL_SUCCESS) { fprintf(stderr, "ERROR: clCreateKernel() 1 => %d\n", err); }
	kernel2 = clCreateKernel(prog, kernelsssp2, &err);  
	if(err != CL_SUCCESS) { fprintf(stderr, "ERROR: clCreateKernel() 2 => %d\n", err); }
	kernel3 = clCreateKernel(prog, kernelsssp3, &err);  
	if(err != CL_SUCCESS) { fprintf(stderr, "ERROR: clCreateKernel() 3 => %d\n", err); }
	kernel4 = clCreateKernel(prog, kernelsssp4, &err);  
	if(err != CL_SUCCESS) { fprintf(stderr, "ERROR: clCreateKernel() 4 => %d\n", err); }
}


//Create device buffers
void create_buffers(){
	//FPGA buffers
    //create the device-side graph structure
    row_d = clCreateBuffer(context, CL_MEM_READ_WRITE, (num_nodes + 1) * sizeof(int), NULL, &err );
    if(err != CL_SUCCESS) { fprintf(stderr, "ERROR: clCreateBuffer row_d (size:%d) => %d\n",  num_nodes , err); }	
    col_d = clCreateBuffer(context, CL_MEM_READ_WRITE, num_edges * sizeof(int), NULL, &err );
    if(err != CL_SUCCESS) { fprintf(stderr, "ERROR: clCreateBuffer col_d (size:%d) => %d\n",  num_edges , err); }
    data_d = clCreateBuffer(context,CL_MEM_READ_WRITE, num_edges * sizeof(int), NULL, &err );
    if(err != CL_SUCCESS) { fprintf(stderr, "ERROR: clCreateBuffer data_d (size:%d) => %d\n", num_edges , err); }	

    //termination variable 
    stop_d = clCreateBuffer(context,CL_MEM_READ_WRITE, sizeof(int), NULL, &err );
    if(err != CL_SUCCESS) { fprintf(stderr, "ERROR: clCreateBuffer stop_d (size:%d) => %d\n", 1 , err); }	

    //create the device-side buffers for sssp
    vector_d1 = clCreateBuffer(context,CL_MEM_READ_WRITE, num_nodes * sizeof(int), NULL, &err );
    if(err != CL_SUCCESS) { fprintf(stderr, "ERROR: clCreateBuffer vector_d1 (size:%d) => %d\n", 1 , err); }
    vector_d2 = clCreateBuffer(context,CL_MEM_READ_WRITE, num_nodes * sizeof(int), NULL, &err );
    if(err != CL_SUCCESS) { fprintf(stderr, "ERROR: clCreateBuffer vector_d2 (size:%d) => %d\n", 1 , err); }
}

void exe_fpga()
{
    timer_refs=gettime();
    //copy data to device side buffers
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
                               data_d, 
                               1, 
                               0, 
                               num_edges * sizeof(int), 
                               csr->data_array, 
                               0, 
                               0, 
                               0);
							   
    if(err != CL_SUCCESS) { fprintf(stderr, "ERROR: clEnqueueWriteBuffer data_d (size:%d) => %d\n", num_nodes, err); }
    
    timer_refe=gettime();
	timer_h2d = timer_refe-timer_refs;
    
    	//OpenCL work dimensions
    int block_size = 64;
    int global_size = (num_nodes%block_size == 0)? num_nodes: (num_nodes/block_size + 1) * block_size;

    size_t local_work[3]   = { block_size,  1, 1};
    size_t global_work[3]  = { global_size, 1, 1}; 
    size_t global_work2[3] = { global_size, 1, 1}; 

	//source vertex 0;
    int sourceVertex = 0;
    
    //set up the kernel arguments
    clSetKernelArg(kernel1, 0, sizeof(void *), (void*) &vector_d1);
    clSetKernelArg(kernel1, 1, sizeof(void *), (void*) &vector_d2);
    clSetKernelArg(kernel1, 2, sizeof(cl_int), (void*) &sourceVertex);
    clSetKernelArg(kernel1, 3, sizeof(cl_int), (void*) &num_nodes);
    
	timer_refs = gettime();

    //launch the initialization kernel
	err = clEnqueueTask(cmd_queue, 
		kernel1, 
		NULL, 
		NULL, 
		NULL);
								 
    if(err != CL_SUCCESS) { fprintf(stderr, "ERROR: kernel1  clEnqueueNDRangeKernel()=>%d failed\n", err); }
    
    clFinish(cmd_queue);
	timer_refe = gettime();
	timer_k1 += (timer_refe-timer_refs);	

    //set up kernel args
	//kernel 2
    clSetKernelArg(kernel2, 0, sizeof(cl_int), (void*) &num_nodes);
    clSetKernelArg(kernel2, 1, sizeof(void *), (void*) &row_d);
    clSetKernelArg(kernel2, 2, sizeof(void *), (void*) &col_d);
    clSetKernelArg(kernel2, 3, sizeof(void *), (void*) &data_d);
    clSetKernelArg(kernel2, 4, sizeof(void *), (void*) &vector_d1);
    clSetKernelArg(kernel2, 5, sizeof(void *), (void*) &vector_d2);
	
    //kernel 3
    clSetKernelArg(kernel3, 0, sizeof(void *), (void*) &vector_d1);
    clSetKernelArg(kernel3, 1, sizeof(void *), (void*) &vector_d2);
    clSetKernelArg(kernel3, 2, sizeof(cl_int), (void*) &num_nodes);

	//kernel 4
    clSetKernelArg(kernel4, 0, sizeof(void *), (void*) &vector_d1);
    clSetKernelArg(kernel4, 1, sizeof(void *), (void*) &vector_d2);
    clSetKernelArg(kernel4, 3, sizeof(cl_int), (void*) &num_nodes);

	int stop = 1;
    int cnt = 0;
    
    timer_refs = gettime();
    //main computation loop
    for(int i = 1; i < num_nodes; i++){
	   //reset the termination variable
	   cnt++;
       stop = 0;
       clSetKernelArg(kernel4, 2, sizeof(void *), (void*) &stop_d);
       timer_refs = gettime();
	   //copy the termination variable to the device
       err = clEnqueueWriteBuffer(cmd_queue, stop_d, 1, 0, sizeof(int), &stop, 0, 0, 0);
       if(err != CL_SUCCESS) { fprintf(stderr, "ERROR: write stop_d (%d)\n", err); }
       	clFinish(cmd_queue);
	    timer_refe = gettime();
	    timer_stop += (timer_refe-timer_refs);	
	   
      timer_refs = gettime();
       //launch the assignment kernel
      err = clEnqueueTask(cmd_queue, 
		kernel3, 
		NULL, 
		NULL, 
		NULL);
									
       if(err != CL_SUCCESS) { fprintf(stderr, "ERROR: kernel3 (%d)\n", err); }
       	clFinish(cmd_queue);
    	timer_refe = gettime();
	    timer_k3 += (timer_refe-timer_refs);	
	   
	   timer_refs = gettime();
	   //launch the min.+ kernel
       err = clEnqueueTask(cmd_queue, 
		kernel2, 
		NULL, 
		NULL, 
		NULL);
	   
       if(err != CL_SUCCESS) { fprintf(stderr, "ERROR: kernel2 (%d)\n", err); }
       	clFinish(cmd_queue);
	    timer_refe = gettime();
	    timer_k2 += (timer_refe-timer_refs);	
	   
	   timer_refs = gettime();
	   //launch the check kernel
       err = clEnqueueTask(cmd_queue, 
		kernel4, 
		NULL, 
		NULL, 
		NULL);
									
       if(err != CL_SUCCESS) { fprintf(stderr, "ERROR: kernel4 (%d)\n", err); }
       	clFinish(cmd_queue);
	    timer_refe = gettime();
	    timer_k4 += (timer_refe-timer_refs);	
	   
       timer_refs = gettime();
       //read the termination variable back
       err = clEnqueueReadBuffer(cmd_queue, stop_d, 1, 0, sizeof(int), &stop, 0, 0, 0);
       if(err != CL_SUCCESS) { fprintf(stderr, "ERROR: read stop_d (%d)\n", err); }	
       	clFinish(cmd_queue);
	    timer_refe = gettime();
	    timer_stop += (timer_refe-timer_refs);	
	   
	   //exit the loop
       if (stop==0) break;
    }
    printf("The total number of iterations = %d\n",cnt);
    timer_refs = gettime();
    //read the cost_array back
    err = clEnqueueReadBuffer(cmd_queue, vector_d1, 1, 0, num_nodes * sizeof(int), cost_array, 0, 0, 0);
    if(err != CL_SUCCESS) { fprintf(stderr, "ERROR: read vector_d1 (%d)\n", err); }
    double timer2=gettime();
    clFinish(cmd_queue);
	timer_refe = gettime();
	timer_d2h += (timer_refe-timer_refs);	
}


//print FPGA timing information
void print_time(){
	printf("################################################################\n");
	printf("################################################################\n");
	printf("H2D: Data write = %lf ms\n",timer_h2d*1000);
	printf("Kernel1: vector_init = %lf ms\n",timer_k1*1000);
	printf("Kernel2: spmv_min_dot_plus_kernel = %lf ms\n",timer_k2*1000);
	printf("Kernel3: vector_assign = %lf ms\n",timer_k3*1000);
	printf("Kernel4: vector_diff = %lf ms\n",timer_k4*1000);
	printf("D2H: Data read = %lf ms\n",timer_d2h*1000);
	printf("Stop_d signal overhead= %lf ms\n",timer_stop*1000);

	double timer_total = 0;
	timer_total = timer_h2d + timer_k1 + timer_k2 + timer_k3 + timer_k4 + timer_d2h + timer_stop;

	printf("################################################################\n");
	printf("################################################################\n");
	printf("Total time: %lf ms\n",timer_total*1000);
	printf("################################################################\n");
	printf("################################################################\n");
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
	timer_stop = 0;
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
	free(cost_array);
	csr->freeArrays();
	free(csr);
	
	free(source);

	//clean up the OpenCL buffers
	clReleaseMemObject(row_d);
	clReleaseMemObject(col_d);
	clReleaseMemObject(data_d);
	clReleaseMemObject(vector_d1);
	clReleaseMemObject(vector_d2);
	clReleaseMemObject(stop_d);

}


