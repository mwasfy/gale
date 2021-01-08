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
#define NUMBER_OF_CONFIGS 9


enum page { PRK_BASE, PRK_SWI, PRK_LU2, SSSP_BASE, SSSP_SWI, SSSP_LU2, MIS_BASE, MIS_SWI, MIS_LU2};

//dataset characteristics 
typedef struct graph_stats {
    std::string graph_name;
	int num_nodes;
	int num_edges;
	int avg_deg;
	int diameter;
} graph_stats;

graph_stats graph_db [NUMBER_OF_GRAPHS];








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
	double time_h2d = 0;
	double time_k1 = 0;
	double time_k2 = 0;
	double time_k3 = 0;
	double time_k4 = 0;
	double time_d2h = 0;
	double time_stop = 0;
	double time_tot = 0;
	
	
	void calculate_totals(){
		time_tot = time_h2d + time_k1 + time_k2 + time_k3 + time_k4 + time_d2h + time_stop;
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
graph_db[11].diameter = 2;

graph_db[12].graph_name = "eco-stmarks";
graph_db[12].num_nodes =  54;
graph_db[12].num_edges = 353;
graph_db[12].avg_deg = 13;
graph_db[12].diameter = 2;

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
