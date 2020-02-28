#include "adevs.h"
#include "Loan.h"
#include "Parameters.h"
#include "BNetwork.h"
#include "Bank.h"
#include <numeric>
//#include <random>

//Strategy

Strategy::Strategy(State s, Action a, double p) {
	state = s;
	action = a;
	h = 0;
	payoff = p;
	dv = 0;
}

void Strategy::setH(int newh) {
	h = newh;
}

int Strategy::getH() {
	return h;
}
void Strategy::setPayoff(double p) {
	payoff = p;
}

Action Strategy::getAction() {
	return action;
}

State Strategy::getState() {
	return state;
}

Action::Action() {
	desiredMatchingInterestRate = 0;
	accept = false;

}
//Action::Action(int d, bool a, bool c) {
Action::Action(int d, bool a) {
	desiredMatchingInterestRate = d;
	accept = a;

}
bool Action::operator >(Action a) const {
	bool temp1 = desiredMatchingInterestRate > a.desiredMatchingInterestRate;
	bool temp2 = desiredMatchingInterestRate < a.desiredMatchingInterestRate;

	if (temp1)
		return temp1;
	else if (temp2)
		return temp1;

	else {
		temp1 = accept > a.accept;
		temp2 = accept < a.accept;
		if (temp1)
			return temp1;
		else if (temp2)
			return temp1;
		else
			return false;
	}
}

bool Action::operator <(Action a) const {
	bool temp1 = desiredMatchingInterestRate < a.desiredMatchingInterestRate;
	bool temp2 = desiredMatchingInterestRate > a.desiredMatchingInterestRate;

	if (temp1)
		return temp1;
	else if (temp2)
		return temp1;

	else {
		temp1 = accept < a.accept;
		temp2 = accept > a.accept;
		if (temp1)
			return temp1;
		else if (temp2)
			return temp1;
		else
			return false;
	}
}

bool Action::operator ==(Action a) const {
	return !(this->operator >(a)) && !(this->operator <(a));
}
//// Simpler version state operators
//bool State::operator >(State s) const {
//	return loanOrDeposit > s.loanOrDeposit;
//
//}
//bool State::operator <(State s) const {
//	return loanOrDeposit < s.loanOrDeposit;
//
//}

// Complete version state operators
bool State::operator >(State s) const {

	bool temp1;
	bool temp2;

	temp1 = loanOrDeposit > s.loanOrDeposit;

	temp2 = loanOrDeposit < s.loanOrDeposit;

	if (temp1)
		return temp1;

	else if (temp2)
		return temp1;

	else {
		temp1 = interestRate > s.interestRate;
		temp2 = interestRate < s.interestRate;

		if (temp1)
			return temp1;

		else if (temp2)
			return temp1;

		else {
			temp1 = ratingSender > s.ratingSender;
			temp2 = ratingSender < s.ratingSender;
			if (temp1)
				return temp1;

			else if (temp2)
				return temp1;
			else {
				temp1 = myRating > s.myRating;
				temp2 = myRating < s.myRating;

				if (temp1)
					return temp1;

				else if (temp2)
					return temp1;

				else {
					temp1 = partnersRatings > s.partnersRatings;
					temp2 = partnersRatings < s.partnersRatings;

					if (temp1)
						return temp1;
					else if (temp2)
						return temp1;

					else {
						temp1 = bid > s.bid;
						temp2 = bid < s.bid;

						if (temp1)
							return temp1;
						else if (temp2)
							return temp1;

						else {

							temp1 = ask > s.ask;
							temp2 = ask < s.ask;

							if (temp1)
								return temp1;
							else if (temp2)
								return temp1;

							else {
								temp1 = partnersBid > s.partnersBid;
								temp2 = partnersBid < s.partnersBid;
								if (temp1)
									return temp1;
								else if (temp2)
									return temp1;
								else {

									temp1 = partnersAsk > s.partnersAsk;
									temp2 = partnersAsk < s.partnersAsk;
									if (temp1)
										return temp1;
									else if (temp2)
										return temp1;
									else {

										temp1 = LqBench > s.LqBench;
										temp2 = LqBench < s.LqBench;

										if (temp1)
											return temp1;
										else if (temp2)
											return temp1;
										else {

											temp1 = deepBackLog > s.deepBackLog;
											temp2 = deepBackLog < s.deepBackLog;

											if (temp1)
												return temp1;
											else if (temp2)
												return temp1;
											else {
												temp1 = totaldeepBackLog
														> s.totaldeepBackLog;

												temp2 = totaldeepBackLog
														< s.totaldeepBackLog;

												if (temp1)
													return temp1;
												else if (temp2)
													return temp1;
												else
													return false;

											}

										}

									}

								}

							}

						}
					}

				}

			}

		}

	}

}


