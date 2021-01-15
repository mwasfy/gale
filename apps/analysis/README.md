Performance model and analysis
-------------------------------------

The implementation of a graph applications domain specific performance model. Moreover, the code collects runtime reports to measure the accuracy of the performance model. It is also responsible for analysisng timing reports and generates statistics and profile information about the PageRank, SSSP, and MIS applications. 

Repository structure 
----------------------------

*  `perf_model.cpp` Implementation of the performance model, analysis, and generating stat reports

*  `perf_model.h` Includes a database of the graph dataset and its characteristics (such as number of nodes, edges, average degree,...). It also includes a database of the synthesized hardware specification (such as number of pipeline stages, frequency, memory access latency,....)

* `timing_info` files of the timing reports of running the application on an arria 10 FPGA


Compilation and Run instructions
---------------------------------------------
Simply type `make` to compile.  

To run the performance model and generate the stat reports type `./zoro` 
