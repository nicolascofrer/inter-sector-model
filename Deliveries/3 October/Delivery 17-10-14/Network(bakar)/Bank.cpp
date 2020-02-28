#include "Bank.h"
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
#include <cstdlib>

using namespace std;




		//Static variables






		int Bank::bankCounter = 0;

		int Bank::getBankCounter(){return bankCounter;}

		map< State , map< Action, int > > Bank::h;
		map< Action ,int > Bank::h_data;


		map< State , map< double, Action > > Bank::ValueFunction;
		map< double, Action > Bank::ValueFunction_data;




		bool Bank::UpdatingProcess = true;
        map< Agent*, int > Bank::ActualAgencyRatings;

        map< Agent*, int > Bank::LaggedAgencyRatings;


///////////////////////////////////////////////////////////////////////////////////////////////////////////
		Bank::Bank(double ProbabilityDefault) :adevs::Atomic<Loan>(),tleft(DBL_MAX)
				//IBDepositRate(2),IBLoanRate(2)

		{
			log.open("log.txt");
			ID = bankCounter;
			//cout << "Constructor Bank " << ID << endl;
			bankCounter++;
			//SetEquityRange(0,50);
			partners=NNeighborhood; //So far, only 3 partners are allowed, but clearly this could be N(i)
			SetAR(1);
            equity= 10; ;
			CSH = 10;
			defaults=0;
			Pdefault = ProbabilityDefault;
			//profits=0;
			ReplacesBank = 9999;
			IBDepositRate = CBDFR+rand()%20-rand()%20 ;
			IBLoanRate = CBLFR+rand()%20-rand()%20 ;

			sendingIBDepositRequest = false;
			sendingIBLoanRequest = false;

			//actionsSet.insert(* new Action(0, true, true));
			//actionsSet.insert(* new Action(0, true, false));
			//actionsSet.insert(* new Action(0, false, true));
			//actionsSet.insert(* new Action(0, false, false));
			//actionsSet.insert(* new Action(1, true, true));
			//actionsSet.insert(* new Action(1, true, false));
			//actionsSet.insert(* new Action(1, false, true));
			//actionsSet.insert(* new Action(1, false, false));

			qd=0;
			qida=0;
			qcblf=0;
			qilr=0;
			ql=0;
			qila=0;
			qidr=0;

			//Initial Rating given by a Rating Agency (an integer value 0:Good, 1:Bad),							0
			ActualAgencyRatings[this] = 0;
            map<State,vector<Action>> stradep;
            map<State,vector<Action>> straloan ;
		};

		Bank::~Bank(){
			log.close();
		}

		void Bank::delta_int()
		{

			if(sendingIBDepositRequest){
				log << "Internal Function Sending IB Deposit Request BANK " << ID <<  endl;
				sendingIBDepositRequest = false;
				qidrQueue.pop_back();

				if (qt.empty()) tleft = DBL_MAX; // No event left

							else{//look for next object to expire
								sort(qt.begin(),qt.end(),NextExpiring);
								tleft = qt.front().get_t_to_maturity();
							}


			}

			else if(sendingIBLoanRequest){
							log << "Internal Function Sending IB Loan Request BANK " << ID <<  endl;
							sendingIBLoanRequest = false;
							qilrQueue.pop_back();

							if (qt.empty()) tleft = DBL_MAX; // No event left

										else{//look for next object to expire
											sort(qt.begin(),qt.end(),NextExpiring);
											tleft = qt.front().get_t_to_maturity();
										}

			}

			else{

				log << "Internal Function expired object " << ID << endl;
                
                Loan *l ;
                l=qt.front().GetMatchedWithLoan() ;
            
                switch (qt.front().getType()) {
                    case 1:{
                    
                        if (l!=NULL){
                 
                 
                        
                            if (l->getType()==2){
                            actualizeCash ((l->getInterestRate()-qt.front().getInterestRate())*(qt.front().getDuration()));
                            if(UpdatingProcess){qt.front().updateAction((l->getInterestRate()-qt.front().getInterestRate())*(qt.front().getDuration()));}
                            }
                    
                            if (l->getType()==22 ) {
                            actualizeCash((l->getInterestRate()-qt.front().getInterestRate())*(qt.front().getDuration())-Clink);
                            if(UpdatingProcess) {qt.front().updateAction((l->getInterestRate()-qt.front().getInterestRate())*(qt.front().getDuration())-Clink);}
                            }
                    
                        }
                        else{ actualizeCash(CBDFR-qt.front().getInterestRate());
                        if(UpdatingProcess){ qt.front().updateAction(CBDFR-qt.front().getInterestRate());}
                
                            }
                        log << "SIZE QT " << qt.size() << endl;
                        //Now change state, it doesn't matter because we are not making any decision in this part
                        if(qt.size() == 0) throw invalid_argument("Empty QT");
                       {qd--; log << "Expired deposit" << endl;}
                        qt.pop_front();
                        
                        //We need to update the time elapsed for all objects
                        std::deque<Loan>::iterator iter = qt.begin();
                        for (; iter != qt.end(); iter++){(*iter).ReduceTimeToMaturity(tleft);} //Time to maturity is reduced by the time elapsed
                        
                        
                        
                        if (qt.empty()) tleft = DBL_MAX; // No event left
                        
                        else{//look for next object to expire
                            sort(qt.begin(),qt.end(),NextExpiring);
                            tleft = qt.front().get_t_to_maturity();
                        }
                        
                
                        break ;
                }
            
                    case 2 :
                    {
                  // if( WillItDefault() ) { actualize_equity( ( (qt.front()).getInterestRate())*(qt.front().getDuration()) ); ////break ;}
                
                    if (l!=NULL) {
                   
                       if (l->getType()==1){
                           actualizeCash ((qt.front().getInterestRate()-l->getInterestRate())*(qt.front().getDuration()));
                           if(UpdatingProcess){qt.front().updateAction((qt.front().getInterestRate()-l->getInterestRate())*(qt.front().getDuration()));}
                   
                   
                       }
                   
                   
                       if (l->getType()==11){
                           actualizeCash ((qt.front().getInterestRate()-l->getInterestRate())*(qt.front().getDuration())-Clink);
                           if(UpdatingProcess){qt.front().updateAction((qt.front().getInterestRate()-l->getInterestRate())*(qt.front().getDuration())-Clink);}
                       } else {
                            actualizeCash((CBDFR-qt.front().getInterestRate())*(qt.front().getDuration()));
                           if(UpdatingProcess){ qt.front().updateAction((CBDFR-qt.front().getInterestRate())*(qt.front().getDuration()));}
                           
                       }
                   }
                        log << "SIZE QT " << qt.size() << endl;
                        //Now change state, it doesn't matter because we are not making any decision in this part
                        if(qt.size() == 0) throw invalid_argument("Empty QT");
                        else if(qt.front().getType()==2) {ql--; log << "Expired Loan" << endl;}
                        qt.pop_front();
                        //We need to update the time elapsed for all objects
                        std::deque<Loan>::iterator iter2 = qt.begin();
                        for (; iter2 != qt.end(); iter2++){(*iter2).ReduceTimeToMaturity(tleft);} //Time to maturity is reduced by the time elapsed
                        
                        
                        
                        
                        
                        if (qt.empty()) tleft = DBL_MAX; // No event left
                        
                        else{//look for next object to expire
                            sort(qt.begin(),qt.end(),NextExpiring);
                            tleft = qt.front().get_t_to_maturity();
                        }
                        
                    
                
                        break ;
                
                    }
                        
                        case 11 :
                    {
                    
                        if (l!=NULL){
                            
                            
                            
                            if (l->getType()==2){
                                actualizeCash ((l->getInterestRate()-qt.front().getInterestRate())*(qt.front().getDuration()));
                                if(UpdatingProcess){qt.front().updateAction((l->getInterestRate()-qt.front().getInterestRate())*(qt.front().getDuration()));}
                            }
                            
                            if (l->getType()==22 ) {
                                actualizeCash((l->getInterestRate()-qt.front().getInterestRate())*(qt.front().getDuration())-Clink);
                                if(UpdatingProcess) {qt.front().updateAction((l->getInterestRate()-qt.front().getInterestRate())*(qt.front().getDuration())-Clink);}
                            }
                            
                        }
                        
                        break ;
                    }
                      
                    case 22: {
                        if (l!=NULL) {
                            
                            if (l->getType()==1){
                                actualizeCash ((qt.front().getInterestRate()-l->getInterestRate())*(qt.front().getDuration()));
                                if(UpdatingProcess){qt.front().updateAction((qt.front().getInterestRate()-l->getInterestRate())*(qt.front().getDuration()));}
                                
                                
                            }
                            
                            
                            if (l->getType()==11){
                                actualizeCash ((qt.front().getInterestRate()-l->getInterestRate())*(qt.front().getDuration())-Clink);
                                if(UpdatingProcess){qt.front().updateAction((qt.front().getInterestRate()-l->getInterestRate())*(qt.front().getDuration())-(2*Clink));}
                            } else {
                                actualizeCash((CBDFR-qt.front().getInterestRate())*(qt.front().getDuration()));
                                if(UpdatingProcess){ qt.front().updateAction((CBDFR-qt.front().getInterestRate())*(qt.front().getDuration()));}
                                
                            }
                        }
                        break ;
                    
                    }
                    
             
                        
                        
//				if((qt.front()).getType() == 11 & (qt.front()).get_borrower()==this ) throw invalid_argument("self IB deposit request when not sending request");
            
			//if( ((qt.front()).getType()==1) | ((qt.front()).getType()==11))
            
          //  {reduce_equity( (qt.front()).GetQuantity()); //what is the amount lent?
                //if(UpdatingProcess)UpdateW(LastState, LastAction, - ( qt.front()).GetQuantity() ); //Update given the realized state is a deposit expiration
			//}

			//In case of loan////////////////////////////////////////////////////////////////////////
			//If there is default, reduce equity. If all is ok, get the profits from a successful loan
			//if((qt.front()).getType()==2){ if( WillItDefault() ) {reduce_equity( ( qt.front()).GetQuantity() ); //what is the amount lent?
				//												if(UpdatingProcess)UpdateW(LastState, LastAction, - (qt.front()).GetQuantity());
			//}//Update given realized state is a loan default
			//else {IncreaseEquity( (qt.front()).GetQuantity() );
			//if(UpdatingProcess) UpdateW(LastState, LastAction, ( qt.front()).GetQuantity() );//Update given realized state is a successful loan
			//}
			//}




		//if((qt.front()).getType() == 11) throw invalid_argument("incoming IB deposit request begin internal function");
		//if((qt.front()).getType() == 11 & (qt.front()).GetLinkedAgent()==this & (qt.front()).get_borrower()==this ) throw invalid_argument("self IB deposit request");
		//if((qt.front()).getType() == 11 & (qt.front()).GetLinkedAgent()==this ) throw invalid_argument("self IB deposit request");


		/*	log << "SIZE QT " << qt.size() << endl;
			//Now change state, it doesn't matter because we are not making any decision in this part
			if(qt.size() == 0) throw invalid_argument("Empty QT");

			if(qt.front().getType()==1) {qd--; log << "Expired deposit" << endl;}
			else if(qt.front().getType()==11) {qida--; log << "Expired IB Deposit" << endl;}
			else if(qt.front().getType()==2) {ql--; log << "Expired Loan" << endl;}
			else {qila--; log << "Expired IB Loan" << endl;}

			//Loan * temp = & qt.front();
			qt.pop_front(); // Remove the expired object
			//I need to delete the pointer to the object


			//We need to update the time elapsed for all objects
			std::deque<Loan>::iterator iter = qt.begin();
				for (; iter != qt.end(); iter++){(*iter).ReduceTimeToMaturity(tleft);} //Time to maturity is reduced by the time elapsed



			if (qt.empty()) tleft = DBL_MAX; // No event left

			else{//look for next object to expire
				sort(qt.begin(),qt.end(),NextExpiring);
				tleft = qt.front().get_t_to_maturity();
			}
*/
			}


		}
        }



		void Bank::delta_ext(double e, const adevs::Bag<Loan>& xb)
            {


			UpdatePartnerInfo();
			if(PartnersVector.find(this)!= PartnersVector.end()) cout << "Bank in own partners set" << endl;
			if(PartnersVector.size()>3) cout << "Partners set with more than 3 partners" << endl;

			// Update the remaining time to maturity
			if (!qt.empty()){ tleft -= e;
			//We need to update the time elapsed for all objects
			std::deque<Loan>::iterator iter4 = qt.begin();
			for (; iter4 != qt.end(); iter4++){(*iter4).ReduceTimeToMaturity(e);} //Time to maturity is reduced by the time elapsed
			}
            
			// Put new events into the queue, here we can to separate loan objects according to the type in the loan object
			adevs::Bag<Loan>::const_iterator iter = xb.begin();
			for (; iter != xb.end(); iter++)//We have to look at each new object coming
			{
                
				log << "START of BAG in BANK " << ID << endl;
				UpdatePartnerInfo();

				/////////////////////////////////////////////////////////////////////////////CUSTOMER DEPOSIT OR IB DEPOSIT////////////////////////////////////////////////////////////////////
				if( ( (*iter).getType() == 1 ) | ( (*iter).getType() == 11) ) {

					log << "external function incoming customer deposit or IB deposit" << endl;
                    

					updateState( (*iter).getType(), &(*iter) );
                    
                    CSH++ ;
                    
                    map <State , vector<Action> > ::const_iterator search=stradep.cbegin() ;
                    search=stradep.find(LastState);
                    if (search==stradep.cend()) {
                        
                        vector<Action> aux ;
                        double x=LastState.interestRate ;
                        while (x<CBLFR) {
                     aux.push_back(Action( x,((x-LastState.interestRate)/DepositorMaturityParameter),0)); // expected maturity
                                          x=x+((CBLFR-CBDFR)/(2*P));
                        }
                        sort(aux.begin(),aux.end(),bestPayoff );


                        stradep[LastState]=aux ;
                         unsigned long n=aux.size() ;
                        //unsigned long   n=((search->second).size());
                        n=(int) ( random() % n+1 );
                        
                        
                        vector <Action> :: iterator  itera=(stradep[LastState]).begin();
                        
                        std::advance(itera, n); // attention ,l'elt peut ne pas exister !!
                        (*iter).setAction((*itera));
                        (*iter).SetdesiredRate((*itera).desiredRate);
                        
                    }
                    
                    else
                    {
                        vector <Action> :: iterator  itera=(stradep[LastState]).begin();
                        if(a.uniform(0, 1)<trembling  ){
                    
                       // (*iter).SetdesiredRate(search->(second[(int)(rand()%((search->second).size())].desiredRate));
                       int  n=(int)((search->second).size());
                            n=(int) ( rand() % n+1);
                            
                            
                            
                            std::advance(itera, n); // attention ,l'elt peut ne pas exister !!
                             (*iter).setAction( (*itera));
                            (*iter).SetdesiredRate((*itera).desiredRate);
                        }
                    else
                         (*iter).setAction((*itera));
                         (*iter).SetdesiredRate((*itera).desiredRate);
                
                        }
                            
                            
                   
                               
                            
                            
                            
                   
                

					//std::set<Action>::const_iterator iter2 = actionsSet.begin();
					log << "Setting Initial belief for deposit" << endl;
					//for (;iter2 != actionsSet.end(); iter2++){
					//	if(h[LastState].find(*iter2) == h[LastState].end())	SetInitialValue(LastState, *iter2 , a.uniform(-1,1) );
					//}

					//The bank always accept any tipe of deposit
				//	if( (ValueFunction[LastState]).rbegin() -> second .accept | ((*iter).getType() == 1) ){//If opt decision is to accept  the incoming IB deposit or customer deposit


						if((*iter).getType() == 1){

							log << "INCOMING CUSTOMER DEPOSIT" << endl;
							std::map<double  , std::deque<Loan*> >::reverse_iterator iter3 = UMLoans.rbegin() ;

							//log << UMLoans.size() << endl;

							for (int n=0; (iter3 != UMLoans.rend()) | (n==0) ; iter3++, n++){
								//log << "IN FOR" << endl;

									//If desired matching rate is lower than the highest rate in unmatched loans
								if(iter3 != UMLoans.rend()){if((*iter).getdesiredRate() <= (*iter3).first  && ( (*iter3).second.size()!=0 )   ) {

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
											qt.push_front((*iter));
										}

										else{
											//qd.push_back(*iter);
											qd++;
											qt.push_back((*iter));
											}
										UpdatePartnersSet(1);

										log << "MATCHED WITH BACKLOG" << endl;
										break;


										}
										//If rate are the same in the own backlog and in the interbank market, flip a coin
										else if( PartnersVectorDR.rbegin()->first == (*iter3).first ){
											//match with own backlog
											log << "MATCHED RANDOMLY WITH BACKLOG OR IB DEPOSIT" << endl;

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
													qt.push_front((*iter));
												}

												else{
													//qd.push_back(*iter);
													qd++;
													qt.push_back((*iter));
												}


											}
											// match with interbank deposit, send request
											else{
												log << "MATCHED RANDOMLY WITH BACKLOG OR IB DEPOSIT" << endl;

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
											log << "MATCHED WITH IB DEPOSIT, BACKLOG RATE OVER DESIRED" << endl;
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
										if( (*iter).getdesiredRate() <= PartnersVectorDR.rbegin()->first ){
											log << "MATCHED WITH IB DEPOSIT, BACKLOG RATE BELOW DESIRED" << endl;
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
											log << "NO MATCH, ADDING TO BACKLOG" << endl;
											UMDeposits[ (*iter).getdesiredRate() ].push_back( &(*iter) );
											if ((*iter).get_t_to_maturity() < tleft) {
												tleft = (*iter).get_t_to_maturity();
											//	qd.push_front(*iter);
												qd++;
												qt.push_front((*iter));
											}
											else{
												//qd.push_back(*iter);
												qd++;
												qt.push_back((*iter));
											}
											UpdatePartnersSet(1);
											break;
										}
									} // END Desired matching rate is higher than in the own backlog

							}//END for that iterates in the backlog

						}// End of customer deposit case

						//START, if bank is accepting an incoming interbank deposit
						//else if((*iter).getType() == 11){
						else{
							log<< "INCOMING IB DEPOSIT FROM " << (*iter).get_IDborrower() << " TO " << (*iter).GetLinkedAgentID() << endl;
							if ((*iter).get_t_to_maturity() < tleft) {
								tleft = (*iter).get_t_to_maturity();
								//qida.push_front(*iter);
								//log << "ADDING IB DEPOSIT TO ACCEPTED IB DEPOSITS" << endl;
								qida++;
								qt.push_front(*new Loan(*iter));
								//qtest.push_back(*iter);
								log << 1 << endl;

							}
							else{
								//qida.push_back(*iter);
								//log << "ADDING IB DEPOSIT TO ACCEPTED IB DEPOSITS" << endl;
								qida++;
								qt.push_back(*iter);
								//qtest.push_back(*iter);
								log << 2 << endl;

							}
							UpdatePartnersSet(1);
							break;


				}// END If opt decision is to accept the incoming IB deposit or customer deposit
				}// END Customer deposit or IB deposit case


///////////////////////////////////////////////////////////////////////////CUSTOMER LOAN OR IB LOAN/////////////////////////////////////////////////////////////////////////////////////////

							//if( ( (*iter).getType() == 2 ) | ( (*iter).getType() == 21) ) {
				else{
								log << "external function incoming customer loan or IB loan" << endl;

								updateState( (*iter).getType(), &(*iter) );
                    CSH -- ;
                    
                    map <State , vector<Action> > ::const_iterator search=straloan.cbegin() ;
                    search=straloan.find(LastState);
                    if (search==straloan.cend()) {
                        
                        vector<Action> aux ;
                        double x=LastState.interestRate ;
                        while (x>CBDFR) {
                            aux.push_back(Action( x,((LastState.interestRate-x)/DepositorMaturityParameter),0));
                            x=x-((CBLFR-CBDFR)/(2*P));
                        }
                        
                        sort(aux.begin(),aux.end(),bestPayoff );
                        straloan[LastState]=aux ;
                        
                         vector <Action> :: iterator  itera=(straloan[LastState]).begin();
                        unsigned long  n=((straloan[LastState]).size());
                        n=(random() % (n+1)) ;
                        
                        std::advance(itera, n);
                        (*iter).SetdesiredRate(((*itera).desiredRate));
                        (*iter).setAction((*itera));
                    }
                    
                    else
                    {
                        vector <Action> :: iterator  itera=(straloan[LastState]).begin();
                        if(a.uniform(0, 1)<trembling  ){
                            
                         
                            int  n=(int)((search->second).size());
                            n=(int) ( rand() % n+1 );
                            
                            
                            
                            std::advance(itera, n);
                            (*iter).SetdesiredRate((*itera).desiredRate);
                        }
                        else
                            (*iter).SetdesiredRate((*itera).desiredRate);
                        
                    }


								//std::set<Action>::const_iterator iter2 = actionsSet.begin();
								log << "Setting Initial belief for loan" << endl;
								//for (;iter2 != actionsSet.end(); iter2++){
								//	if(h[LastState].find(*iter2) == h[LastState].end())	SetInitialValue(LastState, *iter2 , a.uniform(-1,1) );
								//}

							//	if( (ValueFunction[LastState]).rbegin() -> second .accept & CheckCashReserveRequirements() ){//If opt decision is to accept the incoming IB loan or customer loan
								if( true ){//If opt decision is to accept the incoming IB loan or customer loan

									if((*iter).getType() == 2){ //If customer loan request

										log << "INCOMING CUSTOMER LOAN" << endl;
										std::map<double , std::deque<Loan*> >::iterator iter3 = UMDeposits.begin();

										//log << UMLoans.size() << endl;

										for (int n=0; (iter3 != UMDeposits.end()) | (n==0); iter3++, n++){
											//log << "IN FOR" << endl;

												//If desired matching rate is higher than the lowest rate in unmatched loans
											if(iter3 != UMDeposits.end()){if( ( (*iter).getdesiredRate() >= (*iter3).first ) && ( (*iter3).second.size()!=0 )   ) {

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
														qt.push_front((*iter));
													}

													else{
														ql++;
														qt.push_back((*iter));
														}
													UpdatePartnersSet(2);
													break;

													//log << "MATCHED WITH BACKLOG" << endl;

													}
													//If rate are the same in the own backlog and in the interbank market, flip a coin
													else if( PartnersVectorLR.begin()->first == (*iter3).first ){
														//match with own backlog
														log << "MATCHED RANDOMLY WITH BACKLOG OR IB LOAN" << endl;

														if( rand()%2 == 0 ){

															//Set as matched with the unmatched deposit

															//log << (*iter3).second.front()->getID() << " " << (*iter).getID() << endl;
															log << " " << (*iter).getID() << endl;
															if( (*iter3).second.front() == NULL ) throw invalid_argument("NULL POINTER");
															(*iter3).second.front()->SetMatchedWith( &(*iter) );
															(*iter).SetMatchedWith( (*iter3).second.front() );
															//Delete from unmatched deposit queue
															(*iter3).second.pop_front();

															if ((*iter).get_t_to_maturity() < tleft) {
																tleft = (*iter).get_t_to_maturity();
																ql++;
																qt.push_front((*iter));
															}

															else{
																ql++;
																qt.push_back((*iter));
															}


														}
														// match with interbank deposit, send request
														else{
															log << "MATCHED RANDOMLY WITH BACKLOG OR IB DEPOSIT" << endl;

															(*iter).SetLinkedAgent( PartnersVectorLR.begin()->second  ); //This is the partner that should receive the request
															(*iter).setType(21);
															sendingIBLoanRequest = true;
															tleft=0; //Immediately send the request to interbank partner
															qilr++;
															ql++;
															qilrQueue.push_back(*iter);

														}
														UpdatePartnersSet(2);
														break;
													}
													// If best rate is in interbank market
													else if( PartnersVectorLR.begin()->first < (*iter3).first ){
														log << "MATCHED WITH IB LOAN, BACKLOG RATE BELOW DESIRED" << endl;
														(*iter).SetLinkedAgent( PartnersVectorDR.rbegin()->second  ); //This is the partner that should receive the request
														(*iter).setType(21);
														sendingIBLoanRequest = true;
														tleft=0; //Immediately send the request to interbank partner
														qilr++;
														ql++;
														qilrQueue.push_back(*iter);
														UpdatePartnersSet(2);
														break;
													}

									}}//End of case when both own backlog rate and interbank rate are lower than desired matching rate


												//Desired matching rate is lower than in the own backlog
												else{
													// The bank have to look in the interbank market
													// If rate in the interbank is lower than desired matching rate, there is a match
													if( (*iter).getdesiredRate() >= PartnersVectorLR.rbegin()->first ){
														log << "MATCHED WITH IB LOAN, BACKLOG RATE OVER DESIRED" << endl;
														(*iter).SetLinkedAgent( PartnersVectorLR.begin()->second  ); //This is the partner that should receive the request
														(*iter).setType(21);
														sendingIBLoanRequest = true;
														tleft=0; //Immediately send the request to interbank partner
														qilr++;
														ql++;
														qilrQueue.push_back(*iter);
														qt.push_back(*iter);
														UpdatePartnersSet(2);

														break;
													}
													//There is not match in the interbank market
													else {
														//Add this new deposit to the backlog of unmatched loans
														log << "NO MATCH, ADDING TO BACKLOG" << endl;
														UMLoans[ (*iter).getdesiredRate() ].push_back( &(*iter) );
														if ((*iter).get_t_to_maturity() < tleft) {
															tleft = (*iter).get_t_to_maturity();
															ql++;
															qt.push_front((*iter));
														}
														else{
															ql++;
															qt.push_back((*iter));
														}
														UpdatePartnersSet(2);
														break;
													}
												} // END Desired matching rate is higher than in the own backlog

										}//END for that iterates in the backlog
									}// End of customer loan case

									//START, if bank is accepting an incoming interbank loan
									//else if((*iter).getType() == 21)
									else{
										log<< "INCOMING IB LOAN FROM " << (*iter).get_IDlender() << " TO " << (*iter).GetLinkedAgentID() << endl;
										if ((*iter).get_t_to_maturity() <  tleft) {
											tleft = (*iter).get_t_to_maturity();
											qila++;
											qt.push_front(*iter);
											//qtest.push_back(*iter);
											log << "1 " << tleft << endl;

										}
										else{
											qila++;
											qt.push_back(*iter);
											//qtest.push_back(*iter);
											log << "2 " << tleft << endl;

										}
										UpdatePartnersSet(2);
										log << "Size qt " << qt.size() << endl;
										break;
									}
									log << "Size qt " << qt.size() << endl;


							}// END If opt decision is to accept the incoming IB deposit or customer deposit
							}// END Customer loan or IB loan case



			}//BAG for incoming events
