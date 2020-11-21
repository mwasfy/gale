#include "stdlib.h"
#include "stdio.h"
#include <string.h>
#include <math.h>
#include <iostream>
#include <fstream>
#include <algorithm>
#include <sstream>
#include <sys/time.h>
using namespace std;

//enum config { BASE, LU};


typedef struct hls_model_struct {

	int index;
	
	//Hardware parameters 
	double N; // Number of pipeline stages (thread capacity)
	double F; // Frequency
	double II; // Initiation Interval (N_d), or number of stall cycles between iterations
	//double M; // Actual memory accesses per cycle (N_m)
	double mem_cycles;
	//double W; // Maximum memory accesses per cycle
	
	//std::string config_name;
	
	//Dataset parameters
	double L; // Loop trip count
	
	//General parameters
	double R; // Throughput
	double P; // Parallelism degree
	
	//output parameters
	double cycles;
	double exec_time;
	
	
	
	//memory acces parameters
	double t_ideal;
	double mem_bytes;
	
	
	
}hls_model_struct;

void calculate_throughput();
void calculate_exec_time();
void print_time();
void calc_time();