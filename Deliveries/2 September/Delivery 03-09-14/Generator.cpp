#include "adevs.h"
#include "Loan.h"
//#include "proba.hpp"
#include "BNetwork.h"
#include <algorithm>
#include "Generator.h"
#include "Parameters.h"


		Generator::Generator(unsigned long seed): adevs::Atomic<Loan>(){n_banks=1,
			set_time_next_deposit(); set_time_next_loan(); nextl=0; nextd=0;}

		void Generator::delta_int() {
		if(time_next_deposit<=time_next_loan){time_next_loan-=time_next_deposit; set_time_next_deposit(); nextd++;}
		else {time_next_deposit-=time_next_loan; set_time_next_loan(); nextl++;}
		}

		void Generator::output_func(adevs::Bag<Loan>& yb){

			if(time_next_deposit<=time_next_loan){
			Loan* l = new Depositor();
			l->set_borrower(AssignBank (n_banks));
			l->SetLinkedAgent(0);
			yb.insert( *l );

			}

			else{
			Loan* l = new Borrower();
			l->set_lender(AssignBank (n_banks));
			l->SetLinkedAgent(0);
			yb.insert( *l );
			}


		}

		double Generator::ta() {return min(time_next_deposit,time_next_loan);}

		void Generator::delta_ext(double,const adevs::Bag<Loan>&){}
		void Generator::delta_conf(const adevs::Bag<Loan>&){}
		void Generator::gc_output(adevs::Bag<Loan>&){}

		void Generator::set_n_banks(int n){n_banks=n;} //this function is used to compute the agregate network arrival rate

		unsigned int Generator::get_number_of_deposits(){return nextd;}

		unsigned int Generator::get_number_of_loans(){return nextl;}

		void Generator::set_time_next_deposit() { time_next_deposit = r.exponential( 1/(Df*n_banks) ); }

		void Generator::set_time_next_loan() //{time_next_loan = DBL_MAX;}
		{ time_next_loan = r.exponential( 1/(Lf*n_banks) ); }

		unsigned int Generator::AssignBank(int n){
			return rand()%n;  //Only for testing, this should be a truly uniform distributed int, std::uniform_int_distribution maybe?
}

