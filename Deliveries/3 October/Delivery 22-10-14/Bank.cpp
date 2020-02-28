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

using namespace std;

//Static variables

//A small class to merge these two maps h and ValueFunction, difference frequencies for banks
//		map<vector<int> , map<vector<int>,int > > Bank::h;
//		map<vector<int> ,int > Bank::h_data;
int Bank::bankCounter = 0;

int Bank::getBankCounter() {
	return bankCounter;
}

map<State, map<Action, int> > Bank::h;
map<Action, int> Bank::h_data;

//		map< Strategy , int > Bank::h;

//		map<vector<int> , map<double, vector<int> > > Bank::ValueFunction;
//		map< double,vector<int> > Bank::ValueFunction_data;

map<State, map<double, Action> > Bank::ValueFunction;
map<double, Action> Bank::ValueFunction_data;

//		map< Strategy, double > Bank::ValueFunction2;

//		map<vector<int>, map< vector<int>, double > > Bank::ValueFunction2;
//		map< vector<int>, double > Bank::ValueFunction2_data;

map<State, map<Action, double> > Bank::ValueFunction2;
map<Action, double> Bank::ValueFunction2_data;

//		map<vector<int>, map< vector<int>, double > > Bank::InitVal;
//		map< vector<int>, double > Bank::InitVal_data;

//		map< Strategy, double > Bank::InitVal;

map<State, map<Action, double> > Bank::InitVal;
map<Action, double> Bank::InitVal_data;

bool Bank::UpdatingProcess = true;

vector<double> Bank::Rates;

int Bank::RateDepositCB;
int Bank::RateLoanCB;
int Bank::CDepositRate;
int Bank::CLoanRate;

map<Agent*, int> Bank::ActualAgencyRatings;

map<Agent*, int> Bank::LaggedAgencyRatings;

void Bank::setRateRange(int p) {
	vector<double> aux;

	double d = 0.5 * (CBLFR - CBDFR) / p;
	double y0 = 0.5 * (CBLFR + CBDFR);

	for (int i = p; i > 0; i--)
		aux.push_back(y0 - i * d);

	aux.push_back(y0);

	for (int i = p; i > 0; i--)
		aux.push_back(y0 + i * d);

	Rates = aux;
}

void Bank::SetCBDepositRate(int r) {
	RateDepositCB = r;
}
void Bank::SetCBLoanRate(int r) {
	RateLoanCB = r;
}

void Bank::SetCDepositRate(int r) {
	CDepositRate = r;
}

void Bank::SetCLoanRate(int r) {
	CLoanRate = r;
}

int Bank::GetCDepositRate() {
	return CDepositRate;
}

