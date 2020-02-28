
#ifndef _genBorrower_hpp_
#define _genBorrower_hpp_
#include "adevs.h"
#include "agent.h"
#include <list>



class GenBorrower: public adevs::Atomic<IO_Type>
{
	public:


		/// Constructor.
		GenBorrower();
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
		~GenBorrower();
		/// Model output port.
		static const int customer_loan_arrive_port;

	private:

         std::list<Loan*> arrivals;
};

#endif

