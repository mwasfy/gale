/*##############################################################################*/
/*##############################################################################*/
/*##############################################################################*/
/*##############################################################################*/
/*                                                                              */
/* Graph Analysis Learning Engine (GALE):                                       */
/* ---------------------------------------                                      */
/* This tool analyzes the execution profile of graph data-sets                  */
/* with multiple workload distribution scheduling schemes.                      */
/* The tool keeps a library of previously learned execution profiles            */
/* and the corresponding heterogeneous scheduling configuration.                */
/* As the tool learns more, the better and more accurate decision it makes      */
/* in deciding which workload distribution fits a specific graph input.         */
/* ----------------------------------------------------------------------       */
/*                                                                              */
/* Author: Mohamed W. Hassan (mwasfy@vt.edu)                                    */
/* Advisor: Peter M. Athanas (athanas@vt.edu)                                   */
/*                                                                              */
/* ----------------------------------------------------------------------       */
/*                                                                              */
/*##############################################################################*/
/*##############################################################################*/
/*##############################################################################*/
/*##############################################################################*/


#include "stdlib.h"
#include "stdio.h"
#include <string.h>
#include <iostream>
#include <fstream>
#include <algorithm>

using namespace std;
#define NUMBER_OF_GRAPHS 21
#define NUMBER_OF_CONFIGS 9


enum page { FPGA_RESULTS_CCPU, CPU_RESULTS, FPGA_RESULTS, FPGA_RESULTS_C1, FPGA_RESULTS_C2, FPGA_RESULTS_C3, FPGA_RESULTS_C4, FPGA_RESULTS_COPT_1, FPGA_RESULTS_COPT_2};


typedef struct graph_inst {
	//graph instance indexing
    int gid;
    std::string graph_name;
	bool complete = false;
	
	//genereal graph info
	int num_vertices;
	int num_edges;
	double overhead_ratio;
	
	//timing info
	double time_h2d;
	double time_k1;
	double time_k2;
	double time_k3;
	double time_k4;
	double time_d2h;
	double time_overhead;
	double time_tot;
	double time_tot_overhead;
	
	
	void calculate_totals(){
		time_tot = time_h2d + time_k1 + time_k2 + time_k3 + time_k4 + time_d2h;
		time_tot_overhead = time_tot + time_overhead;
		overhead_ratio = time_overhead / time_tot_overhead;
		complete = true;
	}
} graph_inst;

typedef struct config {
	int config_idx;
	std::string config_name;
	graph_inst *config_graphs [NUMBER_OF_GRAPHS];
}config;

void init();
int read_pages(string page_name);
void graph_inst_copy (graph_inst *temp1, graph_inst *temp2);
void graph_inst_reset (graph_inst *temp);
void print_page(int page_num);
page page_convert(const std::string& str);
string get_page_name(int n);
void write_tot_time();
void write_tot_time_overhead();
void write_overhead();
void get_kernel_best_performance();
void adjust();
void shutdown();
