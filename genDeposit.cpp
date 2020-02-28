

#include "genDeposit.hpp"

using namespace std;
using namespace adevs;

// Assign a locally unique number to the arrival port
const int Generator::arrive = 0;


Atomic<IO_Type>()
{

	double next_arrival_time = 0.0;
	double last_arrival_time = 0.0;
	while (true)
	{
		Depositor * depo = new Depositor;
		next_arrival_time = next_arrival_time + exponent( Depositor->entryFrequency);

         depo-> time_issued = next_arrival_time

		arrivals.push_back(depo);
		last_arrival_time = next_arrival_time;
	}
}

double Generator::ta()
{
	// If there are not more customers, next event time is infinity
	if (arrivals.empty()) return DBL_MAX;
	// Otherwise, wait until the next arrival
	return arrivals.front()->time_issued;
}

void Generator::delta_int()
{
	// Remove the first customer.  Because it was used as the
	// output object, it will be deleted during the gc_output()
	// method call at the end of the simulation cycle.
	arrivals.pop_front();
}

void Generator::delta_ext(double e, const Bag<IO_Type>& xb)
{
	/// The generator is input free, and so it ignores external events.
}

void Generator::delta_conf(const Bag<IO_Type>& xb)
{
	/// The generator is input free, and so it ignores input.
	delta_int();
}

void Generator::outpu#ifndef _generator_h_
#define _generator_h_t_func(Bag<IO_Type>& yb)
{
	// First customer in the list is produced as output
	IO_Type output(arrive,arrivals.front());
	yb.insert(output);
}

void Generator::gc_output(Bag<IO_Type>& g)
{
	// Delete the customer that was produced as output
	Bag<IO_Type>::iterator i;
	for (i = g.begin(); i != g.end(); i++)
	{
		delete (*i).value;
	}
}

Generator::~Generator()
{
	/// Delete anything remaining in the arrival list
	list<Depositor*>::iterator i;
	for (i = arrivals.begin(); i != arrivals.end(); i++)
	{
		delete *i;
	}
}
