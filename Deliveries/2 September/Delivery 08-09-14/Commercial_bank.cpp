#include "Commercial_bank.h"
#include "BNetwork.h"
#include "adevs.h"
#include "Loan.h"
#include "Parameters.h"
#include <deque>
#include <algorithm>
#include <map>
#include <string>
#include <iostream>
#include <fstream>
using namespace std;

		//Static variables
		map<vector<int> , map<vector<int>,int > > Commercial_bank::h;
		map<vector<int>,int > Commercial_bank::h_data;

		map<vector<int>, map<double,vector<int> > > Commercial_bank::ValueFunction;
		map< double,vector<int> > Commercial_bank::ValueFunction_data;

		map<vector<int>, map< vector<int>, double > > Commercial_bank::ValueFunction2;
		map< vector<int>, double > Commercial_bank::ValueFunction2_data;

		map<vector<int>, map< vector<int>, double > > Commercial_bank::InitVal;
		map< vector<int>, double > Commercial_bank::InitVal_data;

		bool Commercial_bank::UpdatingProcess = true;

		vector<double> Commercial_bank::LoanRates;
		vector<double> Commercial_bank::DepositRates;
		int Commercial_bank::RateDepositCB;
		int Commercial_bank::RateLoanCB;

		map< Commercial_bank*, int > Commercial_bank::ActualAgencyRatings;

		map< Commercial_bank*, int > Commercial_bank::LaggedAgencyRatings;



		void Commercial_bank::SetDepositRateRange(double a, double b){
			vector<double> aux;

			while(a <= b) {aux.push_back(a);//just a generalization
			a += 0.01;}

			DepositRates = aux;}

		void Commercial_bank::SetLoanRateRange(double a, double b){
			vector<double> aux;

			while(a <= b) {aux.push_back(a);//just a generalization
			a += 0.01;}

			LoanRates = aux;
		}


		void Commercial_bank::SetCBDepositRate(int r){RateDepositCB = r;}
		void Commercial_bank::SetCBLoanRate(int r){RateLoanCB = r;}




		Commercial_bank::Commercial_bank(double ProbabilityDefault):adevs::Atomic<Loan>(),tleft(DBL_MAX)
		{
			//SetEquityRange(0,50);
			partners=NNeighborhood; //So far, only 3 partners are allowed, but clearly this could be N(i)
			SetAR(1);
			equity=10000;
			defaults=0;
			Pdefault = ProbabilityDefault;
			profits=0;

			SetIBDepositRate(CBDFR+rand()%10-rand()%10);		//If all banks start with the same rate there is a problem when searching for the best IB rate from a partner
			SetIBLoanRate(CBLFR+rand()%10-rand()%10);

			ACustomerLoan.push_back(accept);
			ACustomerLoan.push_back(reject);
			ACustomerLoan.push_back(SendToBacklog);


			ACustomerDeposit.push_back(UseCR);
			ACustomerDeposit.push_back(GoToInterbankMarket);

			//Initial Rating given by a Rating Agency (an integer value 0:Good, 1:Bad),							0
			ActualAgencyRatings[this] = 0;
		};

		void Commercial_bank::delta_int()


		{

			//Last state and last action were defined when the last action was taken, this happened in the delta_ext() function
			//Here we have to update value of decisions, because in this function (delta_int) is where we can see realized values of past actions
			//In case of deposit/////////////////////////////////////////////////////////////////////
			if( ((qt.front()).getID()==1) | ((qt.front()).getID()==1)) {reduce_equity( (qt.front()).GetQuantity()); //what is the amount lent?
			if(UpdatingProcess)UpdateW(LastState, LastAction, - ( qt.front()).GetQuantity() ); //Update given the realized state is a deposit expiration
			}

			//In case of loan////////////////////////////////////////////////////////////////////////
			//If there is default, reduce equity. If all is ok, get the profits from a successful loan
			if((qt.front()).getID()==2){ if( WillItDefault() ) {reduce_equity( ( qt.front()).GetQuantity() ); //what is the amount lent?
																if(UpdatingProcess)UpdateW(LastState, LastAction, - (qt.front()).GetQuantity());
			}//Update given realized state is a loan default
			else {IncreaseEquity( (qt.front()).GetQuantity() );
			if(UpdatingProcess) UpdateW(LastState, LastAction, ( qt.front()).GetQuantity() );//Update given realized state is a successful loan
			}
			}

			//Now change state, it doesn't matter because we are not making any decision in this part
			qt.pop_front(); // Remove the expired deposit


			//We need to update the time elapsed for all objects
			std::deque<Loan>::iterator iter = qt.begin();
				for (; iter != qt.end(); iter++){(*iter).ReduceTimeToMaturity(tleft);} //Time to maturity is reduced by the time elapsed

			if (qt.empty()) tleft = DBL_MAX; // No event left
			else{//look for next deposit to expire
				sort(qt.begin(),qt.end(),NextExpiring);
				tleft = qt.begin()->get_t_to_maturity();
			}


		}


		void Commercial_bank::delta_ext(double e, const adevs::Bag<Loan>& xb)
		{
			UpdatePartnerInfo();
			// Update the remaining time to maturity
			if (!qt.empty()){ tleft -= e;

			//We need to update the time elapsed for all objects
			std::deque<Loan>::iterator iter = qt.begin();
			for (; iter != qt.end(); iter++){(*iter).ReduceTimeToMaturity(e);} //Time to maturity is reduced by the time elapsed
			}

			// Put new events into the queue, here we can to separate customers loan from partners loans according to the ID in the loan object
			adevs::Bag<Loan>::const_iterator iter = xb.begin();
			for (; iter != xb.end(); iter++)//We have to look at each new event coming
			{
				// If customer deposit or interbank deposit, then place the deposit or loan into the back of the queue, add a copy to the total queue
				if( ( (*iter).getID() == 1 ) | ( (*iter).getID() == 1) ) {/////////////////////////////////////////////////////////////////////////////
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
					//if (qt.empty()) tleft = (*iter).get_t_to_maturity();
					//qd.push_back(*iter);
					//qt.push_back(*iter);

					//Here the bank has to decide what to do with the new customer deposit
					//Initialize unvisited state-action pairs

					for(unsigned int i=0; i!=ACustomerDeposit.size(); i++){ //Here we add all decisions available at this stage, so far send the deposit to backlog with desired rate, try to match with partner or match with Cash reserves
						if(h[LastState].find(ACustomerDeposit[i]) == h[LastState].end())	SetInitialValue(LastState, ACustomerDeposit.at(i) , a.uniform(-1,1) );
					}

					if( (ValueFunction[LastState]).rbegin() -> second == ACustomerDeposit.at(0) ){//Use CR
						//if (qt.empty()) tleft = (*iter).get_t_to_maturity();
						if ((*iter).get_t_to_maturity() < tleft) {
							tleft = (*iter).get_t_to_maturity();
							if((*iter).getID() == 1)qd.push_front(*iter);
							if((*iter).getID() == 11)qt.push_front(*iter);
							qt.push_front(*iter);}

							else{ if((*iter).getID() == 1) qd.push_back(*iter);
							if((*iter).getID() == 11) qida.push_back(*iter);
							qt.push_back(*iter);

					}}


					if( (ValueFunction[LastState]).rbegin()->second == ACustomerDeposit.at(1) ){
						// If Output is a request for interbank deposit, with a partner


						tleft=0; //Immediately send the request to interbank partner
						qidr.push_back(*iter);
						qt.push_front(*iter);
						//Look for the highest deposit rate offered by partners, at rbegin(), There is a problem with rbegin() here.
						qt.front().SetLinkedAgent( PartnersVectorDR.begin()->second  ); //This is the partner that should receive the request
					//	qt.front().SetLinkedAgent( (qt.front().get_IDlender()+1)%9 ) ; //testing
					//	qt.front().setID(11); //Send deposit request to partner
					//	UpdatePartnersSet(1);

					}




//					if(qt.front().getID()==21){}//Send loan request to partner

					//Count the number of times that the last state has been visited
//					if(h.find(LastState) == h.end() && h[LastState].find(LastAction) == h[LastState].end())
//					{h[LastState][LastAction] = 0;}
//					else h[LastState][LastAction]+=1;


				}

				//If customer loan, then we have to make a decision/////////////////////////////////////////////////////
				if( ( (*iter).getID() == 2 ) | ( (*iter).getID() == 21 ) ) {

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
						if(h[LastState].find(ACustomerLoan[i]) == h[LastState].end())	SetInitialValue(LastState, ACustomerLoan.at(i) , a.uniform(-1,1));
						//This is very adhoc, I expect that accepting a loan would be more profitable than rejecting it.
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
			if( ( qt.front().getID()==1 ) | ( qt.front().getID()==2 ) ) qt.front().SetAsExpired();
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

		void Commercial_bank::SetCDepositRate(int r){CDepositRate = r;}

		void Commercial_bank::SetCLoanRate(int r){CLoanRate = r;}

		int Commercial_bank::GetCDepositRate(){return CDepositRate;}

		int Commercial_bank::GetCLoanRate(){return CLoanRate;}

		void Commercial_bank::SetIBDepositRate(int r){IBDepositRate = r;}

		void Commercial_bank::SetIBLoanRate(int r){IBLoanRate = r;}

		int Commercial_bank::GetIBDepositRate(){return IBDepositRate;}

		int Commercial_bank::GetIBLoanRate(){return IBLoanRate;}



//		void Commercial_bank::SetEquityRange(double a, double b)
//		{	vector<double> aux;
//
//		while(a <= b) {aux.push_back(a);
//		        a += 0.01;}
//
//			Equities = aux;
//		}

		void Commercial_bank::SetEquity(int e){equity = e;}

		void Commercial_bank::IncreaseEquity(int e){equity += e;}

		int Commercial_bank::GetEquity(){return equity;}
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
		void Commercial_bank::AddPartner(Commercial_bank* p){

		set<Commercial_bank*>::iterator it;
		//it = find (PartnersVector.begin(), PartnersVector.end(), p);
		it = PartnersVector.find(p);
		if (it == PartnersVector.end())	{
		PartnersVector.insert(p);
		PartnersVectorDR[ (*p).GetIBLoanRate() ]=p;
		PartnersVectorLR[ (*p).GetIBDepositRate() ]=p;
		PartnersVectorDRderef[ p ]= (*p).GetIBLoanRate();
		PartnersVectorLRderef[ p ]= (*p).GetIBDepositRate();
		}
		else{
		//Remove old info
		PartnersVectorDR.erase( PartnersVectorDRderef[p] );
		PartnersVectorLR.erase( PartnersVectorLRderef[p] );
		//Add updated info
		PartnersVectorDR[ (*p).GetIBLoanRate() ]=p;
		PartnersVectorLR[ (*p).GetIBDepositRate() ]=p;
		PartnersVectorDRderef[ p ]= (*p).GetIBLoanRate();
		PartnersVectorLRderef[ p ]= (*p).GetIBDepositRate();
		}
		}

		int Commercial_bank::GetNPartners(){return PartnersVector.size();}

		int Commercial_bank::GetAR(){return LaggedAgencyRatings[this];}

		void Commercial_bank::SetAR(int a){LaggedAgencyRatings[this] = a;}

		void Commercial_bank::StopUpdatingProcess(){UpdatingProcess = false;}

		void Commercial_bank::StartUpdatingProcess(){UpdatingProcess = true;}

		void Commercial_bank::SendRating(){

		LaggedAgencyRatings[this] = ActualAgencyRatings[this];
		if( ( (CSH+qila.size())/(qd.size()+qida.size()+qcblf.size()) < DowngradeRatio1 ) | ( equity/ (CSH + ql.size() + qila.size()) < DowngradeRatio2 ) )
		ActualAgencyRatings[this] = 1;
		else ActualAgencyRatings[this] = 0;

		};

		void Commercial_bank::SetLqBnch(){
			LqBnch = beta2*(qd.size()+qida.size()+qcblf.size());
		}

		bool Commercial_bank::CheckCashReserveRequirements(){return CSH > (qd.size()+qida.size()+qcblf.size());}

		//Update partners set depending on whether the last action involves a deposit or loan, a = 1 for deposit, a = 2, for loan
		void Commercial_bank::UpdatePartnersSet(int a){
			//set<Commercial_bank*>::iterator iter = PartnersVector.begin();
			Commercial_bank* aux;
			int prevsize;
			if( a == 1 ){
				//while( find(PartnersVector.begin(), PartnersVector.end(), aux) != PartnersVector.end() ){ int n = rand()%NBANKS; aux = BNetwork::getBank(n); }
				while( prevsize == PartnersVector.size() ){ aux = BNetwork::getBank(rand()%NBANKS); prevsize = PartnersVector.size(); PartnersVector.insert(aux);}
				//PartnersVector.erase ( find(PartnersVector.begin(), PartnersVector.end(), (PartnersVectorDR.begin())->second) );
				PartnersVectorLR.erase( PartnersVectorLRderef[ (PartnersVectorDR.begin()->second) ] );
				PartnersVector.erase ( (PartnersVectorDR.begin())->second );
				PartnersVectorDR.erase(PartnersVectorDR.begin()->first);
				AddPartner(aux);
			}

			else {
				//while( find(PartnersVector.begin(), PartnersVector.end(), aux) != PartnersVector.end() ){ int n = rand()%NBANKS; aux = BNetwork::getBank(n); }
				while( prevsize == PartnersVector.size() ){ aux = BNetwork::getBank(rand()%NBANKS); prevsize = PartnersVector.size(); PartnersVector.insert(aux);}

								//PartnersVector.erase ( find(PartnersVector.begin(), PartnersVector.end(), (PartnersVectorLR.rbegin())->second) );
								PartnersVectorDR.erase( (PartnersVectorLR.rbegin()->second)->GetIBDepositRate() );
								PartnersVector.erase ( (PartnersVectorLR.rbegin())->second );
								PartnersVectorLR.erase(PartnersVectorLR.rbegin()->first);
								AddPartner(aux);
			}
			}


		void Commercial_bank::UpdatePartnerInfo(){
			set<Commercial_bank*>::iterator iter = PartnersVector.begin();
				for (;iter != PartnersVector.end(); iter++) AddPartner(*iter); //Update info about partner
		}


		//void Commercial_bank::CancelLoan()
		//void Commercial_bank::ResolveCancelledLoan()
