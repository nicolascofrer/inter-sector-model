#ifndef AGENT_H_
#define AGENT_H_
#include "adevs.h"

class Agent{ //I modified this abstract class, because if this is abstract I can't use SimpleDigraph with the Loan class (because the latter uses Agent*)
double value();
};

class Loan
{
public:

	void setID(int a);

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

	double GetQuantity();


protected:
	int ID;
	Agent * lender ;
    Agent * borrower;

    int IDlender;
    int IDborrower;

    double quantity ;
    //Time issued ;
    //Time maturity;
    double t_issued;
    double t_maturity;

    int interest_rate;
    double liquidity_reward;
    double desired_rate;

    adevs::rv d;
    adevs::rv b;
    double t_to_maturity;


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
