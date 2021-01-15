/**
 * @brief   spmv_csr_scalar_kernel (simple spmv)
 * @param   num_nodes  number of vertices
 * @param   row        csr pointer array
 * @param   col        csr col array 
 * @param   data       csr weigh array 
 * @param   x          input vector
 * @param   y          output vector
 */
__attribute__((num_compute_units(4)))
__kernel void spmv_csr_scalar_kernel(   const int   num_nodes,
		               __global int   *restrict row,
		               __global int   *restrict col, 
		               __global float *restrict data, 
		               __global float *restrict x, 
		               __global float * restrict y)
{
    //get my workitem id
	int tid = get_global_id(0);
	if(tid < num_nodes){
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
 * @brief   inibuffer
 * @param   page_rank1   PageRank array 1
 * @param   page_rank2   PageRank array 2
 * @param   num_nodes    number of vertices
 */
__kernel  void inibuffer(__global float *restrict page_rank1,
                         __global float *restrict page_rank2,
                            const int    num_nodes)
{
    //get my workitem id
    int tid = get_global_id(0);
    //initialize two pagerank arrays
    if (tid < num_nodes){
        page_rank1[tid] = 1 / (float)num_nodes;
        page_rank2[tid] = 0.0f;
    }

}

/**
 * @brief   inicsr
 * @param   row        csr pointer array
 * @param   col        csr col array 
 * @param   data       csr weigh array 
 * @param   col_cnt    array for #. out-going edges
 * @param   num_nodes  number of vertices
 * @param   num_edges  number of edges
 */
__kernel  void inicsr(__global int   *row, 
                      __global int   *restrict col, 
                      __global float *restrict data,
                      __global int   *restrict col_cnt,
                               int    num_nodes, 
                               int    num_edges )
{
    //get my workitem id
    int tid = get_global_id(0);
    if (tid < num_nodes){
        //get the starting and ending pointers
        int start = row[tid];
        int end;
        if (tid + 1 < num_nodes)
           end = row[tid + 1] ;
        else
           end = num_edges;

	    int nid;
        //navigate one row of data
	    for(int edge = start; edge < end; edge++){
            nid = col[edge];
            //Each neighbor will get equal amount of pagerank
	        data[edge] = 1.0/(float)col_cnt[nid];
			//printf("column count[%d]:%d --- data[%d]:%lf\n",nid,col_cnt[nid],edge,data[edge]);
        }
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
                             const int  num_nodes){

    //get my workitem id
    int tid = get_global_id(0);
    //update pagerank value with damping factor
    if (tid < num_nodes){
        page_rank1[tid]	= 0.15f/(float)num_nodes + 0.85f * page_rank2[tid];	
        page_rank2[tid] = 0.0f;
     }
}


