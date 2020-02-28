/**
 * This model is based on the power system example in Chapter 6
 * of "Building Software for Simulation" by J. Nutaro.
 */
#include "adevs.h"
#include <iostream>
#include <fstream>
#include <list>
#define OMC_ADEVS_IO_TYPE adevs::PortValue<double>
#include "twobus.h"
using namespace std;
using namespace adevs;

/**
 * Adds the ability to make step adjustments of the base
 * load P0 in the two bus model.
 */
class twoBusAdjustableLoad:
	public twobus
{
	public:

		static const int load_event;
		static const int control_event;
		static const int sensor_event;

		twoBusAdjustableLoad():
			twobus(),
			load_adj_fraction(0.0),
			control_adj_fraction(0.0)
		{
		}
		void init(double* q)
		{
			twobus::init(q);
			r0 = get_Load_imp_R();
			n = get_Genr_n(); 
			output_event = false;
		}
		double time_event_func(const double* q)
		{
			if (output_event) return 0.0;
			else return twobus::time_event_func(q);
		}
		void internal_event(double* q, const bool* state_event)
		{
			twobus::internal_event(q,state_event);
			output_event = n != get_Genr_n();
			n = get_Genr_n();
		}
		void confluent_event(double* q, const bool* state_event,
			Bag<OMC_ADEVS_IO_TYPE>& xb)
		{
			internal_event(q,state_event);
			external_event(q,0.0,xb);
		}
		void external_event(double* q, double e,
			const Bag<OMC_ADEVS_IO_TYPE>& xb)
		{
			Bag<OMC_ADEVS_IO_TYPE>::const_iterator iter;
			for (iter = xb.begin(); iter != xb.end(); iter++)
			{
				if ((*iter).port == load_event)
					load_adj_fraction = (*iter).value;
				else
					control_adj_fraction = (*iter).value;
			}
			set_Load_imp_R(
				(1.0+load_adj_fraction+control_adj_fraction)*r0);
			update_vars(q,true); 
		}
		void output_func(const double* q, const bool* state_event,
			Bag<OMC_ADEVS_IO_TYPE>& yb)
		{
			if (output_event)
			{
				yb.insert(OMC_ADEVS_IO_TYPE(sensor_event,
					n*get_Genr_freqInterval()/get_Genr_nomFreq()));
			}
		}

	private:
		double r0; 
		double load_adj_fraction, control_adj_fraction;
		int n;
		bool output_event;
};

const int twoBusAdjustableLoad::load_event = 0;
const int twoBusAdjustableLoad::control_event = 1;
const int twoBusAdjustableLoad::sensor_event = 2;

/**
 * Instigate a step change in load.
 */
class StepLoad:
	public Atomic<OMC_ADEVS_IO_TYPE>
{
	public:

		const static int load_event;

		StepLoad():
			Atomic<OMC_ADEVS_IO_TYPE>(),
			when(1.0),
			stepsize(0.01)
		{
		}
		void delta_int() { when = DBL_MAX; }
		void delta_ext(double,const Bag<OMC_ADEVS_IO_TYPE>&){}
		void delta_conf(const Bag<OMC_ADEVS_IO_TYPE>&){}
		void output_func(Bag<OMC_ADEVS_IO_TYPE>& yb)
		{
			yb.insert(OMC_ADEVS_IO_TYPE(load_event,stepsize));
		}
		double ta() { return when; }
		void gc_output(Bag<OMC_ADEVS_IO_TYPE>&){}
	private:
		double when;
		const double stepsize;
};

const int StepLoad::load_event = 0;

/**
 * Communication channel that carries the sensor
 * signal to the actuator at the load. This model
 * passes on unchanged the port value pairs that
 * it receives.
 */
class Comm:
	public Atomic<OMC_ADEVS_IO_TYPE>
{
	public:
		Comm():
			Atomic<OMC_ADEVS_IO_TYPE>(),
			msg_rate(100.0),
			ttg(DBL_MAX)
		{
		}
		void delta_int()
		{
			q.pop_front();
			if (!q.empty())
				ttg = 1.0/msg_rate;
			else
				ttg = DBL_MAX;
		}
		void delta_ext(double e, const Bag<OMC_ADEVS_IO_TYPE>& xb)
		{
			Bag<OMC_ADEVS_IO_TYPE>::const_iterator iter;
			for (iter = xb.begin(); iter != xb.end(); iter++)
				q.push_back(*iter);
			if (ttg < DBL_MAX) ttg -= e;
			else ttg = 1.0/msg_rate;
		}
		void delta_conf(const Bag<OMC_ADEVS_IO_TYPE>& xb)
		{
			delta_int();
			delta_ext(0.0,xb);
		}
		void output_func(Bag<OMC_ADEVS_IO_TYPE>& yb)
		{
			yb.insert(q.front());
		}
		double ta() { return ttg; }
		void gc_output(Bag<OMC_ADEVS_IO_TYPE>&){}
	private:
		const double msg_rate;
		double ttg;
		list<OMC_ADEVS_IO_TYPE> q;
};

