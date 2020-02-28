#include "BNetwork.h"
#include "Agent.h"
using namespace adevs;
using namespace std;
#include "Parameters.h"
BNetwork::BNetwork():
	Network<Loan>() // call the parent constructor
{
	// Add initial banks
	for (int i = 0; i != NBANKS-1 ; i++) add_commercial_bank(PDefaul1Loan1);
	add_commercial_bank(PDefaultLoan2);

}

void BNetwork::getComponents(Set<Devs<Loan>*>& c) {
	// get the list of network's banks
	for (int i = 0; i != NBANKS-1 ; i++){
		//Here we have to set the relations between banks
		for(int j=(i+1); commercial_banks[i]->GetNPartners() < commercial_banks[i]->GetAllowedNPartners(); j++){
			if(commercial_banks[j%(NBANKS)]->GetNPartners() < commercial_banks[j%(NBANKS)]->GetAllowedNPartners()){ //Check if banks can be partners
			commercial_banks[i]->AddPartner(commercial_banks[j%(NBANKS)]);
			commercial_banks[j%(NBANKS)]->AddPartner(commercial_banks[i]);
			}

		}
		c.insert(commercial_banks[i]);}
}


void BNetwork::route(const Loan& loan, Devs<Loan>* src, Bag<Event<Loan> >& r) {
	//Here we route the network's input to specific banks

	// If this is a bank output, then it leaves the network
	if (src != this) { 
		r.insert(Event<Loan>(this,loan));
		return;
	}
	//look for the type of input, this could be changed with portvalue objects
	if(loan.getID()==1)
	//Assign a bank to the loan object
	{r.insert(Event<Loan>(getBank( loan.get_IDborrower() ),loan));}
	//Assign a bank that is being asked for a loan
	else{r.insert(Event<Loan>(getBank( loan.get_IDlender() ),loan));}


}

bool BNetwork::model_transition() {
	//Here we have conditions over banks that modify the network structure, here is the dynamic of the network
	//Remove banks declared insolvent
	vector<Commercial_bank*>::iterator iter = commercial_banks.begin();
	while (iter != commercial_banks.end()) {
		if ((*iter)->checkInsolvent()) iter = commercial_banks.erase(iter);
		else iter++;
	}

	return false;
}

void BNetwork::add_commercial_bank(double PDef) {
	commercial_banks.push_back(new Commercial_bank(PDef));
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



