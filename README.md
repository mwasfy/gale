Graph Analytics framework
-------------------------------------

Author: Mohamed Hassan (mwasfy@vt.edu)

Description: This is a framework to execute, analyze, profile, and model graph applications implemented on FPGAs using HLS

Repository structure 
----------------------------

`apps`: This directory includes the host code (C++) and OpenCL kernels for four graph applications

`graph_parser`: This directory includes C++ code to parse graph data-set input. Different sparse storage formats are used (CSR,COO,...), as well as the ability to transform a graph storage datastructures from format to another (CSR,COO,ELL)


Compilation and Run instructions
---------------------------------------------
The make file in each subdirectory includes make targets for compiling as well as make targets for running the application. 
