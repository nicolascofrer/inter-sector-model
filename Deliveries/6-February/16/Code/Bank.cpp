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

// Model with liquid assets only

using namespace std;

//Static variables

//A small class to merge these two maps h and ValueFunction, difference frequencies for banks
//		map<vector<int> , map<vector<int>,int > > Bank::h;
//		map<vector<int> ,int > Bank::h_data;
int Bank::bankCounter = 0;

double Bank::time = 0;
double Bank::WdV = 0;
double Bank::probTrembling = 0.5;

int Bank::getBankCounter() {
	return bankCounter;
}

double Bank::getWdV() {
	return WdV;
}

void Bank::updateIlliquidAssetPrice(double t) {
	double DeltaT = t - time;
//	cerr << DeltaT/dt << endl;
	int nSteps = round(DeltaT / dt);
	for (int i = 0; i != nSteps; i++) {
//	cerr << "nsteps" << nSteps<< " i "<< i << endl;
		illiquidAssetPrice += -(illiquidAssetPrice - 1) * dt
				+ 0.1 * illiquidAssetPriceProcess.normal(0, 1) * sqrt(dt);
//	cerr << illiquidAssetPrice << endl;
//	cerr << "ilapp rv " << illiquidAssetPriceProcess.normal(0,1) << endl;

	}
}

double Bank::illiquidAssetPrice = 1;
adevs::rv Bank::illiquidAssetPriceProcess;

adevs::rv Bank::def;

adevs::rv Bank::a;

adevs::rv Bank::centralBank;

adevs::rv Bank::randomMatch;

adevs::rv Bank::tremblingRandomVariable;

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

map<int, Strategy*> Bank::loanStrategyMap;
map<int, double> Bank::cumulativeObjectValue;

bool Bank::UpdatingProcess = true;

vector<double> Bank::Rates;
set<State*> Bank::stateSet;
set<Strategy*> Bank::strategySet;

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
//	log.open("log.txt");

	ID = bankCounter;
//log << "Constructor Bank " << ID << endl;
	bankCounter++;
//SetEquityRange(0,50);
	partners = NNeighborhood; //So far, only 3 partners are allowed, but clearly this could be N(i)
	SetAR(0);
	equity = equity0;
	CSH = cash0;
	defaults = 0;
	canceledLoan = -1;
	variablesDebugging.open("Debugging.txt");
	totalH = 0;

	Pdefault = ProbabilityDefault;
	ReplacesBank = 99999;
//	IBDepositRate = CBDFR + rand() % 20 - rand() % 20;
//	IBLoanRate = CBLFR + rand() % 20 - rand() % 20;

	IBDepositRate = RateDepositCB;
	IBLoanRate = RateLoanCB;

	isTrembling = false;
	sendingIBDepositRequest = false;
	sendingIBLoanRequest = false;
	cancelingLoan = false;
	rejectingIBloanrequest = false;
	linkcost = Clink;
	initializeActionSet();

	typename std::set<Action*>::iterator iter = actionsSetPointers.begin();
	for (; iter != actionsSetPointers.end(); iter++) {
		actionsSet.insert(**iter);
		actionsVector.push_back(*iter);
	}
	qd = 0;
	qida = 0;
	qcblf = 0;
	qilr = 0;
	ql = 0;
	qila = 0;
	qidr = 0;

	SetLqBnch();

//Initial Rating given by a Rating Agency (an integer value 0:Good, 1:Bad),							0
	ActualAgencyRatings[this] = 0;
}
;

Bank::~Bank() {
	typename std::vector<Loan*>::iterator iter = lbank.begin();
	for (; iter != lbank.end(); iter++)
		delete *iter;

	typename std::set<Action*>::iterator iter2 = actionsSetPointers.begin();
	for (; iter2 != actionsSetPointers.end(); iter2++)
		delete *iter2;

//	typename std::map<int, Strategy*>::iterator iter3 = loanStrategyMap.begin();
//	for (; iter3 != loanStrategyMap.end(); iter3++)
//		delete (*iter3).second;

}

void Bank::delta_int() {
	cout << "D Int " << " Time is : " << time << ", START DELTA INT in BANK "
			<< ID << endl;

	cout << "Bank " << ID << " qt size " << qt.size() << endl;
	cout << "Bank " << ID << " qila " << qila << endl;
	cout << "Bank " << ID << " qilr " << qilr << endl;
	cout << "Bank " << ID << " qida " << qida << endl;
	cout << "Bank " << ID << " qidr " << qidr << endl;
	cout << "Bank " << ID << " ql " << ql << endl;
	cout << "Bank " << ID << " qd " << qd << endl;

	assert((unsigned int )(qila + qilr + qida + qidr + ql + qd) == qt.size());

	if (sendingIBDepositRequest) {
//		cout << "Internal Function Sending IB Deposit Request BANK " << ID
//				<< endl;
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
//		cout << "Internal Function Sending IB Loan Request BANK " << ID << endl;
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

//		cout << "Internal Function expired object " << ID << endl;
		cout << "Bank " << ID << " Time is: " << Bank::time
				<< " Removing expired object, ID is " << qt.front().getID()
				<< " Was matched: " << qt.front().wasMatched() << " is a copy "
				<< qt.front().isCopy() << " Type is " << qt.front().getType()
				<< endl;

		//	if(qt.front().wasMatched()) assert(matchMap1.count(qt.front())>0 || matchMap2.count(qt.front())>0);

//				if((qt.front()).getType() == 11 & (qt.front()).get_borrower()==this ) throw invalid_argument("self IB deposit request when not sending request");

		// Last state and last action were defined when the last action was taken, this happened in the delta_ext() function
		// Here we have to update value of decisions, because in this function (delta_int) is where we can see realized values of past actions

		// In case of customer deposit/////////////////////////////////////////////////////////////////////
		if ((qt.front()).getType() == 1) {
//			cout << "Case 1" << endl;
			qd--;
			CSH--;
			//log << "Expired deposit" << endl;
			if (UpdatingProcess) {
				updateCumulativeValueObject(qt.front(), false);
				UpdateW(loanStrategyMap[qt.front().getID()],
						cumulativeObjectValue[qt.front().getID()]); //Update given the realized state is a deposit expiration
			}
		}

		// In case of customer loan////////////////////////////////////////////////////////////////////////
		// If there is default, reduce equity. If all is ok, get the profits from a successful loan
		else if ((qt.front()).getType() == 2) {
//			cout << "Case 2" << endl;
			ql--;
			CSH++;
			//log << "Expired Loan" << endl;
			if (WillItDefault()) {
				reduce_equity((qt.front()).GetQuantity());
				if (UpdatingProcess) {
					updateCumulativeValueObject(qt.front(), true);
					UpdateW(loanStrategyMap[qt.front().getID()],
							cumulativeObjectValue[qt.front().getID()]);
				}
			}//Update given realized state is a loan default. Now the LastAction, LastState is associated to the Loan object, and there's a time parameter to be used when discounting

			else {
				//	cout << "h: " << loanStrategyMap[qt.front().getID()]->getH() << endl;
//				CSH++;
				if (UpdatingProcess) {
					updateCumulativeValueObject(qt.front(), false);
					UpdateW(loanStrategyMap[qt.front().getID()],
							cumulativeObjectValue[qt.front().getID()]);	//Update given realized state is a successful loan
				}
			}
		}

		// In case of IB deposit received
		else if ((qt.front()).getType() == 11 && !qt.front().isCopy()) {
//			cout << "Case 3" << endl;
			qida--;
			//log << "Expired IB Deposit" << endl;
			CSH--;

//			assert(qt.front().GetLinkedAgentID() == ID
//							&& qt.front().GetSenderAgentID() != ID);

			if (UpdatingProcess) {
				updateCumulativeValueObject(qt.front(), false);
				UpdateW(loanStrategyMap[qt.front().getID()],
						cumulativeObjectValue[qt.front().getID()]); //Update given the realized state is a deposit expiration
			}
		}

		// Case IB deposit requested.
		// This value does not need to be updated, it's part of another decision
//		else if ((qt.front()).getType() == 11 && qt.front().GetLinkedAgentID() != ID
//				&& qt.front().GetSenderAgentID() == ID
//				&& !qt.front().isCopy()) {
////			cout << "Case 4" << endl;
//
//			qidr--;
//			removeFromVM(qt.front().getID());
//
//			CSH++;
//			//log << "Expired IB Loan requested" << endl;
//			if (BNetwork::IsBankrupt(
//					dynamic_cast<Bank*>(qt.front().GetSenderAgent()))
//					&& !BNetwork::wasIBLiabilityPaid(qt.front().getID())) {
//				// Deposit was not paid back
//				reduce_equity((qt.front()).GetQuantity());
//				if (UpdatingProcess) {
//					updateCumulativeValueObject(qt.front(), true);
//
//					UpdateW(loanStrategyMap[qt.front().getID()],
//							cumulativeObjectValue[qt.front().getID()]);
//				}
//			} //Update given realized state is a loan default. Now the LastAction, LastState is associated to the Loan object, and there's a time parameter to be used when discounting
//
//			else {
//				// Deposit was paid back
//				CSH++;
//				if (UpdatingProcess) {
//					updateCumulativeValueObject(qt.front(), false);
//					UpdateW(loanStrategyMap[qt.front().getID()],
//							cumulativeObjectValue[qt.front().getID()]);	//Update given realized state is a successful loan
//				}
//			}
//		}

		// Case IB loan issued (object received)
		else if ((qt.front()).getType() == 21 && !qt.front().isCopy()) {
//			cout << "Case 5" << endl;

			assert(qila >= 0);

			cerr << "Bank " << ID << " loan ID " << qt.front().getID()
					<< " qila is: " << qila << endl;
			qila--;
			removeFromVM(qt.front().getID());

			CSH++;

//			assert(qt.front().GetLinkedAgentID() == ID
//							&& qt.front().GetSenderAgentID() != ID);

			//log << "Expired IB Loan issued" << endl;
			if (BNetwork::IsBankrupt(  BNetwork::getBankIncludingBankrupt(qt.front().GetSenderAgentID()) )
					&& !BNetwork::wasIBLiabilityPaid(qt.front().getID())) {
				// Loan was not paid back
				reduce_equity((qt.front()).GetQuantity());
				if (UpdatingProcess) {
					updateCumulativeValueObject(qt.front(), true);
					UpdateW(loanStrategyMap[qt.front().getID()],
							cumulativeObjectValue[qt.front().getID()]);
				}
			}//Update given realized state is a loan default. Now the LastAction, LastState is associated to the Loan object, and there's a time parameter to be used when discounting

			else {
				// Loan was paid back
//				CSH++;
				if (UpdatingProcess) {
					updateCumulativeValueObject(qt.front(), false);
					UpdateW(loanStrategyMap[qt.front().getID()],
							cumulativeObjectValue[qt.front().getID()]);	//Update given realized state is a successful loan
				}
			}
		}

		// Case IB borrowing, Loan object sent

		// In case of IB borrowing. IB Loan object sent. This value does not need to be updated, it's part of another decision
//		else if (((qt.front()).getType() == 21 && qt.front().GetLinkedAgentID() != ID)
//				&& qt.front().GetSenderAgentID() == ID
//				&& !qt.front().isCopy()) {
////			cout << "Case 6" << endl;
//			qilr--;
//			//log << "Expired IB Loan requested (IB borrowing)" << endl;
//			CSH--;
//			if (UpdatingProcess) {
//				updateCumulativeValueObject(qt.front(), false);
//				UpdateW(loanStrategyMap[qt.front().getID()],
//						cumulativeObjectValue[qt.front().getID()]); //Update given the realized state is a deposit expiration
//			}
//		}

		else if (qt.front().isCopy() && qt.front().getType() == 11) {
			qidr--;
			CSH++;
			removeFromVM(qt.front().getID());
		}

		else if (qt.front().isCopy() && qt.front().getType() == 21) {
			qilr--;
			CSH--;
		}

		//END Updating W
		// Remember that the queue qt has the IB request to partners as copied objects

		//log << "SIZE QT " << qt.size() << endl;
		//Now change state, it doesn't matter because we are not making any decision in this part
		if (qt.size() == 0)
			throw invalid_argument("Empty QT");

		// check if the matching counterpart object is the next to expire, this will leave an object without match, this should be added again in the backlog

		// I added a member in the Loan class which indicates whether the loan object was matched or not, so we only search in the map if it was a matched object
		//Search in map of matching object the expiring loan object

		//if (qt.front().wasMatched() || qt.front().isCopy()) {
		if (qt.front().wasMatched()) {

//			cout << "Bank " << ID << " Time is: " << Bank::time
//					<< " Object was matched!" << endl;
			typename std::map<Loan, Loan>::iterator it1 = matchMap1.find(
					qt.front());

			typename std::map<Loan, Loan>::iterator it2 = matchMap2.find(
					qt.front());

			// If the maturities are unknown, the bank also need to check for copies. "copy and 21 -> UM deposit"
			if (it1 != matchMap1.end()) {
				it1->second.setMatched(false);
//				cout << "Bank " << ID << " Time is: " << Bank::time
//						<< " Match counterpart expired: Adding to unmatched deposits ID "
//						<< it1->second.getID() << endl;
				//Item matched with the object that is expired
				if ((it1->second.getType() == 1 || it1->second.getType() == 11)
						&& !it1->second.isCopy()) {
					//The object that will be unmatched is a customer deposit or IB deposit
					UMDeposits[it1->second.getDesiredMatchingRate()].push_back(
							it1->second);
				} else if (!it1->second.isCopy())
					//the object that will be unmatched is a customer loan or IB loan
					UMLoans[it1->second.getDesiredMatchingRate()].push_back(
							it1->second);
				matchMap1.erase(it1);
			}

			//The same but search the other way around

			else if (it2 != matchMap2.end()) {
//			else {
				it2->second.setMatched(false);
//				cout << "Bank " << ID << " Time is: " << Bank::time
//						<< " Match couterpart expired: Adding to unmatched deposits ID "
//						<< it2->second.getID() << endl;
				//Item that's matched with the object that is expired
				if ((it2->second.getType() == 1 || it2->second.getType() == 11)
						&& !it2->second.isCopy()) {
					//The object that will be unmatched is a customer deposit or IB deposit
					UMDeposits[it2->second.getDesiredMatchingRate()].push_back(
							it2->second);
				} else if (!it2->second.isCopy())
					//the object that will be unmatched is a customer loan or IB loan
					UMLoans[it2->second.getDesiredMatchingRate()].push_back(
							it2->second);
				matchMap2.erase(it2);
			}
		}

		else {

			typename std::map<int, deque<Loan> >::iterator itBacklogDeposits =
					UMDeposits.find(qt.front().getDesiredMatchingRate());

			typename std::map<int, deque<Loan> >::iterator itBacklogLoans =
					UMLoans.find(qt.front().getDesiredMatchingRate());

			// If the object was unmatched, search it in the backlogs and remove it
			// I wonder if it would be better to iterate in the keys of the map (desired rates) and sort each queue
			if (itBacklogDeposits != UMDeposits.end()) {
				deque<Loan>::iterator itQ;
				itQ = find(itBacklogDeposits->second.begin(),
						itBacklogDeposits->second.end(), qt.front());
				if (itQ != itBacklogDeposits->second.end())
					itBacklogDeposits->second.erase(itQ);
				//cout << "Deleting from UMDeposits" << endl;
			}

			else if (itBacklogLoans != UMLoans.end()) {
//			else {
				//cout << "Deleting from UMLoans" << endl;
				deque<Loan>::iterator itQ;
				itQ = find(itBacklogLoans->second.begin(),
						itBacklogLoans->second.end(), qt.front());
				if (itQ != itBacklogLoans->second.end())
					itBacklogLoans->second.erase(itQ);
			}
		}

		// Updating queue

		qt.pop_front(); // Remove the expired object
		//I need to delete the pointer to the object

		//We need to update the time elapsed for all objects
		typename std::deque<Loan>::iterator iter = qt.begin();
		for (; iter != qt.end(); iter++) {
			(*iter).ReduceTimeToMaturity(tleft);
		} //Time to maturity is reduced by the time elapsed

		if (qt.empty())
			tleft = DBL_MAX; // No event left

		else { //look for next object to expire
			sort(qt.begin(), qt.end(), NextExpiring);
			tleft = qt.front().get_t_to_maturity();
		}

		updateBacklogs();

	}

//log << "END DELTA INT in BANK " << ID << endl;
}

