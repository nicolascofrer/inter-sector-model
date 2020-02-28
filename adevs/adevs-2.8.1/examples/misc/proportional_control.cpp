#include "adevs.h"
#include <iostream>
using namespace std;

typedef adevs::Bag<double> IO_Bag;

/**
 * Linear SISO model of a plant in the form
 *
 * dx/dt = ax + bu
 * y = x
 *
 * with x(0) = 1 and u(0) = 1
 *
 */
class SISOPlantEqns:
	public adevs::ode_system<double>
{
	public:
		/**
		 * Create a plant with the specified inertia coefficient,
		 * control gain, and sampling rate in Hertz.
		 */
		SISOPlantEqns(double a, double b, double sampleFreq):
			adevs::ode_system<double>(2,0),
			a(a),
			b(b),
			u(0),
			sampleInterval(1.0/sampleFreq)
		{
		}
		/**
		 * Create a plant model that is wrapped in a solver.
		 */
		static adevs::Hybrid<double>* makeInstance(double a, double b,
				double sampleFreq)
		{
			SISOPlantEqns* plant = new SISOPlantEqns(a,b,sampleFreq);
			return new adevs::Hybrid<double>(
					plant,
					new adevs::corrected_euler<double>(plant,1E-8,0.001),
					new adevs::linear_event_locator<double>(plant,1E-8));
		}
		void init(double* q)
		{
			q[0] = 0.0; // Sample clock
			q[1] = 1.0; // Initial state
		}
		void der_func(const double* q, double* dq)
		{
			dq[0] = -1.0;
			dq[1] = a*q[0]+b*u;
		}
		void state_event_func(const double*,double*){}
		double time_event_func(const double* q) { return q[0]; }
		void internal_event(double* q, const bool*) { q[0] = sampleInterval; }
		void external_event(double*,double,const IO_Bag& xb) { u = *(xb.begin()); }
		void confluent_event(double* q, const bool* state_event, const IO_Bag& xb)
		{
			internal_event(q,state_event);
			external_event(q,0.0,xb);
		}
		void output_func(const double* q, const bool*, IO_Bag& y)
		{
			y.insert(q[1]);
		}
		void gc_output(IO_Bag&){}
	private:
		const double a, b;
		double u;
		const double sampleInterval;
};

/**
 * Discrete proportional control.
 */
class Control:
	public adevs::Atomic<double>
{
	public:
		/**
		 * Execute control to maintain state at xRef.
		 */
		Control(double xRef):
			adevs::Atomic<double>(),
			xRef(xRef),
			u(0.0),
			active(false)
		{
		}
		void delta_int() { active = !active; }
		void delta_ext(double,const IO_Bag& xb)
		{
			active = true;
			u = xRef-(*(xb.begin()));
		} 
		void delta_conf(const IO_Bag& xb)
		{
			delta_int();
			delta_ext(0.0,xb);
		}
		double ta()
		{
			if (active) return 0.0;
			return adevs_inf<double>();
		}
		void output_func(IO_Bag& yb) { yb.insert(u); }
		void gc_output(IO_Bag&){}
	private:
		const double xRef;
		double u;
		bool active;
};

static const double a = 1.0;
static const double b = 10.0;
static const double freq = 1000.0;
static const double xref = -1.0;

int main()
{
	adevs::Hybrid<double>* plant = SISOPlantEqns::makeInstance(a,b,freq);
	Control* cntrl = new Control(xref);
	adevs::SimpleDigraph<double>* model = new adevs::SimpleDigraph<double>();
	model->add(plant);
	model->add(cntrl);
	model->couple(plant,cntrl);
	model->couple(cntrl,plant);
	adevs::Simulator<double>* sim = new adevs::Simulator<double>(model);
	double t = 0.0;
	while (sim->nextEventTime() < 10.0)
	{
		double x = plant->getState(1);
		cout << t << " " << x << endl;
		t = sim->nextEventTime();
		sim->execNextEvent();
	}
	delete sim;	
	delete model;
}
