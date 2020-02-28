
#ifndef banksim_agent_h
#define banksim_agent_h

#include "adevs.h"
#include <list>
#define DDPARAM   1000	//deposit time to maturity parameter
#define BDPARAM   1000	//deposit time to maturity parameter
#define P 2    //  # OF RATES LEVELS 
#define deposit_facility 0.015
#define lending_facility 0.035
#define customer_loan_interest_rate  0.055
#define customer_deposit_interest_rate 0.025
#define RP 0,85 //  probability  repay
#define FREQcust  // customer arrival rate 


class Agent{
    double value();
};


struct InterestRate {
  InterestRate( double v ): value (v) {};
    
    double value ;
};
std::list<InterestRate> all_rate ;

void assign_all_rate();





class Loan
{
public:
  Loan ();
  Loan ( Agent & a , Agent &b , double q=1 , double m   ):
     lender(a) , borrower (b) , quantity(q) , t_issued(m) {} ;    
    
  /*void repay ();
 double get_interest_rate();
    
    
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
 */   
protected:
 //	int ID;
	Agent & lender ;
  Agent & borrower;
  double quantity ;
  double t_issued;
  double t_maturity;

  InterestRate interest_rate;
  InterestRate desired_rate;
    
  adevs::rv m;
    
  //double t_to_maturity;
  //double liquidity_reward; 
};


class Borrower: public Agent { 
    
public:
  
  Borrower (double a=1 /*, Bank & custo */, double p=RP , double frequency ) :
    toBorrow(a) , repayProbability (p) , entryFrequency(frequency) {};    

    
    
private:
    
double toBorrow;
//Bank& customerOf;
double repayProbability;
double entryFrequency;
   
	
};

class Depositor 
{
public:
  Depositor (): (double a=1 /*, Bank & custo */, double p=RP , double frequency=FREQcust ):
    todep(q) , entryFrequency(frequency) {};    
  ~Depositor ();

private:
  double dep;
//Bank& customerOf;
double entryFrequency;
};




class Rating {
public:
    Rating (double t=0, int r=0) : RatingAssigned(t), creditRating (r){} // 0:Good 1:Bad
private :
    double RatingAssigned ;
    int creditRating ;
    
};



typedef adevs::PortValue<Loan*> IO_Type;


#endif







