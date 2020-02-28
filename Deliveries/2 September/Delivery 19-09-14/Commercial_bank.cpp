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
#include <sstream>
#include <stdexcept>

using namespace std;

		//Static variables

		//A small class to merge these two maps h and ValueFunction, difference frequencies for banks
//		map<vector<int> , map<vector<int>,int > > Commercial_bank::h;
//		map<vector<int> ,int > Commercial_bank::h_data;

		map< State , map< Action, int > > Commercial_bank::h;
		map< Action ,int > Commercial_bank::h_data;

//		map< Strategy , int > Commercial_bank::h;

//		map<vector<int> , map<double, vector<int> > > Commercial_bank::ValueFunction;
//		map< double,vector<int> > Commercial_bank::ValueFunction_data;

		map< State , map< double, Action > > Commercial_bank::ValueFunction;
		map< double, Action > Commercial_bank::ValueFunction_data;

//		map< Strategy, double > Commercial_bank::ValueFunction2;

//		map<vector<int>, map< vector<int>, double > > Commercial_bank::ValueFunction2;
//		map< vector<int>, double > Commercial_bank::ValueFunction2_data;

		map<State, map< Action, double > > Commercial_bank::ValueFunction2;
		map< Action, double > Commercial_bank::ValueFunction2_data;

//		map<vector<int>, map< vector<int>, double > > Commercial_bank::InitVal;
//		map< vector<int>, double > Commercial_bank::InitVal_data;

//		map< Strategy, double > Commercial_bank::InitVal;

		map< State, map< Action, double > > Commercial_bank::InitVal;
		map< Action, double > Commercial_bank::InitVal_data;

		bool Commercial_bank::UpdatingProcess = true;

		vector<double> Commercial_bank::LoanRates;
		vector<double> Commercial_bank::DepositRates;
		int Commercial_bank::RateDepositCB;
		int Commercial_bank::RateLoanCB;
		int Commercial_bank::CDepositRate;
		int Commercial_bank::CLoanRate;

		map< Agent*, int > Commercial_bank::ActualAgencyRatings;

		map< Agent*, int > Commercial_bank::LaggedAgencyRatings;



		void Commercial_bank::SetDepositRateRange(double a, double b){
			vector<double> aux;

			while(a <= b) {aux.push_back(a);//just a generalization
			a += 0.1;}

			DepositRates = aux;}

		void Commercial_bank::SetLoanRateRange(double a, double b){
			vector<double> aux;

			while(a <= b) {aux.push_back(a);//just a generalization
			a += 0.1;}

			LoanRates = aux;
		}


		void Commercial_bank::SetCBDepositRate(int r){RateDepositCB = r;}
		void Commercial_bank::SetCBLoanRate(int r){RateLoanCB = r;}

		void Commercial_bank::SetCDepositRate(int r){CDepositRate = r;}

		void Commercial_bank::SetCLoanRate(int r){CLoanRate = r;}

		int Commercial_bank::GetCDepositRate(){return CDepositRate;}

		int Commercial_bank::GetCLoanRate(){return CLoanRate;}

