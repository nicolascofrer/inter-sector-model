#include "adevs.h"
#include "Loan.h"
#include "Parameters.h"
#include "BNetwork.h"
#include "Bank.h"

//Strategy

Strategy::Strategy(State s, Action a){
	state = s; action = a;
	h = 0;
	payoff = p.uniform(-1,1); //Initial belief about payoff
}

Action::Action(){
	desiredMatchingInterestRate = 0;
	accept = false;
	cancelLoan = false;
}
Action::Action(int d, bool a, bool c){
	desiredMatchingInterestRate = d;
	accept = a;
	cancelLoan = c;
}
bool Action::operator > (Action a) const { return desiredMatchingInterestRate > a.desiredMatchingInterestRate;}
bool State::operator > (State s) const { return totaldeepBackLog > s.totaldeepBackLog;}
bool Strategy::operator > (Strategy s) const { return state > s.state && action > s.action;}

bool Action::operator < (Action a) const { return desiredMatchingInterestRate < a.desiredMatchingInterestRate;}
bool State::operator < (State s) const { return totaldeepBackLog < s.totaldeepBackLog;}
bool Strategy::operator < (Strategy s) const{ return state < s.state && action < s.action;}



	//This ID will tell us what type of loan object it is. Interbank loan, customer deposit, etc...
	void Loan::setID(int a) {ID = a;}

	void Loan::setType(int a) {type = a;}

	void Loan::SetLinkedAgent(Agent* b){linked = b;}

	void Loan::SetRequestSenderAgent(Agent* b){requestSender = b; IDrequestSender = b->getID();}

	Agent* Loan::getRequestSenderAgent() const {return requestSender;}  //I had to use Bank* here because Agent* doesn't inherit adevs::Atomic<Loan>

	Agent* Loan::GetLinkedAgent() const {return linked;}

	//void Loan::SetLinkedAgent(int b){IDlinked = b; linked = BNetwork::getBank(b);}

	int Loan::GetLinkedAgentID() const {return IDlinked;}


	void Loan::SetMatchedWith(Loan* l){ matchedWith = l; matchedWithID = l->ID; }

	int Loan::GetMatchedWithLoanID() const {return matchedWithID;}

	Loan* Loan::GetMatchedWithLoan() const {return matchedWith;}

	void Loan::setAsCopy() { copy = true;}

	bool Loan::isCopy() {return copy;}


	void Loan::SetSenderAgent(Agent* b){requestSender = b; IDrequestSender= b->getID();;}

	Agent* Loan::GetSenderAgent() const {return requestSender;}

	//void Loan::SetSenderAgent(int b){IDrequestSender = b;}

	int Loan::GetSenderAgentID() const {return IDrequestSender;}


	void Loan::set_borrower(Agent* b) {borrower = b; IDborrower = b->getID();}

	//void Loan::set_borrower(int b){IDborrower = b; borrower = b->getID();}

	Agent* Loan::get_borrower() const {return borrower;}

	int Loan::getID() const {return ID;}

	int Loan::getType() const {return type;}

	int Loan::get_IDborrower() const {return IDborrower;}

	void Loan::set_lender(Agent* l) {lender = l; IDlender = l->getID();}

	//void Loan::set_lender(int l) {IDlender = l; lender = BNetwork::getBank(l);}

	Agent* Loan::get_lender() const {return lender;}

	int Loan::get_IDlender() const {return IDlender;}

	double Loan::get_t_to_maturity() const {return t_to_maturity;}

	void Loan::ReduceTimeToMaturity( double t) {t_to_maturity -= t;}

	double Loan::GetQuantity() {return quantity;}

	void Loan::SetAsExpired(){expired = true;}

	bool Loan::IsExpired() const {return expired;}

	int Loan::getInterestRate() const {return interest_rate;}

	void Loan::setDesiredMatchingRate(int r){ desired_rate = r;}

	int Loan::getDesiredMatchingRate(){return desired_rate;}

	void Loan::setNewTimeToMaturity(double t){ t_to_maturity = t;}

//DEPOSITOR
	Depositor::Depositor(): Loan::Loan()
	{
		quantity = 1;
		type = 1;
		set_time_to_maturity();
		expired = false;
		copy = false;
		interest_rate = 0;
	}
	void Depositor::set_time_to_maturity()
	{
		t_to_maturity = d.exponential( DepositorMaturityParameter);
		duration = t_to_maturity;
	}

//BORROWER
	Borrower::Borrower(): Loan::Loan()
	{
		quantity = 1;
		type = 2;
		set_time_to_maturity();
		expired = false;
		copy = false;
		interest_rate = 0;
	}

	void Borrower::set_time_to_maturity()
	{
		t_to_maturity = b.exponential( BorrowerMaturityParameter );
		duration = t_to_maturity;
	}




