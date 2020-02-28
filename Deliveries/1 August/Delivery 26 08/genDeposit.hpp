
#ifndef _genDeposit_hpp_
#define _genDeposit_hpp_
#include "adevs.h"
#include "agent.h"
#include <list>
#include "adevs.h"





class GenDeposit: public adevs::Atomic<IO_Type>
{
	public:
		/// Constructor.
		GenDeposit();
		/// Internal transition function.
		void delta_int();
		/// External transition function.
		void delta_ext(double  e, const adevs::Bag<IO_Type>& xb);
		/// Confluent transition function.
		void delta_conf(const adevs::Bag<IO_Type>& xb);
		/// Output function.
		void output_func(adevs::Bag<IO_Type>& yb);
		/// Time advance function.
		double  ta();
		/// Output value garbage collection.
		void gc_output(adevs::Bag<IO_Type>& g);
		/// Destructor.
		~GenDeposit();
		/// Model output port.

        static const int customer_deposit_arrive_port ;


	private:

         std::list<Loan*> arrivals;
};

#endif