///////////////////////////////////////////////////////////////////////////////////////////////////////////
		Commercial_bank::Commercial_bank(double ProbabilityDefault) :adevs::Atomic<Loan>(),tleft(DBL_MAX)
				//IBDepositRate(2),IBLoanRate(2)

		{
			//SetEquityRange(0,50);
			partners=NNeighborhood; //So far, only 3 partners are allowed, but clearly this could be N(i)
			SetAR(0);
			equity=10000;
			CSH = 100000;
			defaults=0;
			Pdefault = ProbabilityDefault;
			profits=0;

			IBDepositRate = CBDFR+rand()%20-rand()%20 ;
			IBLoanRate = CBLFR+rand()%20-rand()%20 ;

			sendingIBDepositRequest = false;
			sendingIBLoanRequest = false;

			actionsSet.insert(* new Action(0, true, true));
			actionsSet.insert(* new Action(0, true, false));
			actionsSet.insert(* new Action(0, false, true));
			actionsSet.insert(* new Action(0, false, false));
			actionsSet.insert(* new Action(1, true, true));
			actionsSet.insert(* new Action(1, true, false));
			actionsSet.insert(* new Action(1, false, true));
			actionsSet.insert(* new Action(1, false, false));

			qd=0;
			qida=0;
			qcblf=0;
			qilr=0;
			ql=0;
			qila=0;
			qidr=0;

			//Initial Rating given by a Rating Agency (an integer value 0:Good, 1:Bad),							0
			ActualAgencyRatings[this] = 0;
		};

		void Commercial_bank::delta_int()
		{

			if(sendingIBDepositRequest){
				cout << "Internal Function Sending IB Deposit Request" << endl;
				sendingIBDepositRequest = false;
				qidrQueue.pop_back();
			}

			else if(sendingIBLoanRequest){
							cout << "Internal Function Sending IB Loan Request" << endl;
							sendingIBLoanRequest = false;
							qilrQueue.pop_back();
			}

			else{

				cout << "Internal Function expired deposit" << endl;

//				if((qt.front()).getType() == 11 & (qt.front()).get_borrower()==this ) throw invalid_argument("self IB deposit request when not sending request");

			//Last state and last action were defined when the last action was taken, this happened in the delta_ext() function
			//Here we have to update value of decisions, because in this function (delta_int) is where we can see realized values of past actions
			//In case of deposit/////////////////////////////////////////////////////////////////////
			if( ((qt.front()).getType()==1) | ((qt.front()).getType()==11)) {reduce_equity( (qt.front()).GetQuantity()); //what is the amount lent?
			if(UpdatingProcess)UpdateW(LastState, LastAction, - ( qt.front()).GetQuantity() ); //Update given the realized state is a deposit expiration
			}

			//In case of loan////////////////////////////////////////////////////////////////////////
			//If there is default, reduce equity. If all is ok, get the profits from a successful loan
			if((qt.front()).getType()==2){ if( WillItDefault() ) {reduce_equity( ( qt.front()).GetQuantity() ); //what is the amount lent?
																if(UpdatingProcess)UpdateW(LastState, LastAction, - (qt.front()).GetQuantity());
			}//Update given realized state is a loan default
			else {IncreaseEquity( (qt.front()).GetQuantity() );
			if(UpdatingProcess) UpdateW(LastState, LastAction, ( qt.front()).GetQuantity() );//Update given realized state is a successful loan
			}
			}




		//if((qt.front()).getType() == 11) throw invalid_argument("incoming IB deposit request begin internal function");
		//if((qt.front()).getType() == 11 & (qt.front()).GetLinkedAgent()==this & (qt.front()).get_borrower()==this ) throw invalid_argument("self IB deposit request");
		//if((qt.front()).getType() == 11 & (qt.front()).GetLinkedAgent()==this ) throw invalid_argument("self IB deposit request");


			cout << "SIZE QT" << qt.size() << endl;
			//Now change state, it doesn't matter because we are not making any decision in this part
			if(qt.size() == 0) throw invalid_argument("Empty QT");

			if(qt.front().getType()==1) qd--;
			else if(qt.front().getType()==11) qida--;
			else if(qt.front().getType()==2) ql--;
			else qila--;

			//Loan * temp = & qt.front();
			qt.pop_front(); // Remove the expired object
			//I need to delete the pointer to the object


			//We need to update the time elapsed for all objects
			std::deque<Loan>::iterator iter = qt.begin();
				for (; iter != qt.end(); iter++){(*iter).ReduceTimeToMaturity(tleft);} //Time to maturity is reduced by the time elapsed



			if (qt.empty()) tleft = DBL_MAX; // No event left

			else{//look for next object to expire
				sort(qt.begin(),qt.end(),NextExpiring);
				tleft = qt.begin()->get_t_to_maturity();
			}

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

			// Put new events into the queue, here we can to separate loan objects according to the type in the loan object
			adevs::Bag<Loan>::const_iterator iter = xb.begin();
			for (; iter != xb.end(); iter++)//We have to look at each new object coming
			{
				cout << "START of BAG" << endl;
				UpdatePartnerInfo();

				/////////////////////////////////////////////////////////////////////////////CUSTOMER DEPOSIT OR IB DEPOSIT////////////////////////////////////////////////////////////////////
				if( ( (*iter).getType() == 1 ) | ( (*iter).getType() == 11) ) {

					cout << "external function incoming customer deposit or IB deposit" << endl;

					updateState( (*iter).getType(), &(*iter) );

					std::set<Action>::const_iterator iter2 = actionsSet.begin();
					cout << "Setting Initial belief for deposit" << endl;
					for (;iter2 != actionsSet.end(); iter2++){
						if(h[LastState].find(*iter2) == h[LastState].end())	SetInitialValue(LastState, *iter2 , a.uniform(-1,1) );
					}

					//The bank always accept any tipe of deposit
				//	if( (ValueFunction[LastState]).rbegin() -> second .accept | ((*iter).getType() == 1) ){//If opt decision is to accept  the incoming IB deposit or customer deposit


						if((*iter).getType() == 1){

							cout << "INCOMING CUSTOMER DEPOSIT" << endl;
							std::map<int , std::deque<Loan*> >::reverse_iterator iter3 = UMLoans.rbegin();

							//cout << UMLoans.size() << endl;

							for (int n=0; iter3 != UMLoans.rend() | n==0; iter3++, n++){
								//cout << "IN FOR" << endl;

									//If desired matching rate is lower than the highest rate in unmatched loans
								if(iter3 != UMLoans.rend()){if( ( (ValueFunction[LastState]).rbegin() -> second.desiredMatchingInterestRate <= (*iter3).first ) & ( (*iter3).second.size()!=0 )   ) {

										//If Partners rate lower than in the own backlog
										if( (PartnersVectorDR.rbegin()->first) < (*iter3).first )
										{
										//Set matched loan with in the unmatched loan
										(*iter3).second.front()->SetMatchedWith( &(*iter) );
										//Set matched with in the incoming deposit
										(*iter).SetMatchedWith( (*iter3).second.front() );
										//Delete from unmatched loan queue
										(*iter3).second.pop_front();

										if ((*iter).get_t_to_maturity() < tleft) {
											tleft = (*iter).get_t_to_maturity();
											//qd.push_front(*iter);
											qd++;
											qt.push_front(*iter);
										}

										else{
											//qd.push_back(*iter);
											qd++;
											qt.push_back(*iter);
											}
										UpdatePartnersSet(1);

										cout << "MATCHED WITH BACKLOG" << endl;
										break;


										}
										//If rate are the same in the own backlog and in the interbank market, flip a coin
										else if( PartnersVectorDR.rbegin()->first == (*iter3).first ){
											//match with own backlog
											cout << "MATCHED RANDOMLY WITH BACKLOG OR IB DEPOSIT" << endl;

											if( rand()%2 == 0 ){

												//Set matched loan with in the unmatched loan
												(*iter3).second.front()->SetMatchedWith( &(*iter) );
												//Set matched with in the incoming deposit
												(*iter).SetMatchedWith( (*iter3).second.front() );
												//Delete from unmatched loan queue
												(*iter3).second.pop_front();

												if ((*iter).get_t_to_maturity() < tleft) {
													tleft = (*iter).get_t_to_maturity();
												//	qd.push_front(*iter);
													qd++;
													qt.push_front(*iter);
												}

												else{
													//qd.push_back(*iter);
													qd++;
													qt.push_back(*iter);
												}


											}
											// match with interbank deposit, send request
											else{
												cout << "MATCHED RANDOMLY WITH BACKLOG OR IB DEPOSIT" << endl;

												(*iter).SetLinkedAgent( PartnersVectorDR.rbegin()->second  ); //This is the partner that should receive the request
												(*iter).setType(11);
												sendingIBDepositRequest = true;
												tleft=0; //Immediately send the request to interbank partner
												//qidr.push_back(*iter);
												qd++;
												qidr++;
												qidrQueue.push_back(*iter);
												UpdatePartnersSet(1);

											}
											UpdatePartnersSet(1);
											break;
										}
										// If best rate is in interbank market
										else if( PartnersVectorDR.rbegin()->first > (*iter3).first ){
											cout << "MATCHED WITH IB DEPOSIT, BACKLOG RATE OVER DESIRED" << endl;
											(*iter).SetLinkedAgent( PartnersVectorDR.rbegin()->second  ); //This is the partner that should receive the request
											(*iter).setType(11);
											sendingIBDepositRequest = true;
											tleft=0; //Immediately send the request to interbank partner
											//qidr.push_back(*iter);
											qd++;
											qidr++;
											qidrQueue.push_back(*iter);
											UpdatePartnersSet(1);
											break;
										}

						}}//End of case when both own backlog rate and interbank rate are higher than desired matching rate


									//Desired matching rate is higher than in the own backlog
									else{
										// The bank have to look in the interbank market
										// If rate in the interbank is higher than desired matching rate, there is a match
										if( (ValueFunction[LastState]).rbegin() -> second.desiredMatchingInterestRate <= PartnersVectorDR.rbegin()->first ){
											cout << "MATCHED WITH IB DEPOSIT, BACKLOG RATE BELOW DESIRED" << endl;
											(*iter).SetLinkedAgent( PartnersVectorDR.rbegin()->second  ); //This is the partner that should receive the request
											(*iter).setType(11);
											sendingIBDepositRequest = true;
											tleft=0; //Immediately send the request to interbank partner
											//qidr.push_back(*iter);
											qidr++;
											qd++;
											qidrQueue.push_back(*iter);
											UpdatePartnersSet(1);
											break;
										}
										//There is not match in the interbank market
										else {
											//Add this new deposit to the backlog of unmatched deposits
											cout << "NO MATCH, ADDING TO BACKLOG" << endl;
											UMDeposits[ (ValueFunction[LastState]).rbegin() -> second.desiredMatchingInterestRate ].push_back( &(*iter) );
											if ((*iter).get_t_to_maturity() < tleft) {
												tleft = (*iter).get_t_to_maturity();
											//	qd.push_front(*iter);
												qd++;
												qt.push_front(*iter);
											}
											else{
												//qd.push_back(*iter);
												qd++;
												qt.push_back(*iter);
											}
											UpdatePartnersSet(1);
											break;
										}
									} // END Desired matching rate is higher than in the own backlog

							}//END for that iterates in the backlog

						}// End of customer deposit case

						//START, if bank is accepting an incoming interbank deposit
						else if((*iter).getType() == 11){
							cout<< "INCOMING IB DEPOSIT FROM " << (*iter).get_IDborrower() << " TO " << (*iter).GetLinkedAgentID() << endl;
							if ((*iter).get_t_to_maturity() < tleft) {
								tleft = (*iter).get_t_to_maturity();
								//qida.push_front(*iter);
								cout << "ADDING IB DEPOSIT TO ACCEPTED IB DEPOSITS" << endl;
								qida++;
								qt.push_front(*iter);
							}
							else{
								//qida.push_back(*iter);
								cout << "ADDING IB DEPOSIT TO ACCEPTED IB DEPOSITS" << endl;
								qida++;
								qt.push_back(*iter);
							}
							UpdatePartnersSet(1);

						}


		//		}// END If opt decision is to accept the incoming IB deposit or customer deposit
				}// END Customer deposit or IB deposit case


///////////////////////////////////////////////////////////////////////////CUSTOMER LOAN OR IB LOAN/////////////////////////////////////////////////////////////////////////////////////////

							if( ( (*iter).getType() == 2 ) | ( (*iter).getType() == 21) ) {

								cout << "external function incoming customer loan or IB loan" << endl;

								updateState( (*iter).getType(), &(*iter) );

								std::set<Action>::const_iterator iter2 = actionsSet.begin();
								cout << "Setting Initial belief for loan" << endl;
								for (;iter2 != actionsSet.end(); iter2++){
									if(h[LastState].find(*iter2) == h[LastState].end())	SetInitialValue(LastState, *iter2 , a.uniform(-1,1) );
								}

							//	if( (ValueFunction[LastState]).rbegin() -> second .accept & CheckCashReserveRequirements() ){//If opt decision is to accept the incoming IB loan or customer loan
								if( true ){//If opt decision is to accept the incoming IB loan or customer loan

									if((*iter).getType() == 2){ //If customer loan request

										cout << "INCOMING CUSTOMER LOAN" << endl;
										std::map<int , std::deque<Loan*> >::iterator iter3 = UMDeposits.begin();

										//cout << UMLoans.size() << endl;

										for (int n=0; iter3 != UMDeposits.end() | n==0; iter3++, n++){
											//cout << "IN FOR" << endl;

												//If desired matching rate is higher than the lowest rate in unmatched loans
											if(iter3 != UMDeposits.end()){if( ( (ValueFunction[LastState]).begin() -> second.desiredMatchingInterestRate >= (*iter3).first ) & ( (*iter3).second.size()!=0 )   ) {

													//If Partners rate are higher than in the own backlog
													if( (PartnersVectorLR.begin()->first) > (*iter3).first )
													{
													//Set as matched loan with the unmatched deposit
													(*iter3).second.front()->SetMatchedWith( &(*iter) );
													(*iter).SetMatchedWith( (*iter3).second.front() ); //Respect priority
													//Delete from unmatched loan queue
													(*iter3).second.pop_front();

													if ((*iter).get_t_to_maturity() < tleft) {
														tleft = (*iter).get_t_to_maturity();
														ql++;
														qt.push_front(*iter);
													}

													else{
														ql++;
														qt.push_back(*iter);
														}
													UpdatePartnersSet(1);
													break;

													cout << "MATCHED WITH BACKLOG" << endl;

													}
													//If rate are the same in the own backlog and in the interbank market, flip a coin
													else if( PartnersVectorLR.begin()->first == (*iter3).first ){
														//match with own backlog
														cout << "MATCHED RANDOMLY WITH BACKLOG OR IB LOAN" << endl;

														if( rand()%2 == 0 ){

															//Set as matched with the unmatched deposit

															//cout << (*iter3).second.front()->getID() << " " << (*iter).getID() << endl;
															cout << " " << (*iter).getID() << endl;
															if( (*iter3).second.front() == NULL ) throw invalid_argument("NULL POINTER");
															(*iter3).second.front()->SetMatchedWith( &(*iter) );
															(*iter).SetMatchedWith( (*iter3).second.front() );
															//Delete from unmatched deposit queue
															(*iter3).second.pop_front();

															if ((*iter).get_t_to_maturity() < tleft) {
																tleft = (*iter).get_t_to_maturity();
																ql++;
																qt.push_front(*iter);
															}

															else{
																ql++;
																qt.push_back(*iter);
															}


														}
														// match with interbank deposit, send request
														else{
															cout << "MATCHED RANDOMLY WITH BACKLOG OR IB DEPOSIT" << endl;

															(*iter).SetLinkedAgent( PartnersVectorLR.begin()->second  ); //This is the partner that should receive the request
															(*iter).setType(21);
															sendingIBLoanRequest = true;
															tleft=0; //Immediately send the request to interbank partner
															qilr++;
															ql++;
															qilrQueue.push_back(*iter);
															UpdatePartnersSet(1);

														}
														UpdatePartnersSet(1);
														break;
													}
													// If best rate is in interbank market
													else if( PartnersVectorLR.begin()->first < (*iter3).first ){
														cout << "MATCHED WITH IB LOAN, BACKLOG RATE BELOW DESIRED" << endl;
														(*iter).SetLinkedAgent( PartnersVectorDR.rbegin()->second  ); //This is the partner that should receive the request
														(*iter).setType(21);
														sendingIBLoanRequest = true;
														tleft=0; //Immediately send the request to interbank partner
														qilr++;
														ql++;
														qilrQueue.push_back(*iter);
														UpdatePartnersSet(1);
														break;
													}

									}}//End of case when both own backlog rate and interbank rate are lower than desired matching rate


												//Desired matching rate is lower than in the own backlog
												else{
													// The bank have to look in the interbank market
													// If rate in the interbank is lower than desired matching rate, there is a match
													if( (ValueFunction[LastState]).begin() -> second.desiredMatchingInterestRate >= PartnersVectorLR.rbegin()->first ){
														cout << "MATCHED WITH IB LOAN, BACKLOG RATE OVER DESIRED" << endl;
														(*iter).SetLinkedAgent( PartnersVectorLR.begin()->second  ); //This is the partner that should receive the request
														(*iter).setType(21);
														sendingIBLoanRequest = true;
														tleft=0; //Immediately send the request to interbank partner
														qilr++;
														ql++;
														qilrQueue.push_back(*iter);
														UpdatePartnersSet(1);

														break;
													}
													//There is not match in the interbank market
													else {
														//Add this new deposit to the backlog of unmatched loans
														cout << "NO MATCH, ADDING TO BACKLOG" << endl;
														UMLoans[ (ValueFunction[LastState]).rbegin() -> second.desiredMatchingInterestRate ].push_back( &(*iter) );
														if ((*iter).get_t_to_maturity() < tleft) {
															tleft = (*iter).get_t_to_maturity();
															ql++;
															qt.push_front(*iter);
														}
														else{
															ql++;
															qt.push_back(*iter);
														}
														UpdatePartnersSet(1);
														break;
													}
												} // END Desired matching rate is higher than in the own backlog

										}//END for that iterates in the backlog
									}// End of customer loan case

									//START, if bank is accepting an incoming interbank loan
									else if((*iter).getType() == 21){
										cout<< "INCOMING IB LOAN FROM " << (*iter).get_IDlender() << " TO " << (*iter).GetLinkedAgentID() << endl;
										if ((*iter).get_t_to_maturity() <  tleft) {
											tleft = (*iter).get_t_to_maturity();
											qila++;
											qt.push_front(*iter);
											cout << "1 " << tleft << endl;
										}
										else{
											qila++;
											qt.push_back(*iter);
											cout << "2 " << tleft << endl;
										}
										UpdatePartnersSet(1);
										cout << "Size qt " << qt.size() << endl;

									}
									cout << "Size qt " << qt.size() << endl;


							}// END If opt decision is to accept the incoming IB deposit or customer deposit
							}// END Customer loan or IB loan case



			}//BAG for incoming events
			cout << "Size qt " << qt.size() << endl;

			cout << "End delta ext " << "Size qt " << qt.size() << endl;
		}//END DELTA_EXT

		void Commercial_bank::delta_conf(const adevs::Bag<Loan>& xb)
		{
			delta_int();
			delta_ext(0.0,xb);
		}
		void Commercial_bank::output_func(adevs::Bag<Loan>& yb)
		{

			cout << "output function" << endl;
			cout << "Size qt " << qt.size() << endl;

			if(sendingIBDepositRequest){yb.insert(qidrQueue.back());cout << "output function, sending IB Deposit Request" << endl;}

			else if(sendingIBLoanRequest){yb.insert( qilrQueue.back());cout << "output function, sending IB Loan Request" << endl;}

			//if( ( qt.front().getType()==1 ) | ( qt.front().getType()==2 ) | ( qt.front().getType()==11 ) )
			else
			{

				//if( qt.empty() ) throw invalid_argument("EMPTY QT");
				if(qt.front().getType() !=11 | qt.front().getType() !=21)  {
					qt.front().SetAsExpired();
					yb.insert(qt.front());
				}



				if(qt.front().getType()==1 ) cout << "output function, expired deposit" << endl;
				if(qt.front().getType()==2 ) cout << "output function, expired loan" << endl;
				if(qt.front().getType()==11 ) cout << "output function, expired IB deposit" << endl;

			}

			//if(sendingIBDepositRequest){yb.insert(qidr.back());cout << "output function, sending IB Deposit Request" << endl;}

			//if(sendingIBLoanRequest){yb.insert(qilr .back());cout << "output function, sending IB Loan Request" << endl;}





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
		unsigned int Commercial_bank::getDepositQueueSize() { return qd; }

		unsigned int Commercial_bank::getLoanQueueSize() { return ql; }

		unsigned int Commercial_bank::getIBDepositRequestQueueSize() { return qidr; }

		unsigned int Commercial_bank::getIBDepositAcceptedQueueSize() { return qida; }

		//unsigned int Commercial_bank::getLoanQueueSize() { return ql.size(); }

		void Commercial_bank::gc_output(adevs::Bag<Loan>&){}

		double Commercial_bank::value(){return equity;}

		void Commercial_bank::reduce_equity(double shock){ equity-=shock;}

		bool Commercial_bank::checkInsolvent() {return  equity <0 ;}

		bool Commercial_bank::NextExpiring  (const Loan& l1 , const Loan& l2)  { return l1.get_t_to_maturity() < l2.get_t_to_maturity(); }

		bool Commercial_bank::WillItDefault() {return def.uniform(0,1)<Pdefault;}

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

		void Commercial_bank::SetInitialValue(State state, Action action, double v){
			ValueFunction[state][v]=action;
			ValueFunction2[state][action]=v;
		}

		// Function to set continuation value (w)
		void Commercial_bank::UpdateW(State state, Action action, double CurrentV){

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

			if(p == this)
			throw invalid_argument("Adding itself to partners set");

		set<Commercial_bank*>::iterator it;
		//it = find (PartnersVector.begin(), PartnersVector.end(), p);
		it = PartnersVector.find(p);
		if (it == PartnersVector.end())	{
		PartnersVector.insert(p);
		PartnersVectorDR[ p->GetIBLoanRate() ]=p;
		PartnersVectorLR[ p->GetIBDepositRate() ]=p;
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
		if( ( (CSH+qila)/(qd+qida+qcblf) < DowngradeRatio1 ) | ( equity/ (CSH + ql + qila) < DowngradeRatio2 ) )
		ActualAgencyRatings[this] = 1;
		else ActualAgencyRatings[this] = 0;

		};

		void Commercial_bank::SetLqBnch(){
			LqBnch = beta2*(qd+qida+qcblf); //This is rounded to int precision
		}

		bool Commercial_bank::CheckCashReserveRequirements(){return CSH > (qd+qida+qcblf);}

		//Update partners set depending on whether the last action involves a deposit or loan, a = 1 for deposit, a = 2, for loan
		void Commercial_bank::UpdatePartnersSet(int a){
			//set<Commercial_bank*>::iterator iter = PartnersVector.begin();
			Commercial_bank* aux ;
			int prevsize;

			//The same bank could be added again with this setting
			if( a == 1 ){
				PartnersVectorLR.erase( PartnersVectorLRderef[ (PartnersVectorDR.begin()->second) ] );
				PartnersVector.erase ( (PartnersVectorDR.begin())->second );
				PartnersVectorDR.erase(PartnersVectorDR.begin()->first);
				//while( find(PartnersVector.begin(), PartnersVector.end(), aux) != PartnersVector.end() ){ int n = rand()%NBANKS; aux = BNetwork::getBank(n); }
				while( true ){ aux = BNetwork::getBank(rand()%NBANKS); prevsize = PartnersVector.size(); if(aux!= this) PartnersVector.insert( aux ); if(PartnersVector.size()!=prevsize) break;}
				//PartnersVector.erase ( find(PartnersVector.begin(), PartnersVector.end(), (PartnersVectorDR.begin())->second) );
				AddPartner(aux);

			}

			else {
				PartnersVectorDR.erase( (PartnersVectorLR.rbegin()->second)->GetIBDepositRate() );
				PartnersVector.erase ( (PartnersVectorLR.rbegin())->second );
				PartnersVectorLR.erase(PartnersVectorLR.rbegin()->first);

				//while( find(PartnersVector.begin(), PartnersVector.end(), aux) != PartnersVector.end() ){ int n = rand()%NBANKS; aux = BNetwork::getBank(n); }
				while( true ){ aux = BNetwork::getBank(rand()%NBANKS); prevsize = PartnersVector.size(); if(aux!= this) PartnersVector.insert( aux ); if(PartnersVector.size()!=prevsize) break;}

								//PartnersVector.erase ( find(PartnersVector.begin(), PartnersVector.end(), (PartnersVectorLR.rbegin())->second) );

				AddPartner(aux);
			}
			}


		void Commercial_bank::UpdatePartnerInfo(){
			set<Commercial_bank*>::iterator iter = PartnersVector.begin();
				for (;iter != PartnersVector.end(); iter++) AddPartner(*iter); //Update info about partner
		}

		string Commercial_bank::printPartners(){
			string aux;
			set<Commercial_bank*>::iterator iter;

			ostringstream temp;


			for(iter=PartnersVector.begin(); iter!=PartnersVector.end();++iter) {
				temp << ( BNetwork::getBank(*iter) )  << ";";
			}
			return aux=temp.str();
		}


//		int Commercial_bank::GetCDepositRate(){return DepositRates[0];}
//
//		int Commercial_bank::GetCLoanRate(){return DepositRates[0];};
//

		void Commercial_bank::updateState(int type, Loan* loan){

			if(UMLoans.empty())IBDepositRate = RateDepositCB;
			else	IBDepositRate = UMLoans.begin()->first;

			if(UMDeposits.empty()) IBLoanRate = RateLoanCB;
			else IBLoanRate = UMDeposits.rbegin()->first;

			LastState.loanOrDeposit = type;
		//	2) rate of the loan or deposit (this will give me if it is a costmer loan-depost or interbank loan deposit)
			LastState.interestRate = loan->getInterestRate();
		//					3)Rating of the bank sending the loan-deposit from the interbank market (i.e. 0 in the case of a Customer loan-deposit)
			// This is the lagged rating, right?
			LastState.ratingSender = LaggedAgencyRatings[ loan->GetSenderAgent() ];
		//					4)  your rating
			LastState.myRating = ActualAgencyRatings[ this ];
		//					5) partners ratings (with a lag)
			LastState.partnersRatings.clear();

			set<Commercial_bank*>::iterator iter;
			for (iter = PartnersVector.begin(); iter != PartnersVector.end(); iter++)
			LastState.partnersRatings.push_back( LaggedAgencyRatings [ *iter ] );

			//					6) your bid
			LastState.bid = IBDepositRate;
//			//					7) your ask
			LastState.ask = IBLoanRate;
//			int ask;
//		//					8) partners bid
//			std::vector<int> partnersBid;
//		//					9) partner ask
//			std::vector<int> partnersAsk;
//		//					10) your liquidity benchmark (only above or below).
			LastState.LqBench = CSH >= LqBnch;
		//					11) depth backlog at the ask if you receive a deposit (depth backlog at the bid if you receive a loan).
		//					12) TOTAL depth backlog on the upper side if it is a deposit (TOTAL depth backlog on the lower side if it is a loan).

			if( (type == 1) | (type == 11) ) {
				if(!UMDeposits.empty()) LastState.deepBackLog = (UMDeposits.rbegin()->second).size();
				else LastState.deepBackLog = 0;

				LastState.totaldeepBackLog=0;
				std::map< int , std::deque<Loan*> > ::iterator iter = UMDeposits.begin();
				if(!UMDeposits.empty())
				for (;iter != UMDeposits.end(); iter++) LastState.totaldeepBackLog+=(iter->second).size();
				else LastState.totaldeepBackLog = 0;



			}

			else {
				LastState.deepBackLog = (UMLoans.begin()->second).size();

				LastState.totaldeepBackLog=0;
				std::map< int , std::deque<Loan*> > ::iterator iter = UMLoans.begin();
				for (;iter != UMLoans.end(); iter++) LastState.totaldeepBackLog+=(iter->second).size();
			}



		//

		}


		//void Commercial_bank::CancelLoan()
		//void Commercial_bank::ResolveCancelledLoan()
