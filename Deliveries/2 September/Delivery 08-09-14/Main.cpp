#include "BNetwork.h"
#include "Generator.h"
#include "Loan.h"
#include "RatingAgency.h"
#include <iostream>
#include <map>
#include <fstream>
#include "Parameters.h"
#include "Listener.h"

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
	model->couple(generator,bnetwork); //All output from generator is input of BNetwork
	Simulator<Loan>* sim = new Simulator<Loan>(model);


	//Add the rating agency
	RatingAgency* RA = new RatingAgency(bnetwork);
	model->add(RA);

	ofstream myfile;
 	myfile.open (argv[2]);

 	Listener* listener = new Listener();
 	sim->addEventListener(listener);

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
		//if( sim->nextEventTime() <= 4000){
			//sim->execNextEvent();
			//generator->set_n_banks( bnetwork->getCommercial_bankCount() );}


		//if( sim->nextEventTime() > 4000){

			//(bnetwork->getBank(0))->StopUpdatingProcess();

		//Pending: Function that restart parameters to initial conditions

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
		//}

		//if(sim->nextEventTime()>100) (bnetwork->getBank(2))->reduce_equity(2000); //just checking the network dynamic

	}

	//cout <<(bnetwork->getBank(0))->GetEquity() <<endl;
	cerr <<(bnetwork->getBank(0))->GetNPartners() <<endl;
	cerr <<(bnetwork->getBank(1))->GetNPartners() <<endl;
	cerr <<(bnetwork->getBank(2))->GetNPartners() <<endl;
//	cerr <<(bnetwork->getBank(7))->GetNPartners() <<endl;
//	cerr <<(bnetwork->getBank(9))->GetNPartners() <<endl;

	//cout <<(bnetwork->getBank(2))->GetLoanRate() <<endl;
	//cout <<(bnetwork->getBank(2))->GetNumberStatesVisited()<<endl; //Testing the assignment of states as keys
	cout <<(bnetwork->getBank(1))->GetNumberStatesVisited()<<endl; //Testing the assignment of states as keys

	cerr << "OK" ;
	myfile.close();
	delete listener;
	delete sim;
	delete model;
	return 0;
}
