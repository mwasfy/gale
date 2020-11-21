#include "hls_model.h"

//	float x = 16;
//	float y = 1957027;
hls_model_struct * model_inst = (hls_model_struct *)malloc(sizeof(hls_model_struct));

int main(int argc, char **argv){

// 	if(argc == 7){
// 		tmpchar = argv[1];  //graph inputfile
// 		filechar = argv[2];	//kernel file
// 		PLATFORM_NUM_FPGA = atoi(argv[3]);
// 		DEVICE_NUM_FPGA = atoi(argv[4]);
// 		file_format = atoi(argv[5]);
// 		directed = atoi(argv[6]); //Directed/undirected graph
// 	}
// 	else
// 	{
// 		fprintf(stderr, "You did something wrong!\n");
// 		exit(1);
// 	}


	model_inst->index = 0;
	model_inst->N = 589;
	model_inst->F = 275*(1e6);
	model_inst->II = 1;
	model_inst->mem_cycles = 288;
	//model_inst->M = 24; // 2 transaction * 8 bytes per transaction
	//model_inst->W = 145; //34.1GB/s BW div 240MHz
	model_inst->L = 7178413; // Number of edges

	//model_inst->R = 0;
	model_inst->P = 1;
	
	calculate_throughput();
	calculate_exec_time();
	print_time();
	calc_time();
	
}


void calculate_throughput()
{
	//cout << "Intitiation interval: " << model_inst->II <<  endl;
	double temp1,temp2;
	temp1 = (1/(model_inst->II+1));
	//temp2 = 1/ceil(model_inst->M / model_inst->W);
	temp2 = 1/model_inst->mem_cycles;
	cout << "temp1: " << temp1 <<  endl;
	cout << "temp2: " << temp2 <<  endl;
	
	if (temp1< temp2)
		model_inst->R = temp1;
	else
		model_inst->R = temp2;
	
	cout << "Estimated throughput: " << model_inst->R <<  endl;
}

void calculate_exec_time()
{
	//float temp = x + (y/model_inst->R);
	//float n = temp/x;
	model_inst->cycles = (model_inst->N + (model_inst->L / model_inst->R));
	//model_inst->cycles = model_inst->N + (model_inst->L * (model_inst->II+1));

	//model_inst->cycles = temp * model_inst->L;
	model_inst->exec_time = model_inst->cycles / model_inst->F;
}

void print_time()
{
	cout << "Estimated execution time: " << model_inst->exec_time <<  endl;
}

void calc_time()
{
	double tripcount [6];
	double deg [6];
	double real_time [6];
	double est_time[6];
	
	
	//road-roadNet-CA.mtx
	tripcount[0] = 2760388;
	real_time[0] = 0.525;
	deg[0] = 2.8;
	
	//road-asia-osm.mtx
	tripcount[1] = 12711603;
	real_time[1] = 2.74;
	deg[1] = 2.1;

	//web-it-2004.mtx
	tripcount[2] = 7178413;
	real_time[2] = 0.605;
	deg[2] = 6;//28.2

	//socfb-Georgetown15.mtx
	tripcount[3] = 425638;
	real_time[3] = 0.0359;
	deg[3] = 6;//90.4

	//socfb-Duke14.mtx
	tripcount[4] = 506437;
	real_time[4] = 0.04136;
	deg[4] = 6;//102.5

	//web-google.mtx
	tripcount[5] = 2773;
	real_time[5] = 0.01344;
	deg[5] = 0.1;//4.3
	
	
	
	
	
	
	double t_ovh,t_row,burst_size;
	double bl = 8;
	double dq = 4;
	double burst_cnt = 5;
	model_inst->mem_bytes = 4;
	double bw = 16/(model_inst->mem_cycles/model_inst->F);
	double avg_deg = 28.2;

	t_row = 13.5e-9 * 2;

	burst_size = pow(2,burst_cnt) * dq * bl;
	t_ovh = ((model_inst->mem_bytes * model_inst->L) / burst_size) * t_row;
	cout << "Estimated overhead: " << t_ovh  <<  endl;
	cout << endl << endl;

	
	//double bw = (double) 34.1*1024*1024*1024;
	//cout << "BW = " << bw << endl;
	//model_inst->t_ideal = (model_inst->mem_bytes * (model_inst->L/avg_deg)) / bw;
	//cout << "Estimated execution time: " << (model_inst->t_ideal + t_ovh )* 2 <<  endl;
	
	for (int i =0; i<6; i++)
	{
		cout << "#iter " << i << endl;
		est_time[i] = ((model_inst->mem_bytes * (tripcount[i]/deg[i])) / bw) * 2;
		cout << "Estimated execution time: " << est_time[i] <<  endl;
		cout << "Actual execution time: " << real_time[i] <<  endl;
		cout << "Accuracy: " << (1 - (abs(est_time[i] - real_time[i]) / real_time[i])) * 100 <<  endl << endl;
	}

	
}