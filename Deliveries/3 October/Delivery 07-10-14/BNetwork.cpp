#include "BNetwork.h"
#include "Loan.h"
using namespace adevs;
using namespace std;
#include "Parameters.h"
#include "Bank.h"
#include <iostream>
#include <stdexcept>

vector < Bank* > BNetwork::Banks;
//map< Agent*, int > BNetwork::Banks_deref;
//vector < Bank* > BNetwork::Bankrupt;
map< int , Bank* > BNetwork::AllBanks;
//vector< int > BNetwork::BankruptIDs;
//vector< Bank* > BNetwork::ActiveBanksVector;


BNetwork::BNetwork():
	Network<Loan>() // call the parent constructor
{
	// Add initial banks
	for (int i = 0; i != NBANKS-1 ; i++) add_Bank( (double) PDefaul1Loan1);
	add_Bank( (double) PDefaultLoan2);

	Bank::setRateRange(pRatesLevels);
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

			} ////cout << "BANK " <<i <<" has "<< Banks[i]->GetNPartners() << endl;
				}

}

void BNetwork::getComponents(Set<Devs<Loan>*>& c) {
	// get the list of network's banks
	////cout << "Calling getComponents" << endl;
	for (int i = 0; i != NBANKS-1 ; i++) {
		assert(Banks[i] != NULL);
		c.insert(Banks[i]);
	}
	////cout << "End of get components" << endl;
}


void BNetwork::route(const Loan& loan, Devs<Loan>* src, Bag<Event<Loan> >& r) {
	//Here we route the network's input to specific banks

	////cout << "start of route method" <<endl;

//	if ((loan.getType() == 11 || loan.getType() == 21)) {
		//loan.SetLinkedAgent( NULL );
	//	assert( !IsBankrupt( dynamic_cast<Bank*> (loan.GetLinkedAgent()) ));
		//r.insert(Event<Loan>(this,loan));

		//r.insert( Event<Loan>( dynamic_cast<Bank*> (loan.getRequestSenderAgent()) , loan )) ;
	//}
	if	(IsBankrupt( dynamic_cast<Bank*> (loan.get_borrower())) && loan.getType() == 1 ) 	{
		r.insert(Event<Loan>(this,loan));
//		delete &loan;
		assert( !IsBankrupt( dynamic_cast<Bank*> (loan.get_borrower())));
	}
	else if	(IsBankrupt( dynamic_cast<Bank*> (loan.get_lender())) && loan.getType() == 2) {
		r.insert(Event<Loan>(this,loan));
		assert( !IsBankrupt( dynamic_cast<Bank*> (loan.get_lender())));
	}

//	assert( !IsBankrupt( dynamic_cast<Bank*> (loan.GetLinkedAgent()) ));
//	assert( !IsBankrupt( dynamic_cast<Bank*> (loan.get_borrower()) ));
//	assert( !IsBankrupt( dynamic_cast<Bank*> (loan.get_lender()) ));

//	if(find(Bankrupt.begin(),Bankrupt.end(), (dynamic_cast<Bank*> (loan.GetLinkedAgent()))) != Bankrupt.end())
//		throw invalid_argument("Sending to bankrupt bank 1");
//
//	if(find(Bankrupt.begin(),Bankrupt.end(), (dynamic_cast<Bank*> (loan.get_borrower()))) != Bankrupt.end())
//			throw invalid_argument("Borrower is  bankrupt bank 2");
//
//	if(find(Bankrupt.begin(),Bankrupt.end(), (dynamic_cast<Bank*> (loan.get_lender()))) != Bankrupt.end())
//			throw invalid_argument("Lender to bankrupt bank 3");
//
//	if(dynamic_cast<Bank*> (loan.GetLinkedAgent()) == NULL)
//			throw invalid_argument("NULL POINTER 1");
//
//	if(dynamic_cast<Bank*> (loan.get_borrower()) == NULL)
//				throw invalid_argument("NULL POINTER 2");
//
//	if(dynamic_cast<Bank*> (loan.get_lender()) == NULL )
//				throw invalid_argument("NULL POINTER 3");

	// If this is a bank output
	else if (src != this) {

		//assert( !IsBankrupt( dynamic_cast<Bank*> (loan.GetLinkedAgent())) || loan.IsExpired());


	//if (find(Banks.begin(), Banks.end(), src)!=Banks.end()){
		if( loan.IsExpired() ) r.insert(Event<Loan>(this,loan)); //If expired, it's a network's output
		//else r.insert( Event<Loan>( Banks[ Banks_deref[loan.GetLinkedAgent()] ] ,loan) );
		else {

		assert(!IsBankrupt( dynamic_cast<Bank*> (loan.GetLinkedAgent())));
		if( IsBankrupt( dynamic_cast<Bank*> (loan.GetLinkedAgent())) ){cerr << "BANKRUPT" << endl;


		}
		else r.insert( Event<Loan>( dynamic_cast<Bank*> (loan.GetLinkedAgent()) ,loan) );

		////cout << "Route Linked Agent " << (loan.GetLinkedAgent()->getID()) << endl;
		}

	}//If the output of a banks is not an expired loan object, the this is a request to the interbank market, so that object needs to be sent to a partner

	else{

	//If this is an object that arrived at the network from the generator
	//look for the type of input, this could be changed with portvalue objects
	if(loan.getType()==1)
	//Assign a bank to the loan object
	{
		assert( !IsBankrupt( dynamic_cast<Bank*> (loan.get_borrower()) ));
		r.insert(Event<Loan>(dynamic_cast<Bank*> (loan.get_borrower()),loan));

			}
	//Assign a bank that is being asked for a loan
	else{
		assert( !IsBankrupt( dynamic_cast<Bank*> (loan.get_lender()) ));
		r.insert(Event<Loan>( dynamic_cast<Bank*> (loan.get_lender()),loan));

		}
	}

	//cout << "finishing route method" <<endl;
}

