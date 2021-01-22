/*##############################################################################*/
/*##############################################################################*/
/*##############################################################################*/
/*##############################################################################*/
/* Author: Mohamed W. Hassan (mwasfy@vt.edu)                                   											 */
/* Advisor: Peter M. Athanas (athanas@vt.edu)                              												     */
/* ----------------------------------------------------------------------       																*/
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
#define NUMBER_OF_GRAPHS 20
#define NUMBER_OF_CONFIGS 18
#define NUM_OF_ESTIMATIONS 360
#define MEM_BYTES 4
#define LARGE_MOD_FACTOR 256
#define SMALL_MOD_FACTOR 8
//int NUM_OF_ESTIMATIONS = NUMBER_OF_KERNELS * NUMBER_OF_GRAPHS;


enum page {PRK_SWI, PRK_CU2, PRK_CU4, PRK_LU2, PRK_LU4, PRK_LU8, SSSP_SWI, SSSP_CU2, SSSP_CU4, SSSP_LU2, SSSP_LU4, SSSP_LU8, MIS_SWI, MIS_CU2, MIS_CU4, MIS_LU2, MIS_LU4, MIS_LU8};

//perofmance model (graph input linked to a hardware implementation)
typedef struct perf_model {
	int graph_id;
	int model_id;
	string graph_name;
	string kernel_name;
	int num_nodes;
	int num_edges;
	double est_pipe_time;
	double est_mem_time;
	double est_tot_time;
	double real_time;
	double accuracy;
} perf_model;

perf_model * perf_inst[NUM_OF_ESTIMATIONS];

//dataset characteristics 
typedef struct graph_stats {
    std::string graph_name;
	int num_nodes;
	int num_edges;
	int avg_deg;
	int diameter;
} graph_stats;

graph_stats graph_db [NUMBER_OF_GRAPHS];

typedef struct hls_model_struct {
	std::string config_name;
	//Hardware parameters 
	double N; // Number of pipeline stages (thread capacity)
	double F; // Frequency
	double II; // Initiation Interval (N_d), or number of stall cycles between iterations
	double num_mem_accesses_1; // Actual memory accesses per cycle (N_m)/ number of consecutive memory accesses in outer loop
	double num_mem_accesses_2; // number of consecutive memory accesses in inner loop
	double mem_cycles_1;
	double mem_cycles_2;
	double mem_bytes;
	double num_mem_requests; //Numbers of memory requests in one loop iteration
	//double W; // Maximum memory accesses per cycle

	//General parameters
	double P_cu; // Parallelism degree from compute unit replication
	double P_lu; // Parallelism degree from loop unrolling
}hls_model_struct;
hls_model_struct * model_inst [NUMBER_OF_CONFIGS];

