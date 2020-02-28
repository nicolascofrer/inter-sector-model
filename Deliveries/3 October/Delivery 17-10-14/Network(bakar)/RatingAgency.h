#ifndef RATINGAGENCY_H_
#define RATINGAGENCY_H_
#include "adevs.h"
#include "Loan.h"
#include "BNetwork.h"

class RatingAgency:
	public adevs::Atomic<Loan>
{
	public:
		RatingAgency(BNetwork* network);


		void delta_int();
		void output_func(adevs::Bag<Loan>& yb);
		double ta();
		void delta_ext(double,const adevs::Bag<Loan>&);
		void delta_conf(const adevs::Bag<Loan>&);
		void gc_output(adevs::Bag<Loan>&);


	private:
		BNetwork* BankingNetwork;




};



#endif