void Bank::delta_ext(double e, const adevs::Bag<Loan>& xb) {

	cout << "D Ext" << " Time is: " << time << " START DELTA EXT in BANK " << ID
			<< endl;

	cout << "Bank " << ID << " qt size " << qt.size() << endl;
	cout << "Bank " << ID << " qila " << qila << endl;
	cout << "Bank " << ID << " qilr " << qilr << endl;
	cout << "Bank " << ID << " qida " << qida << endl;
	cout << "Bank " << ID << " qidr " << qidr << endl;
	cout << "Bank " << ID << " ql " << ql << endl;
	cout << "Bank " << ID << " qd " << qd << endl;

	assert((unsigned int )(qila + qilr + qida + qidr + ql + qd) == qt.size());

	assert(PartnersVector.find(this) == PartnersVector.end());

	assert(PartnersVector.size() == NNeighborhood);

// Update the remaining time to maturity
	if (!qt.empty()) {
		tleft -= e;
		//We need to update the time elapsed for all objects
		typename std::deque<Loan>::iterator iter4 = qt.begin();
		for (; iter4 != qt.end(); iter4++) {
			(*iter4).ReduceTimeToMaturity(e);
		} //Time to maturity is reduced by the time elapsed
	}

// Put new events into the queue, here we can to separate loan objects according to the type in the loan object
	typename adevs::Bag<Loan>::const_iterator iter = xb.begin();
	int bagIterationnumber = 0;
	for (; iter != xb.end(); iter++) //We have to look at each new object coming
			{
		//log << "Time is " << time << " START of BAG in BANK " << ID << endl;
		// Ensuring that the incoming object is being seen as unmatched
		(*iter).setMatched(false);

		// Trembling
		int trem = (int) (actionsVector.size() * (rand() / (RAND_MAX + 1.0)));
		if (tremblingRandomVariable.uniform(0, 1) < probTrembling) {
//			cout << "Time is " << time << ", Bank " << ID
//					<< " trembled when using its optimal rule, using strategy in position "
//					<< trem << endl;
			isTrembling = true;
		} else
			isTrembling = false;

		UpdatePartnerInfo();
		updateBacklogs();
		// Set time first match as the arrival time
		(*iter).setTimeLastMatch(time);
		/////////////////////////////////////////////////////////////////////////////CUSTOMER DEPOSIT OR IB DEPOSIT////////////////////////////////////////////////////////////////////
		if (((*iter).getType() == 1) | ((*iter).getType() == 11)) {

//			log << "external function incoming customer deposit or IB deposit"
//					<< endl;

			// Update the actual state, including that the bank is receiving a deposit
			State* tempState;
			updateState( (*iter));
			tempState = new State(LastState);

			assert(*tempState==LastState);

			typename std::set<Action>::const_iterator iter2 =
					actionsSet.begin();
			//log << "Setting Initial belief for deposit" << endl;
			if (stateSet.find(tempState) == stateSet.end()) {
//				cerr << "STATE NOT FOUND" << endl;
				for (; iter2 != actionsSet.end(); iter2++) {
					// In this step is useful to remember that if LastState does not exist in h, a new key will be created
//				if (h[LastState].find(*iter2) == h[LastState].end()) {
					SetInitialValue(LastState, *iter2,
							a.uniform(-0.001, 0.001));
					h[LastState][*iter2] = 0;
				}
			}

			if (isTrembling)
				LastAction = *actionsVector[trem];

			else
				LastAction = ValueFunction[LastState].rbegin()->second; // This is the best action the bank could take according to its value function

			tempStrategy = new Strategy(LastState, LastAction,
					ValueFunction2[LastState][LastAction]);
			// Save strategy
			saveToStrategyMap((*iter).getID());
			stateSet.insert(tempState);

			// Increment the counter map h
			h[LastState][LastAction] = h[LastState][LastAction] + 1; // We use this map because this is the actual number of visits. The number h saved in the loanStrategyMap is the number of visits when we took the action

			//The bank always accept any type of deposit
			//	if( (ValueFunction[LastState]).rbegin() -> second .accept | ((*iter).getType() == 1) ){//If opt decision is to accept  the incoming IB deposit or customer deposit

			if ((*iter).getType() == 1) {

				//log << time << " INCOMING CUSTOMER DEPOSIT" << endl;
				typename std::map<int, std::deque<Loan> >::reverse_iterator iter3 =
						UMLoans.rbegin();

				//log << UMLoans.size() << endl;

				for (int n = 0; (iter3 != UMLoans.rend()) | (n == 0);
						iter3++, n++) {
					//log << "IN FOR" << endl;

					//If desired matching rate is lower than the highest rate in unmatched loans
					if (iter3 != UMLoans.rend()) {
						if ((LastAction.desiredMatchingInterestRate
								<= (*iter3).first)
								&& ((*iter3).second.size() != 0)) {

							(*iter3).second.front().setInterestRate(
									(*iter3).first);

							//If Partners rate lower than in the own backlog
							if ((PartnersVectorDR.rbegin()->first)
									< (*iter3).first) {

								//Set matched loan with in the unmatched loan
								(*iter3).second.front().SetMatchedWith(
										(*iter).getID());
								//Set matched with in the incoming deposit
								(*iter).SetMatchedWith(
										(*iter3).second.front().getID());

								if ((*iter).get_t_to_maturity() < tleft) {
									tleft = (*iter).get_t_to_maturity();
									//qd.push_front(*iter);
									qd++;
									CSH++;
									qt.push_front((*iter));

									// Just ensuring that the unmatched object is set as unmatched
									(*iter3).second.front().setMatched(false);
									qt.front().setMatched(false);
									//add the match to the maps
									newMatch(qt.front(),
											(*iter3).second.front());
									matchMap1[qt.front()] =
											(*iter3).second.front();
									matchMap2[(*iter3).second.front()] =
											qt.front();
								}

								else {
									//qd.push_back(*iter);
									qd++;
									CSH++;
									qt.push_back((*iter));
									// Just ensuring that the unmatched object is set as unmatched
									(*iter3).second.front().setMatched(false);
									qt.back().setMatched(false);
									//add the match to the maps
									newMatch(qt.back(),
											(*iter3).second.front());
									matchMap1[qt.back()] =
											(*iter3).second.front();
									matchMap2[(*iter3).second.front()] =
											qt.back();
								}

								//Delete from unmatched loan queue
								(*iter3).second.pop_front();
								UpdatePartnersSet(1);

								//log << "MATCHED WITH BACKLOG" << endl;
								break;

							}
							//If rate are the same in the own backlog and in the interbank market, flip a coin
							else if (PartnersVectorDR.rbegin()->first
									== (*iter3).first) {

								(*iter3).second.front().setInterestRate(
										(*iter3).first);

								//match with own backlog
//								log
//										<< "MATCHED RANDOMLY WITH BACKLOG OR IB DEPOSIT"
//										<< endl;

								if (randomMatch.uniform(0, 1) < 0.5) {

									(*iter3).second.front().SetMatchedWith(
											(*iter).getID());

									(*iter).SetMatchedWith(
											(*iter3).second.front().getID());

									// Update next event time, if the event is the next to expire
									if ((*iter).get_t_to_maturity() < tleft) {
										tleft = (*iter).get_t_to_maturity();
										//	qd.push_front(*iter);
										qd++;
										CSH++;
										qt.push_front((*iter));

										// Just ensuring that the unmatched object is set as unmatched
										(*iter3).second.front().setMatched(
												false);
										qt.front().setMatched(false);
										//add the match to the maps
										newMatch(qt.front(),
												(*iter3).second.front());
										matchMap1[qt.front()] =
												(*iter3).second.front();
										matchMap2[(*iter3).second.front()] =
												qt.front();
									}
//												 If the event is not the next to expire, put it at the back of the queue,
//												 then in the internal function the queue will be sorted using the time left
//												 of each object

									else {

										qd++;
										CSH++;
										qt.push_back((*iter));

										// Just ensuring that the unmatched object is set as unmatched
										(*iter3).second.front().setMatched(
												false);
										qt.back().setMatched(false);
										//add the match to the maps
										newMatch(qt.back(),
												(*iter3).second.front());

										matchMap1[qt.back()] =
												(*iter3).second.front();
										matchMap2[(*iter3).second.front()] =
												qt.back();
									}
									//Delete from unmatched loan queue
									(*iter3).second.pop_front();

								}
								// match with interbank deposit, send request
								else {
//									log
//											<< "MATCHED RANDOMLY WITH BACKLOG OR IB DEPOSIT"
//											<< endl;
									UpdatePartnerInfo();

									Bank* aux;
									//Select a random bank from those with the lowest loan rate
									aux =
											(PartnersVectorDR.rbegin()->second)[(int) (((PartnersVectorDR.rbegin()->second).size())
													* (rand() / (RAND_MAX + 1.0)))];

									assert(!BNetwork::IsBankrupt(aux));

									sendingIBDepositRequest = true;

									//log << "Sending IBDr Bank " << ID << endl;
									tleft = 0; //Immediately send the request to interbank partner
									//qidr.push_back(*iter);
									qd++;
									//CSH++; The cash received is deposited in the partner bank using an IB deposit
									qidr++;
//
//Important										The bank doesn't know the time to maturity of the incoming deposit,
//												so it has to decide one value for it in the IB request.
//												The bank uses the mean, that's the best guess
//												This could be with new Depositor () in order to get a new ID

									qidrQueue.push_back(*iter);
									qidrQueue.back().setNewID();

									valueMatchedIBLoans[ValueFunction[LastState].rbegin()->first] =
											qidrQueue.back().getID();

									// We assume the maturity is known, so the requests have the same maturity
//									qidrQueue.back().setNewTimeToMaturity(
//									DepositorMaturityParameter);

									qidrQueue.back().SetLinkedAgent(aux); //This is the partner that should receive the request

									qidrQueue.back().SetSenderAgent(this);
									qidrQueue.back().setType(11);

//									cout
//											<< "Sending IB deposit request, rate Parter is "
//											<< PartnersVectorDR.rbegin()->first
//											<< endl;
									qidrQueue.back().setInterestRate(
											PartnersVectorDR.rbegin()->first);
//												This is a copy just to avoid complicated communication between banks,
//												we need to know when a matched loan or deposit expires in a partners queue

									qidrQueue.back().setDesiredMatchingRate(
											PartnersVectorDR.rbegin()->first);

									UpdatePartnersSet(1);

									if ((*iter).get_t_to_maturity() < tleft) {
										tleft = (*iter).get_t_to_maturity();

										qt.push_front((*iter));
										// Just ensuring that the unmatched object is set as unmatched
										qidrQueue.back().setMatched(false);
										qt.front().setMatched(false);

//										cout << "Match betwween IDs: "
//												<< qt.front().getID() << " and "
//												<< qidrQueue.back().getID()
//												<< endl;

										//add the match to the maps
										newMatch(qt.front(), qidrQueue.back());

										matchMap1[qt.front()] =
												qidrQueue.back();
										matchMap2[qidrQueue.back()] =
												qt.front();
									}

									else {
										qt.push_back((*iter));

										// Just ensuring that the unmatched object is set as unmatched
										qidrQueue.back().setMatched(false);
										qt.back().setMatched(false);

//										cout << "Match betwween IDs: "
//												<< qt.back().getID() << " and "
//												<< qidrQueue.back().getID()
//												<< endl;

										//add the match to the maps
										newMatch(qt.back(), qidrQueue.back());

										matchMap1[qt.back()] = qidrQueue.back();
										matchMap2[qidrQueue.back()] = qt.back();
									}

									if (qidrQueue.back().get_t_to_maturity()
											< tleft) {
										tleft =
												qidrQueue.back().get_t_to_maturity();

										qt.push_front(qidrQueue.back());
										qt.front().setAsCopy();

										assert(
												qt.front().isCopy()
														&& qt.front().wasMatched()); // Just checking that copies are set as matched items

									}

									else {
										qt.push_back(qidrQueue.back());
										qt.back().setAsCopy();

										assert(
												qt.back().isCopy()
														&& qt.back().wasMatched()); // Just checking that copies are set as matched items

									}

								}
								UpdatePartnersSet(1);
								break;
							}
							// If best rate is in interbank market
							else if (PartnersVectorDR.rbegin()->first
									> (*iter3).first) {
//								log
//										<< "MATCHED WITH IB DEPOSIT, BACKLOG RATE OVER DESIRED"
//										<< endl;
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
								//log << "Sending IBDr Bank " << ID << endl;
								// Immediately send the request to interbank partner
								tleft = 0;

								// Add one deposit to the counter of active deposits
								qd++;

								//CSH++; The cash received is deposited in the partner bank using an IB deposit

								// Add to the IB deposit request counter
								qidr++;
								//Important: The bank doesn't know the time to maturity of the incoming deposit,
								//so it have to decide this for the IB request
								// the bank uses the mean, that's the best guess
								// This could be with new Depositor () in order to get a new ID
								qidrQueue.push_back(*iter);
								qidrQueue.back().setNewID();

								valueMatchedIBLoans[ValueFunction[LastState].rbegin()->first] =
										qidrQueue.back().getID();

								// For the sake of simplicity, the maturity is known
//								qidrQueue.back().setNewTimeToMaturity(
//								DepositorMaturityParameter);

								qidrQueue.back().SetLinkedAgent(aux); //This is the partner that should receive the request

								qidrQueue.back().SetSenderAgent(this);
								qidrQueue.back().setType(11);

//								cout
//										<< "Sending IB deposit request, rate Parter is "
//										<< PartnersVectorDR.rbegin()->first
//										<< endl;

								qidrQueue.back().setInterestRate(
										PartnersVectorDR.rbegin()->first);

								qidrQueue.back().setDesiredMatchingRate(
										PartnersVectorDR.rbegin()->first);

								//This is a copy just to avoid complicated communication between banks, we need to know when a matched loan or deposit expires in a partners queue

								UpdatePartnersSet(1);

								if ((*iter).get_t_to_maturity() < tleft) {
									tleft = (*iter).get_t_to_maturity();

									qt.push_front((*iter));

									// Just ensuring that the unmatched object is set as unmatched
									qidrQueue.back().setMatched(false);
									qt.front().setMatched(false);

//									cout << "Match betwween IDs: "
//											<< qt.front().getID() << " and "
//											<< qidrQueue.back().getID() << endl;

									//add the match to the maps
									newMatch(qt.front(), qidrQueue.back());
									matchMap1[qt.front()] = qidrQueue.back();
									matchMap2[qidrQueue.back()] = qt.front();
								}

								else {
									qt.push_back((*iter));

									// Just ensuring that the unmatched object is set as unmatched
									qidrQueue.back().setMatched(false);
									qt.back().setMatched(false);

//									cout << "Match betwween IDs: "
//											<< qt.front().getID() << " and "
//											<< qidrQueue.back().getID() << endl;

									//add the match to the maps
									newMatch(qt.back(), qidrQueue.back());
									matchMap1[qt.back()] = qidrQueue.back();
									matchMap2[qidrQueue.back()] = qt.back();
								}

								if (qidrQueue.back().get_t_to_maturity()
										< tleft) {
									tleft =
											qidrQueue.back().get_t_to_maturity();

									qt.push_front(qidrQueue.back());
									qt.front().setAsCopy();

									assert(
											qt.front().isCopy()
													&& qt.front().wasMatched()); // Just checking that copies are set as matched items
								}

								else {
									qt.push_back(qidrQueue.back());
									qt.back().setAsCopy();

									assert(
											qt.back().isCopy()
													&& qt.back().wasMatched()); // Just checking that copies are set as matched items

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
						if (LastAction.desiredMatchingInterestRate
								<= PartnersVectorDR.rbegin()->first) {
//							log
//									<< "MATCHED WITH IB DEPOSIT, BACKLOG RATE BELOW DESIRED"
//									<< endl;

							UpdatePartnerInfo();

							Bank* aux;
							//Select a random bank from those with the highest deposit rate
							aux =
									(PartnersVectorDR.rbegin()->second)[(int) (((PartnersVectorDR.rbegin()->second).size())
											* (rand() / (RAND_MAX + 1.0)))];

							assert(!BNetwork::IsBankrupt(aux));

							sendingIBDepositRequest = true;
							//log << "Sending IBDr Bank " << ID << endl;
							tleft = 0; //Immediately send the request to interbank partner
							//qidr.push_back(*iter);
							qd++;
							//CSH++; The cash received is deposited in the partner bank using an IB deposit
							qidr++;
							//Important: The bank doesn't know the time to maturity of the incoming deposit,
							//so it have to decide this for the IB request
							// the bank uses the mean, that's the best guess
							// This could be with new Depositor () in order to get a new ID

							qidrQueue.push_back(*iter);
							qidrQueue.back().setNewID();

							valueMatchedIBLoans[ValueFunction[LastState].rbegin()->first] =
									qidrQueue.back().getID();

							// For the sake of simplicity, banks know the maturity
//							qidrQueue.back().setNewTimeToMaturity(
//							DepositorMaturityParameter);

							qidrQueue.back().SetLinkedAgent(aux); //This is the partner that should receive the request

							qidrQueue.back().SetSenderAgent(this);
							qidrQueue.back().setType(11);

//							cout
//									<< "Sending IB deposit request, rate Parter is "
//									<< PartnersVectorDR.rbegin()->first << endl;

							qidrQueue.back().setInterestRate(
									PartnersVectorDR.rbegin()->first);

							qidrQueue.back().setDesiredMatchingRate(
									PartnersVectorDR.rbegin()->first);

							//This is a copy just to avoid complicated communication between banks, we need to know when
							//a matched loan or deposit expires in a partners queue

							UpdatePartnersSet(1);

							if ((*iter).get_t_to_maturity() < tleft) {
								tleft = (*iter).get_t_to_maturity();

								qt.push_front((*iter));

								// Just ensuring that the unmatched object is set as unmatched
								qidrQueue.back().setMatched(false);
								qt.front().setMatched(false);

//								cout << "Match betwween IDs: "
//										<< qt.front().getID() << " and "
//										<< qidrQueue.back().getID() << endl;

								//add the match to the maps
								newMatch(qt.front(), qidrQueue.back());
								matchMap1[qt.front()] = qidrQueue.back();
								matchMap2[qidrQueue.back()] = qt.front();

								assert(qidrQueue.back().wasMatched());

							}

							else {
								qt.push_back((*iter));

								// Just ensuring that the unmatched object is set as unmatched
								qidrQueue.back().setMatched(false);
								qt.back().setMatched(false);

//								log << "Bank " << ID  << " Match betwween IDs: "
//										<< qt.back().getID() << " and "
//										<< qidrQueue.back().getID() << endl;

								//add the match to the maps
								newMatch(qt.back(), qidrQueue.back());

								matchMap1[qt.back()] = qidrQueue.back();
								matchMap2[qidrQueue.back()] = qt.back();

								assert(matchMap1.count(qt.back()) > 0);
								assert(matchMap2.count(qidrQueue.back()) > 0);

							}

							if (qidrQueue.back().get_t_to_maturity() < tleft) {
								tleft = qidrQueue.back().get_t_to_maturity();

								qt.push_front(qidrQueue.back());
								qt.front().setAsCopy();

								assert(
										qt.front().isCopy()
												&& qt.front().wasMatched()); // Just checking that copies are set as matched items

							}

							else {
								qt.push_back(qidrQueue.back());
								qt.back().setAsCopy();

								assert(
										qt.back().isCopy()
												&& qt.back().wasMatched()); // Just checking that copies are set as matched items

							}

							UpdatePartnersSet(1);
							break;
						}
						//There is not match in the interbank market
						else {
							//Add this new deposit to the backlog of unmatched deposits
							//log << "NO MATCH, ADDING TO BACKLOG" << endl;

							(*iter).setDesiredMatchingRate(
									LastAction.desiredMatchingInterestRate);

							if ((*iter).get_t_to_maturity() < tleft) {
								tleft = (*iter).get_t_to_maturity();
								//	qd.push_front(*iter);
								qd++;
								CSH++;
								// Just ensuring that is added as unmathed
								(*iter).setMatched(false);

								qt.push_front((*iter));
								UMDeposits[LastAction.desiredMatchingInterestRate].push_back(
										qt.front());

							} else {
								//qd.push_back(*iter);
								qd++;
								CSH++;

								// Just ensuring that is added as unmathed
								(*iter).setMatched(false);

								qt.push_back((*iter));
								UMDeposits[LastAction.desiredMatchingInterestRate].push_back(
										qt.back());

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
//				log << time << " INCOMING IB DEPOSIT FROM "
//						<< (*iter).get_IDborrower() << " TO "
//						<< (*iter).GetLinkedAgentID() << endl;

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
				(*iter).setInterestRate(IBDepositRate);
				(*iter).setDesiredMatchingRate(IBDepositRate);

				if ((*iter).get_t_to_maturity() < tleft) {
					tleft = (*iter).get_t_to_maturity();

					//log << "ADDING IB DEPOSIT TO ACCEPTED IB DEPOSITS" << endl;

					//The bank always accept an incoming IB deposit
					qida++;

					qt.push_front(*iter);

					if (IBDepositRate != CDepositRate) {
						updateBacklogs();

//						cout << "Bank ID is " << ID << " Time is " << Bank::time
//								<< " Bag iteration " << bagIterationnumber
//								<< " ID "
//								<< UMLoans.begin()->second.front().getID()
//								<< " Type "
//								<< UMLoans.begin()->second.front().getType()
//								<< " size UMLoans begin queue "
//								<< (UMLoans.begin()->second).size() << endl;

						qt.front().SetMatchedWith(
								(UMLoans.begin()->second.front().getID()));

						// Just ensuring that the unmatched object is set as unmatched
						(UMLoans.begin()->second.front()).setMatched(false);
						qt.front().setMatched(false);

						newMatch(qt.front(), (UMLoans.begin()->second.front()));

						matchMap1[qt.front()] =
								(UMLoans.begin()->second.front());
						matchMap2[(UMLoans.begin()->second.front())] =
								qt.front();

						//Update the interest rate
						matchMap1[qt.front()].setInterestRate(IBDepositRate);

						// Remove from unmatched queue
						(UMLoans.begin()->second).pop_front();
//						if ((UMLoans.begin()->second).empty())
//							cout << "Queue empty" << endl;
//						else {
//							cout
//									<< "Element removed from UMLoans, next element's ID is "
//									<< UMLoans.begin()->second.front().getID()
//									<< " size UMLoans begin queue "
//									<< (UMLoans.begin()->second).size() << endl;
//						}
						updateBacklogs();
					} else {
//						 incoming IB deposit is unmatched
						// set desired rate
						(qt.front()).setDesiredMatchingRate(
								LastAction.desiredMatchingInterestRate);
						// add to backlog of unmatched loans
						UMDeposits[LastAction.desiredMatchingInterestRate].push_back(
								qt.front());
					}

				} else {
					//qida.push_back(*iter);
					//log << "ADDING IB DEPOSIT TO ACCEPTED IB DEPOSITS" << endl;
					qida++;
					qt.push_back(*iter);
					if (IBDepositRate != CDepositRate) {

						//if(find(qt.begin(), qt.end(), *(UMLoans.begin()->second.front())) != qt.end()) cout << "NOT FOUND"<<endl;

						qt.back().SetMatchedWith(
								(UMLoans.begin()->second.front().getID()));

						(UMLoans.begin()->second.front()).setMatched(false);
						qt.back().setMatched(false);

						newMatch(qt.back(), (UMLoans.begin()->second.front()));

						matchMap1[qt.back()] =
								(UMLoans.begin()->second.front());
						matchMap2[(UMLoans.begin()->second.front())] =
								qt.back();

						// set interest rate
						matchMap1[qt.back()].setInterestRate(IBDepositRate);

						// Remove from unmatched queue
						UMLoans.begin()->second.pop_front();

					} else {
						//				 incoming IB deposit is unmatched
						// set desired rate
						(qt.back()).setDesiredMatchingRate(
								LastAction.desiredMatchingInterestRate);
						// add to backlog of unmatched loans
						UMDeposits[LastAction.desiredMatchingInterestRate].push_back(
								qt.back());

					}
				}

				UpdatePartnersSet(1);
				break;

			}// END If opt decision is to accept the incoming IB deposit or customer deposit
		}							// END Customer deposit or IB deposit case

///////////////////////////////////////////////////////////////////////////CUSTOMER LOAN OR IB LOAN/////////////////////////////////////////////////////////////////////////////////////////

		//if( ( (*iter).getType() == 2 ) | ( (*iter).getType() == 21) ) {
		else {
//			log << "external function incoming customer loan or IB loan"
//					<< endl;

			State* tempState;
			updateState( (*iter));
			tempState = new State(LastState);

			assert(*tempState==LastState);

			typename std::set<Action>::const_iterator iter2 =
					actionsSet.begin();
			//log << "Setting Initial belief for deposit" << endl;
			if (stateSet.find(tempState) == stateSet.end()) {
//			cerr << "STATE NOT FOUND" << endl;
				for (; iter2 != actionsSet.end(); iter2++) {
					// In this step is useful to remember that if LastState does not exist in h, a new key will be created
//				if (h[LastState].find(*iter2) == h[LastState].end()) {
					SetInitialValue(LastState, *iter2,
							a.uniform(-0.001, 0.001));
					h[LastState][*iter2] = 0;
//				}
				}
			}

			if (isTrembling)
				LastAction = *actionsVector[(int) (actionsVector.size()
						* (rand() / (RAND_MAX + 1.0)))];
			else
				LastAction = ValueFunction[LastState].rbegin()->second; // This is the best action the bank could take according to its value function

			tempStrategy = new Strategy(LastState, LastAction,
					ValueFunction2[LastState][LastAction]);

			// Save strategy
			saveToStrategyMap((*iter).getID());
			stateSet.insert(tempState);

			// Increment the counter map h
			h[LastState][LastAction] = h[LastState][LastAction] + 1; // We use this map because this is the actual number of visits. The number h saved in the loanStrategyMap is the number of visits when we took the action

			//Accept incoming IB loan or customer loan
			if (LastAction.accept && CheckCashReserveRequirements()) { //If opt decision is to accept the incoming IB loan or customer loan
//			if (true) {	//If opt decision is to accept the incoming IB loan or customer loan
//			if (LastAction.accept) { //If opt decision is to accept the incoming IB loan or customer loan
				if (!CheckCashReserveRequirements()) {
					cumulativeObjectValue[(*iter).getID()] = -2;
				}

				if ((*iter).getType() == 2) { //If customer loan request

					//log << time << " INCOMING CUSTOMER LOAN" << endl;
					typename std::map<int, std::deque<Loan> >::iterator iter3 =
							UMDeposits.begin();

					//log << UMLoans.size() << endl;

					for (int n = 0; (iter3 != UMDeposits.end()) | (n == 0);
							iter3++, n++) {
						//log << "IN FOR" << endl;

						//If desired matching rate is higher than the lowest rate in unmatched loans
						if (iter3 != UMDeposits.end()) {
							if ((LastAction.desiredMatchingInterestRate
									>= (*iter3).first)
									&& ((*iter3).second.size() != 0)) {

								//If Partners rate are higher than in the own backlog, the bank use BL
								if ((PartnersVectorLR.begin()->first)
										> (*iter3).first) {

									(*iter3).second.front().setInterestRate(
											(*iter3).first);

									//Set as matched loan with the unmatched deposit
									(*iter3).second.front().SetMatchedWith(
											(*iter).getID());
									(*iter).SetMatchedWith(
											(*iter3).second.front().getID()); //Respect priority

									if ((*iter).get_t_to_maturity() < tleft) {
										tleft = (*iter).get_t_to_maturity();
										ql++;
										CSH--;
										qt.push_front((*iter));
										// Just ensuring that the unmatched object is set as unmatched
										(*iter3).second.front().setMatched(
												false);
										qt.front().setMatched(false);

										newMatch(qt.front(),
												(*iter3).second.front());

										//add the match to the maps
										matchMap1[qt.front()] =
												(*iter3).second.front();
										matchMap2[(*iter3).second.front()] =
												qt.front();

									}

									else {
										ql++;
										CSH--;
										qt.push_back((*iter));

										// Just ensuring that the unmatched object is set as unmatched
										(*iter3).second.front().setMatched(
												false);
										qt.back().setMatched(false);

										//add the match to the maps
										newMatch(qt.back(),
												(*iter3).second.front());

										matchMap1[qt.back()] =
												(*iter3).second.front();
										matchMap2[(*iter3).second.front()] =
												qt.back();

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

									(*iter3).second.front().setInterestRate(
											(*iter3).first);
									//match with own backlog
//									log
//											<< "MATCHED RANDOMLY WITH BACKLOG OR IB LOAN"
//											<< endl;

									if (randomMatch.uniform(0, 1) < 0.5) {

										//Set as matched with the unmatched deposit

										//log << (*iter3).second.front()->getID() << " " << (*iter).getID() << endl;
										//log << " " << (*iter).getID() << endl;
//										if ((*iter3).second.front() == NULL)
//											throw invalid_argument(
//													"NULL POINTER");

										(*iter3).second.front().SetMatchedWith(
												(*iter).getID());
										(*iter).SetMatchedWith(
												(*iter3).second.front().getID());

										if ((*iter).get_t_to_maturity()
												< tleft) {
											tleft = (*iter).get_t_to_maturity();
											ql++;
											CSH--;
											qt.push_front((*iter));

											// Just ensuring that the unmatched object is set as unmatched
											(*iter3).second.front().setMatched(
													false);
											qt.front().setMatched(false);

											//add the match to the maps
											newMatch(qt.front(),
													(*iter3).second.front());
											matchMap1[qt.front()] =
													(*iter3).second.front();
											matchMap2[(*iter3).second.front()] =
													qt.front();

										}

										else {
											ql++;
											CSH--;
											qt.push_back((*iter));

											// Just ensuring that the unmatched object is set as unmatched
											(*iter3).second.front().setMatched(
													false);

											qt.back().setMatched(false);

											//add the match to the maps
											newMatch(qt.back(),
													(*iter3).second.front());

											matchMap1[qt.back()] =
													(*iter3).second.front();
											matchMap2[(*iter3).second.front()] =
													qt.back();

										}
										//Delete from unmatched deposit queue
										(*iter3).second.pop_front();

									}
									// match with interbank deposit, send request
									else {
//										log
//												<< "3 MATCHED RANDOMLY WITH BACKLOG OR IB LOAN"
//												<< endl;
										UpdatePartnerInfo();

										Bank* aux;
										//Select a random bank from those with the lowest loan rate
										aux =
												(PartnersVectorLR.begin()->second)[(int) (((PartnersVectorLR.begin()->second).size())
														* (rand()
																/ (RAND_MAX
																		+ 1.0)))];

										assert(!BNetwork::IsBankrupt(aux));
										//(*iter).setNewID();
										sendingIBLoanRequest = true;
//										log << "3 Sending IBLr Bank " << ID
//												<< endl;
										tleft = 0; //Immediately send the request to interbank partner
										qilr++;
										ql++;
										//CSH--; The cash requested is borrowed from the partner bank using an IB loan request
										// The loan's time to maturity is known
//////

										qilrQueue.push_back(*iter);

										qilrQueue.back().setNewID();

										qilrQueue.back().SetLinkedAgent(aux); //This is the partner that should receive the request
										qilrQueue.back().SetSenderAgent(this);
										qilrQueue.back().setType(21);
										qilrQueue.back().setInterestRate(
												PartnersVectorLR.begin()->first);
										qilrQueue.back().setDesiredMatchingRate(
												PartnersVectorLR.begin()->first);
										// This is a copy just to avoid complicated communication between banks,
										// we need to know when a matched loan or deposit expires in a partners queue

										if ((*iter).get_t_to_maturity()
												< tleft) {
											tleft = (*iter).get_t_to_maturity();

											qt.push_front((*iter));

											//add the match to the maps

											// Just ensuring that the unmatched object is set as unmatched

											qilrQueue.back().setMatched(false);
											qt.front().setMatched(false);

											newMatch(qt.front(),
													qilrQueue.back());
											matchMap1[qt.front()] =
													qilrQueue.back();
											matchMap2[qilrQueue.back()] =
													qt.front();
										}

										else {
											qt.push_back((*iter));

											qilrQueue.back().setMatched(false);
											qt.back().setMatched(false);

											//add the match to the maps
											newMatch(qt.back(),
													qilrQueue.back());

											matchMap1[qt.back()] =
													qilrQueue.back();
											matchMap2[qilrQueue.back()] =
													qt.back();
										}

										if (qilrQueue.back().get_t_to_maturity()
												< tleft) {
											tleft =
													qilrQueue.back().get_t_to_maturity();

											qt.push_front(qilrQueue.back());
											qt.front().setAsCopy();

											assert(
													qt.front().isCopy()
															&& qt.front().wasMatched()); // Just checking that copies are set as matched items

										}

										else {
											qt.push_back(qilrQueue.back());
											qt.back().setAsCopy();

											assert(
													qt.back().isCopy()
															&& qt.back().wasMatched()); // Just checking that copies are set as matched items

										}

									}
									UpdatePartnersSet(2);
									break;
								}
								// If best rate is in interbank market

								else if (PartnersVectorLR.begin()->first
										< (*iter3).first) {

//
//									log
//											<< "2 MATCHED WITH IB LOAN, BACKLOG RATE BELOW DESIRED"
//											<< endl;
									UpdatePartnerInfo();

									Bank* aux;
									//Select a random bank from those with the lowest loan rate
									aux =
											(PartnersVectorLR.begin()->second)[(int) (((PartnersVectorLR.begin()->second).size())
													* (rand() / (RAND_MAX + 1.0)))];

									assert(!BNetwork::IsBankrupt(aux));

									(*iter).SetLinkedAgent(aux); //This is the partner that should receive the request

									(*iter).SetSenderAgent(this);

									sendingIBLoanRequest = true;
									//log << "2 Sending IBLr Bank " << ID << endl;
									tleft = 0; //Immediately send the request to interbank partner
									qilr++;
									ql++;
									//CSH--; The cash requested is borrowed from the partner bank using an IB loan request
									qilrQueue.push_back(*iter);
									// We need to check if this request was accepted
									qilrQueue.back().setType(21);
									qilrQueue.back().setNewID();
									qilrQueue.back().setInterestRate(
											PartnersVectorLR.begin()->first);
									qilrQueue.back().setDesiredMatchingRate(
											PartnersVectorLR.begin()->first);

									//This is a copy just to avoid complicated communication between banks, we need to know when a matched loan or deposit expires in a partners queue

									UpdatePartnersSet(2);

									if ((*iter).get_t_to_maturity() < tleft) {
										tleft = (*iter).get_t_to_maturity();

										qt.push_front((*iter));

										qilrQueue.back().setMatched(false);
										qt.front().setMatched(false);

										//add the match to the maps
										newMatch(qt.front(), qilrQueue.back());

										matchMap1[qt.front()] =
												qilrQueue.back();
										matchMap2[qilrQueue.back()] =
												qt.front();
									}

									else {
										qt.push_back((*iter));

										qilrQueue.back().setMatched(false);
										qt.back().setMatched(false);

										//add the match to the maps
										newMatch(qt.back(), qilrQueue.back());

										matchMap1[qt.back()] = qilrQueue.back();
										matchMap2[qilrQueue.back()] = qt.back();
									}

									if (qilrQueue.back().get_t_to_maturity()
											< tleft) {
										tleft =
												qilrQueue.back().get_t_to_maturity();

										qt.push_front(qilrQueue.back());
										qt.front().setAsCopy();

										assert(
												qt.front().isCopy()
														&& qt.front().wasMatched()); // Just checking that copies are set as matched items

									}

									else {
										qt.push_back(qilrQueue.back());
										qt.back().setAsCopy();

										assert(
												qt.back().isCopy()
														&& qt.back().wasMatched()); // Just checking that copies are set as matched items

									}

									UpdatePartnersSet(2);
									break;

								}

							}
						} //End of case when both own backlog rate and interbank rate are lower than desired matching rate

						//Desired matching rate is lower than in the own backlog

						else {
							// The bank have to look in the interbank market banasd
							// If rate in the interbank is lower than desired matching rate, there is a match
							if (LastAction.desiredMatchingInterestRate
									>= PartnersVectorLR.rbegin()->first) {

//								log
//										<< "1 MATCHED WITH IB LOAN, BACKLOG RATE OVER DESIRED"
//										<< endl;
								UpdatePartnerInfo();

								Bank* aux;
								//Select a random bank from those with the lowest loan rate
								aux =
										(PartnersVectorLR.begin()->second)[(int) (((PartnersVectorLR.begin()->second).size())
												* (rand() / (RAND_MAX + 1.0)))];

								assert(!BNetwork::IsBankrupt(aux));

								(*iter).SetLinkedAgent(aux);//This is the partner that should receive the request
								(*iter).SetSenderAgent(this);

								sendingIBLoanRequest = true;

								tleft = 0;//Immediately send the request to interbank partner
								qilr++;
								ql++;
								//CSH--; The cash requested is borrowed from the partner bank using an IB loan request
								qilrQueue.push_back(*iter);
								qilrQueue.back().setNewID();
//								log << "Sending IBLr 1 bank " << ID
//										<< " Loan ID "
//										<< qilrQueue.back().getID() << endl;

								qilrQueue.back().setType(21);
								qilrQueue.back().setInterestRate(
										PartnersVectorLR.begin()->first);
								qilrQueue.back().setDesiredMatchingRate(
										PartnersVectorLR.begin()->first);

								// This was misplaced! qt.push_back(*iter);

								// The requested object is a copied just to avoid complicated communication between banks, we need to know when
								//a matched loan or deposit expires in a partners queue

								UpdatePartnersSet(2);

								if ((*iter).get_t_to_maturity() < tleft) {
									tleft = (*iter).get_t_to_maturity();

									qt.push_front((*iter));

									qilrQueue.back().setMatched(false);
									qt.front().setMatched(false);
									//add the match to the maps
									newMatch(qt.front(), qilrQueue.back());

									matchMap1[qt.front()] = qilrQueue.back();
									matchMap2[qilrQueue.back()] = qt.front();
								}

								else {
									qt.push_back((*iter));

									qilrQueue.back().setMatched(false);
									qt.back().setMatched(false);

									//add the match to the maps
									newMatch(qt.back(), qilrQueue.back());

									assert(qilrQueue.back().wasMatched());

									matchMap1[qt.back()] = qilrQueue.back();
									matchMap2[qilrQueue.back()] = qt.back();
								}

								if (qilrQueue.back().get_t_to_maturity()
										< tleft) {
									tleft =
											qilrQueue.back().get_t_to_maturity();

									qt.push_front(qilrQueue.back());
									qt.front().setAsCopy();

									assert(
											qt.front().isCopy()
													&& qt.front().wasMatched()); // Just checking that copies are set as matched items

								}

								else {
									qt.push_back(qilrQueue.back());
									qt.back().setAsCopy();

									//qt.back().setMatched(true);

									assert(qt.back().wasMatched());

									assert(
											qt.back().isCopy()
													&& qt.back().wasMatched()); // Just checking that copies are set as matched items

								}

								UpdatePartnersSet(2);

								break;
							}
							//There is not match in the interbank market
							else {
								//Add this new loan to the backlog of unmatched loans
								//log << "NO MATCH, ADDING TO BACKLOG" << endl;

								(*iter).setDesiredMatchingRate(
										LastAction.desiredMatchingInterestRate);

								if ((*iter).get_t_to_maturity() < tleft) {
									tleft = (*iter).get_t_to_maturity();
									ql++;

									// Just ensuring that is added as unmathed
									(*iter).setMatched(false);

									qt.push_front((*iter));
									CSH--;
									UMLoans[LastAction.desiredMatchingInterestRate].push_back(
											qt.front());

								} else {
									ql++;

									// Just ensuring that is added as unmathed
									(*iter).setMatched(false);

									qt.push_back((*iter));
									CSH--;
									UMLoans[LastAction.desiredMatchingInterestRate].push_back(
											qt.back());

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

					cerr << "Bank " << ID << " accepting IBLoan from bank "
							<< (*iter).GetSenderAgentID() << " ID "
							<< (*iter).getID() << " qila = " << qila << endl;

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

//					log << time << " INCOMING IB LOAN FROM "
//							<< (*iter).get_IDlender() << " TO "
//							<< (*iter).GetLinkedAgentID() << endl;
					qila++;
					CSH--;

					(*iter).setInterestRate(IBLoanRate);
					(*iter).setDesiredMatchingRate(IBLoanRate);
					valueMatchedIBLoans[ValueFunction[LastState].rbegin()->first] =
							(*iter).getID();

					if ((*iter).get_t_to_maturity() < tleft) {
						tleft = (*iter).get_t_to_maturity();

						qt.push_front(*iter);

						// If the backlog is empty, the bank ask the CB
						if (IBLoanRate != RateLoanCB) {
							cerr << IBLoanRate << endl;
							cerr << RateLoanCB << endl;

							qt.front().SetMatchedWith(
									(UMDeposits.rbegin()->second.front().getID()));

							(UMDeposits.rbegin()->second.front()).setMatched(
									false);
							qt.front().setMatched(false);

							newMatch(qt.front(),
									(UMDeposits.rbegin()->second.front()));

							matchMap1[qt.front()] =
									(UMDeposits.rbegin()->second.front());
							matchMap2[(UMDeposits.rbegin()->second.front())] =
									qt.front();

							matchMap1[qt.front()].setInterestRate(IBLoanRate);

							// Remove from unmatched queue
							UMDeposits.rbegin()->second.pop_front();

						} else {
							// incoming IB loan is unmatched
							// set desired rate
							(qt.front()).setDesiredMatchingRate(
									LastAction.desiredMatchingInterestRate);
							// add to backlog of unmatched loans
							UMLoans[LastAction.desiredMatchingInterestRate].push_back(
									qt.front());

						}

					} else {
						qt.push_back(*iter);

						if (IBLoanRate != RateLoanCB) {
//							qt.push_back(*iter);
							qt.back().SetMatchedWith(
									(UMDeposits.rbegin()->second.front().getID()));

							(UMDeposits.rbegin()->second.front()).setMatched(
									false);
							qt.back().setMatched(false);

							newMatch(qt.back(),
									(UMDeposits.rbegin()->second.front()));

							matchMap1[qt.back()] =
									(UMDeposits.rbegin()->second.front());
							matchMap2[(UMDeposits.rbegin()->second.front())] =
									qt.back();

							matchMap1[qt.back()].setInterestRate(IBLoanRate);

							// Remove from unmatched queue
							UMDeposits.rbegin()->second.pop_front();

						} else {
							// incoming IB loan is unmatched
							// set desired rate
							(qt.back()).setDesiredMatchingRate(
									LastAction.desiredMatchingInterestRate);
							// add to backlog of unmatched loans
							UMLoans[LastAction.desiredMatchingInterestRate].push_back(
									qt.back());
						}

					}
					UpdatePartnersSet(2);
					//log << "Size qt " << qt.size() << endl;
					break;
				}
				//log << "Size qt " << qt.size() << endl;

			}// END If opt decision is to accept the incoming IB loan or customer loan

			else {// BEGIN if Optimal decision is to reject the incoming IB Loan or customer loan
				  // Simply cancel the incoming loan that the requester added to its queue of active objects. This bank that is acting as potential lender does nothing with the loan.
				  // It only sends the cancel signal to the network, activating the transition method so the network can tell to the bank asking for the loan that its request was rejected.
				  // All this information flow take place in dt = 0
				  // If the rejected loan comes from a customer, then do nothing.
				if ((*iter).getType() == 21) {
//					log << "Rejecting IBLr with ID " << (*iter).getID()
//							<< " in bank " << ID << " from "
//							<< (*iter).GetSenderAgentID() << endl;
					qila++;
					CSH--;
					qt.push_back((*iter));
					qt.back().setNewTimeToMaturity(DBL_MAX);
					rejectLoanRequest((*iter).getID(),
							(*iter).GetSenderAgentID()); //Send the rejection signal to the network
				}

			} // END if Optimal decision is to reject the incoming IB Loan or customer loan
		}									// END Customer loan or IB loan case

		bagIterationnumber++;
		isTrembling = false;
	}									//BAG for incoming events

//log << "END DELTA EXT in BANK " << ID << endl;
}									//END DELTA_EXT

void Bank::delta_conf(const adevs::Bag<Loan>& xb) {
	delta_int();
	delta_ext(0.0, xb);
}
void Bank::output_func(adevs::Bag<Loan>& yb) {
//log << "START OUTPUT FUNC in BANK " << ID << endl;

//log << "output function BANK " << ID << endl;
//			log << "Size qt " << qt.size() << endl;
//			log << "Size qilr " << qilrQueue.size() << endl;
//			log << "Size qidr " << qidrQueue.size() << endl;

	if (sendingIBDepositRequest) {
		qidrQueue.back().setFromIB(true);
		yb.insert(qidrQueue.back());
		//log << "output function, sending IB Deposit Request" << endl;
	}

	else if (sendingIBLoanRequest) {
//		log << "sending IB loan Request, size qlirQueue " << qilrQueue.size()
//				<< endl;
////				if( qilrQueue.back() == NULL ) throw invalid_argument("NULL POINTER");
//			log << qilrQueue.back() << endl;
		qilrQueue.back().setFromIB(true);
		yb.insert(qilrQueue.back());
//		log << "output function, sending IB Loan Request" << endl;
	}

//if( ( qt.front().getType()==1 ) | ( qt.front().getType()==2 ) | ( qt.front().getType()==11 ) )
	else if (!qt.front().isCopy()) {

//if( qt.empty() ) throw invalid_argument("EMPTY QT");
//if(qt.front().getType() ==1 | qt.front().getType() ==2)  {
		qt.front().SetAsExpired();
		yb.insert(qt.front());
//}

//		if (qt.front().getType() == 1)
//			log << "output function, expired deposit" << endl;
//		else if (qt.front().getType() == 2)
//			log << "output function, expired loan" << endl;
//		else if (qt.front().getType() == 11)
//			log << "output function, expired IB deposit" << endl;
//		//if (qt.front().getType() == 21)
//		else
//			log << "output function, expired IB loan" << endl;

	}

//if(sendingIBDepositRequest){yb.insert(qidr.back());log << "output function, sending IB Deposit Request" << endl;}

//if(sendingIBLoanRequest){yb.insert(qilr .back());log << "output function, sending IB Loan Request" << endl;}

//log << "END OUTPUT FUNC in BANK " << ID << endl;

}
double Bank::ta() {
	return tleft;
}

bool Bank::model_transition() {
//transition activated if insolvent
//	if (checkInsolvent() || cancelingLoan || rejectingIBloanrequest)
//		log << "Transition activated " << ID << endl;
	return checkInsolvent() || cancelingLoan || rejectingIBloanrequest;
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
//	if (equity <= 0) {
//		log << ID << " Is insolvent" << endl;
//		return true;
//	} else
//		return false;
	return equity <= 0;
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

void Bank::SetInitialValue(const State& state, const Action& action, double v) {
	ValueFunction[state][v] = action;
	ValueFunction2[state][action] = v;
}

// Function to set continuation value (w)
void Bank::UpdateW(Strategy* S, double CurrentV) {
// Here we update the belief regarding a strategy-state pair
//	cout << "Updating continuation value" << endl;
	Action action = S->getAction();
	State state = S->getState();
	double V0 = ValueFunction2[state][action];
	int nh = S->getH();
// I prefer to use nh instead of nh%100
	cout << "Time is: " << time << " n is: " << nh << endl;
	ValueFunction2[state][action] = ValueFunction2[state][action] * (nh)
			/ ((nh) + 1) + CurrentV / ((nh) + 1);

	ValueFunction[state][ValueFunction2[state][action]] = action;

	double dV = abs((ValueFunction2[state][action] - V0) / V0);
//	double dV = ((ValueFunction2[state][action] - V0) / V0);

	if (V0 == 0)
		dV = 0;

	S->setdV(dV);
	int maxH = 0;
	WdV = 0;
	totalH = 0;
	typename std::map<int, Strategy*>::iterator it = loanStrategyMap.begin();
// I added 1 to H;
	for (; it != loanStrategyMap.end(); ++it) {
		totalH += ((*it).second->getH() + 1);
		WdV += (((*it).second->getH()) + 1) * ((*it).second->getdV());
		maxH = max(maxH, (*it).second->getH() + 1);
	}

	assert(totalH != 0);

	WdV = WdV / totalH;

	cout << "Weighted dV = " << WdV << endl;
//			<< " Most frequent strategy has frequency = " << maxH
//			<< " Total H = " << totalH << " Size Strategy Map = "
//			<< loanStrategyMap.size() << endl;

	probTrembling = min(sqrt(abs(WdV)) * probTrembling, probTrembling);

}

int Bank::GetNumberStatesVisited() {
//	return h.size();
	return stateSet.size();
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
	return PartnersVector.size();
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
	LqBnch = beta2 * (qd + (qida + qilr) + qcblf);
}

bool Bank::CheckCashReserveRequirements() {
	return CSH > (double) beta1 * (qd + (qida + qilr) + qcblf);
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

	typename set<Bank*>::iterator iter = PartnersVector.begin();
	for (; iter != PartnersVector.end();) {

		if (BNetwork::IsBankrupt(*iter)) {
//			log << "REPLACING BANKRUPT BANK PARTNER OF BANK " << ID << endl;

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
	typename set<Bank*>::iterator iter = PartnersVector.begin();
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

string Bank::printOutput() {

//		generalInfo << "Time,";
//		generalInfo << "Bank ID,";
//		generalInfo << "Cash,";
//		generalInfo << "Customer Loans,";
//		generalInfo << "Customer Deposits,";
//		generalInfo << "IB Loans,";
//		generalInfo << "IB Deposits,";
//		generalInfo << "CB Lending Facility,";
//		generalInfo << "Equity,"
//		generalInfo << "Visible Partners" << endl;
//		generalInfo << "Total H" << endl;
//		generalInfo << "Number States" << endl;
//		generalInfo << "Last Value Function" << endl;
//		generalInfo << "Rd" << endl;
//		generalInfo << "Rl" << endl;

	string aux;

	ostringstream temp;

	temp << time << ",";
	temp << ID << ",";
	temp << CSH << ",";
	temp << ql << ",";
	temp << qd << ",";

//	temp << qila + qidr << ",";
//	temp << qida + qilr << ",";

	temp << qila << ",";
	temp << qidr << ",";
	temp << qida << ",";
	temp << qilr << ",";

	temp << LastState.interbankLiabilities << ",";

	temp << illiquidAssetPrice << ",";

	temp << qcblf << ",";
	temp << equity << ",";
	temp << printPartners() << ",";
	temp << totalH << ",";
	temp << GetNumberStatesVisited() << ",";
	temp << LastStateActionValue() << ",";
	temp << IBDepositRate << ",";
	temp << IBLoanRate << endl;

//	variablesDebugging << "START IN BANK "<< ID <<" " << time << "," << ValueFunction2[state][action] << ","
//				<< totalH << "END IN BANK "<< ID << " " <<endl;

	return aux = temp.str();

}

void Bank::updateState(Loan loan) {

//// Simpler version
//	if (UMLoans.empty())
//		IBDepositRate = RateDepositCB;
//	else
//		IBDepositRate = UMLoans.begin()->first;
//
//	if (UMDeposits.empty())
//		IBLoanRate = RateLoanCB;
//	else
//		IBLoanRate = UMDeposits.rbegin()->first;
//
//// The state is defined by
////		int loanOrDeposit;
////		int cash;
////		int interbankLiabilities;
////		int rateI;
////		int rateO;
//
//	if (loan.getType() == 1 || loan.getType() == 12)
//		LastState.loanOrDeposit = 1;
//	else
//		LastState.loanOrDeposit = 0;
//
//	LastState.cash = CSH;
//
//	LastState.interbankLiabilities = qida + qilr - qidr - qila;
//
//	LastState.rateI = PartnersVectorDR.rbegin()->first;
//
//	LastState.rateO = PartnersVectorLR.begin()->first;

// Complete version

		LastState.interbankLiabilities = qida + qilr - qidr - qila; // This will be removed later on

	if (UMLoans.empty())
		IBDepositRate = RateDepositCB;
	else
		IBDepositRate = UMLoans.begin()->first;

	if (UMDeposits.empty())
		IBLoanRate = RateLoanCB;
	else
		IBLoanRate = UMDeposits.rbegin()->first;

	if (loan.getType() == 1 || loan.getType() == 12)
		LastState.loanOrDeposit = 1;
	else
		LastState.loanOrDeposit = 0;

//	2) rate of the loan or deposit (this will give me if it is a costmer loan-depost or interbank loan deposit)
	LastState.interestRate = loan.getInterestRate();
//					3)Rating of the bank sending the loan-deposit from the interbank market (i.e. 0 in the case of a Customer loan-deposit)
// This is the lagged rating
	LastState.ratingSender = LaggedAgencyRatings[loan.GetSenderAgent()];
//					4)  your rating
	LastState.myRating = ActualAgencyRatings[this];
//					5) partners ratings (with a lag)
	LastState.partnersRatings.clear();

	typename set<Bank*>::iterator iter;
	for (iter = PartnersVector.begin(); iter != PartnersVector.end(); iter++)
		LastState.partnersRatings.push_back(LaggedAgencyRatings[*iter]);

//					6) your bid
	LastState.bid = IBDepositRate;
//			//					7) your ask
	LastState.ask = IBLoanRate;
//			int ask;
//		//					8) partners bid
	LastState.partnersBid.clear();
	LastState.partnersAsk.clear();

	typename map<int, std::vector<Bank*> >::iterator iterMap;
	for (iterMap = PartnersVectorDR.begin(); iterMap != PartnersVectorDR.end();
			iterMap++)
		LastState.partnersBid.push_back((*iterMap).first);

//		//					9) partner ask
	for (iterMap = PartnersVectorLR.begin(); iterMap != PartnersVectorLR.end();
			iterMap++)
		LastState.partnersAsk.push_back((*iterMap).first);

//		//					10) your liquidity benchmark (only above or below).
	LastState.LqBench = CSH >= LqBnch;
//					11) depth backlog at the ask if you receive a deposit (depth backlog at the bid if you receive a loan).
//					12) TOTAL depth backlog on the upper side if it is a deposit (TOTAL depth backlog on the lower side if it is a loan).

	if ((loan.getType() == 1) | (loan.getType()== 11)) {
		if (!UMLoans.empty())
			LastState.deepBackLog = (UMLoans.rbegin()->second).size();
		else
			LastState.deepBackLog = 0;

		LastState.totaldeepBackLog = 0;
		typename std::map<int, std::deque<Loan> >::iterator iter =
				UMLoans.begin();
		if (!UMLoans.empty())
			for (; iter != UMLoans.end(); iter++)
				LastState.totaldeepBackLog += (iter->second).size();
		else
			LastState.totaldeepBackLog = 0;

	}

	else {
		LastState.deepBackLog = (UMDeposits.begin()->second).size();

		LastState.totaldeepBackLog = 0;
		typename std::map<int, std::deque<Loan> >::iterator iter =
				UMDeposits.begin();
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

bool Bank::operator <(Bank a) const {
	return ID < a.getID();
}

double Bank::GetProbDef() {
	return Pdefault;
}

int Bank::GetAssets() {
//this is written in the paper
//return CSH + ql + qila;
//I think it should be
	return CSH + ql + qila + qidr;
}

void Bank::Liquidate() {
//Cancel all IB loans
//for loop in qt that cancels all IBLoans
	typename deque<Loan>::iterator itQt = qt.begin();
	for (; itQt != qt.end(); ++itQt) {
		cerr << "Liquidate: Inspecting ID " << (*itQt).getID() << endl;
		if ((*itQt).getType() == 21 && (*itQt).GetLinkedAgentID() == ID
				&& !(*itQt).isCopy()){
			cancelLoan((*itQt).getID());
			// We have just invalidated the iterator
			itQt=qt.begin();
		}
	}

}

void Bank::ReplaceBankruptPartner(Bank* bank) {
//log << "start replaceBankruptPartner " << endl;

	PartnersVector.erase(bank);

	Bank* aux;
// At most N-1 partners and we impose the restriction that max number is "partners"
	while (PartnersVector.size()
			< (unsigned int) min(partners, (BNetwork::getBankCount() - 1))) {

		aux = BNetwork::getBank((int) (NBANKS * (rand() / (RAND_MAX + 1.0))));
		if ((aux != this) && (!BNetwork::IsBankrupt(aux))
				&& PartnersVector.find(aux) == PartnersVector.end()) {
			PartnersVector.insert(aux);
		}

	}

//log << "end replaceBankruptPartner " << endl;

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
		actionsSetPointers.insert(new Action((n), true));
		actionsSetPointers.insert(new Action((n), false));
//		actionsSetPointers.insert(new Action(n, true, true));
//		actionsSetPointers.insert(new Action(n, true, false));
//		actionsSetPointers.insert(new Action(n, false, true));
//		actionsSetPointers.insert(new Action(n, false, false));
	}
}

void Bank::updateBacklogs() {
	typename map<int, deque<Loan> >::iterator itDeposits = UMDeposits.begin();

	typename map<int, deque<Loan> >::iterator itLoans = UMLoans.begin();

	for (; itDeposits != UMDeposits.end();) {
		if ((itDeposits->second).empty()) {
			UMDeposits.erase(itDeposits);
//			cout << "empty queue in UMD" << endl;
			// the iterator is invalidated after the deletion
			itDeposits = UMDeposits.begin();
//		cout << "end deletion of empty queue in UMD" <<  endl;
		} else
			itDeposits++;

	}

	for (; itLoans != UMLoans.end();) {
		if ((itLoans->second).empty()) {
			UMLoans.erase(itLoans);
//			cout << "empty queue in UML" << endl;
			itLoans = UMLoans.begin();
//		cout << "end deletion of empty queue in UML" <<  endl;
		} else
			itLoans++;
		// the iterator is invalidated after the deletion
	}
//cout << "end updateBacklogs()" << endl;
}

int Bank::getSenderCanceledRequest() {
	return sender;
}
void Bank::rejectLoanRequest(int IDCanceledLoan, int requestsenderID) {
	cerr << "Time " << time << " Bank " << ID
			<< " Call to rejectLoanRequest, rejecting loan " << IDCanceledLoan
			<< endl;
	sender = requestsenderID;
	canceledLoan = IDCanceledLoan;
	rejectingIBloanrequest = true;

	typename deque<Loan>::iterator iterator_qt;
	for (iterator_qt = qt.begin(); iterator_qt != qt.end(); ++iterator_qt) {
		//try to find the canceled loan in the queue qt
		if ((*iterator_qt).getID() == IDCanceledLoan) {
			cout << "Rejected loan found" << endl;
			assert((*iterator_qt).getType() == 21);
			CSH++;
			qila--;
			qt.erase(iterator_qt);
			assert(qila >= 0);
			break;
		}
	}
}

void Bank::cancelLoan(int IDCanceledLoan) {
	cerr << "Call to cancelLoan" << endl;
	canceledLoan = IDCanceledLoan;
	cancelingLoan = true;
	CSH++;

	typename deque<Loan>::iterator iterator_qt;
	for (iterator_qt = qt.begin(); iterator_qt != qt.end(); ++iterator_qt) {
		//try to find the canceled loan in the queue qt
		if ((*iterator_qt).getID() == IDCanceledLoan
				&& (*iterator_qt).getType() == 11) {
			qidr--;
			qt.erase(iterator_qt);
			break;
			assert(qidr >= 0);
		} else if ((*iterator_qt).getID() == IDCanceledLoan
				&& (*iterator_qt).getType() == 21) {
			qila--;
			qt.erase(iterator_qt);
			break;
			assert(qila >= 0);
		}
	}
	//log << "ResolveCanceledLoan method" << endl;
}

void Bank::resolveCanceledLoan(int IDCanceledLoan) {
	typename deque<Loan>::iterator iterator_qt;
//	log << "RESOLVE CANCELED Bank " << ID << " Loan " << IDCanceledLoan
//			<< endl;
	cerr << "Bank " << ID << " Call to resolve canceled loan, time " << time
			<< " ID canceled loan " << IDCanceledLoan << endl;

//cerr << "SIZE QT " << qt.size() << endl;
	for (iterator_qt = qt.begin(); iterator_qt != qt.end(); ++iterator_qt) {//try to find the canceled loan in the queue qt
		//log << "ResolveCanceledLoan method" << endl;
		assert(qt.size() > 0);

		if ((*iterator_qt).getID() == IDCanceledLoan) {

			if ((*iterator_qt).getType() == 11) {
				qida--;
				CSH--;
				assert(qida >= 0);
			} else if ((*iterator_qt).getType() == 21) {
				qilr--;
				CSH--;
				assert(qila >= 0);
			}

			updateCumulativeValueObject((*iterator_qt), false);
			UpdateW(loanStrategyMap[IDCanceledLoan],
					cumulativeObjectValue[IDCanceledLoan]);

			cerr << "Time " << time << " Bank " << ID
					<< " Resolve canceled loan, ID is "
					<< (*iterator_qt).getID() << endl;
			//log << "Removing canceled loan in bank " << ID << endl;
			std::map<Loan, Loan>::iterator it1 = matchMap1.find((*iterator_qt));

			std::map<Loan, Loan>::iterator it2 = matchMap2.find((*iterator_qt));

			std::map<int, deque<Loan> >::iterator itBacklogDeposits =
					UMDeposits.find((*iterator_qt).getDesiredMatchingRate());

			std::map<int, deque<Loan> >::iterator itBacklogLoans = UMLoans.find(
					(*iterator_qt).getDesiredMatchingRate());

			if (it1 != matchMap1.end()) {
				//Item that's matched with the object that is expired
				if (it1->second.getType() == 1 || it1->second.getType() == 11) {
					//The object that will be unmatched is a customer deposit or IB deposit
					UMDeposits[it1->second.getDesiredMatchingRate()].push_back(
							it1->second);
				} else
					//the object that will be unmatched is a customer loan or IB loan
					UMLoans[it1->second.getDesiredMatchingRate()].push_back(
							it1->second);
			}

			//The same but search in the other direction

			else if (it2 != matchMap2.end()) {
				//Item that's matched with the object that is expired
				if (it2->second.getType() == 1 || it2->second.getType() == 11) {
					//The object that will be unmatched is a customer deposit or IB deposit
					UMDeposits[it2->second.getDesiredMatchingRate()].push_back(
							it2->second);
				} else
					//the object that will be unmatched is a customer loan or IB loan
					UMLoans[it2->second.getDesiredMatchingRate()].push_back(
							it2->second);
			}
			// If the object was unmatched, search it in the backlogs and remove it
			// I wonder if it would be better to iterate in the keys of the map (desired rates) and sort each queue
			else if (itBacklogDeposits != UMDeposits.end()) {
				typename deque<Loan>::iterator itQ;
				itQ = find(itBacklogDeposits->second.begin(),
						itBacklogDeposits->second.end(), (*iterator_qt));
				if (itQ != itBacklogDeposits->second.end())
					itBacklogDeposits->second.erase(itQ);
			}

			else if (itBacklogLoans != UMLoans.end()) {
				typename deque<Loan>::iterator itQ;
				itQ = find(itBacklogLoans->second.begin(),
						itBacklogLoans->second.end(), (*iterator_qt));
				if (itQ != itBacklogLoans->second.end())
					itBacklogLoans->second.erase(itQ);
			}
			removeFromVM(IDCanceledLoan);
			qt.erase(iterator_qt); // Remove the canceled object
			break;
		}
	}
}

int Bank::getLF() {
	return qcblf;
}

int Bank::getCD() {
	return qd;
}

int Bank::getIBD() {
	return qida;
}

int Bank::getCSH() {
	return CSH;
}

int Bank::getIBL() {
	return qila;
}

void Bank::setCancelingLoanToFalse() {
	cancelingLoan = false;
}

void Bank::setRejectingLoanToFalse() {
	rejectingIBloanrequest = false;
}

int Bank::getIDCanceledLoan() {
	return canceledLoan;
}

bool Bank::isCancelingLoan() {
	return cancelingLoan;
}

bool Bank::isRejectingIBLRequest() {
	return rejectingIBloanrequest;
}

vector<int> Bank::sendIBLiabilitiesIDs() {
	vector<int> aux;
	typename deque<Loan>::iterator it = qt.begin();
	for (; it != qt.end(); ++it) {
		// If the object is a IB deposit that was deposit in this bank, then add it to the vector of IBDeposits IDs
		//if((*it).GetLinkedAgentID()==ID && (*it).getType()==11 && !(*it).isCopy()) aux.push_back((*it).getID());
		// Look for IB deposits received, or IB Loans requested
		if (((*it).GetLinkedAgentID() == ID && (*it).getType() == 11)
				|| ((*it).GetLinkedAgentID() != ID && (*it).getType() == 21))
			aux.push_back((*it).getID());
	}

	return aux;

}

void Bank::resolveIlliquidity() {
// While the CSH requirement is not
	cerr << "Call to resolveIlliquidity" << endl;
	while (!CheckCashReserveRequirements() && (qila + qidr) > 0) // The bank only can cancel IB transactions
		// Get the ID of the loan that has the lower value, and summit a cancel request to the interbank market
		cancelLoan(valueMatchedIBLoans.begin()->second);
}

void Bank::removeFromVM(int ID) {
	std::map<double, int>::iterator it = valueMatchedIBLoans.begin();
	for (; it != valueMatchedIBLoans.end(); it++) {
		if ((*it).second == ID)
			valueMatchedIBLoans.erase(it);
	}
}

void Bank::newMatch(Loan& l1, Loan& l2) {
// Prior to set each object as matched, we need to update the object leaving a queue of unmatched objects
	updateCumulativeValueObject(l1, false);
	updateCumulativeValueObject(l2, false);
	l1.setMatched(true);
	l2.setMatched(true);

}

void Bank::setTime(double t) {
	time = t;
}

void Bank::saveToStrategyMap(int ID) {
// This function saves to the strategy map the last action and last state, linking an object's ID to a strategy. So we
//	loanStrategyMap[ID] = new Strategy(LastState, LastAction,
//			ValueFunction2[LastState][LastAction]);
	loanStrategyMap[ID] = tempStrategy;
//	if (h[LastState].find(LastAction) == h[LastState].end())
//if(strategySet.find(tempStrategy) == strategySet.end())
	if (strategySet.find(tempStrategy) != strategySet.end())
//		loanStrategyMap[ID]->setH(0);
//	else
		loanStrategyMap[ID]->setH(h[LastState][LastAction]);

	strategySet.insert(tempStrategy);
//	loanStrategyMap[ID]->setPayoff(ValueFunction2[LastState][LastAction]);
}

void Bank::updateCumulativeValueObject(Loan& l, bool isThereDefault) {
	double value;
	double valueCounterpart;
// Here we need to set the dt in the actual matched state
	double dtl = time - l.getTimeLastMatch();
	double dtMatchedCounterpart;

	if (l.wasMatched()) {

//		log << "ID " << l.getID() << " size map 2 " << matchMap2.size()
//				<< endl;

//		log << matchMap2.begin()->first.getID() <<" "<< l.getID() << " "<< matchMap2.begin()->first.get_t_to_maturity()<< " "<< l.get_t_to_maturity() <<  endl;

		assert(matchMap1.count(l) > 0 || matchMap2.count(l) > 0);

		typename std::map<Loan, Loan>::iterator it1 = matchMap1.find(l);

		typename std::map<Loan, Loan>::iterator it2 = matchMap2.find(l);

		assert(it1 != matchMap1.end() || it2 != matchMap2.end());

		Loan MatchCouterpart;
		int isADeposit;
		if (l.getType() == 1 || l.getType() == 12)
			isADeposit = 1;
		else
			isADeposit = -1;

		if (it1 != matchMap1.end()) {
			MatchCouterpart = it1->second;
		} else {
			MatchCouterpart = it2->second;
		}
		// Actual value
		double rateL;
		double rateMC;

		if (l.getType() == 1)
			rateL = customerDepositInterestRate;
		else if (l.getType() == 2)
			rateL = customerLoanInterestRate;
		else
			rateL = Rates[l.getInterestRate()];

		if (MatchCouterpart.getType() == 1)
			rateMC = customerDepositInterestRate;
		else if (MatchCouterpart.getType() == 2)
			rateMC = customerLoanInterestRate;
		else
			rateMC = Rates[MatchCouterpart.getInterestRate()];

		value = (1 - isThereDefault) * isADeposit * dtl * (rateMC - rateL)
				- isThereDefault * (1 + dtl * rateMC) - l.FromIB() * linkcost
				- MatchCouterpart.FromIB() * linkcost;

		// Object was matched. Thus, there's no liquidity penalty or reward.
		dtMatchedCounterpart = time - MatchCouterpart.getTimeLastMatch();

		valueCounterpart = -isADeposit * dtMatchedCounterpart * (rateMC - rateL)

		- l.FromIB() * linkcost - MatchCouterpart.FromIB() * linkcost;

		typename std::map<int, double>::iterator itInCumulativeObjectValueMap1 =
				cumulativeObjectValue.find(MatchCouterpart.getID());

		//copied items are updated in the bank that owns that object
		if (!MatchCouterpart.isCopy()) {
			if (itInCumulativeObjectValueMap1 != cumulativeObjectValue.end()) {
				cumulativeObjectValue[MatchCouterpart.getID()] +=
						valueCounterpart;
			} else
				cumulativeObjectValue[MatchCouterpart.getID()] =
						valueCounterpart;
		}
		MatchCouterpart.setFromIB(false);
		// Reset counterpart's time last match
		MatchCouterpart.setTimeLastMatch(time);
	} else {

		double rateL;

		if (l.getType() == 1)
			rateL = customerDepositInterestRate;
		else if (l.getType() == 2)
			rateL = customerLoanInterestRate;
		else
			rateL = Rates[l.getInterestRate()];

		int isADeposit;
		if (l.getType() == 1 || l.getType() == 12)
			isADeposit = 1;
		else
			isADeposit = -1;

		// Actual value
		int liquidityPenaltyOrReward = (LqBnch < CSH) ? 1 : -1;
		value =
				(1 - isThereDefault) * isADeposit * dtl * (CBDFR - rateL)
						- isThereDefault * (1 + dtl * CBDFR)
						- l.FromIB()
								* linkcost+ dtl*liquidityPenaltyOrReward*liquidityPenaltyParameter;
	}

//copied items are updated in the bank that owns that object
	if (!l.isCopy()) {
		typename std::map<int, double>::iterator itInCumulativeObjectValueMap2 =
				cumulativeObjectValue.find(l.getID());

		if (itInCumulativeObjectValueMap2 != cumulativeObjectValue.end()) {
			cumulativeObjectValue[l.getID()] += value;
		} else
			cumulativeObjectValue[l.getID()] = value;
	}

	l.setFromIB(false);

// Reset time last match in the object being updated
	l.setTimeLastMatch(time);

}