//			log << "Size qt " << qt.size() << endl;
//			log << "Size qilr " << qilrQueue.size() << endl;
//			log << "Size qidr " << qidrQueue.size() << endl;
//			log << "Size qtest " << qtest.size() << endl;
//			log << "End delta ext " << "Size qt " << qt.size() << endl;
		}//END DELTA_EXT

		void Bank::delta_conf(const adevs::Bag<Loan>& xb)
		{
             delta_ext(0.0,xb);
			delta_int();
			
		}
		void Bank::output_func(adevs::Bag<Loan>& yb)
		{

			log << "output function BANK " << ID << endl;
//			log << "Size qt " << qt.size() << endl;
//			log << "Size qilr " << qilrQueue.size() << endl;
//			log << "Size qidr " << qidrQueue.size() << endl;


			if(sendingIBDepositRequest){yb.insert( qidrQueue.back()) ; log << "output function, sending IB Deposit Request" << endl;}

			else if(sendingIBLoanRequest){
				log << "sending IB loan Request, size qlirQueue " << qilrQueue.size() << endl;
//				if( qilrQueue.back() == NULL ) throw invalid_argument("NULL POINTER");
	//			log << qilrQueue.back() << endl;


				yb.insert( qilrQueue.back() );log << "output function, sending IB Loan Request" << endl;}

			//if( ( qt.front().getType()==1 ) | ( qt.front().getType()==2 ) | ( qt.front().getType()==11 ) )
			else
			{

				//if( qt.empty() ) throw invalid_argument("EMPTY QT");
				//if(qt.front().getType() ==1 | qt.front().getType() ==2)  {
					qt.front().SetAsExpired();
					yb.insert(qt.front());
				//}



				if(qt.front().getType()==1 ) log << "output function, expired deposit" << endl;
				if(qt.front().getType()==2 ) log << "output function, expired loan" << endl;
				if(qt.front().getType()==11 ) log << "output function, expired IB deposit" << endl;
				if(qt.front().getType()==21 ) log << "output function, expired IB loan" << endl;


			}

			//if(sendingIBDepositRequest){yb.insert(qidr.back());log << "output function, sending IB Deposit Request" << endl;}

			//if(sendingIBLoanRequest){yb.insert(qilr .back());log << "output function, sending IB Loan Request" << endl;}





		}
		double Bank::ta()
		{
			return tleft;
		}

		bool Bank::model_transition()
		{
		//transition activated if insolvent
			if(checkInsolvent()) cout << "Trasition activated " << ID << endl;
			return checkInsolvent();
		}
		// Get the number of deposits in the queue
		unsigned int Bank::getDepositQueueSize() { return qd; }

		unsigned int Bank::getLoanQueueSize() { return ql; }

		unsigned int Bank::getIBDepositRequestQueueSize() { return qidr; }

		unsigned int Bank::getIBDepositAcceptedQueueSize() { return qida; }

		//unsigned int Bank::getLoanQueueSize() { return ql.size(); }

		void Bank::gc_output(adevs::Bag<Loan>&){}

	//	double Bank::value(){return equity;}

		void Bank::reduce_equity(double shock){ equity-=shock;}

		bool Bank::checkInsolvent() {
			if(equity <=0) {
			cout << ID << " Is insolvent" << endl;
			return true;
			}
			else return false;
		}

        bool Bank::bestPayoff ( const Action& a1 , const Action& a2) { return a1.payoff > a2.payoff ;}

		bool Bank::NextExpiring  (const Loan& l1 , const Loan& l2)  { return l1.get_t_to_maturity() < l2.get_t_to_maturity(); }

		bool Bank::WillItDefault() {return def.uniform(0,1)<Pdefault;}

		void Bank::SetIBDepositRate(double  r){IBDepositRate = r;}

		void Bank::SetIBLoanRate(double r){IBLoanRate = r;}

		double  Bank::GetIBDepositRate(){return IBDepositRate;}

		double  Bank::GetIBLoanRate(){return IBLoanRate;}



