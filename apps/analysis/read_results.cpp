/*##############################################################################*/
/*##############################################################################*/
/*##############################################################################*/
/*##############################################################################*/

/* Author: Mohamed W. Hassan (mwasfy@vt.edu)                                    */
/* Advisor: Peter M. Athanas (athanas@vt.edu)                                   */

/*##############################################################################*/
/*##############################################################################*/
/*##############################################################################*/
/*##############################################################################*/


#include "read_results.h"
#include "stdlib.h"
#include "stdio.h"
#include <string.h>
#include <iostream>
#include <sstream>
#include <fstream>
#include <algorithm>
#include <sys/time.h>
//#include <../util.h>


config *config_page [NUMBER_OF_CONFIGS];
graph_inst *temp = (graph_inst *)malloc(sizeof(graph_inst));

using namespace std;




int main(int argc, char **argv){
	init_data_set();
	init_model_inst();

// 	for (int i = 0; i< NUMBER_OF_GRAPHS; i++)
// 	{
// 		cout << "Graph name: " << graph_db[i].graph_name << " \tNodes: " << graph_db[i].num_nodes << " \tEdges: " << graph_db[i].num_edges << " \tAvg_degree: " << graph_db[i].avg_deg  << endl;
// 	}
// 	for (int i = 0; i< NUMBER_OF_KERNELS; i++)
// 	{
// 		cout << "Kernel name: " << model_inst[i]->config_name << " \tFreq: " << model_inst[i]->F << " \tNo of stages: " << model_inst[i]->N << " \tMem access cycles: " << model_inst[i]->mem_cycles  << endl;
// 	}
	
	init();
	
	for(int p = 0; p < NUMBER_OF_CONFIGS; p++)
	{
		if(!read_pages(get_page_name(p)))
			cout << "FAIL: Page read number " << p << " failed." << endl;
	}
	
	for(int p = 0; p < NUM_OF_ESTIMATIONS; p++)
		calc_perf(perf_inst[p]);
		
//  	for(int p = 0; p < NUMBER_OF_CONFIGS; p++)
// 		print_page(p); 
// 	adjust();
// 	write_tot_time();
// 	write_tot_time_overhead();
	//get_kernel_best_performance();
	//write_overhead();

	shutdown();
}

void init()
{
	for(int p = 0; p < NUMBER_OF_CONFIGS; p++)
	{
		config_page[p] = (config *)malloc(sizeof(config));
		for(int i = 0; i < NUMBER_OF_GRAPHS ; i++)
		{
			config_page[p]->config_graphs[i] = (graph_inst *)malloc(sizeof(graph_inst));
		}
	}
	
	//initialize performance model instances that link graphs to hardware implementations
	int i=0;
	int j=0;
	for(int p = 0; p < NUM_OF_ESTIMATIONS; p++)
	{
		perf_inst[p] = (perf_model *)malloc(sizeof(perf_model));
		perf_inst[p]->graph_id = i;
		perf_inst[p]->model_id = j;
		i++;
		if(i == NUMBER_OF_GRAPHS)
		{
			i = 0;
			j++;
		}
	}
// 	for(int p = 0; p < NUM_OF_ESTIMATIONS; p++)
// 		cout << "perf_inst[" << p << "] graph_id: " << perf_inst[p]->graph_id << " model_id: " << perf_inst[p]->model_id << endl; 
}

