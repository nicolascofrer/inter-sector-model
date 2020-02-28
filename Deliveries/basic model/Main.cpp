#include "BNetwork.h"
#include "Generator.h"
#include "Agent.h"
#include <iostream>
#include <map>
#include <fstream>
#include "proba.hpp"
using namespace adevs;
using namespace std;





int main(int argc, char** argv)
{

	BNetwork* bnetwork = new BNetwork();
	long seed;
	if (argc!=3){ cerr << "argv[1] = random seed and argv[2] = Output file" << endl;
	return 1;}

	seed = atoi(argv[1]);
	Generator* generator = new Generator(seed);
	SimpleDigraph<Loan>* model = new SimpleDigraph<Loan>();
	model->add(bnetwork);
	model->add(generator);
	model->couple(generator,bnetwork);
	Simulator<Loan>* sim = new Simulator<Loan>(model);


	ofstream myfile;
 	myfile.open (argv[2]);

	// Write a header describing the data fields
	myfile << "Time,";
	myfile << "Time between arrivals,";
	myfile << "Number of banks,";
	myfile << "Number of deposit arrivals,";
	myfile << "Number of deposit in queue at bank 1,";
	myfile << "Number of deposit in queue at bank 2,";
	myfile << "Number of deposit in queue at bank 3" << endl;

	while (sim->nextEventTime() <= 200.0)
	{

		myfile << sim->nextEventTime() << ",";
		myfile <<  generator->ta() << ",";
		sim->execNextEvent();
		myfile << bnetwork->getCommercial_bankCount() << ",";
		generator->set_n_banks( bnetwork->getCommercial_bankCount() );
		myfile << generator->get_number_of_deposits() << ",";

		myfile <<  (bnetwork->getBank(0))->getDepositQueueSize()   << ",";
		myfile <<  (bnetwork->getBank(1))->getDepositQueueSize()   << ",";
		myfile <<  (bnetwork->getBank(2))->getDepositQueueSize()   << endl;

		if(sim->nextEventTime()>100) (bnetwork->getBank(2))->reduce_equity(2000); //just checking the network dynamic

	}

	cout << "OK" ;
	myfile.close();
	delete sim;
	delete model;
	return 0;
}
