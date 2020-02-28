#include "BNetwork.h"
#include "Generator.h"
#include "Agent.h"
#include <iostream>
#include <map>
#include <fstream>
#include "Parameters.h"

using namespace adevs;
using namespace std;


int main(int argc, char** argv)
{

	BNetwork* bnetwork = new BNetwork();
	unsigned long seed;
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
	myfile << "Number of loan arrivals,";
	myfile << "Value last state-action,";


	for (int i = 0; i != NBANKS-1 ; i++)
			{myfile <<  "Equity Bank "<< i+1  << ",";}
			myfile << "Equity Bank "<< NBANKS << endl;

	while (sim->nextEventTime() <= 1000.0)
	{

		sim->execNextEvent();
		//write time
		myfile << sim->nextEventTime() << ",";
		//write the time between arrivals
		myfile <<  generator->ta() << ",";
		//write the number of banks
		myfile << bnetwork->getCommercial_bankCount() << ",";
		//write the number of deposits
		myfile << generator->get_number_of_deposits() << ",";
		myfile << generator->get_number_of_loans() << ",";

		generator->set_n_banks( bnetwork->getCommercial_bankCount() );

		myfile <<(bnetwork->getBank(0))->LastStateActionValue() <<",";

		for (int i = 0; i != bnetwork->getCommercial_bankCount()-1 ; i++)
		{myfile <<  (bnetwork->getBank(i))->GetEquity()   << ",";}

		myfile << (bnetwork->getBank( bnetwork->getCommercial_bankCount()-1 ))->GetEquity() << endl;


		//if(sim->nextEventTime()>100) (bnetwork->getBank(2))->reduce_equity(2000); //just checking the network dynamic

	}

	cout <<(bnetwork->getBank(0))->GetEquity() <<endl;
	//cout <<(bnetwork->getBank(2))->GetLoanRate() <<endl;
	//cerr <<(bnetwork->getBank(2))->GetNumberStatesVisited()<<endl; //Testing the assignment of states as keys
	cerr << "OK" ;
	myfile.close();
	delete sim;
	delete model;
	return 0;
}
