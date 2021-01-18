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


#include "perf_model.h"
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
		
	write_stat_report();
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
	model->graph_name = graph_db[model->graph_id].graph_name;
	model->kernel_name = model_inst[model->model_id]->config_name;
	model->num_nodes = graph_db[model->graph_id].num_nodes;
	model->num_edges = graph_db[model->graph_id].num_edges;
	//Calculate edge to node ratio
	double n_to_e = graph_db[model->graph_id].num_edges / graph_db[model->graph_id].num_nodes;
	double cycles = 0;
	double burst_size = 2;
	double est_total_access_1 = 0;
	double est_total_access_2 = 0;
	double LU = model_inst[model->model_id]->P_lu;
	
	//Pipe stages calculation
	cycles += model_inst[model->model_id]->N + (graph_db[model->graph_id].num_nodes * (model_inst[model->model_id]->II+1)/  model_inst[model->model_id]->P_lu);
	model->est_pipe_time = cycles / model_inst[model->model_id]->F;
	
	//Estiamted total accesses for the graph instance (Number of total accesses)
	double temp = (n_to_e / (burst_size * LU)) / LU;
	est_total_access_1 += (graph_db[model->graph_id].num_nodes * model_inst[model->model_id]->num_mem_accesses_1) / LU;
	est_total_access_1 *= model_inst[model->model_id]->mem_cycles_1;
	
	est_total_access_2 += (temp * graph_db[model->graph_id].num_nodes * model_inst[model->model_id]->num_mem_accesses_2) / LU;
	est_total_access_2 += ((temp * graph_db[model->graph_id].num_nodes * model_inst[model->model_id]->num_mem_accesses_2) / LU) * (LU-2);
	est_total_access_2 *= model_inst[model->model_id]->mem_cycles_2;


	if(graph_db[model->graph_id].num_nodes > 2000)
	{
		est_total_access_2 = est_total_access_2 / (LARGE_MOD_FACTOR/(LU/2));
	}
	else 
	{
		est_total_access_2 = est_total_access_2 / (SMALL_MOD_FACTOR/(LU/2));
	}
	

	model->est_mem_time = est_total_access_1 + est_total_access_2;
	model->est_mem_time = model->est_mem_time / model_inst[model->model_id]->F;
	//Total estimated time
	model->est_tot_time = (model->est_pipe_time + model->est_mem_time) * 1000; 
	
	//Get Measured execution time
	model->real_time = get_real_time(graph_db[model->graph_id].graph_name, model->model_id);
}