//bool State::operator <(State s) const {
////return loanOrDeposit < s.loanOrDeposit;;
//	bool temp1;
//	bool temp2;
//
//	temp1 = loanOrDeposit < s.loanOrDeposit;
//	temp2 = loanOrDeposit > s.loanOrDeposit;
//
//
//
//		if (temp1)
//			return temp1;
//
//		else if (temp2)
//			return temp1;
//		else return false;
//
//
//}

bool State::operator <(State s) const {

	bool temp1;
	bool temp2;

	temp1 = loanOrDeposit < s.loanOrDeposit;

	temp2 = loanOrDeposit > s.loanOrDeposit;

	if (temp1)
		return temp1;

	else if (temp2)
		return temp1;

	else {
		temp1 = interestRate < s.interestRate;
		temp2 = interestRate > s.interestRate;

		if (temp1)
			return temp1;

		else if (temp2)
			return temp1;

		else {
			temp1 = ratingSender < s.ratingSender;
			temp2 = ratingSender > s.ratingSender;
			if (temp1)
				return temp1;

			else if (temp2)
				return temp1;
			else {
				temp1 = myRating < s.myRating;
				temp2 = myRating > s.myRating;

				if (temp1)
					return temp1;

				else if (temp2)
					return temp1;

				else {
					temp1 = partnersRatings < s.partnersRatings;
					temp2 = partnersRatings > s.partnersRatings;

					if (temp1)
						return temp1;
					else if (temp2)
						return temp1;

					else {
						temp1 = bid < s.bid;
						temp2 = bid > s.bid;

						if (temp1)
							return temp1;
						else if (temp2)
							return temp1;

						else {

							temp1 = ask < s.ask;
							temp2 = ask > s.ask;

							if (temp1)
								return temp1;
							else if (temp2)
								return temp1;

							else {
								temp1 = partnersBid < s.partnersBid;
								temp2 = partnersBid > s.partnersBid;
								if (temp1)
									return temp1;
								else if (temp2)
									return temp1;
								else {

									temp1 = partnersAsk < s.partnersAsk;
									temp2 = partnersAsk > s.partnersAsk;
									if (temp1)
										return temp1;
									else if (temp2)
										return temp1;
									else {

										temp1 = LqBench < s.LqBench;
										temp2 = LqBench > s.LqBench;

										if (temp1)
											return temp1;
										else if (temp2)
											return temp1;
										else {

											temp1 = deepBackLog < s.deepBackLog;
											temp2 = deepBackLog > s.deepBackLog;

											if (temp1)
												return temp1;
											else if (temp2)
												return temp1;
											else {
												temp1 = totaldeepBackLog
														< s.totaldeepBackLog;

												temp2 = totaldeepBackLog
														> s.totaldeepBackLog;

												if (temp1)
													return temp1;
												else if (temp2)
													return temp1;
												else
													return false;

											}

										}

									}

								}

							}

						}
					}

				}

			}

		}

	}
}

bool State::operator ==(State s) const {
	return !(this->operator >(s)) && !(this->operator <(s));
}

void State::setStateNumber(int n) {
	stateNumber = n;
}

int State::getStateNumber() const {
	return stateNumber;
}

bool Strategy::operator >(Strategy s) const {
	bool temp1 = state > s.state;
	bool temp2 = state < s.state;

	if (temp1)
		return temp1;
	else if (temp2)
		return temp1;

	else {
		temp1 = action > s.action;
		temp2 = action < s.action;
		if (temp1)
			return temp1;
		else if (temp2)
			return temp1;
		else {
			temp1 = h > s.h;
			temp2 = h < s.h;
			if (temp1)
				return temp1;
			else if (temp2)
				return temp1;
			else {
				temp1 = payoff > s.payoff;
				temp2 = payoff < s.payoff;
				if (temp1)
					return temp1;
				else if (temp2)
					return temp1;
				else
					return false;

			}

		}
	}

}


bool Strategy::operator <(Strategy s) const {
	bool temp1 = state < s.state;
	bool temp2 = state > s.state;

	if (temp1)
		return temp1;
	else if (temp2)
		return temp1;

	else {
		temp1 = action < s.action;
		temp2 = action > s.action;
		if (temp1)
			return temp1;
		else if (temp2)
			return temp1;
		else {
			temp1 = h < s.h;
			temp2 = h > s.h;
			if (temp1)
				return temp1;
			else if (temp2)
				return temp1;
			else {
				temp1 = payoff < s.payoff;
				temp2 = payoff > s.payoff;
				if (temp1)
					return temp1;
				else if (temp2)
					return temp1;
				else
					return false;

			}

		}
	}

}

bool Strategy::operator ==(Strategy s) const {
	return !(this->operator >(s)) && !(this->operator <(s));
}


void Strategy::setdV(double dv) {
	this->dv = dv;
}
double Strategy::getdV() {
	return dv;
}
//This ID will tell us what type of loan object it is. Interbank loan, customer deposit, etc...
void Loan::setID(int a) {
	ID = a;
}

void Loan::setType(int a) {
	type = a;
}

