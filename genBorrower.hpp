
#ifndef _genBorrower_hpp_
#define _genBorrower_hpp_
#include "adevs.h"
#include "agent.hpp"
#include <list>

typedef adevs::PortValue<Borrower*> IO_Type;

class Generator: public adevs::Atomic<IO_Type>
{
	public:


		/// Constructor.
		Generator();
		/// Internal transition function.
		void delta_int();
		/// External transition function.
		void delta_ext(double e, const adevs::Bag<IO_Type>& xb);
		/// Confluent transition function.
		void delta_conf(const adevs::Bag<IO_Type>& xb);
		/// Output function.
		void output_func(adevs::Bag<IO_Type>& yb);
		/// Time advance function.
		double ta();
		/// Output value garbage collection.
		void gc_output(adevs::Bag<IO_Type>& g);
		/// Destructor.
		~Generator();
		/// Model output port.
		static const int arrive;

	private:

         std::list<Borrower*> arrivals;
};

#endif
