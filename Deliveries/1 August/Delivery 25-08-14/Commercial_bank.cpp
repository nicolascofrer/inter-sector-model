#include "Commercial_bank.h"
#include "adevs.h"
#include "Agent.h"
#include <deque>
#include <algorithm>	// Needed for sort() method
#include <iostream>
#include <map>
#include <string>
#include <iostream>
#include <fstream>

using namespace std;

		map<vector<int> , map<vector<int>,int > > Commercial_bank::h;
		map<vector<int>,int > Commercial_bank::h_data;

		map<vector<int>, map<double,vector<int> > > Commercial_bank::ValueFunction;
		map< double,vector<int> > Commercial_bank::ValueFunction_data;

		map<vector<int>, map< vector<int>, double > > Commercial_bank::ValueFunction2;
		map< vector<int>, double > Commercial_bank::ValueFunction2_data;

		map<vector<int>, map< vector<int>, double > > Commercial_bank::InitVal;
		map< vector<int>, double > Commercial_bank::InitVal_data;



		Commercial_bank::Commercial_bank(double ProbabilityDefault):adevs::Atomic<Loan>(),tleft(DBL_MAX)
		{
			//SetEquityRange(0,50);
			equity=100;
			defaults=0;
			Pdefault = ProbabilityDefault;
			profits=0;
			SetDepositRateRange(0,0.1);
			SetLoanRateRange(0,0.1);
			DepositRate=0;
			LoanRate=5;

			accept.push_back(1);
			reject.push_back(1);
			accept.push_back(1);
			reject.push_back(0);

			a.push_back(accept);
			a.push_back(reject);

		};

		void Commercial_bank::delta_int()


		{


			//form state vector for the first time
			if(LastState.empty()){
				LastState.push_back(qd.size());
				LastState.push_back(ql.size());
				LastState.push_back(equity);

			}

			//Update the last state vector
			LastState[0] = qd.size();
			LastState[1] = ql.size();
			LastState[2] = equity;
			//Count the number of times that the last state has been visited
			if(h.find(LastState) == h.end() && h[LastState].find(LastAction) == h[LastState].end())
			{h[LastState][LastAction] = 0;}
			else h[LastState][LastAction]+=1;




			//In case of deposit
			if((qt.front()).getID()==1) {reduce_equity( (qt.front()).GetQuantity()); //what is the amount lent?
			UpdateW(LastState, LastAction, - (qt.front()).GetQuantity()); //Update given the realized state is a deposit expiration
			}

			//In case of loan
			//If there is default, reduce equity. If all is ok, get the profits from a successful loan
			if((qt.front()).getID()==2){ if( WillItDefault() ) {reduce_equity( (qt.front()).GetQuantity()); //what is the amount lent?
																UpdateW(LastState, LastAction, - (qt.front()).GetQuantity());
			}//Update given realized state is a loan default

										else {IncreaseEquity( (qt.front()).GetQuantity() );
										UpdateW(LastState, LastAction, (qt.front()).GetQuantity() );//Update given realized state is a succesfull loan
										}
										}


			qt.pop_front(); // Remove the expired deposit

			if (qt.empty()) tleft = DBL_MAX; // No event left

			else{//look for next deposit to expire
				sort(qt.begin(),qt.end(),NextExpiring);
				tleft = qt.begin()->get_t_to_maturity()-tleft;
			} //- tleft because the borrower or depositor was already for tleft time at the bank







		}


		void Commercial_bank::delta_ext(double e, const adevs::Bag<Loan>& xb)
		{

			//Initialize the last state vector if needed
			if(LastState.empty()){
							LastState.push_back(qd.size());
							LastState.push_back(ql.size());
							LastState.push_back(equity);
						}

			//Update the last state vector
			LastState[0] = qd.size();
			LastState[1] = ql.size();
			LastState[2] = equity;

			//Count the number of times that the last state has been visited
			if(h.find(LastState) == h.end() && h[LastState].find(LastAction) == h[LastState].end())
			{h[LastState][LastAction] = 0;}
			else h[LastState][LastAction]+=1;



			// Update the remaining time to maturity
			if (!qt.empty()) tleft -= e;
			// Put new events into the queue
			adevs::Bag<Loan>::const_iterator iter = xb.begin();
			for (; iter != xb.end(); iter++)
			{
				// Place the deposit or loan into the back of the queue, add a copy to the total queue
				if( (*iter).getID() == 1) {
					// If queue is empty
					if (qt.empty()) tleft = (*iter).get_t_to_maturity();
					qd.push_back(*iter);
					qt.push_back(*iter);
				}

				else {//Here the bank has to decide whether accept the loan or not
					//Initialize unvisited state-action pairs


					for(int i=0; i!=2; i++){ //Here we add all decisions available at this stage, so far only if accept a loan or not
						if(h[LastState][a.at(i)]==0)	SetInitialValue(LastState, a.at(i) , 2-4*i); //this 2-4i solve my problem...finally
					}

					//Last key of the map is the optimal action given the state
					//If loan is accepted add to queue and set time if needed, otherwise ignore the request
					if( (ValueFunction[LastState]).rbegin()->second == a.at(0) ) {//if optimal action is to accept the loan given the state
					LastAction = a[0];
					if (qt.empty()) tleft = (*iter).get_t_to_maturity();
					ql.push_back(*iter);
					qt.push_back(*iter);
						}
					else LastAction = a[1];
					//LastAction = a[1];
				}

			}


		}
		void Commercial_bank::delta_conf(const adevs::Bag<Loan>& xb)
		{
			delta_int();
			delta_ext(0.0,xb);
		}
		void Commercial_bank::output_func(adevs::Bag<Loan>& yb)
		{
			// Output is the expired loan or deposit
			yb.insert(qt.front());

		}
		double Commercial_bank::ta()
		{
			return tleft;
		}

		bool Commercial_bank::model_transition()
		{
		//transition activated if insolvent
			return (checkInsolvent());
		}
		// Get the number of deposits in the queue
		unsigned int Commercial_bank::getDepositQueueSize() { return qd.size(); }

		unsigned int Commercial_bank::getLoanQueueSize() { return ql.size(); }

		void Commercial_bank::gc_output(adevs::Bag<Loan>&){}

		double Commercial_bank::value(){return equity;}

		void Commercial_bank::reduce_equity(double shock){ equity-=shock;}

		bool Commercial_bank::checkInsolvent() {return  equity <0 ;}

		bool Commercial_bank::NextExpiring  (const Loan& l1 , const Loan& l2)  { return l1.get_t_to_maturity() < l2.get_t_to_maturity(); }

		bool Commercial_bank::WillItDefault() {return def.uniform(0,1)<Pdefault;}

		void Commercial_bank::SetDepositRate(int r){DepositRate = r;}

		void Commercial_bank::SetLoanRate(int r){LoanRate = r;}

		void Commercial_bank::SetDepositRateRange(double a, double b){
			vector<double> aux;

			while(a <= b) {aux.push_back(a);
			        a += 0.01;}

			DepositRates = aux;}

		void Commercial_bank::SetLoanRateRange(double a, double b){
			vector<double> aux;

			while(a <= b) {aux.push_back(a);
			        a += 0.01;}

			LoanRates = aux;
		}

		double Commercial_bank::GetDepositRate(){return DepositRates[DepositRate];}

		double Commercial_bank::GetLoanRate(){return LoanRates[LoanRate];}

