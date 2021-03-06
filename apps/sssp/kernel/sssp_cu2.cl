#define BIG_NUM 99999999

/**
 * @brief   min.+
 * @param   num_nodes  Number of vertices
 * @param   row        CSR pointer array
 * @param   col        CSR column array 
 * @param   data       Weight array 
 * @param   x          Input vector 
 * @param   y          Output vector
 */
 __attribute__((num_compute_units(2)))
__kernel void spmv_min_dot_plus_kernel(    const int num_rows,
		                  __global int *restrict row,
		                  __global int *restrict col, 
		                  __global int *restrict data, 
		                  __global int * restrict x, 
		                  __global int * restrict y)
{
    //get my workitem id
	int tid = get_global_id(0);
	
	if(tid < num_rows){
        //get the start and end pointers
	    int row_start = row[tid];
	    int row_end   = row[tid+1];
        
		//perform + for each pair of elements and a reduction with min
	    int min = x[tid];
	    for( int i = row_start; i < row_end; i++){
	        if (data[i] + x[col[i]] < min) 
		     min = data[i] + x[col[i]];
	    }
	    y[tid] = min;
	}
}

/**
 * @brief   vector_init
 * @param   vector1      vector1
 * @param   vector2      vector2
 * @param   i            source vertex id
 * @param   num_nodes    number of vertices
 */
__kernel void
vector_init(__global int *vector1,
            __global int *vector2,
    	       const int i, 
               const int num_nodes){

	int tid = get_global_id(0);
	
	if (tid < num_nodes) {
	    //if it is the source vertex
		if (tid == i){
            vector1[tid] = 0;
            vector2[tid] = 0;
        }
		//if it a non-source vertex
		else{
		   vector1[tid] = BIG_NUM;
		   vector2[tid] = BIG_NUM;
        }
	}
}

/**
 * @brief   vector_assign
 * @param   vector1      vector1
 * @param   vector2      vector2
 * @param   num_nodes    number of vertices
 */
__kernel void
vector_assign(__global int *vector1,
              __global int *vector2,
                 const int num_nodes){

	int tid = get_global_id(0);
		
	if (tid < num_nodes)
	    vector1[tid] = vector2[tid];
}

/**
 * @brief   vector_diff
 * @param   vector1      vector1
 * @param   vector2      vector2
 * @param   stop         termination variable
 * @param   num_nodes    number of vertices
 */
__kernel void
vector_diff(__global int   *vector1,
            __global int   *vector2,
            __global bool  *stop,
		       const int    num_nodes){

	int tid = get_global_id(0);

	if (tid < num_nodes){
	    if (vector2[tid] != vector1[tid])
		    *stop = 1;
	}
}

