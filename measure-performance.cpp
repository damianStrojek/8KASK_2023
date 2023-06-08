//
// 	measure-performance.cpp - main file of a project
//
// 	2022-2023	Damian Strojek @damianStrojek
// 			Piotr Garbowski @dideek
// 			Jakub Wasniewski @wisnia01
//
// An application for monitoring performance and energy consumption in a computing cluster
//
// mpicxx -std=c++2a measure-performance.cpp metrics.cpp metrics-display.cpp metrics-save.cpp node-synchronization.cpp -o measure-performance
// mpirun -mca orte_keep_fqdn_hostnames t -mca btl_tcp_if_exclude docker0,docker_gwbridge,lo -hostfile hostfile.des measure-performance
//
// Project realised in academic years 2022-2023
// Gdansk University of Technology, Department of Computer Systems Architecture
// Supervised by PhD Eng Pawel Czarnul
// 

// External libraries
#include <iostream>
#include <string>
#include <cstring>
#include <stdexcept>
#include <termios.h>
#include <unistd.h>
#include <fcntl.h>
#include <mpi.h>
#include "json.hpp"
// Internal headers
#include "metrics.h"
#include "metrics-save.h"
#include "metrics-display.h"
#include "node-synchronization.h"

#define GPROCESSID 1				// PID of process that we are focused on (G stands for global)
#define DATA_BATCH 5				// How many times you want to download metrics
using json = nlohmann::json;

int main(int argc, char **argv){

	SystemMetrics systemMetrics;
	ProcessorMetrics processorMetrics;
	InputOutputMetrics inputOutputMetrics;
	MemoryMetrics memoryMetrics;
	NetworkMetrics networkMetrics;
	PowerMetrics powerMetrics;
	AllMetrics allMetrics;

	const char* dateCommand = "date +'%d%m-%H%M'";
	//allMetrics.nodeTimestamp = exec(dateCommand);
	//allMetrics.nodeTimestamp.pop_back();

	//std::string fileName = "results/" + exec(dateCommand) + "_metrics.json";
	std::string fileName = "results/" +"test_metrics.json";
	std::ofstream outputFile(fileName, std::ios::out);
	if(!outputFile.is_open()) std::cerr << "\n\n\t [ERROR] Unable to open file " << fileName << " for writing.\n";
	
	int rank, clusterSize;
	MPI_Init(&argc, &argv);
	MPI_Comm_rank(MPI_COMM_WORLD, &rank);
	MPI_Comm_size(MPI_COMM_WORLD, &clusterSize);

	MPI_Datatype systemMetricsType = createMpiSystemMetricsType();
	MPI_Datatype processorMetricsType = createMpiProcessorMetricsType();
	MPI_Datatype inputOutputMetricsType = createMpiInputOutputMetricsType();
	MPI_Datatype memoryMetricsType = createMpiMemoryMetricsType();
	MPI_Datatype networkMetricsType = createMpiNetworkMetricsType();
	MPI_Datatype powerMetricsType = createMpiPowerMetricsType();
	MPI_Datatype allMetricsType;

	int blocklengths[] = {1, 1, 1, 1, 1, 1};
    	MPI_Datatype types[] = {
		systemMetricsType, processorMetricsType, inputOutputMetricsType,
		memoryMetricsType, networkMetricsType, powerMetricsType};
    	MPI_Aint offsets[] = {
		offsetof(struct AllMetrics, systemMetrics),
		offsetof(struct AllMetrics, processorMetrics),
		offsetof(struct AllMetrics, inputOutputMetrics),
		offsetof(struct AllMetrics, memoryMetrics),
		offsetof(struct AllMetrics, networkMetrics),
		offsetof(struct AllMetrics, powerMetrics)};

	MPI_Type_create_struct(6, blocklengths, offsets, types, &allMetricsType);
	MPI_Type_commit(&allMetricsType);
	AllMetrics* allMetricsArray = new AllMetrics[clusterSize];
	nlohmann::json jsonArray;

	// Download metrics in constant batches
	for(int i = 0; i < DATA_BATCH; i++){

		//auto start = std::chrono::high_resolution_clock::now();

		getSystemMetrics(allMetrics.systemMetrics);
		getProcessorMetrics(allMetrics.processorMetrics);
		getInputOutputMetrics(allMetrics.inputOutputMetrics);
		getMemoryMetrics(allMetrics.memoryMetrics);
		getNetworkMetrics(allMetrics.networkMetrics);
		getPowerMetrics(allMetrics.powerMetrics);

		if(rank)
			MPI_Send(&allMetrics, 1, allMetricsType, 0, 0, MPI_COMM_WORLD);
		else {
			allMetricsArray[0] = allMetrics;
			for(int j = 1; j < clusterSize; j++)
				MPI_Recv(&allMetricsArray[j], 1, allMetricsType, j, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
			for(int j = 0; j < clusterSize; j++){
				std::cout << "\n\t[NODE " << j << " METRICS]\n\n";
				printMetrics(&allMetricsArray[j].systemMetrics, &allMetricsArray[j].processorMetrics, \
						&allMetricsArray[j].inputOutputMetrics, &allMetricsArray[j].memoryMetrics, \
						&allMetricsArray[j].networkMetrics, &allMetricsArray[j].powerMetrics);
			}
			jsonArray.push_back(metricsToJson(allMetricsArray,clusterSize,i));


		}
		
		//auto end = std::chrono::high_resolution_clock::now();
		//auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end-start);
		//std::cout << "Time taken to get all measures:" << duration.count() << "microseconds\n";
		if(rank==0) outputFile << jsonArray.dump(4);
		// Save metrics to file
		//writeToJSON(outputFile, allMetrics);
	}
	
	outputFile.close();
	MPI_Type_free(&systemMetricsType);
	MPI_Type_free(&processorMetricsType);
	MPI_Type_free(&inputOutputMetricsType);
	MPI_Type_free(&memoryMetricsType);
	MPI_Type_free(&networkMetricsType);
	MPI_Type_free(&powerMetricsType);
	MPI_Type_free(&allMetricsType);
	delete[] allMetricsArray;
   	MPI_Finalize();
	return 0;
};