//		void Commercial_bank::SetEquityRange(double a, double b)
//		{	vector<double> aux;
//
//		while(a <= b) {aux.push_back(a);
//		        a += 0.01;}
//
//			Equities = aux;
//		}

		void Commercial_bank::SetEquity(double e){equity = e;}

		void Commercial_bank::IncreaseEquity(double e){equity += e;}

		double Commercial_bank::GetEquity(){return equity;}
		//{return Equities[equity];}

		void Commercial_bank::SetInitialValue(vector<int> state, vector<int> action, double v){
			ValueFunction[state][v]=action;
			ValueFunction2[state][action]=v;
		}
		// Function to set actual profit, state's profit (\pi)
		//void Commercial_bank::SetProfit(vector<int>);
		// Function to set continuation value (w)
		void Commercial_bank::UpdateW(vector<int> state, vector<int> action, double CurrentV){

			ValueFunction2[state][action]= ValueFunction2[state][action] * h[state][action]/(h[state][action]+1) + CurrentV/(h[state][action]+1);

			ValueFunction[state].erase( ValueFunction2[state][action] );

			ValueFunction[state][ ValueFunction2[state][action] ] = action;

		}

		int Commercial_bank::GetNumberStatesVisited(){return h.size();}

		double Commercial_bank::LastStateActionValue(){return ValueFunction2[LastState][LastAction];}

