/**
 * @brief   spmv_csr_scalar_kernel (simple spmv)
 * @param   num_nodes  number of vertices
 * @param   row        csr pointer array
 * @param   col        csr col array 
 * @param   data       csr weigh array 
 * @param   x          input vector
 * @param   y          output vector
 */
__kernel void
spmv_csr_scalar_kernel(   const int   num_nodes,
		               __global int   *restrict row,
		               __global int   *restrict col, 
		               __global float *restrict data, 
		               __global float *restrict x, 
		               __global float *restrict y)
{
    //get my workitem id
	int tid = 0;
	#pragma unroll 2
	for(tid = 0; tid < num_nodes; tid++)
	{
	    //get the start and end pointers
		int row_start = row[tid];
		int row_end   = row[tid+1];
		float sum = 0;
		//navigate one row and sum all the elements
		for( int j = row_start; j < row_end; j++)
		   sum += data[j] * x[col[j]];
		y[tid] += sum;
	}
}


/**
 * @brief   pagerank2
 * @param   page_rank1   PageRank array 1
 * @param   page_rank2   PageRank array 2
 * @param   num_nodes    number of vertices
 */
__kernel void pagerank2(__global float *restrict page_rank1,
                        __global float *restrict page_rank2,
                             const int  num_nodes,
                             const float norm_factor){

    //get my workitem id
    int tid = 0;
    #pragma unroll 2
    for(tid = 0; tid < num_nodes; tid++)
    {
        //update pagerank value with damping factor
        page_rank1[tid]	= norm_factor + 0.85f * page_rank2[tid];	
        page_rank2[tid] = 0.0f;
     }
}


