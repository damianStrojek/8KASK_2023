//
//      metrics-display.cpp - file with definitions of functions related to displaying the metrics
//
//      2022-2023	Damian Strojek @damianStrojek
// 		    	Piotr Garbowski @dideek
// 		    	Jakub Wasniewski @wisnia01
//

// External libraries
#include <iostream>
#include <string>
#include <chrono>
#include <iomanip>
// Internal headers
#include "metrics.h"
#include "metrics-display.h"

void printMetricPair(std::string metricName, int metricValue, std::string metricUnit, 
			std::string metricNameTwo, int metricValueTwo, std::string metricUnitTwo){

	std::string value = std::to_string(metricValue), value2 = std::to_string(metricValueTwo);

	std::cout << metricName << std::right << std::setfill('.') << std::setw(30-metricName.length()) << value << metricUnit;
	for(int i=0; i<20-metricUnit.length(); i++) std::cout << ' ';

	std::cout << std::left << metricNameTwo << std::right << std::setfill('.') << 
		std::setw(30-metricNameTwo.length()) << value2 << metricUnitTwo << std::endl;

    return;
};

void printMetrics(SystemMetrics* systemMetrics, ProcessorMetrics* processorMetrics, 
			InputOutputMetrics* inputOutputMetrics, MemoryMetrics* memoryMetrics, 
			NetworkMetrics* networkMetrics, PowerMetrics* powerMetrics){

	auto now = std::chrono::system_clock::now();
  	std::time_t now_c = std::chrono::system_clock::to_time_t(now);
  	std::cout << std::put_time(std::localtime(&now_c), "%Y-%m-%d %X") << '\n';

	std::cout << "System:";
	for(int i=0;i<43;i++) std::cout << ' ';

	std::cout << "Network:" << '\n';
	printMetricPair("Running Processes", systemMetrics->processesRunning,"", "Received Packets",networkMetrics->receivedData,"");
	printMetricPair("All Processes", systemMetrics->processesAll,"","Received Packets Rate", networkMetrics->receivePacketRate,"KB/s");
	printMetricPair("Context Switch Rate", systemMetrics->contextSwitchRate,"/s","Sent Packets",networkMetrics->sentData,"");
	printMetricPair("Interrupt Rate", systemMetrics->interruptRate,"/s","Sent Packets Rate",networkMetrics->sendPacketsRate,"KB/s");
	std::cout << '\n';

	std::cout << "Memory:";
	for(int i=0;i<43;i++) std::cout << ' ';

	std::cout << "Processor:" << '\n';
	printMetricPair("Memory Used", memoryMetrics->memoryUsed,"MB","Time User",processorMetrics->timeUser,"USER_HZ");
	printMetricPair("Memory Cached", memoryMetrics->memoryCached,"MB","Time System",processorMetrics->timeSystem,"USER_HZ");
	printMetricPair("Swap Used", memoryMetrics->swapUsed,"MB","Time Idle",processorMetrics->timeIdle,"USER_HZ");
	printMetricPair("Swap Cached", memoryMetrics->swapCached,"MB","Time I/O Wait",processorMetrics->timeIoWait,"USER_HZ");
	printMetricPair("Memory Active", memoryMetrics->memoryActive,"MB","Time IRQ",processorMetrics->timeIRQ,"USER_HZ");
	printMetricPair("Memory Inactive", memoryMetrics->memoryInactive,"MB","Time Steal",processorMetrics->timeSteal,"USER_HZ");
	printMetricPair("Pages Read", memoryMetrics->pageInRate, "pages/s", "Cache L2 Hit Rate", processorMetrics->cacheL2HitRate, "");
	printMetricPair("Pages Saved", memoryMetrics->pageOutRate, "pages/s", "Cache L2 Miss Rate", processorMetrics->cacheL2MissRate, "");

	std::cout << "Power:";
	for(int i=0;i<43;i++) std::cout << ' ';

	std::cout << '\n'<< "I/O for PID 1:" << '\n';
	printMetricPair("Data Read ",inputOutputMetrics->dataRead,"MB", "Power for Cores", powerMetrics->coresPower, "W");
	printMetricPair("Data Written ",inputOutputMetrics->dataWritten,"MB", "Processor Power", powerMetrics->processorPower, "W");
	printMetricPair("Read Operations ",inputOutputMetrics->readOperationsRate,"/s", "Memory Power", powerMetrics->memoryPower, "W");
	printMetricPair("Write Operations ",inputOutputMetrics->writeOperationsRate,"/s", "GPU Power", powerMetrics->gpuPower, "W");
	std::cout << '\n';

    return;
};