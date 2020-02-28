#include "BNetwork.h"
#include "Loan.h"
using namespace adevs;
using namespace std;
#include "Parameters.h"
#include <iostream>

vector< Commercial_bank*> BNetwork::commercial_banks;
map< Agent*, int > BNetwork::commercial_banks_deref;

BNetwork::BNetwork():
	Network<Loan>() // call the parent constructor
{
	// Add initial banks
	for (int i = 0; i != NBANKS-1 ; i++) add_commercial_bank(PDefaul1Loan1);
	add_commercial_bank(PDefaultLoan2);

	Commercial_bank::SetDepositRateRange(customerDepositInterestRate,customerDepositInterestRate+0.1);
	Commercial_bank::SetLoanRateRange(customerLoanInterestRate,customerLoanInterestRate+0.1);
	Commercial_bank::SetCBDepositRate(CBDFR);
	Commercial_bank::SetCBLoanRate(CBLFR);
	Commercial_bank::SetCDepositRate(0);
	Commercial_bank::SetCLoanRate(0);

	//Here we have to set the relations between banks

	for (int i = 0; i != NBANKS ; i++)
				{
			for(int j=(i+1), n = 0; ( commercial_banks[i]->GetNPartners() < commercial_banks[i]->GetAllowedNPartners() ) & ( n!=NBANKS ) ; j++, n++){
				if(( commercial_banks[j%(NBANKS-1)]->GetNPartners() < commercial_banks[j%(NBANKS-1)]->GetAllowedNPartners() ) & ( commercial_banks[j%(NBANKS-1)] !=commercial_banks[i] ) )  { //Check if banks can be partners
				commercial_banks[i]->AddPartner(commercial_banks[j%(NBANKS-1)]);
				//commercial_banks[j%(NBANKS)]->AddPartner(commercial_banks[i]);
				}

			}}

}

void BNetwork::getComponents(Set<Devs<Loan>*>& c) {
	// get the list of network's banks
	for (int i = 0; i != NBANKS-1 ; i++) c.insert(commercial_banks[i]);
}


void BNetwork::route(const Loan& loan, Devs<Loan>* src, Bag<Event<Loan> >& r) {
	//Here we route the network's input to specific banks

	// If this is a bank output
	if (src != this) {
	//if (find(commercial_banks.begin(), commercial_banks.end(), src)!=commercial_banks.end()){
		if( loan.IsExpired() ) r.insert(Event<Loan>(this,loan)); //If expired, it's a network's output
		else r.insert( Event<Loan>( commercial_banks[ commercial_banks_deref[loan.GetLinkedAgent()] ] ,loan) );
	}//If the output of a banks is not an expired loan object, the this is a request to the interbank market, so that object needs to be sent to a partner

	else{ //If this is an object that arrived at the network from the generator
	//look for the type of input, this could be changed with portvalue objects
	if(loan.getType()==1)
	//Assign a bank to the loan object
	{r.insert(Event<Loan>(getBank( loan.get_IDborrower() ),loan));}
	//Assign a bank that is being asked for a loan
	else{r.insert(Event<Loan>( getBank(loan.get_IDlender()),loan));}
	}


}

bool BNetwork::model_transition() {
	//Here we have conditions over banks that modify the network structure, here is the dynamic of the network
	//Remove banks declared insolvent
	vector<Commercial_bank*>::iterator iter = commercial_banks.begin();
	while (iter != commercial_banks.end()) {
		if ((*iter)->checkInsolvent()) iter = commercial_banks.erase(iter);
		else iter++;
	}

	//Update the list
	for( unsigned int i=0; i!=commercial_banks.size(); i++){ commercial_banks_deref[ commercial_banks[i] ] = i;}

	return false;
}

void BNetwork::add_commercial_bank(double PDef) {

	commercial_banks.push_back(new Commercial_bank(PDef));
	commercial_banks_deref[ commercial_banks.back() ] = commercial_banks.size()-1;

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

int BNetwork::getBank (Agent* a){return (commercial_banks_deref[a]);}


void BNetwork::UpdateAgencyRatings(){

	vector<Commercial_bank*>::iterator iter;
	for (iter = commercial_banks.begin(); iter != commercial_banks.end(); iter++)
	(*iter)->SendRating();
}
