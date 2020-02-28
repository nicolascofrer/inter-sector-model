#include "adevs.h"
#include <iostream>
using namespace std;
using namespace adevs;

/**
 * This is the circuit example from "Building Software for Simulation". It
 * prints the trajectory of the model to stdout; if this is captured in a file,
 * then the gnuplot command file 'plot' can be used to plot the voltage and
 * diode and switch states.
 */
class Circuit: public ode_system<bool>
{
	public:
		Circuit():
			ode_system<bool>(1,1), // one state variable and event function
			s(1),d(0), // diode and switch states
			vs(1.0),C(1.0),Rs(1.0),Rl(1.0), // device parameters
			vop(0.5),vcl(0.25){} 
		void init(double* q) { q[0] = 0.0; } // vc = 0
		void der_func(const double* q, double *dq)
		{
			// ODE form of the differential equations
			if (!s && !d) dq[0] = 0.0; 
			else if (!s && d) dq[0] = -q[0]/(C*Rl);
			else if (s && !d) dq[0] = (vs-q[0])/(C*Rs);
			else dq[0] = ((vs-q[0])/Rs-q[0]/Rl)/C;
		}
		void state_event_func(const double *q, double* z) 
		{
			// This model uses the implicit form of the diode event
			if (d==0) z[0] = q[0]-vop; 
			else z[0] = q[0]-vcl;
		}
		// As written here, this model does not have any time events
		double time_event_func(const double* q) { return DBL_MAX; }
		void internal_event(double* q, const bool* events)
		{
			assert(events[0]); // only one event type; make sure it fired
			d = !d;
		}
		void external_event(double*,double,const Bag<bool>& xb)
		{
			s = *(xb.begin());
		}
		void confluent_event(double* q,const bool* events,
				const Bag<bool>& xb)
		{
			internal_event(q,events);
			external_event(q,0.0,xb);
		}
		void output_func(const double* q, const bool* events,
				Bag<bool>& yb)
		{
			assert(events[0]);
			yb.insert(!d);
		}
		void gc_output(Bag<bool>&){}
		bool getDiode() const { return d; }
		bool getSwitch() const { return s; }
	private:
		bool s, d;
		const double vs, C, Rs, Rl, vop, vcl;
};

class StateListener: public EventListener<bool>
{
	public:
		StateListener(Hybrid<bool>* c1, Circuit* c2):c1(c1),c2(c2){}
		void stateChange(Atomic<bool>*,double t)
		{
			cout << t << " " << c1->getState(0) << " " << 
				c2->getSwitch() << " " << c2->getDiode() << endl;
		}
		void outputEvent(Event<bool>,double){}
	private:
		Hybrid<bool>* c1; Circuit* c2;
};

int main()
{
	// Create the model
	Circuit* circuit = new Circuit();
	Hybrid<bool>* hybrid_model = new Hybrid<bool>(
			circuit,new corrected_euler<bool>(circuit,1E-5,0.01),
			new linear_event_locator<bool>(circuit,1E-5));
	// Create the simulator
	Simulator<bool>* sim = new Simulator<bool>(hybrid_model);
	sim->addEventListener(new StateListener(hybrid_model,circuit));
	// Simulate until the switch opens
	while (sim->nextEventTime() <= 1.0) sim->execNextEvent();
	// Open the switch
	Bag<Event<bool> > xb; xb.insert(Event<bool>(hybrid_model,0));
	sim->computeNextState(xb,1.0);
	// Simulate for another second
	while (sim->nextEventTime() <= 4.0) sim->execNextEvent();
	delete sim; delete hybrid_model;
	return 0;
}
