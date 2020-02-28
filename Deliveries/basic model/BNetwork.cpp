#include "BNetwork.h"
#include "Agent.h"


using namespace adevs;
using namespace std;
#define NBANKS   3	//Number of banks at the beginning

BNetwork::BNetwork():
	Network<Loan>() // call the parent constructor
{
	// Add initial banks
	for (int i = 0; i != NBANKS ; i++) add_commercial_bank();
}

void BNetwork::getComponents(Set<Devs<Loan>*>& c) {
	// get the list of network's banks
	vector<Commercial_bank*>::iterator iter;
	for (iter = commercial_banks.begin(); iter != commercial_banks.end(); iter++)
		c.insert(*iter);
}

void BNetwork::route(const Loan& loan, Devs<Loan>* src, Bag<Event<Loan> >& r) {
	//Here we route the network's input to specific banks

	// If this is a bank output, then it leaves the network
	if (src != this) { 
		r.insert(Event<Loan>(this,loan));
		return;
	}


	//Assign a bank to the loan object
	Commercial_bank* b = getBank( loan.IDborrower );

	r.insert(Event<Loan>(b,loan));
}

bool BNetwork::model_transition() {			//Here we have conditions over banks that modify the network structure, here is the dynamic of the network
	//Remove banks declared insolvent
	vector<Commercial_bank*>::iterator iter = commercial_banks.begin();
	while (iter != commercial_banks.end()) {
		if ((*iter)->checkInsolvent()) iter = commercial_banks.erase(iter);
		else iter++;
	}

	return false;
}

void BNetwork::add_commercial_bank() {
	commercial_banks.push_back(new Commercial_bank());
	commercial_banks.back()->setParent(this);

}


int BNetwork::getCommercial_bankCount() {
	return commercial_banks.size();
}

BNetwork::~BNetwork() {
	// Delete all banks
	vector<Commercial_bank*>::iterator iter;
	for (iter = commercial_banks.begin(); iter != commercial_banks.end(); iter++)
		delete *iter;
}

Commercial_bank* BNetwork::getBank (int n){return (commercial_banks[n]);}