bool BNetwork::model_transition() {
	//Here we have conditions over banks that modify the network structure, here is the dynamic of the network
	//Remove banks declared insolvent
	vector<Bank*>::iterator iter = Banks.begin();
	while (iter != Banks.end()) {
		//if ((*iter)->checkInsolvent()) iter = Banks.erase(iter);
		if ((*iter)->checkInsolvent()) {
			//cerr << "Liquidating Bank " << (*iter)->getID() << " Pointer " << *iter << endl;

//			BankruptIDs.push_back((*iter)->getID());
//			(*iter)->setReplacesBank((*iter)->getID());
//			(*iter)->Liquidate();
//			(*iter)->setParent(NULL);
////			delete *iter;
			//*iter = NULL;


			//Bankrupt.push_back(*iter);
		//	//cout << "Replacing bank " << (*iter)->getID() << " "<< *iter << " (Bankrupt) with bank ";
		//	double pdef = (*iter)->GetProbDef();
			//delete *iter;
			//ActiveBanksVector.erase(find(ActiveBanksVector.begin(),ActiveBanksVector.end(),Bankrupt.back()));
			//(*iter) = NULL;
			//iter = Banks.erase( iter )	;
			pDefBankruptBank = (*iter)->GetProbDef();
			AllBanks[(*iter)->getID()] = NULL;
			AllBanks.erase((*iter)->getID());
			iter = Banks.erase( iter )	;

			add_Bank( pDefBankruptBank );

			Bank* aux;
			while( Banks.back()->GetNPartners()<Banks.back()->GetAllowedNPartners() ){ aux = BNetwork::getBank( (int)(NBANKS*(rand()/(RAND_MAX+1.0))) ); if(aux!=Banks.back()){
				assert(!IsBankrupt(aux));
				Banks.back()->AddPartner(aux);
			}
			}


//			////cout << Banks.back()->getID() << " "<< Banks.back() << endl;
//			////cout << "In Bankrupt set was added " << Bankrupt.back()->getID() << " Pointer is " << Bankrupt.back() << endl;
//			cerr << "Adding Partners for the new bank" << endl;
			//Banks.back()->setReplacesBank( Bankrupt.back()->getID());

		//while( (*iter)->GetNPartners()<(*iter)->GetAllowedNPartners() ){ aux = BNetwork::getBank( (int) rand()%NBANKS ); if(*iter!=aux) (*iter)->AddPartner(aux);}
		}
		else iter++;
	}





	iter = Banks.begin();
	//cerr << Banks.size();
	while (iter != Banks.end()) {
		//assert(*iter == NULL);
		assert(!IsBankrupt(*iter));
		assert((*iter)->GetNPartners());
		(*iter)->UpdatePartnerInfo();
	//	//cout << (*iter)->getID() << endl;
	//	//cout << (*iter)->GetNPartners() << endl;
		//cout << "Updating Bank " << (*iter)->getID() << " N Partners " << (*iter)->GetNPartners() << endl;
		iter++;
	}

//cerr << "Number of Banks is " << getBankCount() << endl;


	//Update the list
	//for( unsigned int i=0; i!=Banks.size(); i++){ Banks_deref[ Banks[i] ] = i;}
//cout << "End of Network Transition" << endl;
	return false;
}

