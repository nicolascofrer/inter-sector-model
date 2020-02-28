#ifndef __Genr_h_
#define __Genr_h_
#include "adevs.h"
#include "Agent.h"
#include "proba.hpp"
#include "BNetwork.h"
#define Df   0.5	//Deposit arrival frec, per bank
#define Lf   0.00001	//loan arrival frec, per bank


class Generator:
	public adevs::Atomic<Loan>
{
	public:


		Generator(unsigned long seed): adevs::Atomic<Loan>(),n_banks(1),nextd(0){ set_time_next_deposit(); set_time_next_loan();}

		void delta_int() {

		if(deposit==1){set_time_next_deposit(); nextd++;}
		else {set_time_next_loan(); nextl++;}

		}

		void output_func(adevs::Bag<Loan>& yb){

			if(deposit==1){
			Loan* l = new Depositor();
			l->set_borrower(assign_bank (n_banks));
			yb.insert( *l );}

			else{
			Loan* l = new Borrower();
			l->set_lender(assign_bank (n_banks));
			yb.insert( *l );}


		}

		double ta() {
			if(time_next_deposit<time_next_loan) deposit=1;
			else deposit=0;

			return min(time_next_deposit,time_next_loan); }

		void delta_ext(double,const adevs::Bag<Loan>&){}
		void delta_conf(const adevs::Bag<Loan>&){}

		void gc_output(adevs::Bag<Loan>&){}

		void set_n_banks(int n){n_banks=n;} //this function is used to compute the agregate network arrival rate

		int get_number_of_deposits(){return nextd;}

		int get_number_of_loans(){return nextl;}

	private:

		double time_next_deposit;	//deposits arriving to the banking network

		double time_next_loan; //loans arriving to the banking network

		//Random variable, time between deposit arrivals
		adevs::rv d;
		//Random variable, time between loan arrivals
		adevs::rv l;

		void set_time_next_deposit() { time_next_deposit = d.exponential( 1/(Df*n_banks) ); }

		void set_time_next_loan() { time_next_loan = l.exponential( 1/(Lf*n_banks) ); }

		int n_banks;
		//ID
		int nextd;

		int nextl;
		//check if next event is a deposit or a loan, deposit 1, loan 0
		int deposit;
		//choose a random bank
		int assign_bank(int n){
			return rand()%n;  //Only for testing, this should be a truly uniform distributed int, std::uniform_int_distribution maybe?
}
};

#endif