int Bank::GetCLoanRate() {
	return CLoanRate;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////
Bank::Bank(double ProbabilityDefault) :
		adevs::Atomic<Loan>(), tleft(DBL_MAX)
//IBDepositRate(2),IBLoanRate(2)

{
	log.open("log.txt");
	ID = bankCounter;
	//log << "Constructor Bank " << ID << endl;
	bankCounter++;
	//SetEquityRange(0,50);
	partners = NNeighborhood; //So far, only 3 partners are allowed, but clearly this could be N(i)
	SetAR(0);
	equity = 2;
	CSH = 2;
	defaults = 0;
	Pdefault = ProbabilityDefault;

	ReplacesBank = 99999;
//	IBDepositRate = CBDFR + rand() % 20 - rand() % 20;
//	IBLoanRate = CBLFR + rand() % 20 - rand() % 20;

	IBDepositRate = RateDepositCB;
	IBLoanRate = RateLoanCB;

	sendingIBDepositRequest = false;
	sendingIBLoanRequest = false;

	initializeActionSet();

	std::set<Action*>::iterator iter = actionsSetPointers.begin();
	for (; iter != actionsSetPointers.end(); iter++)
		actionsSet.insert(**iter);

	qd = 0;
	qida = 0;
	qcblf = 0;
	qilr = 0;
	ql = 0;
	qila = 0;
	qidr = 0;

	//Initial Rating given by a Rating Agency (an integer value 0:Good, 1:Bad),							0
	ActualAgencyRatings[this] = 0;
}
;

Bank::~Bank() {
	std::vector<Loan*>::iterator iter = lbank.begin();
	for (; iter != lbank.end(); iter++)
		delete *iter;

	std::set<Action*>::iterator iter2 = actionsSetPointers.begin();
	for (; iter2 != actionsSetPointers.end(); iter2++)
		delete *iter2;

}

void Bank::delta_int() {

	log << "START DELTA INT in BANK " << ID << endl;

	if (sendingIBDepositRequest) {
		log << "Internal Function Sending IB Deposit Request BANK " << ID
				<< endl;
		sendingIBDepositRequest = false;
		qidrQueue.pop_back();

		if (qt.empty())
			tleft = DBL_MAX; // No event left

		else { //look for next object to expire
			sort(qt.begin(), qt.end(), NextExpiring);
			tleft = qt.front().get_t_to_maturity();
		}

	}

	else if (sendingIBLoanRequest) {
		log << "Internal Function Sending IB Loan Request BANK " << ID << endl;
		sendingIBLoanRequest = false;
		qilrQueue.pop_back();

		if (qt.empty())
			tleft = DBL_MAX; // No event left

		else { //look for next object to expire
			sort(qt.begin(), qt.end(), NextExpiring);
			tleft = qt.front().get_t_to_maturity();
		}

	}

	else {

		log << "Internal Function expired object " << ID << endl;

//				if((qt.front()).getType() == 11 & (qt.front()).get_borrower()==this ) throw invalid_argument("self IB deposit request when not sending request");

		//Last state and last action were defined when the last action was taken, this happened in the delta_ext() function
		//Here we have to update value of decisions, because in this function (delta_int) is where we can see realized values of past actions
		//In case of deposit/////////////////////////////////////////////////////////////////////
		if (((qt.front()).getType() == 1) | ((qt.front()).getType() == 11)) {
			reduce_equity((qt.front()).GetQuantity());
			if (UpdatingProcess)
				UpdateW(LastState, LastAction, -(qt.front()).GetQuantity()); //Update given the realized state is a deposit expiration
		}

		//In case of loan////////////////////////////////////////////////////////////////////////
		//If there is default, reduce equity. If all is ok, get the profits from a successful loan
		if ((qt.front()).getType() == 2) {
			if (WillItDefault()) {
				reduce_equity((qt.front()).GetQuantity());
				if (UpdatingProcess)
					UpdateW(LastState, LastAction, -(qt.front()).GetQuantity());
			}			//Update given realized state is a loan default
			else {
				IncreaseEquity((qt.front()).GetQuantity());
				if (UpdatingProcess)
					UpdateW(LastState, LastAction, (qt.front()).GetQuantity());	//Update given realized state is a successful loan
			}
		}

		//if((qt.front()).getType() == 11) throw invalid_argument("incoming IB deposit request begin internal function");
		//if((qt.front()).getType() == 11 & (qt.front()).GetLinkedAgent()==this & (qt.front()).get_borrower()==this ) throw invalid_argument("self IB deposit request");
		//if((qt.front()).getType() == 11 & (qt.front()).GetLinkedAgent()==this ) throw invalid_argument("self IB deposit request");

		log << "SIZE QT " << qt.size() << endl;
		//Now change state, it doesn't matter because we are not making any decision in this part
		if (qt.size() == 0)
			throw invalid_argument("Empty QT");

		if (qt.front().getType() == 1) {
			qd--;
			log << "Expired deposit" << endl;
		} else if (qt.front().getType() == 11) {
			qida--;
			log << "Expired IB Deposit" << endl;
		} else if (qt.front().getType() == 2) {
			ql--;
			log << "Expired Loan" << endl;
		} else {
			qila--;
			log << "Expired IB Loan" << endl;
		}

		// check if the matching counterpart object is the next to expire, this will leave an object without match, this should be added again in the backlog

		//Search in map of matching object the expiring loan object

		std::map<Loan*, Loan*>::iterator it1 = matchMap1.find(&qt.front());

		std::map<Loan*, Loan*>::iterator it2 = matchMap2.find(&qt.front());

		std::map<int, deque<Loan*> >::iterator itBacklogDeposits =
				UMDeposits.find(qt.front().getDesiredMatchingRate());

		std::map<int, deque<Loan*> >::iterator itBacklogLoans = UMLoans.find(
				qt.front().getDesiredMatchingRate());

		if (it1 != matchMap1.end()) {
			//Item that's matched with the object that is expired
			if (it1->second->getType() == 1 || it1->second->getType() == 11) {
				//The object that will be unmatched is a customer deposit or IB deposit
				UMDeposits[it1->second->getDesiredMatchingRate()].push_back(
						it1->second);
			} else
				//the object that will be unmatched is a customer loan or IB loan
				UMLoans[it1->second->getDesiredMatchingRate()].push_back(
						it1->second);
		}

		//The same but search in the other direction

		else if (it2 != matchMap2.end()) {
			//Item that's matched with the object that is expired
			if (it2->second->getType() == 1 || it2->second->getType() == 11) {
				//The object that will be unmatched is a customer deposit or IB deposit
				UMDeposits[it2->second->getDesiredMatchingRate()].push_back(
						it2->second);
			} else
				//the object that will be unmatched is a customer loan or IB loan
				UMLoans[it2->second->getDesiredMatchingRate()].push_back(
						it2->second);
		}
		// If the object was unmatched, search it in the backlogs and remove it
		// I wonder if it would be better to iterate in the keys of the map (desired rates) and sort each queue
		else if (itBacklogDeposits != UMDeposits.end()) {
			deque<Loan*>::iterator itQ;
			itQ = find(itBacklogDeposits->second.begin(),
					itBacklogDeposits->second.end(), &qt.front());
			if (itQ != itBacklogDeposits->second.end())
				itBacklogDeposits->second.erase(itQ);
		}

		else if (itBacklogLoans != UMLoans.end()) {
			deque<Loan*>::iterator itQ;
			itQ = find(itBacklogLoans->second.begin(),
					itBacklogLoans->second.end(), &qt.front());
			if (itQ != itBacklogLoans->second.end())
				itBacklogLoans->second.erase(itQ);
		}

		qt.pop_front(); // Remove the expired object
		//I need to delete the pointer to the object

		//We need to update the time elapsed for all objects
		std::deque<Loan>::iterator iter = qt.begin();
		for (; iter != qt.end(); iter++) {
			(*iter).ReduceTimeToMaturity(tleft);
		} //Time to maturity is reduced by the time elapsed

		if (qt.empty())
			tleft = DBL_MAX; // No event left

		else { //look for next object to expire
			sort(qt.begin(), qt.end(), NextExpiring);
			tleft = qt.front().get_t_to_maturity();
		}

	}

	log << "END DELTA INT in BANK " << ID << endl;
}

void Bank::delta_ext(double e, const adevs::Bag<Loan>& xb) {

	log << "START DELTA EXT in BANK " << ID << endl;

	//UpdatePartnerInfo();
	if (PartnersVector.find(this) != PartnersVector.end())
		log << "Bank in own partners set" << endl;
	if (PartnersVector.size() > 3)
		log << "Bank " << ID << " Partners set with more than 3 partners"
				<< endl;

	// Update the remaining time to maturity
	if (!qt.empty()) {
		tleft -= e;
		//We need to update the time elapsed for all objects
		std::deque<Loan>::iterator iter4 = qt.begin();
		for (; iter4 != qt.end(); iter4++) {
			(*iter4).ReduceTimeToMaturity(e);
		} //Time to maturity is reduced by the time elapsed
	}

	// Put new events into the queue, here we can to separate loan objects according to the type in the loan object
	adevs::Bag<Loan>::const_iterator iter = xb.begin();
	for (; iter != xb.end(); iter++) //We have to look at each new object coming
			{
		log << "START of BAG in BANK " << ID << endl;
		UpdatePartnerInfo();
		updateBacklogs();

		/////////////////////////////////////////////////////////////////////////////CUSTOMER DEPOSIT OR IB DEPOSIT////////////////////////////////////////////////////////////////////
		if (((*iter).getType() == 1) | ((*iter).getType() == 11)) {

			log << "external function incoming customer deposit or IB deposit"
					<< endl;

			updateState((*iter).getType(), &(*iter));

			std::set<Action>::const_iterator iter2 = actionsSet.begin();
			log << "Setting Initial belief for deposit" << endl;
			for (; iter2 != actionsSet.end(); iter2++) {
				if (h[LastState].find(*iter2) == h[LastState].end())
					SetInitialValue(LastState, *iter2, a.uniform(-1, 1));
			}

			//The bank always accept any tipe of deposit
			//	if( (ValueFunction[LastState]).rbegin() -> second .accept | ((*iter).getType() == 1) ){//If opt decision is to accept  the incoming IB deposit or customer deposit

			if ((*iter).getType() == 1) {

				log << "INCOMING CUSTOMER DEPOSIT" << endl;
				std::map<int, std::deque<Loan*> >::reverse_iterator iter3 =
						UMLoans.rbegin();

				//log << UMLoans.size() << endl;

				for (int n = 0; (iter3 != UMLoans.rend()) | (n == 0);
						iter3++, n++) {
					//log << "IN FOR" << endl;

					//If desired matching rate is lower than the highest rate in unmatched loans
					if (iter3 != UMLoans.rend()) {
						if (((ValueFunction[LastState]).rbegin()->second.desiredMatchingInterestRate
								<= (*iter3).first)
								& ((*iter3).second.size() != 0)) {

							//If Partners rate lower than in the own backlog
							if ((PartnersVectorDR.rbegin()->first)
									< (*iter3).first) {
								//Set matched loan with in the unmatched loan
								(*iter3).second.front()->SetMatchedWith(
										&(*iter));
								//Set matched with in the incoming deposit
								(*iter).SetMatchedWith((*iter3).second.front());

								if ((*iter).get_t_to_maturity() < tleft) {
									tleft = (*iter).get_t_to_maturity();
									//qd.push_front(*iter);
									qd++;
									CSH++;
									qt.push_front((*iter));
									//add the match to the maps
									matchMap1[&qt.front()] =
											(*iter3).second.front();
									matchMap2[(*iter3).second.front()] =
											&qt.front();
								}

								else {
									//qd.push_back(*iter);
									qd++;
									CSH++;
									qt.push_back((*iter));
									//add the match to the maps
									matchMap1[&qt.back()] =
											(*iter3).second.front();
									matchMap2[(*iter3).second.front()] =
											&qt.back();
								}

								//Delete from unmatched loan queue
								(*iter3).second.pop_front();
								UpdatePartnersSet(1);

								log << "MATCHED WITH BACKLOG" << endl;
								break;

							}
							//If rate are the same in the own backlog and in the interbank market, flip a coin
							else if (PartnersVectorDR.rbegin()->first
									== (*iter3).first) {
								//match with own backlog
								log
										<< "MATCHED RANDOMLY WITH BACKLOG OR IB DEPOSIT"
										<< endl;

								if (randomMatch.uniform(0, 1) < 0.5) {

									//Set matched deposit with in the unmatched loan
									(*iter3).second.front()->SetMatchedWith(
											&(*iter));
									//Set matched with in the incoming deposit
									(*iter).SetMatchedWith(
											(*iter3).second.front());

									// Update next event time, if the event is the next to expire
									if ((*iter).get_t_to_maturity() < tleft) {
										tleft = (*iter).get_t_to_maturity();
										//	qd.push_front(*iter);
										qd++;
										CSH++;
										qt.push_front((*iter));
										//add the match to the maps
										matchMap1[&qt.front()] =
												(*iter3).second.front();
										matchMap2[(*iter3).second.front()] =
												&qt.front();
									}
//												 If the event is not the next to expire, put it at the back of the queue,
//												 then in the internal function the queue will be sorted using the time left
//												 of each object

									else {

										qd++;
										CSH++;
										qt.push_back((*iter));

										//add the match to the maps
										matchMap1[&qt.back()] =
												(*iter3).second.front();
										matchMap2[(*iter3).second.front()] =
												&qt.back();
									}
									//Delete from unmatched loan queue
									(*iter3).second.pop_front();

								}
								// match with interbank deposit, send request
								else {
									log
											<< "MATCHED RANDOMLY WITH BACKLOG OR IB DEPOSIT"
											<< endl;
									UpdatePartnerInfo();

									Bank* aux;
									//Select a random bank from those with the lowest loan rate
									aux =
											(PartnersVectorDR.rbegin()->second)[(int) (((PartnersVectorDR.rbegin()->second).size())
													* (rand() / (RAND_MAX + 1.0)))];

									assert(!BNetwork::IsBankrupt(aux));

									sendingIBDepositRequest = true;
									tleft = 0; //Immediately send the request to interbank partner
									//qidr.push_back(*iter);
									qd++;
									CSH++;
									qidr++;
//
//Important										The bank doesn't know the time to maturity of the incoming deposit,
//												so it has to decide this for the IB request
//												the bank uses the mean, that's the best guess
//												This could be with new Depositor () in order to get a new ID

									qidrQueue.push_back(*iter);
									qidrQueue.back().setNewTimeToMaturity(
									DepositorMaturityParameter);

									qidrQueue.back().SetLinkedAgent(aux); //This is the partner that should receive the request

									qidrQueue.back().SetRequestSenderAgent(
											this);
									qidrQueue.back().setType(11);

//												This is a copy just to avoid complicated communication between banks,
//												we need to know when a matched loan or deposit expires in a partners queue

									UpdatePartnersSet(1);

									if ((*iter).get_t_to_maturity() < tleft) {
										tleft = (*iter).get_t_to_maturity();

										qt.push_front((*iter));

										//add the match to the maps
										matchMap1[&qt.front()] =
												&qidrQueue.back();
										matchMap2[&qidrQueue.back()] =
												&qt.front();
									}

									else {
										qt.push_back((*iter));

										//add the match to the maps
										matchMap1[&qt.back()] =
												&qidrQueue.back();
										matchMap2[&qidrQueue.back()] =
												&qt.back();
									}

									if (qidrQueue.back().get_t_to_maturity()
											< tleft) {
										tleft =
												qidrQueue.back().get_t_to_maturity();

										qt.push_front(qidrQueue.back());
										qt.front().setAsCopy();
									}

									else {
										qt.push_back(qidrQueue.back());
										qt.back().setAsCopy();
									}

								}
								UpdatePartnersSet(1);
								break;
							}
							// If best rate is in interbank market
							else if (PartnersVectorDR.rbegin()->first
									> (*iter3).first) {
								log
										<< "MATCHED WITH IB DEPOSIT, BACKLOG RATE OVER DESIRED"
										<< endl;
								UpdatePartnerInfo();
								Bank* aux;
								//Select a random bank from those with the lowest loan rate
								aux =
										(PartnersVectorDR.rbegin()->second)[(int) (((PartnersVectorDR.rbegin()->second).size())
												* (rand() / (RAND_MAX + 1.0)))];

								// Check that the selected bank is not a bankrupt one
								assert(!BNetwork::IsBankrupt(aux));

								// Set status as sending IB deposit request
								sendingIBDepositRequest = true;

								// Immediately send the request to interbank partner
								tleft = 0;

								// Add one deposit to the counter of active deposits
								qd++;

								// Add one unit to the cash account
								CSH++;

								// Add to the IB deposit request counter
								qidr++;
								//Important: The bank doesn't know the time to maturity of the incoming deposit,
								//so it have to decide this for the IB request
								// the bank uses the mean, that's the best guess
								// This could be with new Depositor () in order to get a new ID
								qidrQueue.push_back(*iter);
								qidrQueue.back().setNewTimeToMaturity(
								DepositorMaturityParameter);

								qidrQueue.back().SetLinkedAgent(aux); //This is the partner that should receive the request

								qidrQueue.back().SetRequestSenderAgent(this);
								qidrQueue.back().setType(11);

								//This is a copy just to avoid complicated communication between banks, we need to know when a matched loan or deposit expires in a partners queue

								UpdatePartnersSet(1);

								if ((*iter).get_t_to_maturity() < tleft) {
									tleft = (*iter).get_t_to_maturity();

									qt.push_front((*iter));

									//add the match to the maps
									matchMap1[&qt.front()] = &qidrQueue.back();
									matchMap2[&qidrQueue.back()] = &qt.front();
								}

								else {
									qt.push_back((*iter));

									//add the match to the maps
									matchMap1[&qt.back()] = &qidrQueue.back();
									matchMap2[&qidrQueue.back()] = &qt.back();
								}

								if (qidrQueue.back().get_t_to_maturity()
										< tleft) {
									tleft =
											qidrQueue.back().get_t_to_maturity();

									qt.push_front(qidrQueue.back());
									qt.front().setAsCopy();
								}

								else {
									qt.push_back(qidrQueue.back());
									qt.back().setAsCopy();
								}

								UpdatePartnersSet(1);
								break;
							}

						}
					} //End of case when both own backlog rate and interbank rate are higher than desired matching rate

					//Desired matching rate is higher than in the own backlog
					else {
						// The bank have to look in the interbank market
						// If rate in the interbank is higher than desired matching rate, there is a match
						if ((ValueFunction[LastState]).rbegin()->second.desiredMatchingInterestRate
								<= PartnersVectorDR.rbegin()->first) {
							log
									<< "MATCHED WITH IB DEPOSIT, BACKLOG RATE BELOW DESIRED"
									<< endl;
							UpdatePartnerInfo();

							Bank* aux;
							//Select a random bank from those with the lowest loan rate
							aux =
									(PartnersVectorDR.rbegin()->second)[(int) (((PartnersVectorDR.rbegin()->second).size())
											* (rand() / (RAND_MAX + 1.0)))];

							assert(!BNetwork::IsBankrupt(aux));

							sendingIBDepositRequest = true;
							tleft = 0; //Immediately send the request to interbank partner
							//qidr.push_back(*iter);
							qd++;
							CSH++;
							qidr++;
							//Important: The bank doesn't know the time to maturity of the incoming deposit,
							//so it have to decide this for the IB request
							// the bank uses the mean, that's the best guess
							// This could be with new Depositor () in order to get a new ID
							qidrQueue.push_back(*iter);
							qidrQueue.back().setNewTimeToMaturity(
							DepositorMaturityParameter);

							qidrQueue.back().SetLinkedAgent(aux); //This is the partner that should receive the request

							qidrQueue.back().SetRequestSenderAgent(this);
							qidrQueue.back().setType(11);

							//This is a copy just to avoid complicated communication between banks, we need to know when
							//a matched loan or deposit expires in a partners queue

							UpdatePartnersSet(1);

							if ((*iter).get_t_to_maturity() < tleft) {
								tleft = (*iter).get_t_to_maturity();

								qt.push_front((*iter));

								//add the match to the maps
								matchMap1[&qt.front()] = &qidrQueue.back();
								matchMap2[&qidrQueue.back()] = &qt.front();
							}

							else {
								qt.push_back((*iter));

								//add the match to the maps
								matchMap1[&qt.back()] = &qidrQueue.back();
								matchMap2[&qidrQueue.back()] = &qt.back();
							}

							if (qidrQueue.back().get_t_to_maturity() < tleft) {
								tleft = qidrQueue.back().get_t_to_maturity();

								qt.push_front(qidrQueue.back());
								qt.front().setAsCopy();
							}

							else {
								qt.push_back(qidrQueue.back());
								qt.back().setAsCopy();
							}

							UpdatePartnersSet(1);
							break;
						}
						//There is not match in the interbank market
						else {
							//Add this new deposit to the backlog of unmatched deposits
							log << "NO MATCH, ADDING TO BACKLOG" << endl;

							(*iter).setDesiredMatchingRate(
									(ValueFunction[LastState]).rbegin()->second.desiredMatchingInterestRate);

							if ((*iter).get_t_to_maturity() < tleft) {
								tleft = (*iter).get_t_to_maturity();
								//	qd.push_front(*iter);
								qd++;
								CSH++;
								qt.push_front((*iter));
								UMDeposits[(ValueFunction[LastState]).rbegin()->second.desiredMatchingInterestRate].push_back(
										&qt.front());

							} else {
								//qd.push_back(*iter);
								qd++;
								CSH++;
								qt.push_back((*iter));
								UMDeposits[(ValueFunction[LastState]).rbegin()->second.desiredMatchingInterestRate].push_back(
										&qt.back());

							}
							UpdatePartnersSet(1);
							break;
						}
					} // END Desired matching rate is higher than in the own backlog

				} //END for that iterates in the backlog

			} // END of customer deposit case

			//START, if bank is accepting an incoming interbank deposit
			//else if((*iter).getType() == 11){
			else {
				log << "INCOMING IB DEPOSIT FROM " << (*iter).get_IDborrower()
						<< " TO " << (*iter).GetLinkedAgentID() << endl;

				// Is useful to remember how the IB rates are fixed,
				// the bank uses the object in backlog which led to the
				// highest rate spread

//					if (UMLoans.empty())
//						IBDepositRate = RateDepositCB;
//					else
//						IBDepositRate = UMLoans.begin()->first;
//
//					if (UMDeposits.empty())
//						IBLoanRate = RateLoanCB;
//					else
//						IBLoanRate = UMDeposits.rbegin()->first;

				if ((*iter).get_t_to_maturity() < tleft) {
					tleft = (*iter).get_t_to_maturity();

					//log << "ADDING IB DEPOSIT TO ACCEPTED IB DEPOSITS" << endl;

					//The bank always accept an incoming IB deposit
					qida++;

					qt.push_front(*iter);
					qt.front().SetMatchedWith(
							(UMLoans.begin()->second.front()));

					matchMap1[&qt.front()] = (UMLoans.begin()->second.front());
					matchMap2[(UMLoans.begin()->second.front())] = &qt.front();

					//HERE ----

				} else {
					//qida.push_back(*iter);
					//log << "ADDING IB DEPOSIT TO ACCEPTED IB DEPOSITS" << endl;
					qida++;
					qt.push_back(*iter);
					qt.back().SetMatchedWith((UMLoans.begin()->second.front()));

					matchMap1[&qt.back()] = (UMLoans.begin()->second.front());
					matchMap2[(UMLoans.begin()->second.front())] = &qt.back();

				}
				UpdatePartnersSet(1);
				break;

			}// END If opt decision is to accept the incoming IB deposit or customer deposit
		}							// END Customer deposit or IB deposit case

///////////////////////////////////////////////////////////////////////////CUSTOMER LOAN OR IB LOAN/////////////////////////////////////////////////////////////////////////////////////////

		//if( ( (*iter).getType() == 2 ) | ( (*iter).getType() == 21) ) {
		else {
			log << "external function incoming customer loan or IB loan"
					<< endl;

			updateState((*iter).getType(), &(*iter));

			std::set<Action>::const_iterator iter2 = actionsSet.begin();

			log << "Setting Initial belief for loan" << endl;
			for (; iter2 != actionsSet.end(); iter2++) {
				if (h[LastState].find(*iter2) == h[LastState].end())
					SetInitialValue(LastState, *iter2, a.uniform(-1, 1));
			}

			//	if( (ValueFunction[LastState]).rbegin() -> second .accept & CheckCashReserveRequirements() ){//If opt decision is to accept the incoming IB loan or customer loan
			if (true) {	//If opt decision is to accept the incoming IB loan or customer loan

				if ((*iter).getType() == 2) { //If customer loan request

					log << "INCOMING CUSTOMER LOAN" << endl;
					std::map<int, std::deque<Loan*> >::iterator iter3 =
							UMDeposits.begin();

					//log << UMLoans.size() << endl;

					for (int n = 0; (iter3 != UMDeposits.end()) | (n == 0);
							iter3++, n++) {
						//log << "IN FOR" << endl;

						//If desired matching rate is higher than the lowest rate in unmatched loans
						if (iter3 != UMDeposits.end()) {
							if (((ValueFunction[LastState]).begin()->second.desiredMatchingInterestRate
									>= (*iter3).first)
									& ((*iter3).second.size() != 0)) {

								//If Partners rate are higher than in the own backlog, the bank looks for the lowest borrowing rate
								if ((PartnersVectorLR.begin()->first)
										> (*iter3).first) {
									//Set as matched loan with the unmatched deposit
									(*iter3).second.front()->SetMatchedWith(
											&(*iter));
									(*iter).SetMatchedWith(
											(*iter3).second.front()); //Respect priority

									if ((*iter).get_t_to_maturity() < tleft) {
										tleft = (*iter).get_t_to_maturity();
										ql++;
										CSH--;
										qt.push_front((*iter));
										//add the match to the maps
										matchMap1[&qt.front()] =
												(*iter3).second.front();
										matchMap2[(*iter3).second.front()] =
												&qt.front();

									}

									else {
										ql++;
										CSH--;
										qt.push_back((*iter));
										//add the match to the maps
										matchMap1[&qt.back()] =
												(*iter3).second.front();
										matchMap2[(*iter3).second.front()] =
												&qt.back();

									}

									//Delete from unmatched loan queue
									(*iter3).second.pop_front();
									UpdatePartnersSet(2);
									break;

									//log << "MATCHED WITH BACKLOG" << endl;

								}
								//If rate are the same in the own backlog and in the interbank market, flip a coin
								else if (PartnersVectorLR.begin()->first
										== (*iter3).first) {
									//match with own backlog
									log
											<< "MATCHED RANDOMLY WITH BACKLOG OR IB LOAN"
											<< endl;

									if (randomMatch.uniform(0, 1) < 0.5) {

										//Set as matched with the unmatched deposit

										//log << (*iter3).second.front()->getID() << " " << (*iter).getID() << endl;
										log << " " << (*iter).getID() << endl;
										if ((*iter3).second.front() == NULL)
											throw invalid_argument(
													"NULL POINTER");

										(*iter3).second.front()->SetMatchedWith(
												&(*iter));
										(*iter).SetMatchedWith(
												(*iter3).second.front());

										if ((*iter).get_t_to_maturity()
												< tleft) {
											tleft = (*iter).get_t_to_maturity();
											ql++;
											CSH--;
											qt.push_front((*iter));
											//add the match to the maps
											matchMap1[&qt.front()] =
													(*iter3).second.front();
											matchMap2[(*iter3).second.front()] =
													&qt.front();

										}

										else {
											ql++;
											CSH--;
											qt.push_back((*iter));
											//add the match to the maps
											matchMap1[&qt.back()] =
													(*iter3).second.front();
											matchMap2[(*iter3).second.front()] =
													&qt.back();

										}
										//Delete from unmatched deposit queue
										(*iter3).second.pop_front();

									}
									// match with interbank deposit, send request
									else {
										log
												<< "MATCHED RANDOMLY WITH BACKLOG OR IB DEPOSIT"
												<< endl;
										UpdatePartnerInfo();

										Bank* aux;
										//Select a random bank from those with the lowest loan rate
										aux =
												(PartnersVectorLR.begin()->second)[(int) (((PartnersVectorLR.begin()->second).size())
														* (rand()
																/ (RAND_MAX
																		+ 1.0)))];

										assert(!BNetwork::IsBankrupt(aux));

										(*iter).SetLinkedAgent(aux); //This is the partner that should receive the request

										(*iter).SetRequestSenderAgent(this);
										sendingIBLoanRequest = true;
										tleft = 0; //Immediately send the request to interbank partner
										qilr++;
										ql++;
										CSH--;
										// The loan's time to maturity is known
										qilrQueue.push_back(*iter);
										qilrQueue.back().setType(21);
										// This is a copy just to avoid complicated communication between banks,
										// we need to know when a matched loan or deposit expires in a partners queue

										if ((*iter).get_t_to_maturity()
												< tleft) {
											tleft = (*iter).get_t_to_maturity();

											qt.push_front((*iter));

											//add the match to the maps
											matchMap1[&qt.front()] =
													&qilrQueue.back();
											matchMap2[&qilrQueue.back()] =
													&qt.front();
										}

										else {
											qt.push_back((*iter));

											//add the match to the maps
											matchMap1[&qt.back()] =
													&qilrQueue.back();
											matchMap2[&qilrQueue.back()] =
													&qt.back();
										}

										if (qilrQueue.back().get_t_to_maturity()
												< tleft) {
											tleft =
													qilrQueue.back().get_t_to_maturity();

											qt.push_front(qilrQueue.back());
											qt.front().setAsCopy();
										}

										else {
											qt.push_back(qilrQueue.back());
											qt.back().setAsCopy();
										}

									}
									UpdatePartnersSet(2);
									break;
								}
								// If best rate is in interbank market

								else if (PartnersVectorLR.begin()->first
										< (*iter3).first) {

//

									log
											<< "MATCHED WITH IB LOAN, BACKLOG RATE BELOW DESIRED"
											<< endl;
									UpdatePartnerInfo();

									Bank* aux;
									//Select a random bank from those with the lowest loan rate
									aux =
											(PartnersVectorLR.begin()->second)[(int) (((PartnersVectorLR.begin()->second).size())
													* (rand() / (RAND_MAX + 1.0)))];

									assert(!BNetwork::IsBankrupt(aux));

									(*iter).SetLinkedAgent(aux); //This is the partner that should receive the request

									(*iter).SetRequestSenderAgent(this);

									sendingIBLoanRequest = true;
									tleft = 0; //Immediately send the request to interbank partner
									qilr++;
									ql++;
									CSH--;
									qilrQueue.push_back(*iter);
									// We need to check if this request was accepted
									qilrQueue.back().setType(21);

									//This is a copy just to avoid complicated communication between banks, we need to know when a matched loan or deposit expires in a partners queue

									UpdatePartnersSet(2);

									if ((*iter).get_t_to_maturity() < tleft) {
										tleft = (*iter).get_t_to_maturity();

										qt.push_front((*iter));

										//add the match to the maps
										matchMap1[&qt.front()] =
												&qilrQueue.back();
										matchMap2[&qilrQueue.back()] =
												&qt.front();
									}

									else {
										qt.push_back((*iter));

										//add the match to the maps
										matchMap1[&qt.back()] =
												&qilrQueue.back();
										matchMap2[&qilrQueue.back()] =
												&qt.back();
									}

									if (qilrQueue.back().get_t_to_maturity()
											< tleft) {
										tleft =
												qilrQueue.back().get_t_to_maturity();

										qt.push_front(qilrQueue.back());
										qt.front().setAsCopy();
									}

									else {
										qt.push_back(qilrQueue.back());
										qt.back().setAsCopy();
									}

									UpdatePartnersSet(2);
									break;

								}

							}
						} //End of case when both own backlog rate and interbank rate are lower than desired matching rate

						//Desired matching rate is lower than in the own backlog

						else {
							// The bank have to look in the interbank market
							// If rate in the interbank is lower than desired matching rate, there is a match
							if ((ValueFunction[LastState]).begin()->second.desiredMatchingInterestRate
									>= PartnersVectorLR.rbegin()->first) {

								log
										<< "MATCHED WITH IB LOAN, BACKLOG RATE OVER DESIRED"
										<< endl;
								UpdatePartnerInfo();

								Bank* aux;
								//Select a random bank from those with the lowest loan rate
								aux =
										(PartnersVectorLR.begin()->second)[(int) (((PartnersVectorLR.begin()->second).size())
												* (rand() / (RAND_MAX + 1.0)))];

								assert(!BNetwork::IsBankrupt(aux));

								(*iter).SetLinkedAgent(aux);//This is the partner that should receive the request
								(*iter).SetRequestSenderAgent(this);

								sendingIBLoanRequest = true;
								tleft = 0;//Immediately send the request to interbank partner
								qilr++;
								ql++;
								CSH--;
								qilrQueue.push_back(*iter);
								qilrQueue.back().setType(21);
								qt.push_back(*iter);

								// The requested object is a copied just to avoid complicated communication between banks, we need to know when
								//a matched loan or deposit expires in a partners queue

								UpdatePartnersSet(2);

								if ((*iter).get_t_to_maturity() < tleft) {
									tleft = (*iter).get_t_to_maturity();

									qt.push_front((*iter));

									//add the match to the maps
									matchMap1[&qt.front()] = &qilrQueue.back();
									matchMap2[&qilrQueue.back()] = &qt.front();
								}

								else {
									qt.push_back((*iter));

									//add the match to the maps
									matchMap1[&qt.back()] = &qilrQueue.back();
									matchMap2[&qilrQueue.back()] = &qt.back();
								}

								if (qilrQueue.back().get_t_to_maturity()
										< tleft) {
									tleft =
											qilrQueue.back().get_t_to_maturity();

									qt.push_front(qilrQueue.back());
									qt.front().setAsCopy();
								}

								else {
									qt.push_back(qilrQueue.back());
									qt.back().setAsCopy();
								}

								UpdatePartnersSet(2);

								break;
							}
							//There is not match in the interbank market
							else {
								//Add this new loan to the backlog of unmatched loans
								log << "NO MATCH, ADDING TO BACKLOG" << endl;

								assert(
										(ValueFunction[LastState]).rbegin()->second.desiredMatchingInterestRate!=NULL);

								(*iter).setDesiredMatchingRate(
										(ValueFunction[LastState]).rbegin()->second.desiredMatchingInterestRate);

								if ((*iter).get_t_to_maturity() < tleft) {
									tleft = (*iter).get_t_to_maturity();
									ql++;
									qt.push_front((*iter));
									CSH--;
									UMLoans[(ValueFunction[LastState]).rbegin()->second.desiredMatchingInterestRate].push_back(
											&qt.front());

								} else {
									ql++;
									qt.push_back((*iter));
									CSH--;
									UMLoans[(ValueFunction[LastState]).rbegin()->second.desiredMatchingInterestRate].push_back(
											&qt.back());

								}
								UpdatePartnersSet(2);
								break;
							}
						} // END Desired matching rate is higher than in the own backlog

					}
					//END for that iterates in the backlog
				} // End of customer loan case

				//START, if bank is accepting an incoming interbank loan
				//else if((*iter).getType() == 21)
				else {

					// Is useful to remember how the IB rates are fixed,
					// the bank uses the object in backlog which led to the
					// highest rate spread

					//					if (UMLoans.empty())
					//						IBDepositRate = RateDepositCB;
					//					else
					//						IBDepositRate = UMLoans.begin()->first;
					//
					//					if (UMDeposits.empty())
					//						IBLoanRate = RateLoanCB;
					//					else
					//						IBLoanRate = UMDeposits.rbegin()->first;

					log << "INCOMING IB LOAN FROM " << (*iter).get_IDlender()
							<< " TO " << (*iter).GetLinkedAgentID() << endl;
					if ((*iter).get_t_to_maturity() < tleft) {
						tleft = (*iter).get_t_to_maturity();
						qila++;
						CSH--;
						qt.push_front(*iter);

						// If the backlog is empty, the bank ask the CB
						if(IBLoanRate == RateLoanCB)

						//assert(IBLoanRate != RateLoanCB);

						qt.front().SetMatchedWith(
								(UMDeposits.rbegin()->second.front()));

						matchMap1[&qt.front()] =
								(UMDeposits.rbegin()->second.front());
						matchMap2[(UMDeposits.rbegin()->second.front())] =
								&qt.front();

						log << "1 " << tleft << endl;

					} else {
						qila++;
						CSH--;
						qt.push_back(*iter);

						qt.push_back(*iter);
						qt.back().SetMatchedWith(
								(UMDeposits.rbegin()->second.front()));

						matchMap1[&qt.back()] =
								(UMDeposits.rbegin()->second.front());
						matchMap2[(UMDeposits.rbegin()->second.front())] =
								&qt.back();

						log << "2 " << tleft << endl;

					}
					UpdatePartnersSet(2);
					log << "Size qt " << qt.size() << endl;
					break;
				}
				log << "Size qt " << qt.size() << endl;

			}// END If opt decision is to accept the incoming IB deposit or customer deposit
		}									// END Customer loan or IB loan case

	}									//BAG for incoming events

	log << "END DELTA EXT in BANK " << ID << endl;

}									//END DELTA_EXT

void Bank::delta_conf(const adevs::Bag<Loan>& xb) {
	delta_int();
	delta_ext(0.0, xb);
}
void Bank::output_func(adevs::Bag<Loan>& yb) {
	log << "START OUTPUT FUNC in BANK " << ID << endl;

	log << "output function BANK " << ID << endl;
//			log << "Size qt " << qt.size() << endl;
//			log << "Size qilr " << qilrQueue.size() << endl;
//			log << "Size qidr " << qidrQueue.size() << endl;

	if (sendingIBDepositRequest) {
		yb.insert(qidrQueue.back());
		log << "output function, sending IB Deposit Request" << endl;
	}

	else if (sendingIBLoanRequest) {
		log << "sending IB loan Request, size qlirQueue " << qilrQueue.size()
				<< endl;
//				if( qilrQueue.back() == NULL ) throw invalid_argument("NULL POINTER");
//			log << qilrQueue.back() << endl;

		yb.insert(qilrQueue.back());
		log << "output function, sending IB Loan Request" << endl;
	}

//if( ( qt.front().getType()==1 ) | ( qt.front().getType()==2 ) | ( qt.front().getType()==11 ) )
	else if (!qt.front().isCopy()) {

//if( qt.empty() ) throw invalid_argument("EMPTY QT");
//if(qt.front().getType() ==1 | qt.front().getType() ==2)  {
		qt.front().SetAsExpired();
		yb.insert(qt.front());
//}

		if (qt.front().getType() == 1)
			log << "output function, expired deposit" << endl;
		if (qt.front().getType() == 2)
			log << "output function, expired loan" << endl;
		if (qt.front().getType() == 11)
			log << "output function, expired IB deposit" << endl;
		if (qt.front().getType() == 21)
			log << "output function, expired IB loan" << endl;

	}

//if(sendingIBDepositRequest){yb.insert(qidr.back());log << "output function, sending IB Deposit Request" << endl;}

//if(sendingIBLoanRequest){yb.insert(qilr .back());log << "output function, sending IB Loan Request" << endl;}

	log << "END OUTPUT FUNC in BANK " << ID << endl;

}
double Bank::ta() {
	return tleft;
}

bool Bank::model_transition() {
	//transition activated if insolvent
	if (checkInsolvent())
		log << "Transition activated " << ID << endl;
	return checkInsolvent();
}
// Get the number of deposits in the queue
unsigned int Bank::getDepositQueueSize() {
	return qd;
}

unsigned int Bank::getLoanQueueSize() {
	return ql;
}

unsigned int Bank::getIBDepositRequestQueueSize() {
	return qidr;
}

unsigned int Bank::getIBDepositAcceptedQueueSize() {
	return qida;
}

unsigned int Bank::getIBLoanRequestQueueSize() {
	return qilr;
}

unsigned int Bank::getIBLoanAcceptedQueueSize() {
	return qila;
}

void Bank::gc_output(adevs::Bag<Loan>&) {
}

//	double Bank::value(){return equity;}

void Bank::reduce_equity(double shock) {
	equity -= shock;
}

bool Bank::checkInsolvent() {
	if (equity <= 0) {
		log << ID << " Is insolvent" << endl;
		return true;
	} else
		return false;
}

bool Bank::NextExpiring(const Loan& l1, const Loan& l2) {
	return l1.get_t_to_maturity() < l2.get_t_to_maturity();
}

bool Bank::WillItDefault() {
	return def.uniform(0, 1) < Pdefault;
}

void Bank::SetIBDepositRate(int r) {
	IBDepositRate = r;
}

void Bank::SetIBLoanRate(int r) {
	IBLoanRate = r;
}

int Bank::GetIBDepositRate() {
	return IBDepositRate;
}

int Bank::GetIBLoanRate() {
	return IBLoanRate;
}

void Bank::SetEquity(int e) {
	equity = e;
}

void Bank::IncreaseEquity(int e) {
	equity += e;
}

int Bank::GetEquity() {
	return equity;
}

int Bank::getCBLendingFacility() {
	return qcblf;
}

int Bank::getCash() {
	return CSH;
}
//{return Equities[equity];}

void Bank::SetInitialValue(State state, Action action, double v) {
	ValueFunction[state][v] = action;
	ValueFunction2[state][action] = v;
}

// Function to set continuation value (w)
void Bank::UpdateW(State state, Action action, double CurrentV) {

	ValueFunction2[state][action] = ValueFunction2[state][action]
			* h[state][action] / (h[state][action] + 1)
			+ CurrentV / (h[state][action] + 1);

	ValueFunction[state][ValueFunction2[state][action]] = action;

}

int Bank::GetNumberStatesVisited() {
	return (int) h.size();
}

double Bank::LastStateActionValue() {
	return ValueFunction2[LastState][LastAction];
}

void Bank::SetNPartners(int n) {
	partners = n;
}
//Return allowed number of partners
int Bank::GetAllowedNPartners() {
	return partners;
}

//Add the partner as a value in the map, the rate is the key so the map is sorted in decreasing order, in the same way as ValueFunction
void Bank::AddPartner(Bank* p) {

	if (p == this)
		throw invalid_argument("Adding itself to partners set");

	assert(p != NULL);
	assert(!BNetwork::IsBankrupt(p));

	// check that we previously removed old info
	assert(
			PartnersVectorDRderef.find(p) == PartnersVectorDRderef.end()
					&& PartnersVectorLRderef.find(p)
							== PartnersVectorLRderef.end());
	{

		PartnersVector.insert(p);
		PartnersVectorDR[p->GetIBDepositRate()].push_back(p);
		PartnersVectorLR[p->GetIBLoanRate()].push_back(p);
		PartnersVectorDRderef[p] = p->GetIBDepositRate();
		PartnersVectorLRderef[p] = p->GetIBLoanRate();

	}

}

int Bank::GetNPartners() {
	return (int) PartnersVector.size();
}

int Bank::GetAR() {
	return LaggedAgencyRatings[this];
}

void Bank::SetAR(int a) {
	LaggedAgencyRatings[this] = a;
}

void Bank::StopUpdatingProcess() {
	UpdatingProcess = false;
}

void Bank::StartUpdatingProcess() {
	UpdatingProcess = true;
}

void Bank::SendRating() {

	LaggedAgencyRatings[this] = ActualAgencyRatings[this];
	if (((CSH + qila) / (qd + qida + qcblf) < DowngradeRatio1)
			| (equity / (CSH + ql + qila) < DowngradeRatio2))
		ActualAgencyRatings[this] = 1;
	else
		ActualAgencyRatings[this] = 0;

}
;

void Bank::SetLqBnch() {
	LqBnch = beta2 * (qd + qida + qcblf); //This is rounded to int precision
}

bool Bank::CheckCashReserveRequirements() {
	return CSH > (qd + qida + qcblf);
}

//Update partners set depending on whether the last action involves a deposit or loan, a = 1 for deposit, a = 2, for loan
void Bank::UpdatePartnersSet(int a) {
	Bank* aux;

	assert(PartnersVector.size() != 0);
	//The same bank could be added again with this setting
	if (a == 1) {
//Select a random bank from those with the lowest deposit rate
		aux =
				(PartnersVectorDR.begin()->second)[(int) (((PartnersVectorDR.begin()->second).size())
						* (rand() / (RAND_MAX + 1.0)))];

		PartnersVector.erase(aux);

		while (PartnersVector.size()
				< (unsigned int) min(partners, (BNetwork::getBankCount() - 1))) {
			aux = BNetwork::getBank(
					(int) (NBANKS * (rand() / (RAND_MAX + 1.0))));
			if ((aux != this) && (!BNetwork::IsBankrupt(aux))
					&& PartnersVector.find(aux) == PartnersVector.end())
				PartnersVector.insert(aux);
		}

		UpdatePartnerInfo();

	}

	else {
//Select a random bank from those with the highest loan rate
		aux =
				(PartnersVectorLR.rbegin()->second)[(int) (((PartnersVectorLR.rbegin()->second).size())
						* (rand() / (RAND_MAX + 1.0)))];

		PartnersVector.erase(aux);

		while (PartnersVector.size()
				< (unsigned int) min(partners, (BNetwork::getBankCount() - 1))) {
			aux = BNetwork::getBank(
					(int) (NBANKS * (rand() / (RAND_MAX + 1.0))));
			if ((aux != this) && (!BNetwork::IsBankrupt(aux))
					&& PartnersVector.find(aux) == PartnersVector.end())
				PartnersVector.insert(aux);
		}

		UpdatePartnerInfo();

	}
}

void Bank::UpdatePartnerInfo() {
	assert(PartnersVector.size() != 0);

	//Clear partners information to make sure any info about a bankrupt bank is deleted. Then the actual info will be added.
	PartnersVectorDR.clear();
	PartnersVectorLR.clear();
	PartnersVectorDRderef.clear();
	PartnersVectorLRderef.clear();

	set<Bank*>::iterator iter = PartnersVector.begin();
	for (; iter != PartnersVector.end();) {

		if (BNetwork::IsBankrupt(*iter)) {
			log << "REPLACING BANKRUPT BANK PARTNER OF BANK " << ID << endl;

			ReplaceBankruptPartner(*iter);
			//Start again because the iterator was invalidated when replacing the bankrupt bank
			PartnersVectorDR.clear();
			PartnersVectorLR.clear();
			PartnersVectorDRderef.clear();
			PartnersVectorLRderef.clear();

			iter = PartnersVector.begin();

		}
//Update info about partner
		else {
			AddPartner(*iter);
			iter++;
		}

	}

	assert(
			find(PartnersVector.begin(), PartnersVector.end(), this)
					== PartnersVector.end());

}

string Bank::printPartners() {
	string aux;
	set<Bank*>::iterator iter = PartnersVector.begin();
	assert(!BNetwork::IsBankrupt(this));

	ostringstream temp;
	//log << PartnersVector.size() << endl;
	//log << ID << endl;
	assert(PartnersVector.size() != 0);
//log << "BANK " << ID << " SIZE PARTNERS SET " << PartnersVector.size() << endl;
	for (; iter != PartnersVector.end(); iter++) {
//log << "PRINTING PARTNERS OF " << ID << endl;
		assert(!BNetwork::IsBankrupt(*iter));

		temp << (*iter)->getID() << ";";

	}
	return aux = temp.str();
}

//		int Bank::GetCDepositRate(){return DepositRates[0];}
//
//		int Bank::GetCLoanRate(){return DepositRates[0];};
//

void Bank::updateState(int type, Loan* loan) {

	if (UMLoans.empty())
		IBDepositRate = RateDepositCB;
	else
		IBDepositRate = UMLoans.begin()->first;

	if (UMDeposits.empty())
		IBLoanRate = RateLoanCB;
	else
		IBLoanRate = UMDeposits.rbegin()->first;

	LastState.loanOrDeposit = type;
//	2) rate of the loan or deposit (this will give me if it is a costmer loan-depost or interbank loan deposit)
	LastState.interestRate = loan->getInterestRate();
//					3)Rating of the bank sending the loan-deposit from the interbank market (i.e. 0 in the case of a Customer loan-deposit)
// This is the lagged rating
	LastState.ratingSender = LaggedAgencyRatings[loan->GetSenderAgent()];
	//					4)  your rating
	LastState.myRating = ActualAgencyRatings[this];
	//					5) partners ratings (with a lag)
	LastState.partnersRatings.clear();

	set<Bank*>::iterator iter;
	for (iter = PartnersVector.begin(); iter != PartnersVector.end(); iter++)
		LastState.partnersRatings.push_back(LaggedAgencyRatings[*iter]);

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

	if ((type == 1) | (type == 11)) {
		if (!UMLoans.empty())
			LastState.deepBackLog = (int)(UMLoans.rbegin()->second).size();
		else
			LastState.deepBackLog = 0;

		LastState.totaldeepBackLog = 0;
		std::map<int, std::deque<Loan*> >::iterator iter = UMLoans.begin();
		if (!UMLoans.empty())
			for (; iter != UMLoans.end(); iter++)
				LastState.totaldeepBackLog += (iter->second).size();
		else
			LastState.totaldeepBackLog = 0;

	}

	else {
		LastState.deepBackLog = (int)(UMDeposits.begin()->second).size();

		LastState.totaldeepBackLog = 0;
		std::map<int, std::deque<Loan*> >::iterator iter = UMDeposits.begin();
		for (; iter != UMDeposits.end(); iter++)
			LastState.totaldeepBackLog += (iter->second).size();
	}

}

int Bank::getID() {
	return ID;
}

int Bank::getReplacesBank() {
	return ReplacesBank;
}

void Bank::setReplacesBank(int bank) {
	ReplacesBank = bank;
}

bool Bank::operator ==(Bank a) const {
	return ID == a.getID();
}

double Bank::GetProbDef() {
	return Pdefault;
}

int Bank::GetAssets() {
	return CSH + ql + qila;
}

void Bank::Liquidate() {
	tleft = DBL_MAX;
	PartnersVector.clear();
	equity = 0;
	CSH = 0;
}

void Bank::ReplaceBankruptPartner(Bank* bank) {
	log << "start replaceBankruptPartner " << endl;

	PartnersVector.erase(bank);

	Bank* aux;
// At most N-1 partners and we impose the restriction that max number is "partners"
	while (PartnersVector.size() < min(partners, (BNetwork::getBankCount() - 1))) {

		aux = BNetwork::getBank((int) (NBANKS * (rand() / (RAND_MAX + 1.0))));
		if ((aux != this) && (!BNetwork::IsBankrupt(aux))
				&& PartnersVector.find(aux) == PartnersVector.end()) {
			PartnersVector.insert(aux);
		}

	}

	log << "end replaceBankruptPartner " << endl;

}

void Bank::askCentralBank() {
	// If uniform is below 1-phiCB, the CB accept the request
	if (centralBank.uniform(0, 1) < 1 - phiCB)
		CSH++;
}

bool Bank::isPartner(Bank* bank) {
	return PartnersVector.find(bank) != PartnersVector.end();
}

void Bank::initializeActionSet() {
	for (int n = 0; n < 2 * pRatesLevels + 1; n++) {
		actionsSetPointers.insert(new Action(n, true, true));
		actionsSetPointers.insert(new Action(n, true, false));
		actionsSetPointers.insert(new Action(n, false, true));
		actionsSetPointers.insert(new Action(n, false, false));
	}
}

void Bank::updateBacklogs() {
	map<int, deque<Loan*> >::iterator itDeposits = UMDeposits.begin();

	map<int, deque<Loan*> >::iterator itLoans = UMLoans.begin();

	for (; itDeposits != UMDeposits.end(); itDeposits++) {
		if ((itDeposits->second).size() == 0)
			UMDeposits.erase(itDeposits);
	}

	for (; itLoans != UMLoans.end(); itLoans++) {
		if ((itLoans->second).size() == 0)
			UMLoans.erase(itLoans);
	}

}

void Bank::cancelLoan(Loan* loan) {
}

void Bank::resolveCancelledLoan(Loan* loan) {
}

void Bank::resolveIlliquidity() {
}
