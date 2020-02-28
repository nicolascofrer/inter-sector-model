#ifndef __Genr_h_
#define __Genr_h_
#include "adevs.h"
#include "Loan.h"
#include "BNetwork.h"
#include <algorithm>


class Generator:
	public adevs::Atomic<Loan>
{
	public:
		Generator(unsigned long seed);
		~Generator();
		void delta_int();
		void output_func(adevs::Bag<Loan>& yb);
		double ta();
		void delta_ext(double,const adevs::Bag<Loan>&);
		void delta_conf(const adevs::Bag<Loan>&);
		void gc_output(adevs::Bag<Loan>&);

		void set_n_banks(int n);

		unsigned int get_number_of_deposits();

		unsigned int get_number_of_loans();

	private:

		double time_next_deposit;//deposits arriving to the banking network

		double time_next_loan;//loans arriving to the banking network

		//Random variable, time between arrivals
		adevs::rv r;

		adevs::rv differentArrivalRates;

		int n_banks;

		void set_time_next_deposit();

		void set_time_next_loan();
		//{ time_next_loan = l.exponential( 1/(Lf*n_banks) ); }

		//ID
		int nextd;

		int nextl;
		//check if next event is a deposit or a loan, deposit 1, loan 0
		int deposit;
		//choose a random bank
		Bank* AssignBank(int n);

		std::vector<Loan*> l;
};

#endif
