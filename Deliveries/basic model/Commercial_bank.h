#ifndef _Commercial_bank_h_
#define _Commercial_bank_h_
#include "adevs.h"
#include "Agent.h"
#include <cassert>
#include <deque>

class Commercial_bank: public adevs::Atomic<Loan>, public Agent
{
	public:
		Commercial_bank():adevs::Atomic<Loan>(),tleft(DBL_MAX),equity(10){}

		void delta_int()
		{	qd.pop_front(); // Remove the expired deposit

		tleft = DBL_MAX;
		if (qd.empty()) tleft = DBL_MAX; // No deposit left

		else{
		std::deque<Loan>::iterator iter = qd.begin();
		for (; iter != qd.end(); iter++)
		{ if( (*iter).get_t_to_maturity() <= tleft )
			{tleft = (*iter).get_t_to_maturity(); //find the next deposit to expire and push on front
			qd.push_front(*iter);
			qd.erase(iter);}

			}
		}
		}


		void delta_ext(double e, const adevs::Bag<Loan>& xb)
		{
			// Update the remaining time to maturity
			if (!qd.empty()) tleft -= e;
			// Put new deposits into the queue
			adevs::Bag<Loan>::const_iterator iter = xb.begin();
			for (; iter != xb.end(); iter++) 
			{
				// If queue is empty
				if (qd.empty()) tleft = (*iter).get_t_to_maturity();
				// Put the deposit into the back of the queue
				qd.push_back(*iter);
			}

			
		}
		void delta_conf(const adevs::Bag<Loan>& xb)
		{
			delta_int();
			delta_ext(0.0,xb);
		}
		void output_func(adevs::Bag<Loan>& yb)
		{
			// remove deposit withdrawn
			yb.insert(qd.front());
		}
		double ta()
		{
			return tleft;
		}

		bool model_transition()
		{
		//transition activated if insolvent
			return (checkInsolvent() );
		}
		// Get the number of deposits in the queue
		unsigned int getDepositQueueSize() { return qd.size(); }

		void gc_output(adevs::Bag<Loan>&){}

		double value(){return equity;}

		void reduce_equity(double shock){ equity-=shock;}

		bool checkInsolvent() {
		if ( equity <0  ) return true ;
		else return false ;
		}


	private:
		//time to maturity
		adevs::rv time_to_maturity;
		// Queue for deposits
		std::deque<Loan> qd;

		// Queue for loans
		std::deque<Loan> ql;
		// Time remaining on the deposit
		double tleft; 

		double equity;

};

#endif