//Calculate the performance estimation for one graph instance on hardware implementation
void calc_perf(perf_model * model)
{
	//Calculate edge to node ratio
	double n_to_e = graph_db[model->graph_id].num_edges / graph_db[model->graph_id].num_nodes;
	double cycles = 0;
	
	//Pipe stages calculation
	cycles += model_inst[model->model_id]->N + (graph_db[model->graph_id].num_nodes * (model_inst[model->model_id]->II+1));
	double est_pipe_time = cycles / model_inst[model->model_id]->F;

	//Bandwidth calculation
	double tot_access_bytes = MEM_BYTES * model_inst[model->model_id]->num_mem_requests * (model_inst[model->model_id]->num_mem_accesses_1 + model_inst[model->model_id]->num_mem_accesses_2);
	double mem_access_time = model_inst[model->model_id]->mem_cycles / model_inst[model->model_id]->F;
	model->BW = tot_access_bytes / mem_access_time;
	
	//Estiamted total accesses for the graph instance (Number of total accesses)
	double est_total_access = 0;
	//est_total_access += graph_db[model->graph_id].num_nodes * model_inst[model->model_id]->num_mem_accesses_1 * 2;
	//est_total_access += graph_db[model->graph_id].avg_deg * model_inst[model->model_id]->num_mem_accesses_2 * graph_db[model->graph_id].num_nodes;
	//est_total_access += n_to_e * model_inst[model->model_id]->num_mem_accesses_2 * graph_db[model->graph_id].num_nodes;
	double burst_size = 4;
	if(n_to_e > burst_size)
	{
		double temp = n_to_e / burst_size;
		est_total_access = temp * graph_db[model->graph_id].num_nodes * model_inst[model->model_id]->num_mem_accesses_2 ;
	}
	else
	{
		est_total_access = n_to_e * graph_db[model->graph_id].num_nodes * model_inst[model->model_id]->num_mem_accesses_2 ;
	}
	
	//Estiamted total accesses for the graph instance (Size of total accesses "Bytes")

// 	double temp = graph_db[model->graph_id].avg_deg * model_inst[model->model_id]->mem_bytes;
// 	double est_total_bytes = est_total_access / temp ;
	double est_total_bytes = est_total_access / 16 ;
	
	//Estimate the memory access time. Divide total memory access bytes by the bandwidth
	double bw = (double) 2400*1024*1024;
	bw = bw/8;
	//double est_mem_time = (est_total_bytes / bw);
	//double est_mem_time = (est_total_bytes / model->BW) * 1000;
	//double est_mem_time = est_total_bytes * mem_access_time * 1000;
	double est_mem_time = model_inst[model->model_id]->mem_cycles * est_total_bytes;
	est_mem_time = est_mem_time / model_inst[model->model_id]->F;
	//Total estimated time
	model->est_time = (est_pipe_time + est_mem_time) * 1000; 
	
	//Get Measured execution time
	double real_time = get_real_time(graph_db[model->graph_id].graph_name, model->model_id);
	
	cout << "Kernel: " << model_inst[model->model_id]->config_name << "\t Graph: " << graph_db[model->graph_id].graph_name <<  endl;
	cout << "---------------------------------------------------------------------" << endl;
	cout << "Estimated pipe time: " << est_pipe_time * 1000 <<  endl;
	cout << "Estimated mem time: " << est_mem_time * 1000 <<  endl;
	cout << "Estimated execution time: " << model->est_time <<  endl;
	cout << "Actual execution time: " << real_time <<  endl;
	cout << "Accuracy: " << (1 - (abs(model->est_time - real_time) / real_time)) * 100 <<  "%" << endl;
	cout << "Est to real ratio: " << model->est_time / real_time * 100 << "%" << endl;
	cout << "---------------------------------------------------------------------"  << endl;
	cout << "---------------------------------------------------------------------"  << endl << endl;
}

//Retrieve the real measured execution time
double get_real_time(const std::string& str, int n)
{	
	int page_id;
	int gr_id = 100;
	if(n == 0)
	{
		page_id = 1;
	}
	else if(n == 1)
	{
		page_id = 4;
	}
	else if(n == 2)
	{
		page_id = 7;
	}
	
	for(int x = 0 ; x < NUMBER_OF_GRAPHS; x++)
	{
		if(config_page[page_id]->config_graphs[x]->graph_name == str)
		{
			gr_id = config_page[page_id]->config_graphs[x]->gid;
			break;
		}
	}
	
	if(gr_id == 100)
		return -1;

	if(n == 0)
	{
		return config_page[page_id]->config_graphs[gr_id]->time_k1;
	}
	else if(n == 1)
	{
		return config_page[page_id]->config_graphs[gr_id]->time_k2;
	}
	else if(n == 2)
	{
		return config_page[page_id]->config_graphs[gr_id]->time_k2;
	}
	else
		return -1;
}

