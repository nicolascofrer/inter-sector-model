//#include "Bank.h"
#ifndef LOAN_H_
#define LOAN_H_
class Agent
{ //I modified this abstract class, because if this is abstract I can't use SimpleDigraph with the Loan class (because the latter uses Agent*)
public:
	virtual int getID()=0;
	//double value();

};

class Action {
    
public:
    Action () ; 

    Action ( double a , double b , int c );
    double desiredRate ;
    double payoff ;
    int n ;
    bool operator > (Action a) const;
    bool operator < (Action a) const;
};

/*class Action{

public:
	Action();
	Action(double  d, bool a, bool c);
	double  desiredMatchingInterestRate;
	bool accept;
	bool cancelLoan;


	bool operator >  (Action a) const;
	bool operator <  (Action a) const;
};*/

class State{

public:

	bool operator >  (State s) const;
	bool operator <  (State s) const;

	int loanOrDeposit;
//	2) rate of the loan or deposit (this will give me if it is a costmer loan-depost or interbank loan deposit)
	double  interestRate;
//					3)Rating of the bank sending the loan-deposit from the interbank market (i.e. 0 in the case of a Customer loan-deposit)
	int ratingSender;
//					4)  your rating
	int myRating;
//					5) partners ratings (with a lag)
	std::vector<double> partnersRatings;
	//					6) your bid
	double bid;
	//					7) your ask
	double ask;
//					8) partners bid
	std::vector<double> partnersBid;
//					9) partner ask
	std::vector<double> partnersAsk;
//					10) your liquidity benchmark (only above or below).
	bool LqBench;
//					11) depth backlog at the ask if you receive a deposit (depth backlog at the bid if you receive a loan).
	int deepBackLog;
//					12) TOTAL depth backlog on the upper side if it is a deposit (TOTAL depth backlog on the lower side if it is a loan).
	int totaldeepBackLog;
//




};

/*class Strategy{
public:
	Strategy(State s, Action a);
	int h; //number of times state-action has been visited
	double payoff;
	Action action;
	State state;
	adevs::rv p;
	bool operator >  (Strategy s) const;
	bool operator <  (Strategy s) const;

};*/


class Loan
{
public:

	void setID(int a);

	void setType(int a);

	void SetLinkedAgent(Agent* b);

	//void SetLinkedAgent(int b);

	int GetLinkedAgentID() const;

	Agent* GetLinkedAgent() const; //I had to use Bank* here because Agent* doesn't inherit adevs::Atomic<Loan>


	void SetMatchedWith(Loan* l);

	int GetMatchedWithLoanID() const;

	Loan* GetMatchedWithLoan() const;


	void SetSenderAgent(Agent* b);

	//void SetSenderAgent(int b);

	int GetSenderAgentID() const;

	Agent* GetSenderAgent() const; //I had to use Bank* here because Agent* doesn't inherit adevs::Atomic<Loan>


	void set_borrower(Agent* b);

	//void set_borrower(int b);

	Agent* get_borrower() const;

	int getID() const;

	int getType() const;

	int get_IDborrower() const;

	void set_lender(Agent* l);

	//void set_lender(int l);

	Agent* get_lender() const;

	int get_IDlender() const;

	double get_t_to_maturity() const;

	void ReduceTimeToMaturity( double t);

	double GetQuantity();

	void SetAsExpired();

	bool IsExpired() const;

	double getInterestRate() const;
    void   SetdesiredRate (double e) ;
    double getdesiredRate() const;
    
    
    void setAction (Action a);
    void updateAction( double e ) ; 
    
protected:
	int ID;
	int type;
	Agent* lender ;
	Agent* borrower;
	Agent* linked;
	Agent* requestSender;

    Loan * matchedWith;
    int matchedWithID;

    int IDlender;
    int IDborrower;
    int IDlinked;
    int IDrequestSender;

    double quantity ;
    //Time issued ;
    //Time maturity;
    //double t_issued;
    //double t_maturity;

    double interest_rate;
    double liquidity_reward;
    double desired_rate;

    adevs::rv d;
    adevs::rv b;
    double t_to_maturity;
    double duration;
    bool expired;
    
    
    
    Action *a ;
    
    


};

class Depositor: public Loan {

public:

	Depositor();


private:

	void set_time_to_maturity();

};

class Borrower: public Loan {

public:

	Borrower();


private:

	void set_time_to_maturity();

};






#endif