//Calculate the performance estimation for one graph instance on hardware implementation
// void calc_perf(perf_model * model)
// {
// 	model->graph_name = graph_db[model->graph_id].graph_name;
// 	model->kernel_name = model_inst[model->model_id]->config_name;
// 	model->num_nodes = graph_db[model->graph_id].num_nodes;
// 	model->num_edges = graph_db[model->graph_id].num_edges;
// 	//Calculate edge to node ratio
// 	double n_to_e = graph_db[model->graph_id].num_edges / graph_db[model->graph_id].num_nodes;
// 	double cycles = 0;
// 	double burst_size = 4;
// 	double est_total_access_1 = 0;
// 	double est_total_access_2 = 0;
// 	
// 	//Pipe stages calculation
// 	cycles += model_inst[model->model_id]->N + ((graph_db[model->graph_id].num_nodes/model_inst[model->model_id]->P_lu) * (model_inst[model->model_id]->II+1));
// 	//cycles /= model_inst[model->model_id]->P_lu;
// 	model->est_pipe_time = cycles / model_inst[model->model_id]->F;
// 	
// 	//Estiamted total accesses for the graph instance (Number of total accesses)
// 	double temp = (n_to_e / burst_size);
// 	double LU_FACTOR =  model_inst[model->model_id]->P_lu - 2;
// 	est_total_access_1 += graph_db[model->graph_id].num_nodes * model_inst[model->model_id]->num_mem_accesses_1;
// 	est_total_access_1 *= model_inst[model->model_id]->mem_cycles_1;
// 	est_total_access_1 /= model_inst[model->model_id]->P_lu;
// 	
// // 	est_total_access_2 += temp * (graph_db[model->graph_id].num_nodes) * model_inst[model->model_id]->num_mem_accesses_2 ;
// // 	est_total_access_2 *= model_inst[model->model_id]->mem_cycles_2;
// // 	est_total_access_2 /= model_inst[model->model_id]->P_lu;
// 
// 	double iters = (graph_db[model->graph_id].num_nodes) / (model_inst[model->model_id]->P_lu);
// 	est_total_access_2 += (temp * iters * model_inst[model->model_id]->num_mem_accesses_2);
// 	est_total_access_2 += (temp * iters);
// 	est_total_access_2 += (temp * iters * model_inst[model->model_id]->num_mem_accesses_2 *LU_FACTOR) ;
// 	est_total_access_2 += (temp * iters *LU_FACTOR);
// 	est_total_access_2 *= model_inst[model->model_id]->mem_cycles_2;
// 
// 
// 	if(graph_db[model->graph_id].num_nodes > 2000)
// 	{
// 		est_total_access_2 = est_total_access_2 / LARGE_MOD_FACTOR;
// 	}
// 	else 
// 	{
// 		est_total_access_2 = est_total_access_2 / SMALL_MOD_FACTOR;
// 	}
// 	
// 
// 	model->est_mem_time = est_total_access_1 + est_total_access_2;
// 	model->est_mem_time = model->est_mem_time / model_inst[model->model_id]->F;
// 	//Total estimated time
// 	model->est_tot_time = (model->est_pipe_time + model->est_mem_time) * 1000; 
// 	
// 	//Get Measured execution time
// 	model->real_time = get_real_time(graph_db[model->graph_id].graph_name, model->model_id);
// }

//Retrieve the real measured execution time
double get_real_time(const std::string& str, int n)
{	
	int page_id = n;
	int gr_id = 100;
	
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

	if (n==0 || n==2 || n==3)
	{
		return config_page[page_id]->config_graphs[gr_id]->time_k3 / config_page[page_id]->config_graphs[gr_id]->num_iter;
	}
	else if (n==1 || n==4 || n==5 || n==6)
	{
		return config_page[page_id]->config_graphs[gr_id]->time_k1 / config_page[page_id]->config_graphs[gr_id]->num_iter;
	}
	else 
	{
		return config_page[page_id]->config_graphs[gr_id]->time_k2 / config_page[page_id]->config_graphs[gr_id]->num_iter;
	}

}