typedef struct graph_inst {
	//graph instance indexing
    int gid;
    std::string graph_name;
	
	//genereal graph info
	int num_vertices;
	int num_edges;
	int num_iter;
	
	//timing info
	double time_h2d = 0;
	double time_k1 = 0;
	double time_k2 = 0;
	double time_k3 = 0;
	double time_k4 = 0;
	double time_d2h = 0;
	double time_stop = 0;
	double time_tot = 0;
	
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
void write_stat_report();
void write_tot_time();
void write_tot_time_overhead();
void write_overhead();
void get_kernel_best_performance();
void adjust();
void shutdown();

void calc_perf(perf_model * model);
void calc_perf_parallel(perf_model * model);
void eval_accuracy();
void evaluate_extensions();
void evaluate_optimizations();
double get_real_time(const std::string& str, int n);

//initialize the hardware model parameters
void init_model_inst()
{
	for(int i = 0; i < NUMBER_OF_CONFIGS; i++)
		model_inst[i] = (hls_model_struct *)malloc(sizeof(hls_model_struct));
		
// 	model_inst[0]->config_name = "PRK_BASE";
// 	model_inst[0]->N = 393;
// 	model_inst[0]->F = 240*(1e6);
// 	model_inst[0]->II = 1;
// 	model_inst[0]->mem_cycles_1 = 280;
// 	model_inst[0]->mem_cycles_2 = 192;
// 	model_inst[0]->mem_bytes = 4;
// 	model_inst[0]->num_mem_accesses_1 = 1;
// 	model_inst[0]->num_mem_accesses_2 = 2; // 2 in parallel then 1
// 	model_inst[0]->num_mem_requests = 4;
// 	model_inst[0]->P_cu = 1;
// 	model_inst[0]->P_lu = 1;
	
	model_inst[0]->config_name = "PRK_SWI";
	model_inst[0]->N = 589;
	model_inst[0]->F = 240*(1e6);
	model_inst[0]->II = 1;
	model_inst[0]->mem_cycles_1 = 288;
	model_inst[0]->mem_cycles_2 = 288;
	model_inst[0]->mem_bytes = 4;
	model_inst[0]->num_mem_accesses_1 = 0;
	model_inst[0]->num_mem_accesses_2 = 2;
	model_inst[0]->num_mem_requests = 4;
	model_inst[0]->P_cu = 1;
	model_inst[0]->P_lu = 1;
	
	model_inst[1]->config_name = "PRK_CU2";
	model_inst[1]->N = 393;
	model_inst[1]->F = 240*(1e6);
	model_inst[1]->II = 1;
	model_inst[1]->mem_cycles_1 = 280;
	model_inst[1]->mem_cycles_2 = 192;
	model_inst[1]->mem_bytes = 4;
	model_inst[1]->num_mem_accesses_1 = 0;
	model_inst[1]->num_mem_accesses_2 = 3; //2 +1+1
	model_inst[1]->num_mem_requests = 4;
	model_inst[1]->P_cu = 2;
	model_inst[1]->P_lu = 1;
	
	model_inst[2]->config_name = "PRK_CU4";
	model_inst[2]->N = 393;
	model_inst[2]->F = 240*(1e6);
	model_inst[2]->II = 1;
	model_inst[2]->mem_cycles_1 = 280;
	model_inst[2]->mem_cycles_2 = 192;
	model_inst[2]->mem_bytes = 4;
	model_inst[2]->num_mem_accesses_1 = 0;
	model_inst[2]->num_mem_accesses_2 = 3; //2 +1+!
	model_inst[2]->num_mem_requests = 4;
	model_inst[2]->P_cu = 4;
	model_inst[2]->P_lu = 1;
	
	model_inst[3]->config_name = "PRK_LU2";
	model_inst[3]->N = 653;
	model_inst[3]->F = 240*(1e6);
	model_inst[3]->II = 1;
	model_inst[3]->mem_cycles_1 = 320;
	model_inst[3]->mem_cycles_2 = 320;
	model_inst[3]->mem_bytes = 4;
	model_inst[3]->num_mem_accesses_1 = 0;//2
	model_inst[3]->num_mem_accesses_2 = 6;//2 + 1 + 1 + 2 + 1 + 1
	model_inst[3]->num_mem_requests = 4;
	model_inst[3]->P_cu = 1;
	model_inst[3]->P_lu = 2;
	
	model_inst[4]->config_name = "PRK_LU4";
	model_inst[4]->N = 417;
	model_inst[4]->F = 240*(1e6);
	model_inst[4]->II = 1;
	model_inst[4]->mem_cycles_1 = 202;
	model_inst[4]->mem_cycles_2 = 202;
	model_inst[4]->mem_bytes = 4;
	model_inst[4]->num_mem_accesses_1 = 0; // 2
	model_inst[4]->num_mem_accesses_2 = 14; // 2 + 1+ 1 + 1 + 2 + 1 + 1 + 1 + 2 + 1 + 1 + 2 + 1 + 1=14
	model_inst[4]->num_mem_requests = 4;
	model_inst[4]->P_cu = 1;
	model_inst[4]->P_lu = 4;
	
	model_inst[5]->config_name = "PRK_LU8";
	model_inst[5]->N = 465;
	model_inst[5]->F = 240*(1e6);
	model_inst[5]->II = 1;
	model_inst[5]->mem_cycles_1 = 226;
	model_inst[5]->mem_cycles_2 = 226;
	model_inst[5]->mem_bytes = 4;
	model_inst[5]->num_mem_accesses_1 = 0;// 2
	model_inst[5]->num_mem_accesses_2 = 30; //2+1+1+1+2+1+1+1+2+1+1+1+2+1+1+1+2+1+1+1+2+1+1+1+2+1+1+2+1+1=30
	model_inst[5]->num_mem_requests = 4;
	model_inst[5]->P_cu = 1;
	model_inst[5]->P_lu = 8;
	
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////

// 	model_inst[7]->config_name = "SSSP_BASE";
// 	model_inst[7]->N = 387;
// 	model_inst[7]->F = 240*(1e6);
// 	model_inst[7]->II = 1;
// 	model_inst[7]->mem_cycles_1 = 280;
// 	model_inst[7]->mem_cycles_2 = 192;
// 	model_inst[7]->mem_bytes = 4;
// 	model_inst[7]->num_mem_accesses_1 = 1; // prefetching 
// 	model_inst[7]->num_mem_accesses_2 = 2; // 2 + 1
// 	model_inst[7]->num_mem_requests = 4;
// 	model_inst[7]->P_cu = 1;
// 	model_inst[7]->P_lu = 1;
	
	model_inst[6]->config_name = "SSSP_SWI";
	model_inst[6]->N = 585;
	model_inst[6]->F = 240*(1e6);
	model_inst[6]->II = 1;
	model_inst[6]->mem_cycles_1 = 288;
	model_inst[6]->mem_cycles_2 = 288;
	model_inst[6]->mem_bytes = 4;
	model_inst[6]->num_mem_accesses_1 = 0; // prefetching 
	model_inst[6]->num_mem_accesses_2 = 2;
	model_inst[6]->num_mem_requests = 4;
	model_inst[6]->P_cu = 1;
	model_inst[6]->P_lu = 1;
		
	model_inst[7]->config_name = "SSSP_CU2";
	model_inst[7]->N = 387;
	model_inst[7]->F = 240*(1e6);
	model_inst[7]->II = 1;
	model_inst[7]->mem_cycles_1 = 280;
	model_inst[7]->mem_cycles_2 = 192;
	model_inst[7]->mem_bytes = 4;
	model_inst[7]->num_mem_accesses_1 = 0; // 2 
	model_inst[7]->num_mem_accesses_2 = 2;// 2 +1 
	model_inst[7]->num_mem_requests = 4;
	model_inst[7]->P_cu = 2;
	model_inst[7]->P_lu = 1;
	
	model_inst[8]->config_name = "SSSP_CU4";
	model_inst[8]->N = 387;
	model_inst[8]->F = 240*(1e6);
	model_inst[8]->II = 1;
	model_inst[8]->mem_cycles_1 = 280;
	model_inst[8]->mem_cycles_2 = 192;
	model_inst[8]->mem_bytes = 4;
	model_inst[8]->num_mem_accesses_1 = 0; // 2 parallel
	model_inst[8]->num_mem_accesses_2 = 2;// 2 +1
	model_inst[8]->num_mem_requests = 4;
	model_inst[8]->P_cu = 4;
	model_inst[8]->P_lu = 1;
	
	model_inst[9]->config_name = "SSSP_LU2";
	model_inst[9]->N = 651 + 333;
	model_inst[9]->F = 240*(1e6);
	model_inst[9]->II = 1;
	model_inst[9]->mem_cycles_1 = 321;
	model_inst[9]->mem_cycles_2 = 321;
	model_inst[9]->mem_bytes = 4;
	model_inst[9]->num_mem_accesses_1 = 0; // 3 in parallel 
	model_inst[9]->num_mem_accesses_2 = 5; // 2 + 1 + 1 + 2 + 1 = 5
	model_inst[9]->num_mem_requests = 4;
	model_inst[9]->P_cu = 1;
	model_inst[9]->P_lu = 2;
		
	model_inst[10]->config_name = "SSSP_LU4";
	model_inst[10]->N = 413 + 194;
	model_inst[10]->F = 240*(1e6);
	model_inst[10]->II = 1;
	model_inst[10]->mem_cycles_1 = 202;
	model_inst[10]->mem_cycles_2 = 202;
	model_inst[10]->mem_bytes = 4;
	model_inst[10]->num_mem_accesses_1 = 0; // 3 
	model_inst[10]->num_mem_accesses_2 = 11; //2+1+2+2+1+2+2+1+1+2+1
	model_inst[10]->num_mem_requests = 4;
	model_inst[10]->P_cu = 1;
	model_inst[10]->P_lu = 4;
	
	model_inst[11]->config_name = "SSSP_LU8";
	model_inst[11]->N = 461 + 238;
	model_inst[11]->F = 240*(1e6);
	model_inst[11]->II = 1;
	model_inst[11]->mem_cycles_1 = 226;
	model_inst[11]->mem_cycles_2 = 226;
	model_inst[11]->mem_bytes = 4;
	model_inst[11]->num_mem_accesses_1 = 0; // 3 
	model_inst[11]->num_mem_accesses_2 = 23; //2+1+2+2+1+2+2+!+2+2+1+2+2+1+2+2+1+2+2+1+1+2+1
	model_inst[11]->num_mem_requests = 4;	
	model_inst[11]->P_cu = 1;
	model_inst[11]->P_lu = 8;
	
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////

// 	model_inst[14]->config_name = "MIS_BASE";
// 	model_inst[14]->N = 585;
// 	model_inst[14]->F = 240*(1e6);
// 	model_inst[14]->II = 1;
// 	model_inst[14]->mem_cycles_1 = 193;
// 	model_inst[14]->mem_cycles_2 = 193;
// 	model_inst[14]->mem_bytes = 4;
// 	model_inst[14]->num_mem_accesses_1 = 1;
// 	model_inst[14]->num_mem_accesses_2 = 3;//1+2+1
// 	model_inst[14]->num_mem_requests = 4;
// 	model_inst[14]->P_cu = 1;
// 	model_inst[14]->P_lu = 1;

	model_inst[12]->config_name = "MIS_SWI";
	model_inst[12]->N = 592 + 204;
	model_inst[12]->F = 240*(1e6);
	model_inst[12]->II = 1;
	model_inst[12]->mem_cycles_1 = 193;
	model_inst[12]->mem_cycles_2 = 193;
	model_inst[12]->mem_bytes = 4;
	model_inst[12]->num_mem_accesses_1 = 0; //1
	model_inst[12]->num_mem_accesses_2 = 3; //1+2+1
	model_inst[12]->num_mem_requests = 4;
	model_inst[12]->P_cu = 1;
	model_inst[12]->P_lu = 1;
	
	model_inst[13]->config_name = "MIS_CU2";
	model_inst[13]->N = 585;
	model_inst[13]->F = 240*(1e6);
	model_inst[13]->II = 1;
	model_inst[13]->mem_cycles_1 = 193;
	model_inst[13]->mem_cycles_2 = 193;
	model_inst[13]->mem_bytes = 4;
	model_inst[13]->num_mem_accesses_1 = 0;
	model_inst[13]->num_mem_accesses_2 = 3;
	model_inst[13]->num_mem_requests = 4;
	model_inst[13]->P_cu = 2;
	model_inst[13]->P_lu = 1;
	
	model_inst[14]->config_name = "MIS_CU4";
	model_inst[14]->N = 585;
	model_inst[14]->F = 240*(1e6);
	model_inst[14]->II = 1;
	model_inst[14]->mem_cycles_1 = 193;
	model_inst[14]->mem_cycles_2 = 193;
	model_inst[14]->mem_bytes = 4;
	model_inst[14]->num_mem_accesses_1 = 0;
	model_inst[14]->num_mem_accesses_2 = 3;
	model_inst[14]->num_mem_requests = 4;
	model_inst[14]->P_cu = 4;
	model_inst[14]->P_lu = 1;
	
	model_inst[15]->config_name = "MIS_LU2";
	model_inst[15]->N = 592 + 441;
	model_inst[15]->F = 240*(1e6);
	model_inst[15]->II = 1;
	model_inst[15]->mem_cycles_1 = 193 + 50;
	model_inst[15]->mem_cycles_2 = 193;
	model_inst[15]->mem_bytes = 4;
	model_inst[15]->num_mem_accesses_1 = 1;
	model_inst[15]->num_mem_accesses_2 = 9; //1+1+2+1+1+1+1+2+1
	model_inst[15]->num_mem_requests = 4;
	model_inst[15]->P_cu = 1;
	model_inst[15]->P_lu = 2;
	
	model_inst[16]->config_name = "MIS_LU4";
	model_inst[16]->N = 631 + 899;
	model_inst[16]->F = 240*(1e6);
	model_inst[16]->II = 1;
	model_inst[16]->mem_cycles_1 = 206 + 71;
	model_inst[16]->mem_cycles_2 = 206;
	model_inst[16]->mem_bytes = 4;
	model_inst[16]->num_mem_accesses_1 = 1;
	model_inst[16]->num_mem_accesses_2 = 18; //1+1+1+1+2+1+1+1+2+1+1+1+2+1+1+1+2+1
	model_inst[16]->num_mem_requests = 4;
	model_inst[16]->P_cu = 1;
	model_inst[16]->P_lu = 4;
	
	model_inst[17]->config_name = "MIS_LU8";
	model_inst[17]->N = 718 + 1999;
	model_inst[17]->F = 240*(1e6);
	model_inst[17]->II = 1;
	model_inst[17]->mem_cycles_1 = 235 + 115;
	model_inst[17]->mem_cycles_2 = 277;
	model_inst[17]->mem_bytes = 4;
	model_inst[17]->num_mem_accesses_1 = 1;
	model_inst[17]->num_mem_accesses_2 = 38; //1+1+1+1+1+1+1+1+1+2+1+1+2+1+1+1+2+1+1+1+2+1+1+1+2+1+1+1+2+1+1+1+2+1+1+1+2+1
	model_inst[17]->num_mem_requests = 4;
	model_inst[17]->P_cu = 1;
	model_inst[17]->P_lu = 8;
}

//initialize the dataset properties
void init_data_set()
{
graph_db[0].graph_name = "road-road-usa";
graph_db[0].num_nodes = 23947347 ;
graph_db[0].num_edges = 28854313;
graph_db[0].avg_deg = 2;
graph_db[0].diameter = 10;

graph_db[1].graph_name = "ca-coauthors-dblp";
graph_db[1].num_nodes =  540486;
graph_db[1].num_edges = 15245730;
graph_db[1].avg_deg = 56;
graph_db[1].diameter = 7;

graph_db[2].graph_name = "road-roadNet-CA";
graph_db[2].num_nodes =  1957027;
graph_db[2].num_edges = 2760389;
graph_db[2].avg_deg = 3;
graph_db[2].diameter = 442;

graph_db[3].graph_name = "socfb-Georgetown15";
graph_db[3].num_nodes =  9414;
graph_db[3].num_edges = 425639;
graph_db[3].avg_deg = 90;
graph_db[3].diameter = 3;

graph_db[4].graph_name = "web-it-2004";
graph_db[4].num_nodes = 509338;
graph_db[4].num_edges = 7178414;
graph_db[4].avg_deg = 28;
graph_db[4].diameter = 8;

graph_db[5].graph_name = "road-asia-osm";
graph_db[5].num_nodes =  11950757;
graph_db[5].num_edges = 12711604;
graph_db[5].avg_deg = 2;
graph_db[5].diameter = 1377;

graph_db[6].graph_name = "socfb-Duke14";
graph_db[6].num_nodes = 9885;
graph_db[6].num_edges = 506438;
graph_db[6].avg_deg = 103;
graph_db[6].diameter = 3;

graph_db[7].graph_name = "web-google";
graph_db[7].num_nodes =  1299;
graph_db[7].num_edges = 2774;
graph_db[7].avg_deg = 4;
graph_db[7].diameter = 8;

graph_db[8].graph_name = "cit-DBLP";
graph_db[8].num_nodes =  12591;
graph_db[8].num_edges = 49744;
graph_db[8].avg_deg = 7;
graph_db[8].diameter = 5;

graph_db[9].graph_name = "cit-patent";
graph_db[9].num_nodes =  3774768;
graph_db[9].num_edges = 16518948;
graph_db[9].avg_deg = 9;
graph_db[9].diameter = 9;

graph_db[10].graph_name = "email-enron-large";
graph_db[10].num_nodes =  33696;
graph_db[10].num_edges = 180812;
graph_db[10].avg_deg = 11;
graph_db[10].diameter = 5;

graph_db[11].graph_name = "eco-everglades";
graph_db[11].num_nodes = 69;
graph_db[11].num_edges = 911;
graph_db[11].avg_deg = 26;
graph_db[11].diameter = 6;

graph_db[12].graph_name = "eco-stmarks";
graph_db[12].num_nodes =  54;
graph_db[12].num_edges = 353;
graph_db[12].avg_deg = 13;
graph_db[12].diameter = 3;

graph_db[13].graph_name = "eco-foodweb-baydry";
graph_db[13].num_nodes =  128;
graph_db[13].num_edges = 2137;
graph_db[13].avg_deg = 33;
graph_db[13].diameter = 2;

graph_db[14].graph_name = "ca-hollywood-2009";
graph_db[14].num_nodes = 1069126;
graph_db[14].num_edges = 56306654;
graph_db[14].avg_deg = 105;
graph_db[14].diameter = 4;

graph_db[15].graph_name = "web-Stanford";
graph_db[15].num_nodes =  281903;
graph_db[15].num_edges = 2312498;
graph_db[15].avg_deg = 16;
graph_db[15].diameter = 10;

graph_db[16].graph_name = "socfb-uci-uni";
graph_db[16].num_nodes =  58790782;
graph_db[16].num_edges = 92208196;
graph_db[16].avg_deg = 3;
graph_db[16].diameter = 9;

graph_db[17].graph_name = "socfb-A-anon";
graph_db[17].num_nodes =  3097165;
graph_db[17].num_edges = 23667394;
graph_db[17].avg_deg = 15;
graph_db[17].diameter = 6;

graph_db[18].graph_name = "rt-retweet-crawl";
graph_db[18].num_nodes =  1112702;
graph_db[18].num_edges = 2278852;
graph_db[18].avg_deg = 4;
graph_db[18].diameter = 6;

graph_db[19].graph_name = "web-wikipedia2009";
graph_db[19].num_nodes =  1864433;
graph_db[19].num_edges = 4507315;
graph_db[19].avg_deg = 5;
graph_db[19].diameter = 9;
}
