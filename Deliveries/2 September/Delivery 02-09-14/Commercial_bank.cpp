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

		bool Commercial_bank::UpdatingProcess = true;

		Commercial_bank::Commercial_bank(double ProbabilityDefault):adevs::Atomic<Loan>(),tleft(DBL_MAX)
		{
			//SetEquityRange(0,50);
			partners=2; //So far, only 3 partners are allowed, but clearly this could be N(i)
			SetAR(1);
			equity=10000;
			defaults=0;
			Pdefault = ProbabilityDefault;
			profits=0;
			SetDepositRateRange(0,0.1);
			SetLoanRateRange(0,0.1);
			DepositRate=0;
			LoanRate=5;

			ACustomerLoan.push_back(accept);
			ACustomerLoan.push_back(reject);

			//ACustomerDeposit

		};

		void Commercial_bank::delta_int()


		{

			//Last state and last action were defined when the last action was taken, this happened in the delta_ext() function
			//Here we have to update value of decisions, because in this function (delta_int) is where we can see realized values of past actions
			//In case of deposit/////////////////////////////////////////////////////////////////////
			if((qt.front()).getID()==1) {reduce_equity( (qt.front()).GetQuantity()); //what is the amount lent?
			if(UpdatingProcess)UpdateW(LastState, LastAction, - (qt.front()).GetQuantity()); //Update given the realized state is a deposit expiration
			}

			//In case of loan////////////////////////////////////////////////////////////////////////
			//If there is default, reduce equity. If all is ok, get the profits from a successful loan
			if((qt.front()).getID()==2){ if( WillItDefault() ) {reduce_equity( (qt.front()).GetQuantity()); //what is the amount lent?
																if(UpdatingProcess)UpdateW(LastState, LastAction, - (qt.front()).GetQuantity());
			}//Update given realized state is a loan default
			else {IncreaseEquity( (qt.front()).GetQuantity() );
			if(UpdatingProcess) UpdateW(LastState, LastAction, (qt.front()).GetQuantity() );//Update given realized state is a successful loan
			}
			}

			//Now change state, it doesn't matter because we are not making any decision in this part
			qt.pop_front(); // Remove the expired deposit

			if (qt.empty()) tleft = DBL_MAX; // No event left

			else{//look for next deposit to expire
				sort(qt.begin(),qt.end(),NextExpiring);
				tleft = qt.begin()->get_t_to_maturity()-tleft;
			} //- tleft because the borrower or depositor was already for tleft time at the bank


		}


		void Commercial_bank::delta_ext(double e, const adevs::Bag<Loan>& xb)
		{

			// Update the remaining time to maturity
			if (!qt.empty()) tleft -= e;
			// Put new events into the queue, here we can to separate customers loan from partners loans according to the ID in the loan object
			adevs::Bag<Loan>::const_iterator iter = xb.begin();
			for (; iter != xb.end(); iter++)//We have to look at each new event coming
			{
				// If customer deposit, then place the deposit or loan into the back of the queue, add a copy to the total queue
				if( (*iter).getID() == 1) {/////////////////////////////////////////////////////////////////////////////
					//Initialize the last state vector if needed
					if(LastState.empty()){
						LastState.push_back(qd.size());
						LastState.push_back(ql.size());
						//LastState.push_back(equity);
					}
					//Update the last state vector, last state vector is the state before the time when we make the decision
					LastState[0] = qd.size();
					LastState[1] = ql.size();

					// If queue is empty
					if (qt.empty()) tleft = (*iter).get_t_to_maturity();
					qd.push_back(*iter);
					qt.push_back(*iter);

					//Here the bank has to decide what to do with the new customer deposit
					//Initialize unvisited state-action pairs

//					for(int i=0; i!=ACustomerDeposit.size(); i++){ //Here we add all decisions available at this stage, so far send the deposit to backlog with desired rate, try to match with partner or match with Cash reserves
//						if(h[LastState].find(ACustomerDeposit[i]) == h[LastState].end())	SetInitialValue(LastState, ACustomerDeposit.at(i) , 0.001-0.002*i);
//					}



					//Count the number of times that the last state has been visited
//					if(h.find(LastState) == h.end() && h[LastState].find(LastAction) == h[LastState].end())
//					{h[LastState][LastAction] = 0;}
//					else h[LastState][LastAction]+=1;


				}

				//If customer loan, then we have to make a decision/////////////////////////////////////////////////////
				if( (*iter).getID() == 2) {

					//Initialize the last state vector if needed
					if(LastState.empty()){
						LastState.push_back(qd.size());
						LastState.push_back(ql.size());
					}

					//Update the last state vector, last state vector is the state before the time when we make the decision
					LastState[0] = qd.size();
					LastState[1] = ql.size();

					//Here the bank has to decide whether accept the loan or not
					//Initialize unvisited state-action pairs

					for(int i=0; i!=2; i++){ //Here we add all decisions available at this stage, so far only if accept a loan or not
						if(h[LastState].find(ACustomerLoan[i]) == h[LastState].end())	SetInitialValue(LastState, ACustomerLoan.at(i) , -0.01+0.005*i);
						//This is very adhoc, I expect that accepting a loan would be more profitable than rejecting it, so I give that action a little more value (tiny perturbation so the agent can decide at the beginning)
					}

					//Last key of the map is the optimal action given the state, because the way map is sorted automatically
					//If loan is accepted add to queue and set time if needed, otherwise ignore the request

					if( (ValueFunction[LastState]).rbegin()->second == ACustomerLoan.at(0) ) //if optimal action is to accept the loan given the state (remember that optimal action is at rbegin(), do the following
					{
					LastAction = ACustomerLoan[0];
					if (qt.empty()) tleft = (*iter).get_t_to_maturity();
					ql.push_back(*iter);
					qt.push_back(*iter);
						}
					if( (ValueFunction[LastState]).rbegin()->second == ACustomerLoan.at(1) ){ //If optimal action is to reject the loan...only reject it and do nothing
						LastAction = ACustomerLoan[1];
					}

					//Count the number of times that the last state has been visited
					if(h.find(LastState) == h.end() && h[LastState].find(LastAction) == h[LastState].end())
					{h[LastState][LastAction] = 0;}
					else h[LastState][LastAction]+=1;

				}

				//If interbank market deposit or loan request... the same, the difference is that in this case we have to send an output to the network using output_func(), the BNetwork::route send the event to the specific partner that is indicated in the loan object sent
				//Remember that output_func is called just before the delta_int. I have to work on this

			}

		}
		void Commercial_bank::delta_conf(const adevs::Bag<Loan>& xb)
		{
			delta_int();
			delta_ext(0.0,xb);
		}
		void Commercial_bank::output_func(adevs::Bag<Loan>& yb)
		{
			// If Output is the expired loan or deposit
			yb.insert(qt.front());

			// Send loan or deposit request to partner
//			if(time_next_deposit<=time_next_loan){
//				Loan* l = new Depositor();
//				l->set_borrower(AssignBank (n_banks));
//				yb.insert( *l );
//
//			}
//
//			else{
//				Loan* l = new Borrower();
//				l->set_lender(AssignBank (n_banks));
//				yb.insert( *l );
//			}



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

		int Commercial_bank::GetDepositRate(){return DepositRates[DepositRate];}

		int Commercial_bank::GetLoanRate(){return LoanRates[LoanRate];}

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

		// Function to set continuation value (w)
		void Commercial_bank::UpdateW(vector<int> state, vector<int> action, double CurrentV){

			ValueFunction2[state][action]= ValueFunction2[state][action] * h[state][action]/(h[state][action]+1) + CurrentV/(h[state][action]+1);

			ValueFunction[state][ ValueFunction2[state][action] ] = action;

		}

		int Commercial_bank::GetNumberStatesVisited(){return h.size();}

		double Commercial_bank::LastStateActionValue(){return ValueFunction2[LastState][LastAction];}

		void Commercial_bank::SetNPartners(int n){partners=n;}
		//Return allowed number of partners
		int Commercial_bank::GetAllowedNPartners(){return partners;}

		//Add the partner as a value in the map, the rate is the key so the map is sorted in decreasing order, in the same way as ValueFunction
		void Commercial_bank::AddPartner(Commercial_bank* p){PartnersSet.insert(p);
		PartnersVectorBR[ (*p).GetLoanRate() ][ (*p).GetAR() ]=p;
		PartnersVectorLR[ (*p).GetDepositRate() ][ (*p).GetAR() ]=p;
		}

		int Commercial_bank::GetNPartners(){return PartnersSet.size();}

		int Commercial_bank::GetAR(){return AR;}

		void Commercial_bank::SetAR(int a){AR=a;}

		void Commercial_bank::StopUpdatingProcess(){UpdatingProcess=false;}

		void Commercial_bank::StartUpdatingProcess(){UpdatingProcess=true;}


