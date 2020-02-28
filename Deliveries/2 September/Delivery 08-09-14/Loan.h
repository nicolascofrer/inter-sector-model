
#ifndef LOAN_H_
#define LOAN_H_
class Agent
{ //I modified this abstract class, because if this is abstract I can't use SimpleDigraph with the Loan class (because the latter uses Agent*)
double value();
};


class Loan
{
public:

	void setID(int a);

	void SetLinkedAgent(Agent* b);

	void SetLinkedAgent(int b);

	int GetLinkedAgentID() const;

	Agent* GetLinkedAgent() const; //I had to use Commercial_bank* here because Agent* doesn't inherit adevs::Atomic<Loan>

	void set_borrower(Agent* b);

	void set_borrower(int b);

	Agent* get_borrower() const;

	int getID() const;

	int get_IDborrower() const;

	void set_lender(Agent* l);

	void set_lender(int l);

	Agent* get_lender() const;

	int get_IDlender() const;

	double get_t_to_maturity() const;

	void ReduceTimeToMaturity( double t);

	double GetQuantity();

	void SetAsExpired();

	bool IsExpired() const;

protected:
	int ID;
	Agent * lender ;
    Agent * borrower;
    Agent * linked;

    int IDlender;
    int IDborrower;
    int IDlinked;

    double quantity ;
    //Time issued ;
    //Time maturity;
    //double t_issued;
    //double t_maturity;

    int interest_rate;
    double liquidity_reward;
    double desired_rate;

    adevs::rv d;
    adevs::rv b;
    double t_to_maturity;
    double duration;
    bool expired;


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
