#ifndef _Network_h_
#define _Network_h_
#include "adevs.h"
#include "Loan.h"
#include "Commercial_bank.h"
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
		int getCommercial_bankCount();
		//Get a pointer to specific bank
		static Commercial_bank* getBank (int n);

		static int getBank (Agent* a);

		void UpdateAgencyRatings();

		static void UpdateRates();
	private:
		// Bank set
		static std::vector< Commercial_bank*> commercial_banks;
		//this map is used to get the number of the bank
		static std::map< Agent*, int > commercial_banks_deref;



		// Method for adding a bank
		void add_commercial_bank(double PDef);




};

#endif
