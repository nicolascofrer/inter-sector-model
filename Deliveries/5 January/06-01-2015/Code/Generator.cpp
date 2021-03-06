#include "adevs.h"
#include "Loan.h"
//#include "proba.hpp"
#include "BNetwork.h"
#include <algorithm>
#include "Generator.h"
#include "Parameters.h"


		Generator::Generator(unsigned long seed): adevs::Atomic<Loan>(){//n_banks=1,
			set_time_next_deposit(); set_time_next_loan(); nextl=0; nextd=0;}

		Generator::~Generator(){
			std::vector<Loan*>::iterator iter = l.begin();
			for (; iter != l.end(); iter++)
			delete *iter;
		}

		void Generator::delta_int() {
		if(time_next_deposit<=time_next_loan){time_next_loan-=time_next_deposit; set_time_next_deposit(); nextd++;}
		else {time_next_deposit-=time_next_loan; set_time_next_loan(); nextl++;}
		}

		void Generator::output_func(adevs::Bag<Loan>& yb){

			if(time_next_deposit<=time_next_loan){
			l.push_back(new Depositor());
			l.back()->set_borrower( AssignBank (BNetwork::getBankCount()));
			//l->SetLinkedAgent(0);
			//l.back()->setID(nextd);
			yb.insert( *l.back() );
			}

			else{
			l.push_back(new Borrower());
			l.back()->set_lender(  AssignBank (BNetwork::getBankCount()));
			//l->SetLinkedAgent(0);
			//l.back()->setID(nextl);
			yb.insert( *l.back() );
			}
		}

		double Generator::ta() {return min(time_next_deposit,time_next_loan);}

		void Generator::delta_ext(double,const adevs::Bag<Loan>&){}
		void Generator::delta_conf(const adevs::Bag<Loan>&){}
		void Generator::gc_output(adevs::Bag<Loan>&){}

		//void Generator::set_n_banks(int n){n_banks=n;} //this function is used to compute the agregate network arrival rate

		unsigned int Generator::get_number_of_deposits(){return nextd;}

		unsigned int Generator::get_number_of_loans(){return nextl;}

		void Generator::set_time_next_deposit() { time_next_deposit = r.exponential( 1/(Df*BNetwork::getBankCount()) ); }

		void Generator::set_time_next_loan() //{time_next_loan = DBL_MAX;}
		{ time_next_loan = r.exponential( 1/(Lf*BNetwork::getBankCount()) ); }

		Bank* Generator::AssignBank(int n){

			return BNetwork::getBank( (int)(n*(rand()/(RAND_MAX+1.0))) );  //Only for testing, this should be a truly uniform distributed int, std::uniform_int_distribution maybe?


}

