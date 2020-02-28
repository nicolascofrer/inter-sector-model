#include "BNetwork.h"
#include "Generator.h"
#include "Loan.h"
#include "RatingAgency.h"
#include "Bank.h"
#include <iostream>
#include <map>
#include <fstream>
#include "Parameters.h"
#include "Listener.h"
#include <time.h>
using namespace adevs;
using namespace std;

int main(int argc, char** argv) {
//	int main() {

	int runtime=atoi(argv[1]);
	clock_t t1, t2;
	t1 = clock();
	BNetwork* bnetwork = new BNetwork();
	unsigned long seed;
	if (argc != 2) {
		//cerr << "argv[1] = random seed and argv[2] = Output file" << endl;
		cerr << "Please enter an integer for runtime argument" << endl;
		return 1;
	}

	//seed = atoi(argv[1]);
	seed=1;
	Generator* generator = new Generator(seed);
	SimpleDigraph<Loan>* model = new SimpleDigraph<Loan>();
	model->add(bnetwork);
	model->add(generator);
	model->couple(generator, bnetwork); //All output from generator is input of BNetwork
	Simulator<Loan>* sim = new Simulator<Loan>(model);

	//Add the rating agency
	RatingAgency* RA = new RatingAgency(bnetwork);
	model->add(RA);

	//Output files with info coming directly from banks, transactions are written by listener
	ofstream generalInfo;
//	generalInfo.open(argv[2]);
	generalInfo.open("generalInfo.txt");

	// 	Cash
	ofstream cashOutputfile;
	cashOutputfile.open("cash.txt");
	// 	Customer loans
	ofstream customerLoansOutputfile;
	customerLoansOutputfile.open("customerLoans.txt");
	// 	Customer deposits
	ofstream customerDepositsOutputfile;
	customerDepositsOutputfile.open("customerDeposits.txt");
	// 	IB loans
	ofstream IBLoansOutputfile;
	IBLoansOutputfile.open("IBLoans.txt");
	// 	IB deposits
	ofstream IBDepositsOutputfile;
	IBDepositsOutputfile.open("IBDeposits.txt");
	// 	CB lending facility
	ofstream CBLendingFacilityOutputfile;
	CBLendingFacilityOutputfile.open("CBLendingFacility.txt");
	// 	Equity
	ofstream equityOutputfile;
	equityOutputfile.open("equity.txt");

	//Partners Set
	ofstream networkOutputfile;
	networkOutputfile.open("network.txt");

	//dV
	ofstream dV;
	dV.open("dV.txt");

	Listener* listener = new Listener();
	sim->addEventListener(listener);

	// Write a header describing the data fields
	generalInfo << "Time,";
	cashOutputfile << "Time,";
	customerLoansOutputfile << "Time,";
	customerDepositsOutputfile << "Time,";
	IBLoansOutputfile << "Time,";
	IBDepositsOutputfile << "Time,";
	CBLendingFacilityOutputfile << "Time,";
	equityOutputfile << "Time,";

	networkOutputfile << "Time,";
	//Other files
	dV << "Time,dV"<< endl;

	for (int i = 0; i != NMAX; i++) {
		networkOutputfile << "Partners Bank " << i << ",";
	}
	networkOutputfile << "Partners Bank " << NMAX << endl;

	generalInfo << "Time between arrivals,";
	generalInfo << "Number of banks,";
	generalInfo << "Number of deposit arrivals,";
	generalInfo << "Number of loan arrivals" << endl;

	//Cash
	for (int i = 0; i != NMAX; i++) {
		cashOutputfile << "Cash Bank " << i << ",";
	}
	cashOutputfile << "Cash Bank " << NMAX << endl;
	//customerLoans
	for (int i = 0; i != NMAX; i++) {
		customerLoansOutputfile << "Customer Loans Bank " << i << ",";
	}
	customerLoansOutputfile << "Customer Loans Bank " << NMAX << endl;
	//customerDeposits
	for (int i = 0; i != NMAX; i++) {
		customerDepositsOutputfile << "Customer Deposits Bank " << i << ",";
	}
	customerDepositsOutputfile << "Customer Deposits Bank " << NMAX << endl;
	//IBLoans
	for (int i = 0; i != NMAX; i++) {
		IBLoansOutputfile << "IB Loans Bank " << i << ",";
	}
	IBLoansOutputfile << "IB Loans Bank " << NMAX << endl;
	//IBDeposits
	for (int i = 0; i != NMAX; i++) {
		IBDepositsOutputfile << "IB Deposits Bank " << i << ",";
	}
	IBDepositsOutputfile << "IB Deposits Bank " << NMAX << endl;
	//CBLendingFacility
	for (int i = 0; i != NMAX; i++) {
		CBLendingFacilityOutputfile << "CB Lending Facility Bank " << i << ",";
	}
	CBLendingFacilityOutputfile << "CB Lending Facility Bank " << NMAX << endl;
	//equity
	for (int i = 0; i != NMAX; i++) {
		equityOutputfile << "Equity Bank " << i << ",";
	}
	equityOutputfile << "Equity Bank " << NMAX << endl;

	while (sim->nextEventTime() <= runtime) {
//		cout << "Next event time is "<< sim->nextEventTime() << endl;
		//if( sim->nextEventTime() <= 4000){
		//sim->execNextEvent();
		//generator->set_n_banks( bnetwork->getBankCount() );}

//		if( Bank::getWdV() < 0.000000001)
//		{
//		Bank::StopUpdatingProcess();
//		cout << "Updating has stopped" << endl;
//		}
		//Pending: Function that restart parameters to initial conditions
		//generator->set_n_banks( bnetwork->getBankCount() );

		Bank::setTime( sim->nextEventTime());

		sim->execNextEvent();
		//write time to output files
		generalInfo << sim->nextEventTime() << ",";
		cashOutputfile << sim->nextEventTime() << ",";
		customerLoansOutputfile << sim->nextEventTime() << ",";
		customerDepositsOutputfile << sim->nextEventTime() << ",";
		IBLoansOutputfile << sim->nextEventTime() << ",";
		IBDepositsOutputfile << sim->nextEventTime() << ",";
		CBLendingFacilityOutputfile << sim->nextEventTime() << ",";
		equityOutputfile << sim->nextEventTime() << ",";
		networkOutputfile << sim->nextEventTime() << ",";
		dV << sim->nextEventTime() << ",";
		// Write change in weighted dV
		dV << Bank::getWdV() << endl;

		//write the time between arrivals
		generalInfo << generator->ta() << ",";
		//write the number of banks
		generalInfo << BNetwork::getBankCount() << ",";
		//write the number of deposits
		generalInfo << generator->get_number_of_deposits() << ",";
		generalInfo << generator->get_number_of_loans() << ",";
		//Cash
		for (int i = 0; i != NMAX - 1; i++) {
			if (!BNetwork::IsBankrupt(bnetwork->getBankIncludingBankrupt(i)))

				cashOutputfile
						<< (bnetwork->getBankIncludingBankrupt(i))->getCash()
						<< ",";

			else
				cashOutputfile << "" << ",";
		}
		if (!BNetwork::IsBankrupt(bnetwork->getBankIncludingBankrupt(NMAX)))

			cashOutputfile
					<< (bnetwork->getBankIncludingBankrupt(NMAX))->getCash()
					<< endl;

		else
			cashOutputfile << "" << endl;

		//customerLoan
		for (int i = 0; i != NMAX - 1; i++) {
			if (!BNetwork::IsBankrupt(bnetwork->getBankIncludingBankrupt(i)))

				customerLoansOutputfile
						<< (bnetwork->getBankIncludingBankrupt(i))->getLoanQueueSize()
						<< ",";

			else
				customerLoansOutputfile << "" << ",";
		}
		if (!BNetwork::IsBankrupt(bnetwork->getBankIncludingBankrupt(NMAX)))

			customerLoansOutputfile
					<< (bnetwork->getBankIncludingBankrupt(NMAX))->getLoanQueueSize()
					<< endl;

		else
			customerLoansOutputfile << "" << endl;

		//customerDeposit
		for (int i = 0; i != NMAX - 1; i++) {
			if (!BNetwork::IsBankrupt(bnetwork->getBankIncludingBankrupt(i)))

				customerDepositsOutputfile
						<< (bnetwork->getBankIncludingBankrupt(i))->getDepositQueueSize()
						<< ",";

			else
				customerDepositsOutputfile << "" << ",";
		}
		if (!BNetwork::IsBankrupt(bnetwork->getBankIncludingBankrupt(NMAX)))

			customerDepositsOutputfile
					<< (bnetwork->getBankIncludingBankrupt(NMAX))->getDepositQueueSize()
					<< endl;

		else
			customerDepositsOutputfile << "" << endl;

		//IBLoans
		for (int i = 0; i != NMAX - 1; i++) {
			if (!BNetwork::IsBankrupt(bnetwork->getBankIncludingBankrupt(i)))

				IBLoansOutputfile
						<< (bnetwork->getBankIncludingBankrupt(i))->getIBLoanAcceptedQueueSize()
						<< ",";

			else
				IBLoansOutputfile << "" << ",";
		}
		if (!BNetwork::IsBankrupt(bnetwork->getBankIncludingBankrupt(NMAX)))

			IBLoansOutputfile
					<< (bnetwork->getBankIncludingBankrupt(NMAX))->getIBLoanAcceptedQueueSize()
					<< endl;

		else
			IBLoansOutputfile << "" << endl;

		//IBDeposits
		for (int i = 0; i != NMAX - 1; i++) {
			if (!BNetwork::IsBankrupt(bnetwork->getBankIncludingBankrupt(i)))

				IBDepositsOutputfile
						<< (bnetwork->getBankIncludingBankrupt(i))->getIBDepositAcceptedQueueSize()
						<< ",";

			else
				IBDepositsOutputfile << "" << ",";
		}
		if (!BNetwork::IsBankrupt(bnetwork->getBankIncludingBankrupt(NMAX)))

			IBDepositsOutputfile
					<< (bnetwork->getBankIncludingBankrupt(NMAX))->getIBDepositAcceptedQueueSize()
					<< endl;

		else
			IBDepositsOutputfile << "" << endl;

		//CBLendingFacility
		for (int i = 0; i != NMAX - 1; i++) {
			if (!BNetwork::IsBankrupt(bnetwork->getBankIncludingBankrupt(i)))

				CBLendingFacilityOutputfile
						<< (bnetwork->getBankIncludingBankrupt(i))->getCBLendingFacility()
						<< ",";

			else
				CBLendingFacilityOutputfile << "" << ",";
		}
		if (!BNetwork::IsBankrupt(bnetwork->getBankIncludingBankrupt(NMAX)))

			CBLendingFacilityOutputfile
					<< (bnetwork->getBankIncludingBankrupt(NMAX))->getCBLendingFacility()
					<< endl;

		else
			CBLendingFacilityOutputfile << "" << endl;

		//Equity
		for (int i = 0; i != NMAX - 1; i++) {
			if (!BNetwork::IsBankrupt(bnetwork->getBankIncludingBankrupt(i)))

				equityOutputfile
						<< (bnetwork->getBankIncludingBankrupt(i))->GetEquity()
						<< ",";

			else
				equityOutputfile << "" << ",";
		}
		if (!BNetwork::IsBankrupt(bnetwork->getBankIncludingBankrupt(NMAX)))

			equityOutputfile
					<< (bnetwork->getBankIncludingBankrupt(NMAX))->GetEquity()
					<< endl;

		else
			equityOutputfile << "" << endl;

		//Partners info
		for (int i = 0; i != NMAX - 1; i++) {
			if (!BNetwork::IsBankrupt(bnetwork->getBankIncludingBankrupt(i)))

				networkOutputfile
						<< (bnetwork->getBankIncludingBankrupt(i))->printPartners()
						<< ",";

			else
				networkOutputfile << "" << ",";
		}

		if (!BNetwork::IsBankrupt(bnetwork->getBankIncludingBankrupt(NMAX)))

			networkOutputfile
					<< (bnetwork->getBankIncludingBankrupt( NMAX))->printPartners()
					<< endl;

		else
			networkOutputfile << "" << endl;

	}

	cout << "Next event time is "<< sim->nextEventTime() << endl;
	cout << "Number of active banks is " << BNetwork::getBankCount() << endl;
	cout << "Number of banks constructed is " << Bank::getBankCounter() << endl;

	generalInfo.close();
	networkOutputfile.close();
	cashOutputfile.close();
	customerLoansOutputfile.close();
	customerDepositsOutputfile.close();
	IBLoansOutputfile.close();
	IBDepositsOutputfile.close();
	CBLendingFacilityOutputfile.close();
	equityOutputfile.close();
	dV.close();

	delete listener;
	delete sim;
	delete model;
	t2 = clock();
	float d((float) t2 - (float) t1);
	float seconds = d / CLOCKS_PER_SEC;
	cout
			<< "----------Finished without error (more precisely, without aborting)----------"
			<< endl;
	cout << "Execution Time " << seconds << " seconds" << endl;

	return 0;
}