int read_pages(string page_name)
{
	int page_id = page_convert(page_name);
	config_page[page_id]->config_idx = page_id;
	config_page[page_id]->config_name = page_name;
	string page_path_post = ".o";
	string page_path = page_name + page_path_post;
	//cout << "Page path: " << page_path << endl;
	//cout << "Page name: " << config_page[page_id]->config_name << " Page ID: " << config_page[page_id]->config_idx << endl;
	
	
	int gidx = 0;
	stringstream ss;
	int offset = 0;
	int temp_int;
	int nodes;
	int edges;
	double temp_double;
	double time_val;
	
	ifstream inFile(page_path.c_str());
	if (inFile.fail())
	{
		cerr << "Unable to open file for reading." << endl;
		return 0;
	}

	string nextLine;
	

	while (std::getline(inFile, nextLine))
	{
		if(nextLine.compare(0,7,"Opening") == 0)
		{
			temp->gid = gidx;
			gidx++;
			
			std::size_t pos = nextLine.find("dataset");
			std::string gname = nextLine.substr (pos+8);
			pos = gname.find(".");
			std::string gname2 = gname.substr (0,pos);
			temp->graph_name = gname2;
			//cout << "DEBUG::(GRAPH ID): " << temp->gid << " (GRAPH NAME): " << temp->graph_name << endl;			
		}
		else if(nextLine.compare(0,4,"Read") == 0)
		{
			ss << nextLine;
			string stemp;
			while (!ss.eof()) { 
				/* extracting word by word from stream */
				ss >> stemp; 
		  
				/* Checking the given word is integer or not */
				if (stringstream(stemp) >> temp_int)
				{
					if(offset == 0)
					{
						nodes = temp_int;
						offset++;
					}						
					if(offset == 1)
					{
						edges = temp_int;
					}
				}
			}
			temp->num_vertices = nodes;
			temp->num_edges = edges;
			offset = 0;
			//cout << "Nodes = " << nodes << " Edges = " << edges << endl; 
			nodes = 0;
			edges = 0;
			temp_int = 0;
			stemp.clear();
			ss.str("");
			ss.clear();
		}
		else if(nextLine.compare(0,4,"H2D:") == 0)
		{
			ss << nextLine;
			string stemp;
			while (!ss.eof()) { 
				/* extracting word by word from stream */
				ss >> stemp; 
		  
				/* Checking the given word is integer or not */
				if (stringstream(stemp) >> temp_double)
				{
					time_val = temp_double;					
				}
			}
			temp->time_h2d = time_val;
			//cout << "time value " << time_val << endl; 
			time_val = 0;
			temp_double = 0;
			stemp.clear();
			ss.str("");
			ss.clear();
		}
		else if(nextLine.compare(0,7,"Kernel1") == 0)
		{
			ss << nextLine;
			string stemp;
			while (!ss.eof()) { 
				/* extracting word by word from stream */
				ss >> stemp; 
		  
				/* Checking the given word is integer or not */
				if (stringstream(stemp) >> temp_double)
				{
					time_val = temp_double;					
				}
			}
			temp->time_k1 = time_val;
			//cout << "time value k1 " << time_val << endl; 
			time_val = 0;
			temp_double = 0;
			stemp.clear();
			ss.str("");
			ss.clear();
		}
		else if(nextLine.compare(0,7,"Kernel2") == 0)
		{
			ss << nextLine;
			string stemp;
			while (!ss.eof()) { 
				/* extracting word by word from stream */
				ss >> stemp; 
		  
				/* Checking the given word is integer or not */
				if (stringstream(stemp) >> temp_double)
				{
					time_val = temp_double;					
				}
			}
			temp->time_k2 = time_val;
			//cout << "time value k1 " << time_val << endl; 
			time_val = 0;
			temp_double = 0;
			stemp.clear();
			ss.str("");
			ss.clear();
		}
		else if(nextLine.compare(0,7,"Kernel3") == 0)
		{
			ss << nextLine;
			string stemp;
			while (!ss.eof()) { 
				/* extracting word by word from stream */
				ss >> stemp; 
		  
				/* Checking the given word is integer or not */
				if (stringstream(stemp) >> temp_double)
				{
					time_val = temp_double;					
				}
			}
			temp->time_k3 = time_val;
			//cout << "time value k1 " << time_val << endl; 
			time_val = 0;
			temp_double = 0;
			stemp.clear();
			ss.str("");
			ss.clear();
		}
		else if(nextLine.compare(0,7,"Kernel4") == 0)
		{
			ss << nextLine;
			string stemp;
			while (!ss.eof()) { 
				/* extracting word by word from stream */
				ss >> stemp; 
		  
				/* Checking the given word is integer or not */
				if (stringstream(stemp) >> temp_double)
				{
					time_val = temp_double;					
				}
			}
			temp->time_k4 = time_val;
			//cout << "time value k1 " << time_val << endl; 
			time_val = 0;
			temp_double = 0;
			stemp.clear();
			ss.str("");
			ss.clear();
		}
		else if(nextLine.compare(0,4,"D2H:") == 0)
		{
			ss << nextLine;
			string stemp;
			while (!ss.eof()) { 
				/* extracting word by word from stream */
				ss >> stemp; 
		  
				/* Checking the given word is integer or not */
				if (stringstream(stemp) >> temp_double)
				{
					time_val = temp_double;					
				}
			}
			temp->time_d2h = time_val;
			//cout << "time value k1 " << time_val << endl; 
			time_val = 0;
			temp_double = 0;
			stemp.clear();
			ss.str("");
			ss.clear();
		}
		else if(nextLine.compare(0,6,"Stop_d") == 0)
		{
			ss << nextLine;
			string stemp;
			while (!ss.eof()) { 
				/* extracting word by word from stream */
				ss >> stemp; 
		  
				/* Checking the given word is integer or not */
				if (stringstream(stemp) >> temp_double)
				{
					time_val = temp_double;					
				}
			}
			temp->time_stop = time_val;
			//cout << "time value k1 " << time_val << endl; 
			time_val = 0;
			temp_double = 0;
			stemp.clear();
			ss.str("");
			ss.clear();
		}
		else if(nextLine.compare(0,30,"The total number of iterations") == 0)
		{
			ss << nextLine;
			string stemp;
			while (!ss.eof()) { 
				/* extracting word by word from stream */
				ss >> stemp; 
		  
				/* Checking the given word is integer or not */
				if (stringstream(stemp) >> temp_double)
				{
					time_val = temp_double;					
				}
			}
			temp->time_stop = time_val;
			//cout << "time value k1 " << time_val << endl; 
			time_val = 0;
			temp_double = 0;
			stemp.clear();
			ss.str("");
			ss.clear();
		}
		else if(nextLine.compare(0,11,"Total time:") == 0)
		{
			ss << nextLine;
			string stemp;
			while (!ss.eof()) { 
				/* extracting word by word from stream */
				ss >> stemp; 
		  
				/* Checking the given word is integer or not */
				if (stringstream(stemp) >> temp_double)
				{
					time_val = temp_double;					
				}
			}
			temp->time_tot = time_val;
			//cout << "time value k1 " << time_val << endl; 
			time_val = 0;
			temp_double = 0;
			stemp.clear();
			ss.str("");
			ss.clear();
			graph_inst_copy(temp,config_page[page_id]->config_graphs[temp->gid]);
		}


	}
	
	inFile.close();
	graph_inst_reset(temp);
	return 1;
}