void Loan::SetLinkedAgent(Agent* b) {
	linked = b;
}

//void Loan::SetRequestSenderAgent(Agent* b) {
//	requestSender = b;
//	IDrequestSender = b->getID();
//}
//
//Agent* Loan::getRequestSenderAgent() const {
//	return requestSender;
//}  //I had to use Bank* here because Agent* doesn't inherit adevs::Atomic<Loan>

Agent * Loan::GetLinkedAgent() const {
	return linked;
}

//void Loan::SetLinkedAgent(int b){IDlinked = b; linked = BNetwork::getBank(b);}

int Loan::GetLinkedAgentID() const {
	return IDlinked;
}

void Loan::SetMatchedWith(int l) {
//matchedWith = l;
//matchedWithID = l->getID();
	matchedWithID = l;
}

int Loan::GetMatchedWithLoanID() const {
	return matchedWithID;
}

Loan * Loan::GetMatchedWithLoan() const {
	return matchedWith;
}

void Loan::setAsCopy() {
	copy = true;
}

bool Loan::isCopy() {
	return copy;
}

void Loan::SetSenderAgent(Agent* b) {
	requestSender = b;
	IDrequestSender = b->getID();
	;
}

Agent * Loan::GetSenderAgent() const {
	return requestSender;
}

//void Loan::SetSenderAgent(int b){IDrequestSender = b;}

int Loan::GetSenderAgentID() const {
	return IDrequestSender;
}

void Loan::set_borrower(Agent* b) {
	borrower = b;
	IDborrower = b->getID();
}

//void Loan::set_borrower(int b){IDborrower = b; borrower = b->getID();}

Agent * Loan::get_borrower() const {
	return borrower;
}

int Loan::getID() const {
	return ID;
}

int Loan::getType() const {
	return type;
}

int Loan::get_IDborrower() const {
	return IDborrower;
}

void Loan::set_lender(Agent* l) {
	lender = l;
	IDlender = l->getID();
}

//void Loan::set_lender(int l) {IDlender = l; lender = BNetwork::getBank(l);}

Agent * Loan::get_lender() const {
	return lender;
}

int Loan::get_IDlender() const {
	return IDlender;
}

double Loan::get_t_to_maturity() const {
	return t_to_maturity;
}

void Loan::ReduceTimeToMaturity(double t) {
	t_to_maturity -= t;
}

double Loan::GetQuantity() {
	return quantity;
}

void Loan::SetAsExpired() {
	expired = true;
}

bool Loan::IsExpired() const {
	return expired;
}

int Loan::getInterestRate() const {
	return interest_rate;
}

void Loan::setDesiredMatchingRate(int r) {
	desired_rate = r;
}

int Loan::getDesiredMatchingRate() {
	return desired_rate;
}

void Loan::setNewTimeToMaturity(double t) {
	t_to_maturity = t;
}


bool Loan::operator <(const Loan& l1) const {
	return ID < l1.ID;
}

bool Loan::operator >(const Loan& l1) const {
	return ID > l1.ID;
}

bool Loan::operator ==(const Loan& l) const {
	return ID == l.ID;
}


void Loan::setNewID() {
	ID = LoanCounter;
	LoanCounter++;
	;
}

bool Loan::wasMatched() {
	return matched;
}

void Loan::setMatched(bool m) {
	matched = m;
}

void Loan::setTimeLastMatch(double t) {
	timeLastMatch = t;
}

double Loan::getTimeLastMatch() {
	return timeLastMatch;
}

bool Loan::FromIB() {
	return comesFromIB;
}

void Loan::setFromIB(bool b) {
	comesFromIB = b;
}

void Loan::setInterestRate(int r) {
	interest_rate = r;
}

//Counter for ID assignment
int Loan::LoanCounter = 0;
int Loan::getNLoans() {
	return LoanCounter;
}

//DEPOSITOR
Depositor::Depositor() :
		Loan::Loan() {
	ID = LoanCounter;
	LoanCounter++;
	quantity = 1;
	type = 1;
	set_time_to_maturity();
	expired = false;
	copy = false;
	interest_rate = 0;
	matched = false;
	comesFromIB = false;
}
void Depositor::set_time_to_maturity() {
	t_to_maturity = rloan.exponential( DepositorMaturityParameter);
	cout << " t_to_maturity DEPOSIT " << t_to_maturity << endl;
	duration = t_to_maturity;
}

//BORROWER
Borrower::Borrower() :
		Loan::Loan() {
	ID = LoanCounter;
	LoanCounter++;
	quantity = 1;
	type = 2;
	set_time_to_maturity();
	expired = false;
	copy = false;
	interest_rate = 0;
	matched = false;
	comesFromIB = false;
}

void Borrower::set_time_to_maturity() {
	t_to_maturity = rloan.exponential( BorrowerMaturityParameter);
	cout << " t_to_maturity LOAN " << t_to_maturity << endl;
	duration = t_to_maturity;
}

adevs::rv Loan::rloan;

