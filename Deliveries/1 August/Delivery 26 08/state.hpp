//
//  state.hpp
//  banksim
//
//  Created by Bakar AHMEDOU  on 18/08/2014.
//  Copyright (c) 2014 bakar. All rights reserved.
//

#ifndef banksim_state_hpp
#define banksim_state_hpp
#include "bank.hpp"
#include "agent.h"
#include <list>
#include <map> 
#include <vector>
#include <adevs.h>




class State {
    // constructeur à appeler dans la bank
    State (Loan l , ) : incoming=l.interestRate , lqbnch=
public:
    
    InterstRate incoming ;
    bool lqbnch ;
    Rating creditRate;
    rate_level backlog ;
    std::vector<Rating> RatingBankPartners;
    std::vector<rate_level> lendingrateBankPartners;
    std::vector<rate_level> borrowingRateBankPartners;
    };






struct Action {

    
        desired ;
        double payoff ;
        int visited ;
    
        }


    
class  Strategy {
        public :
        rate_level findOptimalRate(State&)
        void updatePayoff(Action&,double)
       // bool converged()
        void setInitialValues()
        
        
    private:
    
      std::map <state & s, std::vector<Action> > stategy_dep ;
      adevs::rv probaTrembling ;
    };
    
#endif 
    rate_level Strategy::findOptimalRate ( State s) {    
        
       
        
        
     
    

    
/*
class Match_deposit_with_loan : public Action {
public:
    void do_action ();
    double payoff () ;
    
private :
    
    
    double payoff ;
    int visited;
    int action_index;
    
};

class Match_deposit_with_interbank_loan : public Action {
public:
    void do_action ();
    double payoff () ;
    
private :
    
    double payoff ;
    int visited;
    int action_index;
    

};

class put_deposit_in_backlog : public Action {
public:
    void do_action ();
    double payoff () ;
    
private :
    
    double payoff ;
    int visited;
    int action_index;
    
    
    
};
*/