/**
 * Calculate the control signal from a sensor
 * event.
 */
class Control:
	public Atomic<OMC_ADEVS_IO_TYPE>
{
	public:

		const static int control_event;
		const static int sensor_event;

		Control():
			Atomic<OMC_ADEVS_IO_TYPE>(),
			sig(0.0),
			act(false),
			gain(10.0)
		{
		}
		void delta_int() { act = false; }
		void delta_ext(double,const Bag<OMC_ADEVS_IO_TYPE>& xb)
		{
			sig = (*(xb.begin())).value;
			act = true;
		}
		void delta_conf(const Bag<OMC_ADEVS_IO_TYPE>& xb)
		{
			delta_int();
			delta_ext(0.0,xb);
		}
		void output_func(Bag<OMC_ADEVS_IO_TYPE>& yb)
		{
			yb.insert(OMC_ADEVS_IO_TYPE(control_event,-sig*gain));
		}
		double ta()
		{
			if (act) return 0.0;
			else return DBL_MAX;
		}
		void gc_output(Bag<OMC_ADEVS_IO_TYPE>&){}
	private:
		double sig;
		bool act;
		const double gain;
};

const int Control::control_event = 0;
const int Control::sensor_event = 1;

/**
 * Model that connects the above two.
 */
class SmartGrid:
	public Digraph<double>
{
	public:
		SmartGrid():
			Digraph<double>(),
			fout("soln")
		{
			pwr_sys = new twoBusAdjustableLoad();
			Hybrid<OMC_ADEVS_IO_TYPE>* hybrid_model =
				new Hybrid<OMC_ADEVS_IO_TYPE>(
				pwr_sys,
				new rk_45<OMC_ADEVS_IO_TYPE>(pwr_sys,1E-4,0.01),
				new linear_event_locator<OMC_ADEVS_IO_TYPE>(pwr_sys,1E-5));
			StepLoad* step_load = new StepLoad();
			Control* control = new Control();
			Comm* comm = new Comm();
			add(hybrid_model);
			add(step_load);
			add(control);
			add(comm);
			couple(step_load,step_load->load_event,
				hybrid_model,pwr_sys->load_event);
			couple(control,control->control_event,
				hybrid_model,pwr_sys->control_event);
			couple(hybrid_model,pwr_sys->sensor_event,
				comm,pwr_sys->sensor_event);
			couple(comm,pwr_sys->sensor_event,
				control,control->sensor_event);
		}
		void print_vars(double t);
		~SmartGrid() { fout.close(); }
	private:
		twoBusAdjustableLoad* pwr_sys;
		ofstream fout;
};

void SmartGrid::print_vars(double t)
{
	if (t == 0)
	{
		fout << "# t,w,Pm,Pe,Ef,theta,,T1.va,T1.vb,V,V0" << endl;
	}
	fout << t << " " 
		<< pwr_sys->get_Genr_w()*pwr_sys->get_Genr_nomFreq() << " "
		<< pwr_sys->get_Genr_n()*pwr_sys->get_Genr_freqInterval() << " "
		<< pwr_sys->get_Genr_Pm() << " "
		<< pwr_sys->get_Genr_Pe() << " "
		<< pwr_sys->get_Genr_Ef() << " "
		<< pwr_sys->get_Genr_theta() << " "
		<< pwr_sys->get_L12_T1_va() << " "
		<< pwr_sys->get_L12_T1_vb() << " "
		<< pwr_sys->get_Genr_V() << " "
		<< pwr_sys->get_Genr_V0() << " "
		<< endl;
}

int main()
{
	SmartGrid* model = new SmartGrid();
	// Create the simulator
	Simulator<OMC_ADEVS_IO_TYPE>* sim =
		new Simulator<OMC_ADEVS_IO_TYPE>(model);
	model->print_vars(0);
	while (sim->nextEventTime() <= 20.0)
	{
		double t = sim->nextEventTime();
		sim->execNextEvent();
		model->print_vars(t);
	}
	delete sim;
	delete model;
	return 0;
}
