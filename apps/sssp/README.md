Single Source Shortest Path (SSSP)
-------------------------------------

Implementation of SSSP applications for both ND_Range and Single Work Item (or Single Task) 

Repository structure 
----------------------------

*  `sssp_base.cpp` NDRange host code

*  `sssp_swi.cpp` Single Task host code

* `kernel` Directory containing OpenCL kernels, including data parallel and loop unrolling optimized kernels


Compilation and Run instructions
---------------------------------------------
Simply type `make` to compile. Although some environment variables in the make file may need adjustment according to system. 

To run an example type `make run_base` or `make run_swi` 
