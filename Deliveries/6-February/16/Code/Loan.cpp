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
//	cancelLoan = false;
}
//Action::Action(int d, bool a, bool c) {
Action::Action(int d, bool a) {
	desiredMatchingInterestRate = d;
	accept = a;
//	cancelLoan = c;
}
bool Action::operator >(Action a) const {
	return desiredMatchingInterestRate > a.desiredMatchingInterestRate;
}

bool Action::operator <(Action a) const {
	return desiredMatchingInterestRate < a.desiredMatchingInterestRate;
}

//// Simpler version state operators
//bool State::operator >(State s) const {
//	return rateI > s.rateI && rateO > s.rateO && loanOrDeposit > s.loanOrDeposit
//			&& cash > s.cash && interbankLiabilities > s.interbankLiabilities;
//
//}
//bool State::operator <(State s) const {
//	return rateI < s.rateI && rateO < s.rateO && loanOrDeposit < s.loanOrDeposit
//			&& cash < s.cash && interbankLiabilities < s.interbankLiabilities;
//
//}

// Complete version state operators
bool State::operator >(State s) const {

return loanOrDeposit>s.loanOrDeposit &&

interestRate>s.interestRate &&

ratingSender> s.ratingSender &&

myRating>s.myRating &&

accumulate(partnersRatings.begin(), partnersRatings.end(), 0) > accumulate(s.partnersRatings.begin(), s.partnersRatings.end(), 0) &&

bid>s.bid &&

ask>s.ask &&

accumulate(partnersBid.begin(), partnersBid.end(), 0) > accumulate(s.partnersBid.begin(), s.partnersBid.end(), 0) &&

accumulate(partnersAsk.begin(), partnersAsk.end(), 0) > accumulate(s.partnersAsk.begin(), s.partnersAsk.end(), 0) &&

LqBench>s.LqBench &&

deepBackLog>s.deepBackLog&&

totaldeepBackLog>s.totaldeepBackLog;

}

bool State::operator <(State s) const {

return loanOrDeposit<s.loanOrDeposit &&

interestRate<s.interestRate &&

ratingSender< s.ratingSender &&

myRating<s.myRating &&

accumulate(partnersRatings.begin(), partnersRatings.end(), 0) < accumulate(s.partnersRatings.begin(), s.partnersRatings.end(), 0) &&

bid<s.bid &&

ask<s.ask &&

accumulate(partnersBid.begin(), partnersBid.end(), 0) < accumulate(s.partnersBid.begin(), s.partnersBid.end(), 0) &&

accumulate(partnersAsk.begin(), partnersAsk.end(), 0) < accumulate(s.partnersAsk.begin(), s.partnersAsk.end(), 0) &&

LqBench<s.LqBench &&

deepBackLog<s.deepBackLog&&

totaldeepBackLog<s.totaldeepBackLog;

}

bool State::operator == (State s)const{
	return !(this->operator >(s)) && !(this->operator <(s));
}


bool Strategy::operator >(Strategy s) const {
return state > s.state && action > s.action && h > s.h && payoff > s.payoff;
}

bool Strategy::operator <(Strategy s) const {
return state < s.state && action < s.action && h < s.h && payoff < s.payoff;
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

Agent* Loan::GetLinkedAgent() const {
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

Loan* Loan::GetMatchedWithLoan() const {
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

Agent* Loan::GetSenderAgent() const {
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

Agent* Loan::get_borrower() const {
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

Agent* Loan::get_lender() const {
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

bool Loan::operator ==(const Loan& l) const {
return ID == l.ID;
}

bool Loan::operator <(const Loan& l1) const {
return ID < l1.ID;
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
t_to_maturity = r.exponential( DepositorMaturityParameter);
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
t_to_maturity = r.exponential( BorrowerMaturityParameter);
cout << " t_to_maturity LOAN " << t_to_maturity << endl;
duration = t_to_maturity;
}

adevs::rv Loan::r;