//		void Bank::SetEquityRange(double a, double b)
//		{	vector<double> aux;
//
//		while(a <= b) {aux.push_back(a);
//		        a += 0.01;}
//
//			Equities = aux;
//		}
            void Bank::actualizeCash ( double e ){ CSH+=e ;}

		void Bank::SetEquity(double e){equity = e;}

		void Bank::IncreaseEquity(double  e){equity += e;}

		double  Bank::GetEquity(){return equity;}
		//{return Equities[equity];}
     double  Bank::getCash() { return CSH ;}
		/*void Bank::SetInitialValue(State state, Action action, double v){
			ValueFunction[state][v]=action;
			ValueFunction2[state][action]=v;
		}

		// Function to set continuation value (w)
		void Bank::UpdateW(State state, Action action, double CurrentV){

			ValueFunction2[state][action]= ValueFunction2[state][action] * h[state][action]/(h[state][action]+1) + CurrentV/(h[state][action]+1);

			ValueFunction[state][ ValueFunction2[state][action] ] = action;

		}
*/
		//int Bank::GetNumberStatesVisited(){return h.size();}

		//double Bank::LastStateActionValue(){return ValueFunction2[LastState][LastAction];}

		void Bank::SetNPartners(int n){partners=n;}
		//Return allowed number of partners
		int Bank::GetAllowedNPartners(){return partners;}

		//Add the partner as a value in the map, the rate is the key so the map is sorted in decreasing order, in the same way as ValueFunction
		void Bank::AddPartner(Bank* p){

			if(p == this)
			throw invalid_argument("Adding itself to partners set");

		set<Bank*>::iterator it;
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

		int Bank::GetNPartners(){return (int) PartnersVector.size();}

		int Bank::GetAR(){return LaggedAgencyRatings[this];}

		void Bank::SetAR(int a){LaggedAgencyRatings[this] = a;}

		void Bank::StopUpdatingProcess(){UpdatingProcess = false;}

		void Bank::StartUpdatingProcess(){UpdatingProcess = true;}

		void Bank::SendRating(){

		LaggedAgencyRatings[this] = ActualAgencyRatings[this];
		if( ( (CSH+qila)/(qd+qida+qcblf) < DowngradeRatio1 ) | ( equity/ (CSH + ql + qila) < DowngradeRatio2 ) )
		ActualAgencyRatings[this] = 1;
		else ActualAgencyRatings[this] = 0;

		};

		void Bank::SetLqBnch(){
			LqBnch = beta2*(qd+qida+qcblf); //This is rounded to int precision
		}

		bool Bank::CheckCashReserveRequirements(){return CSH > (qd+qida+qcblf);}

		//Update partners set depending on whether the last action involves a deposit or loan, a = 1 for deposit, a = 2, for loan
		void Bank::UpdatePartnersSet(int a){
			//set<Bank*>::iterator iter = PartnersVector.begin();
			Bank* aux ;
			int prevsize;

			//The same bank could be added again with this setting
			if( a == 1 ){
				PartnersVectorLR.erase( PartnersVectorLRderef[ (PartnersVectorDR.begin()->second) ] );
				PartnersVector.erase ( (PartnersVectorDR.begin())->second );
				PartnersVectorDR.erase(PartnersVectorDR.begin()->first);
				//while( find(PartnersVector.begin(), PartnersVector.end(), aux) != PartnersVector.end() ){ int n = rand()%NBANKS; aux = BNetwork::getBank(n); }
				while( true ){ aux = BNetwork::getBank( (int) rand()%NBANKS); prevsize = (int) PartnersVector.size(); if(aux!= this) PartnersVector.insert( aux ); if(PartnersVector.size()!=prevsize) break;}
				//PartnersVector.erase ( find(PartnersVector.begin(), PartnersVector.end(), (PartnersVectorDR.begin())->second) );
				AddPartner(aux);

			}

			else {
				PartnersVectorDR.erase( (PartnersVectorLR.rbegin()->second)->GetIBDepositRate() );
				PartnersVector.erase ( (PartnersVectorLR.rbegin())->second );
				PartnersVectorLR.erase(PartnersVectorLR.rbegin()->first);

				//while( find(PartnersVector.begin(), PartnersVector.end(), aux) != PartnersVector.end() ){ int n = rand()%NBANKS; aux = BNetwork::getBank(n); }
				while( true ){ aux = BNetwork::getBank( (int) rand()%NBANKS); prevsize =(int) PartnersVector.size(); if(aux!= this) PartnersVector.insert( aux ); if(PartnersVector.size()!=prevsize) break;}

								//PartnersVector.erase ( find(PartnersVector.begin(), PartnersVector.end(), (PartnersVectorLR.rbegin())->second) );

				AddPartner(aux);
			}
			}


		void Bank::UpdatePartnerInfo(){
			set<Bank*>::iterator iter = PartnersVector.begin();
				for (;iter != PartnersVector.end(); iter++) {
					if(BNetwork::IsBankrupt(*iter)) {cout << "REPLACING BANKRUPT BANK "<< (*iter)->getID() << " PARTNER OF BANK " << ID << endl;   ReplaceBankruptPartner(*iter);}
					else AddPartner(*iter); //Update info about partner
				}
		}

		string Bank::printPartners(){
			string aux;
			set<Bank*>::iterator iter;

			ostringstream temp;
			//cout << PartnersVector.size() << endl;
			//cout << ID << endl;

			for(iter=PartnersVector.begin(); iter!=PartnersVector.end();++iter) {
				//cout << "PRINTING PARTNERS OF " << ID << endl;
				if(BNetwork::IsBankrupt(*iter)) cout << "PRINTING BANKRUPT PARTNER, POINTER IS " << *iter <<  endl;
			//	cout << "Pointer " << *iter << endl;
			//	cout << "Object " << (*iter)->getID() << endl;

				temp <<  (*iter)->getID()   << ";";
			}
			return aux=temp.str();
		}


//		int Bank::GetCDepositRate(){return DepositRates[0];}
//
//		int Bank::GetCLoanRate(){return DepositRates[0];};
//

		void Bank::updateState(int type, Loan* loan){

			if(UMLoans.empty())IBDepositRate = CBDFR;
			else	IBDepositRate = UMLoans.begin()->first;

			if(UMDeposits.empty()) IBLoanRate = CBLFR;
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

			set<Bank*>::iterator iter;
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
				if(!UMDeposits.empty()) LastState.deepBackLog = (int)(UMDeposits.rbegin()->second).size();
				else LastState.deepBackLog = 0;

				LastState.totaldeepBackLog=0;
				std::map< double , std::deque<Loan*> > ::iterator iter = UMDeposits.begin();
				if(!UMDeposits.empty())
				for (;iter != UMDeposits.end(); iter++) LastState.totaldeepBackLog+=(iter->second).size();
				else LastState.totaldeepBackLog = 0;



			}

			else {
				LastState.deepBackLog =(int)(UMLoans.begin()->second).size();

				LastState.totaldeepBackLog=0;
				std::map< double , std::deque<Loan*> > ::iterator iter = UMLoans.begin();
				for (;iter != UMLoans.end(); iter++) LastState.totaldeepBackLog+=(iter->second).size();
			}



		}

		int Bank::getID(){return ID;}

		int Bank::getReplacesBank(){return ReplacesBank;}

		void Bank::setReplacesBank(int bank){ReplacesBank = bank;}

		bool Bank::operator ==  (Bank a) const {return ID == a.getID();}

		double Bank::GetProbDef(){return Pdefault;}

		int Bank::GetAssets(){return CSH + ql + qila;}

		void Bank::Liquidate(){tleft = DBL_MAX; PartnersVector.clear();}

		void Bank::ReplaceBankruptPartner(Bank* bank){


							Bank* aux ;
							int prevsize;
							//while( find(PartnersVector.begin(), PartnersVector.end(), aux) != PartnersVector.end() ){ int n = rand()%NBANKS; aux = BNetwork::getBank(n); }
							while( true ){ aux = BNetwork::getBank( (int) rand()%NBANKS ); prevsize =(int) PartnersVector.size(); if(aux!= this) PartnersVector.insert( aux ); if(PartnersVector.size()!=prevsize) break;}
							//PartnersVector.erase ( find(PartnersVector.begin(), PartnersVector.end(), (PartnersVectorDR.begin())->second) );
							PartnersVectorLR.erase( PartnersVectorLRderef[ bank ] );
							PartnersVectorDR.erase( PartnersVectorDRderef[ bank ] );
							PartnersVectorDRderef.erase( bank );
							PartnersVectorLRderef.erase( bank );
							PartnersVector.erase ( bank );
							AddPartner(aux);

		}

		//void Bank::CancelLoan()
		//void Bank::ResolveCancelledLoan()