void graph_inst_reset (graph_inst *temp)
{
	temp->gid = 0;
	temp->graph_name.erase (temp->graph_name.begin(), temp->graph_name.end());
	temp->num_vertices = 0;
	temp->num_edges = 0;
	temp->time_h2d = 0;
	temp->time_k1 = 0;
	temp->time_k2 = 0;
	temp->time_k3 = 0;
	temp->time_k4 = 0;
	temp->time_d2h = 0;
	temp->time_stop = 0;
	temp->time_tot = 0;
}

void graph_inst_copy (graph_inst *temp1, graph_inst *temp2)
{
	//cout << "Copying graph ID: " << temp1->gid << endl;
	temp2->gid = temp1->gid;
	temp2->graph_name = temp1->graph_name;
	temp2->num_vertices = temp1->num_vertices;
	temp2->num_edges = temp1->num_edges;
	temp2->time_h2d = temp1->time_h2d;
	temp2->time_k1 = temp1->time_k1;
	temp2->time_k2 = temp1->time_k2;
	temp2->time_k3 = temp1->time_k3;
	temp2->time_k4 = temp1->time_k4;
	temp2->time_d2h = temp1->time_d2h;
	temp2->time_stop = temp1->time_stop;
	temp2->time_tot = temp1->time_tot;
}

void print_page(int page_num)
{
	cout << "printing page: " << config_page[page_num]->config_name << " page index: " << config_page[page_num]->config_idx << endl; 
	for(int x = 0 ; x < NUMBER_OF_GRAPHS; x++)
	{
		cout << "(GRAPH ID): " << config_page[page_num]->config_graphs[x]->gid << " (GRAPH NAME): " << config_page[page_num]->config_graphs[x]->graph_name << endl;
		cout << "Nodes: " << config_page[page_num]->config_graphs[x]->num_vertices << " Edges: " << config_page[page_num]->config_graphs[x]->num_edges << endl;
		cout << "H2D: " << config_page[page_num]->config_graphs[x]->time_h2d << 
				" K1: " << config_page[page_num]->config_graphs[x]->time_k1 << 
				" K2: " << config_page[page_num]->config_graphs[x]->time_k2 << 
				" K3: " << config_page[page_num]->config_graphs[x]->time_k3 << 
				" K4: " << config_page[page_num]->config_graphs[x]->time_k4 << 
				" D2H: " << config_page[page_num]->config_graphs[x]->time_d2h << 
				" Stop: " << config_page[page_num]->config_graphs[x]->time_stop << 
				" Tot: " << config_page[page_num]->config_graphs[x]->time_tot <<  endl;
	}
}

