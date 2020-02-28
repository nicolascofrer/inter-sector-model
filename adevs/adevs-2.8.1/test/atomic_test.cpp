#include <iostream>
#include <cassert>
#include "adevs.h"
using namespace std;
using namespace adevs;

class genr: public Atomic<char>
{
	public:
		genr(double period, int ticks):
		Atomic<char>(),
		period(period),
		ticks(ticks),
		count(0),
		sigma(period)
		{
		}
		double ta()
		{
			return sigma;
		}
		void delta_int()
		{
			count++;
			assert(count <= ticks);
			if (count == ticks) sigma = DBL_MAX;
		}
		void delta_ext(double, const Bag<char>&)
		{
			sigma = DBL_MAX;
		}
		void delta_conf(const Bag<char>&)
		{
			sigma = DBL_MAX;
		}
		void output_func(Bag<char>& y)
		{
			y.insert('a');
		}
		void gc_output(Bag<char>& g)
		{
			assert(g.count('a') > 0);
		}
		~genr()
		{
		}
		int getTickCount() 
		{ 
			return count; 
		}
	private:
		double period;
		int ticks;
		int count;
		double sigma;
};

void test1()
{
	genr* g = new genr(10.0,10);
	Simulator<char> sim(g);
	while (sim.nextEventTime() < DBL_MAX)
	{
		sim.execNextEvent();
	}
	assert(g->getTickCount() == 10);
	delete g;
}

void test2()
{
	genr* g = new genr(10.0,10);
	Simulator<char> sim(g);
	while (sim.nextEventTime() < DBL_MAX)
	{
		sim.computeNextOutput();
		sim.execNextEvent();
	}
	assert(g->getTickCount() == 10);
	delete g;
}

class MyEventListener: public EventListener<char>
{
	public:
		MyEventListener() { count = 0; t_last = 0.0; }
		void outputEvent(Event<char> x, double t)
		{
			count++;
			t_last = t;
		}
		int count;
		double t_last;
};

void test3()
{
	genr* g = new genr(10.0,10);
	Simulator<char> sim(g);
	MyEventListener listener;
	sim.addEventListener(&listener);
	while (sim.nextEventTime() < DBL_MAX)
	{
		sim.computeNextOutput();
		assert(listener.t_last == sim.nextEventTime());
		sim.execNextEvent();
	}
	assert(listener.count == 10);
	assert(g->getTickCount() == 10);
	delete g;
}

void test4()
{
	genr* g = new genr(10.0,10);
	Simulator<char> sim(g);
	Bag<Event<char> > input;
	sim.computeNextState(input,5.0);
	assert(sim.nextEventTime() == 10.0);
	sim.computeNextState(input,6.0);
	assert(sim.nextEventTime() == 10.0);
	sim.computeNextOutput();
	sim.computeNextState(input,sim.nextEventTime());
	assert(sim.nextEventTime() == 20.0);
	assert(g->getTickCount() == 1);
	sim.computeNextOutput();
	assert(sim.nextEventTime() == 20.0);
	sim.computeNextState(input,12.0);
	assert(sim.nextEventTime() == 20.0);
	assert(g->getTickCount() == 1);
	sim.execNextEvent();
	assert(g->getTickCount() == 2);
	assert(sim.nextEventTime() == 30.0);
	delete g;
}

void test5()
{
	genr* g = new genr(10.0,10);
	Simulator<char> sim(g);
	Bag<Event<char> > input;
	Event<char> event(g,'a');
	input.insert(event);
	sim.computeNextState(input,5.0);
	assert(sim.nextEventTime() == DBL_MAX);
	assert(g->getTickCount() == 0);
	delete g;
}

int main()
{
	test1();
	test2();
	test3();
	test4();
	test5();
	return 0;
}
