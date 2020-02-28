#ifndef _Network_h_
#define _Network_h_
#include "adevs.h"
#include "Loan.h"
#include "Bank.h"
#include <list>

class BNetwork: public adevs::Network<Loan> {
	public:
		BNetwork();
		void getComponents(adevs::Set<adevs::Devs<Loan>*>& c);
		void route(const Loan& loan, adevs::Devs<Loan>* src,
				adevs::Bag<adevs::Event<Loan> >& r);
		bool model_transition();
		~BNetwork();
		// Get the number of banks
		static int getBankCount();
		//Get a pointer to specific bank
		static Bank* getBank (int n);

		//static int getBank (Agent* a);

		void UpdateAgencyRatings();

		static void UpdateRates();

		static int getBankCounter();

		static bool IsBankrupt( Bank* bank);

		static Bank* getBankIncludingBankrupt(int n);

	private:
		// Bank set
		static std::vector< Bank* > Banks; //active banks
		//this map is used to get the number of the bank
		static std::map< Agent*, int > Banks_deref;
		//set of banks that went bankrupt
		static std::vector< Bank* > Bankrupt;
		//map including bankrupt
		static std::map< int , Bank* > AllBanks;

		// Method for adding a bank
		void add_Bank(double PDef);




};

#endif
