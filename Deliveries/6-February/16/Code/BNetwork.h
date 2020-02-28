#ifndef _Network_h_
#define _Network_h_
#include "adevs.h"
#include "Loan.h"
#include "Bank.h"
#include <list>

class BNetwork: public adevs::Network<Loan> {
public:
	BNetwork();
	~BNetwork();
	void getComponents(adevs::Set<adevs::Devs<Loan>*>& c);
	void route(const Loan& loan, adevs::Devs<Loan>* src,
			adevs::Bag<adevs::Event<Loan> >& r);
	bool model_transition();
	// Get the number of banks
	static int getBankCount();
	//Get a pointer to specific bank
	static Bank* getBank(int n);

	//static Bank* getBankforGenerator (int n);

	//static int getBank (Agent* a);

	void UpdateAgencyRatings();

	static void UpdateRates();

	static bool IsBankrupt(Bank* bank);

	static Bank* getBankIncludingBankrupt(int n);

	// Check if the IBDeposit received was paid or IB Loan requested after Bankruptcy
	static bool wasIBLiabilityPaid(int ID);

private:
	// Bank set
	static std::vector<Bank*> Banks; //active banks

	//this map is used to get the number of the bank
	//static std::map< Agent*, int > Banks_deref;
	//set of banks that went bankrupt
	//static std::vector< Bank* > Bankrupt;
	//map including bankrupt
	static std::map<int, Bank*> AllBanks;

	// Set with the IDs of IB deposits that will be paid
	static std::set<int> IDsIBliabilityPaid;

	//static std::vector< int > BankruptIDs;

	//	static std::vector< Bank* > ActiveBanksVector;

	// Method for adding a bank
	void add_Bank(double PDef);

	double pDefBankruptBank;

};

#endif