int read_pages(string page_name)
{
	int page_id = page_convert(page_name);
	config_page[page_id]->config_idx = page_id;
	config_page[page_id]->config_name = page_name;
	string page_path_pre = "timing_info/";
	string page_path_post = ".o";
	string page_path = page_path_pre + page_name + page_path_post;
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
		else if(nextLine.compare(0,3,"The") == 0)
		{
			int temp_1;
			ss << nextLine;
			string stemp;
			while (!ss.eof()) { 
				/* extracting word by word from stream */
				ss >> stemp; 
		  
				/* Checking the given word is integer or not */
				if (stringstream(stemp) >> temp_int)
				{
					temp_1 = temp_int;
				}
			}
			//cout << "Iters  = " << temp_1 << endl; 
			temp->num_iter = temp_1;
			temp_int = 0;
			temp_1 = 0;
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
	temp->num_iter = 0;
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
	temp2->num_iter = temp1->num_iter;
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
		cout << "Iteration count: " << config_page[page_num]->config_graphs[x]->num_iter << endl;
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
	else if(str == "PRK_CU2") return PRK_CU2;
	else if(str == "PRK_CU4") return PRK_CU4;
    else if(str == "PRK_LU2") return PRK_LU2;
    else if(str == "PRK_LU4") return PRK_LU4;
    else if(str == "PRK_LU8") return PRK_LU8;
    else if(str == "SSSP_BASE") return SSSP_BASE;
    else if(str == "SSSP_SWI") return SSSP_SWI;
    else if(str == "SSSP_CU2") return SSSP_CU2;
    else if(str == "SSSP_CU4") return SSSP_CU4;
    else if(str == "SSSP_LU2") return SSSP_LU2;
    else if(str == "SSSP_LU4") return SSSP_LU4;
    else if(str == "SSSP_LU8") return SSSP_LU8;
	else if(str == "MIS_BASE") return MIS_BASE;
	else if(str == "MIS_SWI") return MIS_SWI;
	else if(str == "MIS_CU2") return MIS_CU2;
	else if(str == "MIS_CU4") return MIS_CU4;
	else if(str == "MIS_LU2") return MIS_LU2;
	else if(str == "MIS_LU4") return MIS_LU4;
	else if(str == "MIS_LU8") return MIS_LU8;
}

string get_page_name(int n)
{
    if(n == 0) return "PRK_BASE";
    else if(n == 1) return "PRK_SWI";
    else if(n == 2) return "PRK_CU2";
    else if(n == 3) return "PRK_CU4";
    else if(n == 4) return "PRK_LU2";
	else if(n == 5) return "PRK_LU4";
	else if(n == 6) return "PRK_LU8";
	else if(n == 7) return "SSSP_BASE";
	else if(n == 8) return "SSSP_SWI";
    else if(n == 9) return "SSSP_CU2";
    else if(n == 10) return "SSSP_CU4";
    else if(n == 11) return "SSSP_LU2";
    else if(n == 12) return "SSSP_LU4";
	else if(n == 13) return "SSSP_LU8";
	else if(n == 14) return "MIS_BASE";
	else if(n == 15) return "MIS_SWI";
	else if(n == 16) return "MIS_CU2";
    else if(n == 17) return "MIS_CU4";
    else if(n == 18) return "MIS_LU2";
    else if(n == 19) return "MIS_LU4";
    else if(n == 20) return "MIS_LU8";
}

void write_stat_report()
{
	ofstream outfile("performance_model_stat.rpt");
	for(int i = 0; i < NUM_OF_ESTIMATIONS; i++)
	if(perf_inst[i]->kernel_name == "PRK_SWI" || perf_inst[i]->kernel_name == "PRK_LU2" || perf_inst[i]->kernel_name == "PRK_LU4" || perf_inst[i]->kernel_name == "PRK_LU8" || perf_inst[i]->kernel_name == "SSSP_SWI" || perf_inst[i]->kernel_name == "SSSP_LU2" || perf_inst[i]->kernel_name == "SSSP_LU4" || perf_inst[i]->kernel_name == "SSSP_LU8" || perf_inst[i]->kernel_name == "MIS_SWI" || perf_inst[i]->kernel_name == "MIS_LU2" || perf_inst[i]->kernel_name == "MIS_LU4" || perf_inst[i]->kernel_name == "MIS_LU8")
	{
		outfile << "Kernel: " << perf_inst[i]->kernel_name << "\t Graph: " << perf_inst[i]->graph_name <<  endl;
		outfile << "Nodes: " << perf_inst[i]->num_nodes << "\t Edges: " << perf_inst[i]->num_edges << endl;
		outfile << "---------------------------------------------------------------------" << endl;
		outfile << "Estimated pipe time: " << perf_inst[i]->est_pipe_time * 1000 <<  endl;
		outfile << "Estimated mem time: " << perf_inst[i]->est_mem_time * 1000 <<  endl;
		outfile << "Estimated execution time: " << perf_inst[i]->est_tot_time <<  endl;
		outfile << "Actual execution time: " << perf_inst[i]->real_time <<  endl;
		outfile << "Accuracy: " << (1 - (abs(perf_inst[i]->est_tot_time - perf_inst[i]->real_time) / perf_inst[i]->real_time)) * 100 <<  "%" << endl;
		outfile << "%error: " << (abs(perf_inst[i]->real_time - perf_inst[i]->est_tot_time) / perf_inst[i]->real_time) * 100 <<  "%" << endl;
		outfile << "---------------------------------------------------------------------"  << endl;
		outfile << "---------------------------------------------------------------------"  << endl << endl;
	}	
	outfile.close();
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
	for(int p = 0; p < NUMBER_OF_CONFIGS; p++)
		free(model_inst[p]);
	for(int p = 0; p < NUM_OF_ESTIMATIONS; p++)
		free(perf_inst[p]); 
}