void BNetwork::add_Bank(double PDef) {
	//cout << "ADDING BANK " << Bank::getBankCounter() << endl;
	Banks.push_back(new Bank(PDef));
	//Banks[ Banks.size() ] = new Bank(PDef) ;
	//Banks_deref[ Banks.back() ] = Banks.size()-1;
	Banks.back()->setParent(this);
	//Banks[ Banks.size()-1 ]->setParent(this);
	//AllBanks[ Bank::getBankCounter() -1 ] = Banks[ Banks.size()-1 ];
	AllBanks[ Banks.back()->getID() ] = Banks.back();
	//ActiveBanksVector.push_back(Banks[ Banks.size()-1 ]);

}

BNetwork::~BNetwork(){
	std::vector< Bank* >::iterator iter = Banks.begin();
		for (; iter != Banks.end(); iter++)
			delete *iter;



}


int BNetwork::getBankCount() {
	//if(Banks.size() != ActiveBanksVector.size()) cerr << Banks.size() << " " << ActiveBanksVector.size() <<endl;
	return Banks.size();
}


Bank* BNetwork::getBank (int n){return Banks[n];}

//Bank* BNetwork::getBankforGenerator (int n){
//
////	//cout << "size map " << Banks.size() << endl;
////	//cout << "size vector " << ActiveBanksVector.size() << endl;
//	return ActiveBanksVector[n]; };


Bank* BNetwork::getBankIncludingBankrupt (int n){return AllBanks[n];}

//int BNetwork::getBank (Agent* a){return (Banks_deref[a]);}


void BNetwork::UpdateAgencyRatings(){

	vector< Bank*>::iterator iter;
	for (iter = Banks.begin(); iter != Banks.end(); iter++)
	(*iter)->SendRating();
}

//bool BNetwork::IsBankrupt( Bank* bank)//{if(Bankrupt.size()==0) return false; else return  find(Bankrupt.begin(), Bankrupt.end(), bank) != Bankrupt.end() ;}
//{if(Bankrupt.size()==0) return false; else return  bank == NULL ;}

bool BNetwork::IsBankrupt( Bank* bank){ return bank == NULL || find(Banks.begin(), Banks.end(), bank) == Banks.end();}

//bool BNetwork::IsBankrupt( int bank){ if (BankruptIDs.size()==0) return false; else return find(BankruptIDs.begin(), BankruptIDs.end(), bank) != BankruptIDs.end();}
