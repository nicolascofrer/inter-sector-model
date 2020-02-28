#ifndef AGENT_H_
#define AGENT_H_
#include "adevs.h"
#define DDPARAM   1000	//deposit time to maturity parameter
#define BDPARAM   1000	//deposit time to maturity parameter

class Agent{ //I modified this abstract class, because if this is abstract I can't use SimpleDigraph with the Loan class (because the latter uses Agent*)
double value();
};

class Loan
{
public:

//void repay ();
//double get_intere+st_rate();


	void set_borrower(Agent* b) {borrower = b;}

	void set_borrower(int b){IDborrower = b;}

	Agent* get_borrower() {return borrower;}

	int get_IDborrower(){return IDborrower;}

	void set_lender(Agent* l) {lender = l;}

	void set_lender(int l) {IDlender = l;}

	Agent* get_lender() {return lender;}

	int get_IDlender() {return IDlender;}

	double get_t_to_maturity(){return t_to_maturity;}

	int IDlender;

	int IDborrower;

protected:
	int ID;
	Agent * lender ;
    Agent * borrower;

    double quantity ;
    //Time issued ;
    //Time maturity;
    double t_issued;
    double t_maturity;

    int interest_rate;
    double liquidity_reward;
    double desired_rate;

    adevs::rv m;

    double t_to_maturity;


};

class Depositor: public Loan {

public:

	Depositor(){set_time_to_maturity();
	quantity=1;}

private:

	void set_time_to_maturity(){t_to_maturity = m.exponential( DDPARAM ); }

};

class Borrower: public Loan {

public:

	Borrower(){set_time_to_maturity();
	quantity=1;}


private:

	void set_time_to_maturity(){t_to_maturity = m.exponential( BDPARAM ); }
	double repayProbability;
};


//
//
//// time_issued
//
//class Depositor: public Agent {
//
//    Depositor (double x=1, double freq=0.1 ,t=0): toDeposit(x), entryFrequency(freq), time_issued(t){};
//
//
//
//Private:
//    double toDeposit;
//    //Bank& customerOf ;
//    double entryFrequency;
//   double  time_issued ;
//
//
//
//};
//
//
//class Borrower : public Agent {
//
//    Borrower ( double x=1,double rp=0.85,  double freq=0.1 ): toBorrow(x) , repayProbability(rp) , entryFrequency (freq) {};
//
//private :
//
//double toBorrow ;
//// Bank& customerOf ;
//double repayProbability;
//double entryFrequency ;
//double  time_issued ;
//};
//
///// Create an abbreviation for the Commercial banks's input/output type.
//typedef adevs::PortValue<Loan*> IO_Type;
//

#endif