page page_convert(const std::string& str)
{
	if(str == "PRK_BASE") return PRK_BASE;
	else if(str == "PRK_SWI") return PRK_SWI;
    else if(str == "PRK_LU2") return PRK_LU2;
    else if(str == "SSSP_BASE") return SSSP_BASE;
    else if(str == "SSSP_SWI") return SSSP_SWI;
    else if(str == "SSSP_LU2") return SSSP_LU2;
	else if(str == "MIS_BASE") return MIS_BASE;
	else if(str == "MIS_SWI") return MIS_SWI;
	else if(str == "MIS_LU2") return MIS_LU2;
}

string get_page_name(int n)
{
    if(n == 0) return "PRK_BASE";
    else if(n == 1) return "PRK_SWI";
    else if(n == 2) return "PRK_LU2";
    else if(n == 3) return "SSSP_BASE";
    else if(n == 4) return "SSSP_SWI";
	else if(n == 5) return "SSSP_LU2";
	else if(n == 6) return "MIS_BASE";
	else if(n == 7) return "MIS_SWI";
	else if(n == 8) return "MIS_LU2";
}

// void write_tot_time()
// {
// 	double min=999999999;
// 	int min_config;
// 	ofstream outfile("total_time.csv");
// 	
// 	outfile << "Graph,CPU,FPGA,Config_1,Config_2,Config_3,Config_4,FPGA_OPT1,FPGA_OPT2,Fastest" << endl;
// 	
// 	for(int g = 0; g < NUMBER_OF_GRAPHS ; g++)
// 	{
// 		outfile << config_page[0]->config_graphs[g]->graph_name << ",";
// 		for(int p = 1; p < NUMBER_OF_CONFIGS; p++)
// 		{
// 			outfile << config_page[p]->config_graphs[g]->time_tot << ",";
// 			if(config_page[p]->config_graphs[g]->time_tot < min)
// 			{
// 				min = config_page[p]->config_graphs[g]->time_tot;
// 				min_config = p;
// 			}
// 		}
// 		outfile << config_page[min_config]->config_name;
// 		outfile << endl;
// 		min=999999999;
// 	}
// 	
// 	outfile.close();
// }
// 
// 
// void write_tot_time_overhead()
// {
// 	double min=999999999;
// 	int min_config;
// 	ofstream outfile("total_time_overhead.csv");
// 	
// 	outfile << "Graph,CPU,FPGA,Config_1,Config_2,Config_3,Config_4,FPGA_OPT1,FPGA_OPT2,Fastest" << endl;
// 	
// 	for(int g = 0; g < NUMBER_OF_GRAPHS ; g++)
// 	{
// 		outfile << config_page[0]->config_graphs[g]->graph_name << ",";
// 		for(int p = 1; p < NUMBER_OF_CONFIGS; p++)
// 		{
// 			outfile << config_page[p]->config_graphs[g]->time_tot_overhead << ",";
// 			if(config_page[p]->config_graphs[g]->time_tot_overhead < min)
// 			{
// 				min = config_page[p]->config_graphs[g]->time_tot_overhead;
// 				min_config = p;
// 			}
// 		}
// 		outfile << config_page[min_config]->config_name;
// 		outfile << endl;
// 		min=999999999;
// 	}
// 	
// 	outfile.close();
// }
// 
// 
// void write_overhead()
// {
// 
// 	ofstream outfile("overhead_ratio.csv");
// 	
// 	outfile << "Graph,CPU,FPGA,Config_1,Config_2,Config_3,Config_4,FPGA_OPT1,FPGA_OPT2" << endl;
// 	
// 	for(int g = 0; g < NUMBER_OF_GRAPHS ; g++)
// 	{
// 		outfile << config_page[0]->config_graphs[g]->graph_name << ",";
// 		for(int p = 0; p < NUMBER_OF_CONFIGS; p++)
// 		{
// 			outfile << config_page[p]->config_graphs[g]->overhead_ratio << ",";
// 		}
// 		outfile << endl;
// 	}
// 	
// 	outfile.close();
// }
// 
// 
// void get_kernel_best_performance()
// {
// 	double min1 = 9999999999999;
// 	double min2 = 9999999999999;
// 	double min3 = 9999999999999;
// 	double min4 = 9999999999999;
// 	int config1[NUMBER_OF_GRAPHS];
// 	int config2[NUMBER_OF_GRAPHS];
// 	int config3[NUMBER_OF_GRAPHS];
// 	int config4[NUMBER_OF_GRAPHS];
// 	int check = 0;
// 	ofstream outfile("kernel_best_performance.csv");
// 	
// 	//outfile << "Graph,CPU,FPGA,Config_1,Config_2,Config_3" << endl;
// 	outfile << "Graph,K1_config,K1_time,overhead,K2_config,K2_time,overhead,K3_config,K3_time,overhead,K4_config,K4_time,overhead" << endl;
// 
// 	for(int g = 0; g < NUMBER_OF_GRAPHS ; g++)
// 	{
// 		for(int p = 0; p < NUMBER_OF_CONFIGS; p++)
// 		{
// 			if(config_page[p]->config_graphs[g]->time_k1 < min1)
// 			{
// 				min1 = config_page[p]->config_graphs[g]->time_k1;
// 				config1[g] = p;
// 			}
// 			if(config_page[p]->config_graphs[g]->time_k2 < min2)
// 			{
// 				min2 = config_page[p]->config_graphs[g]->time_k2;
// 				config2[g] = p;
// 			}
// 			if(config_page[p]->config_graphs[g]->time_k3 < min3)
// 			{
// 				min3 = config_page[p]->config_graphs[g]->time_k3;
// 				config3[g] = p;
// 			}
// 			if(config_page[p]->config_graphs[g]->time_k4 < min4)
// 			{
// 				min4 = config_page[p]->config_graphs[g]->time_k4;
// 				config4[g] = p;
// 			}
// 		}
// 		min1 = 9999999999999;
// 		min2 = 9999999999999;
// 		min3 = 9999999999999;
// 		min4 = 9999999999999;
// 	}
// 	
// 	for(int g = 0; g < NUMBER_OF_GRAPHS ; g++)
// 	{
// 		outfile << config_page[0]->config_graphs[g]->graph_name << ",";
// 		
// 		outfile << config_page[config1[g]]->config_name << ",";
// 		outfile << config_page[config1[g]]->config_graphs[g]->time_k1 << ",";
// 		outfile << config_page[config1[g]]->config_graphs[g]->overhead_ratio << ",";
// 		
// 		outfile << config_page[config2[g]]->config_name << ",";
// 		outfile << config_page[config2[g]]->config_graphs[g]->time_k1 << ",";
// 		outfile << config_page[config2[g]]->config_graphs[g]->overhead_ratio << ",";
// 		
// 		outfile << config_page[config3[g]]->config_name << ",";
// 		outfile << config_page[config3[g]]->config_graphs[g]->time_k1 << ",";
// 		outfile << config_page[config3[g]]->config_graphs[g]->overhead_ratio << ",";
// 		
// 		outfile << config_page[config4[g]]->config_name << ",";
// 		outfile << config_page[config4[g]]->config_graphs[g]->time_k1 << ",";
// 		outfile << config_page[config4[g]]->config_graphs[g]->overhead_ratio << ",";
// 		
// 		outfile << endl;
// 	}
// 	
// 	outfile.close();
// }
// 
// void adjust()
// {
// 	for(int g = 0; g < NUMBER_OF_GRAPHS ; g++)
// 	{
// 		config_page[1]->config_graphs[g]->time_h2d = config_page[0]->config_graphs[g]->time_h2d;
// 		config_page[1]->config_graphs[g]->time_d2h = config_page[0]->config_graphs[g]->time_d2h;
// 		config_page[1]->config_graphs[g]->calculate_totals();
// 	}
// }

void shutdown()
{
	for(int p = 0; p < NUMBER_OF_CONFIGS; p++)
	{
		for(int i = 0; i < NUMBER_OF_GRAPHS ; i++)
		{
			free(config_page[p]->config_graphs[i]);
		}
		free(config_page[p]);
	}
	free(temp);
	for(int p = 0; p < NUMBER_OF_KERNELS; p++)
		free(model_inst[p]);
	for(int p = 0; p < NUM_OF_ESTIMATIONS; p++)
		free(perf_inst[p]); 
}





