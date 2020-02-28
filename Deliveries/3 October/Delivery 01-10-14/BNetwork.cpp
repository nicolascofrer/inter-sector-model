#include "BNetwork.h"
#include "Loan.h"
using namespace adevs;
using namespace std;
#include "Parameters.h"
#include "Bank.h"
#include <iostream>

vector< Bank*> BNetwork::Banks;
map< Agent*, int > BNetwork::Banks_deref;
vector < Bank* > BNetwork::Bankrupt;
map< int , Bank* > BNetwork::AllBanks;




BNetwork::BNetwork():
	Network<Loan>() // call the parent constructor
{
	// Add initial banks
	for (int i = 0; i != NBANKS-1 ; i++) add_Bank(PDefaul1Loan1);
	add_Bank(PDefaultLoan2);

	Bank::SetDepositRateRange(customerDepositInterestRate,customerDepositInterestRate+0.1);
	Bank::SetLoanRateRange(customerLoanInterestRate,customerLoanInterestRate+0.1);
	Bank::SetCBDepositRate(CBDFR);
	Bank::SetCBLoanRate(CBLFR);
	Bank::SetCDepositRate(0);
	Bank::SetCLoanRate(0);

	//Here we have to set the relations between banks

	for (int i = 0; i != NBANKS ; i++)
				{
			for(int j=(i+1), n = 0; ( Banks[i]->GetNPartners() < Banks[i]->GetAllowedNPartners() ) & ( n!=NBANKS ) ; j++, n++){
			//for(int j=i+1; ( Banks[i]->GetNPartners() < Banks[i]->GetAllowedNPartners() ) ; j++){

				//if(( Banks[j%(NBANKS-1)]->GetNPartners() < Banks[j%(NBANKS-1)]->GetAllowedNPartners() ) & ( Banks[j%(NBANKS-1)] !=Banks[i] ) )  { //Check if banks can be partners
				if(  Banks[j%(NBANKS)] != Banks[i]  )  { //Check if banks can be partners

					Banks[i]->AddPartner(Banks[j%(NBANKS)]);
				//Banks[j%(NBANKS)]->AddPartner(Banks[i]);
				}

			} cout << "BANK " <<i <<" has "<< Banks[i]->GetNPartners() << endl;
				}

}

void BNetwork::getComponents(Set<Devs<Loan>*>& c) {
	// get the list of network's banks
	for (int i = 0; i != NBANKS-1 ; i++) c.insert(Banks[i]);
}


void BNetwork::route(const Loan& loan, Devs<Loan>* src, Bag<Event<Loan> >& r) {
	//Here we route the network's input to specific banks

	// If this is a bank output
	if (src != this) {
	//if (find(Banks.begin(), Banks.end(), src)!=Banks.end()){
		if( loan.IsExpired() ) r.insert(Event<Loan>(this,loan)); //If expired, it's a network's output
		//else r.insert( Event<Loan>( Banks[ Banks_deref[loan.GetLinkedAgent()] ] ,loan) );
		else r.insert( Event<Loan>( dynamic_cast<Bank*> (loan.GetLinkedAgent()) ,loan) );
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
	vector<Bank*>::iterator iter = Banks.begin();
	while (iter != Banks.end()) {
		//if ((*iter)->checkInsolvent()) iter = Banks.erase(iter);
		if ((*iter)->checkInsolvent()) {
			cout << "Liquidating Bank " << (*iter)->getID() << " Pointer " << *iter << endl;
			(*iter)->Liquidate();
			Bankrupt.push_back(*iter);
			cout << "Replacing bank " << (*iter)->getID() << " "<< *iter << " (Bankrupt) with bank ";
			double pdef = (*iter)->GetProbDef();
			//delete *iter;
			//(*iter) = NULL;
			iter = Banks.erase(iter);
			add_Bank( pdef );
			cout << Banks.back()->getID() << " "<< Banks.back() << endl;
			cout << "In Bankrupt set was added " << Bankrupt.back()->getID() << " Pointer is " << Bankrupt.back() << endl;
			cout << "Adding Partners for the new bank" << endl;

		Bank* aux;
			while( Banks.back()->GetNPartners()<3 ){ aux = BNetwork::getBank( (int) rand()%NBANKS ); if(Banks.back()!=aux) Banks.back()->AddPartner(aux);}
		}
		else iter++;
	}

	iter = Banks.begin();
	while (iter != Banks.end()) {
		(*iter)->UpdatePartnerInfo(); iter++;
	}




	//Update the list
	//for( unsigned int i=0; i!=Banks.size(); i++){ Banks_deref[ Banks[i] ] = i;}

	return false;
}

void BNetwork::add_Bank(double PDef) {

	Banks.push_back(new Bank(PDef));
	//Banks_deref[ Banks.back() ] = Banks.size()-1;
	Banks.back()->setParent(this);
	AllBanks[ Bank::getBankCounter() -1 ] = Banks.back();

}

BNetwork::~BNetwork(){
	std::vector<Bank*>::iterator iter = Banks.begin();
		for (; iter != Banks.end(); iter++)
			delete *iter;

	iter = Bankrupt.begin();
		for (; iter != Bankrupt.end(); iter++)
			delete *iter;
}


int BNetwork::getBankCount() {
	return Banks.size();
}


Bank* BNetwork::getBank (int n){return Banks[n];}

Bank* BNetwork::getBankIncludingBankrupt (int n){return AllBanks[n];}

//int BNetwork::getBank (Agent* a){return (Banks_deref[a]);}


void BNetwork::UpdateAgencyRatings(){

	vector<Bank*>::iterator iter;
	for (iter = Banks.begin(); iter != Banks.end(); iter++)
	(*iter)->SendRating();
}

bool BNetwork::IsBankrupt( Bank* bank){return find(Bankrupt.begin(), Bankrupt.end(), bank) != Bankrupt.end();}

