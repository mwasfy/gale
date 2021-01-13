#define BIGNUM 99999999

/**
* init kernel
* @param s_array   set array
* @param c_array   status array
* @param cu_array  status update array
* @param num_nodes number of vertices
* @param num_edges number of edges
*/
__kernel void init(__global int *restrict s_array,
         	   __global int *restrict c_array, 
                   __global int *restrict cu_array, 
                            int num_nodes,
                            int num_edges){

    //get my workitem id
    int tid = 0;
    for (tid = 0; tid < num_nodes; tid++){
        //set the status array: not processed
        c_array[tid]  = -1;
        cu_array[tid] = -1;
        s_array[tid] = 0;
    }
}
			
/**
* mis1 kernel
* @param row          csr pointer array
* @param col          csr column index array
* @param node_value   node value array
* @param s_array      set array
* @param c_array node status array
* @param min_array    node value array
* @param stop node    value array
* @param num_nodes    number of vertices
* @param num_edges    number of edges
*/
__kernel void mis1(  __global int *restrict row, 
                     __global int *restrict col, 
                     __global float *restrict node_value,
                     __global int *restrict s_array,
                     __global int *restrict c_array,
                     __global float *restrict min_array,
                     __global int *restrict stop,
                              int num_nodes,
                              int num_edges){

    //get workitem id
    int tid = 0;
    for (tid = 0; tid < num_nodes; tid++){
       //if the vertex is not processed
       if(c_array[tid] == -1){       
	    *stop = 1;
            //get the start and end pointers
	    int start = row[tid];
	    int end;
            if (tid + 1 < num_nodes)
               end = row[tid + 1] ;
            else
               end = num_edges;

            //navigate the neighbor list and find the min
	    float min = BIGNUM;
	    for(int edge = start; edge < end; edge++){
	        if (c_array[col[edge]] == -1){
                    if(node_value[col[edge]] < min)
                      min = node_value[col[edge]]; 
                }
            }
            min_array[tid] = min;
        }
    }
}

/**
* mis2 kernel
* @param row          csr pointer array
* @param col          csr column index array
* @param node_value   node value array
* @param s_array      set array
* @param c_array      status array
* @param cu_array     status update array
* @param min_array    node value array
* @param num_nodes    number of vertices
* @param num_edges    number of edges
*/
__kernel void  mis2(  __global int *restrict row, 
                      __global int *restrict col, 
                      __global float *restrict node_value,	
                      __global int *restrict s_array,
                      __global int *restrict c_array,
                      __global int *restrict cu_array,
                      __global float *restrict min_array,
                               int num_nodes,
                               int num_edges)
{

    //get my workitem id
    int tid = 0;
    for (tid = 0; tid < num_nodes; tid++){

       if(node_value[tid] < min_array[tid]  && c_array[tid] == -1){  
          // -1: not processed -2: inactive 2: independent set
          //put the item into the independent set   
          s_array[tid] = 2;

          //get the start and end pointers
          int start = row[tid];
          int end;

          if (tid + 1 < num_nodes)
             end = row[tid + 1] ;
          else
             end = num_edges;

          //set the status to inactive
          c_array[tid] = -2;

          //mark all the neighnors inactive
          for(int edge = start; edge < end; edge++){
             if (c_array[col[edge]] == -1)
                 //use status update array to avoid race
	         cu_array[col[edge]] = -2;
	  }

       }	
    }
}

/**
* mis3 kernel
* @param cu_array     status update array
* @param  c_array     status array
* @param num_nodes    number of vertices
*/
__kernel void  mis3(  __global int *restrict cu_array, 
                      __global int *restrict c_array, 
                               int num_nodes)
{

    //get my workitem id
    int tid = 0;
    //set the status array
    for (tid = 0; tid < num_nodes; tid++){
        if (cu_array[tid] == -2)
            c_array[tid] = cu_array[tid];
    }
}
