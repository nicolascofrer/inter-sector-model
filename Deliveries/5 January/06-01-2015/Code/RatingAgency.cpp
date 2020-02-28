#include "RatingAgency.h"
#include "Parameters.h"
#include "BNetwork.h"
#include "adevs.h"
#include "Loan.h"


		RatingAgency::RatingAgency(BNetwork* network): adevs::Atomic<Loan>(){BankingNetwork = network;}


		void RatingAgency::delta_int(){BankingNetwork->UpdateAgencyRatings(); }
		void RatingAgency::output_func(adevs::Bag<Loan>& yb){}
		double RatingAgency::ta(){return delta;}
		void RatingAgency::delta_ext(double,const adevs::Bag<Loan>&){}
		void RatingAgency::delta_conf(const adevs::Bag<Loan>&){}
		void RatingAgency::gc_output(adevs::Bag<Loan>&){}


