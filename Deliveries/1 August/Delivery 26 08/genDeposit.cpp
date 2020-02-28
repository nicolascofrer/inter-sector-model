#include "genDeposit.hpp"

using namespace std;
using namespace adevs;


const int GenDeposit::customer_deposit_arrive_port = 0;


Atomic<IO_Type>()
{

	double next_arrival_time = 0.0;
	double last_arrival_time = 0.0;
	while (true)
	{
		Depositor * depositor = new Depositor;
		Loan * laon = new Loan ;
		next_arrival_time = next_arrival_time + exponent( depositor->entryFrequency);

        loan-> lender= depositor ;
        loan-> time_issued = next_arrival_time;
        loan-> interest_rate=
        loan->

		arrivals.push_back(depo);
		last_arrival_time = next_arrival_time;
	}
}

double Generator::ta()
{

	if (arrivals.empty()) return DBL_MAX;

	return arrivals.front()->time_issued;
}

void Generator::delta_int()
{

	arrivals.pop_front();
}

void Generator::delta_ext(Time e, const Bag<IO_Type>& xb)
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

	IO_Type output(customer_deposit_arrive_port,arrivals.front());
	yb.insert(output);
}

void Generator::gc_output(Bag<IO_Type>& g)
{

	Bag<IO_Type>::iterator i;
	for (i = g.begin(); i != g.end(); i++)
	{
		delete (*i).value;
	}
}

Generator::~Generator()
{

	list<Loan*>::iterator i;
	for (i = arrivals.begin(); i != arrivals.end(); i++)
	{
		delete *i;
	}
}
