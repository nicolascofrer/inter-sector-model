#ifndef _electrical_model_eqns_h_
#define _electrical_model_eqns_h_
#include <map>
#include "ElectricalData.h"
#include "adevs.h"
#include "adevs_rk45_improved.h"
#include "events.h"

/**
 * This class contains the electrical and mechanical dynamics for the
 * simulated power system.
 */
class ElectricalModelEqns: 
	public adevs::ode_system<adevs::PortValue<BasicEvent*> >
{
	public:
		/**
		 * Input to the LoadAdj port must be a LoadAdjustEvent
		 */
		static const int LoadAdj;
		/**
		 * Input to the SetLoad port must be a LoadEvent
		*/
		static const int SetLoad;
		/**
		 * This input port is used to set a sampling condition 
		 * for a generator state variable. The input should be
		 * a GenrSampleEvent that describes the sampling condition
		 * for the generator. Only one condition can be pending at
		 * any time. Old conditions will always be displaced by
		 * new conditions. Any condition that fires is immediately
		 * removed.
		 */
		static const int SetGenrSample;
		/**
		 * This input port is used to set generation control parameters. It accepts
		 * SetPointEvent objects.
		 */
		static const int Control;
		/**
		 * This input port is used to trip a generator offline. It accepts a
		 * GenrFailEvent object. The BusID should be the bus that the generator
		 * is attached to.
		 */
		static const int GenrTrip;
		/**
		 * These output ports produces generator state variable samples in the form of 
		 * GenrSampleEvent objects. There is one output port for each generator and the 
		 * port number is equal to the generator bus number.
		 */
		int* GenrSampleOutput;
		/**
		 * Create a model from the specified initial data. If the no_events flag
		 * is true, then discrete events will be disabled.
		 */
		ElectricalModelEqns(ElectricalData* data, bool no_events = false);
		/// Destructor
		~ElectricalModelEqns();
		/// Adevs methods below
		void init(double* q);
		void der_func(const double *q, double* dq);
		void state_event_func(const double* q, double *z);
		double time_event_func(const double* q);
		void internal_event(double* q, const bool* state_event);
		void external_event(double* q, double e,
				const adevs::Bag<adevs::PortValue<BasicEvent*> >& xb);
		void confluent_event(double* q, const bool* state_event,
			const adevs::Bag<adevs::PortValue<BasicEvent*> >& xb);
		void output_func(const double* q, const bool* state_event,
			adevs::Bag<adevs::PortValue<BasicEvent*> >& yb);
		void gc_output(adevs::Bag<adevs::PortValue<BasicEvent*> >& gb);
		/**
		 * Update current and voltage calculates to reflect the state q.
		 * Call this method before asking for the voltage or current. This
		 * calculates new values for the generator exicators and the voltages
		 * and currents at the generator and load terminals.
		 */
		void updateVoltageAndInjCurrent(const double* q);
		/// Get the complex voltage at the specified bus
		Complex getVoltage(unsigned bus);
		/// Get the complex current injected into the specified bus
		Complex getInjCurrent(unsigned bus);
		/// Get the frequency in radians at the specified generator.
		double getGenrFreq(unsigned genr_number, const double *q);
		/// Get the mechanical power at the specified generator.
		double getMechPower(unsigned genr_number, const double *q);
		/// Is the generator offline?
		bool genrOffLine(unsigned genr_number);
		/// Get the electrical data associated with this model
		ElectricalData* getElectricalData() { return data; }
		/// Store the bus frequencies at state q in the array freq
		void getBusFreqs(const double* q, double* freq);
	private:
		// Electrical data for the model
		ElectricalData* data;
		// Diagonal matrix of node impedences
		Complex* load_Yt;
		Complex* genr_Yt;
		// Node voltages and currents
		Complex *voltage, *current;
		// Injected currents
		Complex* load_inj_current;
		Complex* genr_inj_current;
		// Excitation voltage at the generators
		Complex *Ef;
		// Breaker status at the generator
		bool* breaker_closed;
		// Excitation state at the generator
		typedef enum { SAT, NOT_SAT, FALLING } ExcitationState;
		ExcitationState* excite_status; 
		// Power output set point
		double* PS;
		// Sampling condition map
		std::map<unsigned int,GenrSampleEvent> sample_conds;
		// Generator index lookup table
		std::map<unsigned,unsigned> genr_bus_lookup; 
		// Look for state events?
		bool no_events;
		// Load adjustment made because of a discrete input
		Complex load_adj;
		// Network admittance matrix
		AdmittanceNetwork Y;
		// Compute voltage from current and Y matrix
		void solve_for_voltage();
};

#endif
