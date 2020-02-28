//
//  bank.hpp
//  banksim
//
//  Created by Bakar AHMEDOU  on 11/08/2014.
//  Copyright (c) 2014 bakar. All rights reserved.
//

#ifndef banksim_bank_hpp
#define banksim_bank_hpp


#include "Agent.hpp"
#include "adevs.h"
#include <list>



class Bank:public Agent, public adevs::Atomic<IO_Type>{
public:
    /// Constructor.
    Bank();
    /// Internal transition function.
    void delta_int();
    /// External transition function.
    void delta_ext(double e, const adevs::Bag<IO_Type>& xb);
    /// Confluent transition function.
    void delta_conf(const adevs::Bag<IO_Type>& xb);
    /// Time advance function.
    double ta();
    /// Output function.
    void output_func(adevs::Bag<IO_Type>& yb);
    /// Output value garbage collection.
    void gc_output(adevs::Bag<IO_Type>& g);
    /// Destructor.
    ~Bank();
    
    /// Model input port.
    static const int customer_loan_arrival_port ;
    static const int customer_deposit_arrival_port ;
    
    static const int interbank_loan_arrival_port[NUM_BANK-1]  ;
    
    /// Model output port.
    static const int interbank_loan_departure_port[NUM_BANK-1] ;
    
    
    static const int state ;
    
    
    
    
private:
    
    ///
    std::list <Loan>centralbankDeposit ;
    std::list<Loan&>centralbankBorrowing;
    std::list<Loan&> interbankBorrowinglist<Loan> interbankLoan;
    std ::list<Loan&> deposits;
    std ::list<Loan&> loans ;
    
    std::list<Loan >unmatchedloans;
    std::list <Loan>unmatchedDeposit;
    std::map<Loan&,Loan&> matchings;
    
    ///
    double equity ;
    double deposits;
    double interbankBorrowing ;
    double centralebankBorrowing;
    
    
    double loans ;
    double interbankLoan;
    double cash ;
    
    
    
    ///
    double liquidity_benchmark ;
    int depositRate ;
    int lendingRate ;
    Rating laggedRate;
    Rating creditRate;
    double reviewFrequency ;
    std::list<Bank>potentialPartners;
    Stategy &depositRateStrategy  ;
    Stategy &loanRateStrategy ;
    static int alpha ;
    static int costLink ;
    State state;
    
    int status ;
    
    ///Bank's  functions
    
    
    bool checkInsolvent();
    bool chekReserveRequirement (double reserveRequirement);
    bool checkLiquidityDefault() ;
    void liquidate() ;
    void reserveActions() ;
    
    bool acceptLoanApplication ( Borrower & potentialBorrower );
    bool acceptInterbankLoanApplication (Bank &   potentialBorrower, Loan & potentialLoan  );
    bool takeDepositAction ( Loan &);
    bool takeLoanActtion (Loan & );
    void repayLoan( Loan& );
    
    
    
    void resolveCancelledLoan(Loan &);
    void loanCancelled (Loan & );
    
    void updatePotentialPartners(list <bank> & allBank );
    
} ;
#endif


/// in centralbank.cpp

#include "Bank.hpp"
#include <iostream>
using namespace std;
using namespace adevs;

const int Bank::alpha= ;
const int Bank::costLink=  ;



const int Bank::customer_loan_arrival_port=0 ;
const int Bank::customer_deposit_arrival_port=1 ;
const int Bank::interbank_loan_arrival_port[NUM_BANK_n]={ 2,3,4};



const int Bank::interbank_loan_departure_port[NUM_BANK_n] = { 5, 6 ,7 };

Bank::Bank():
Atomic<IO_Type>()
{
    
}

void Bank::delta_int()
{
    creditRate=rateBank( *this );
    
}

//
void Bank::delta_ext(double e, const Bag<IO_Type>& x)
{
    
	Bag<IO_Type>::const_iterator iter = x.begin();
	for (; iter != x.end(); iter++)
	{
        switch ((*iter).port))
        {
                
            case customer_deposit_arrival_port:
                takeDepositAction ((*iter).value);
                // deposiT rATE STRATEGIE GH
                state = State::State();
                depositRateStrategy.findoptimalrate(state);
                
                
                
                break;
                
            case customer_loan_arrival_port:
                
                acceptLoanApplication ( potentialBorrower );
                takeLoanActtion ((*iter).value);
                // LOAN RATE STRATEGIE
                
                break;
                
            case interbank_loan_arrival_port;
                
                break;
                
            case interbank_loan_departure_port;
                
        }
        
    }
}


void Bank::delta_conf(const Bag<IO_Type>& x)
{
    delta_ext(0.0,x);
	delta_int();
    
}

double ::ta()
{
	// If there are customers getting into line, then produce output
	// immediately.
	if (!deciding.empty())
	{
		return 0.0;
	}
	// Otherwise, wait for another customer
	else
	{
		return DBL_MAX;
	}
}

void Bank::output_func(Bag<IO_Type>& y)
{
    
    list<pair<int,Loan*> >::iterator i = deciding.begin();
	for (; i != deciding.end(); i++)
	{
		IO_Type event((*i).first,(*i).second);
		y.insert(event);
	}
}

void Bank::gc_output(Bag<IO_Type>& g)
{
	Bag<IO_Type>::iterator iter = g.begin();
	for (; iter != g.end(); iter++)
	{
		delete (*iter).value;
	}
}

Bank::~Bank()
{
    
}





bool Bank::checkInsolvent() {
    
    if ( equity <0  ) return true ;
    else return false ;
    
}

bool Bank::chekReserveRequirement (double reserveRequirement){
    reserveRequirement =0.02*deposits*interbankBorrowing*centralebankBorrowing;
    if (cash >=reserveRequirement ) return true ;
    else return false ;
}





Rating RatingAgency::rateBank (Bank & b , double downgrading_ratio_of_type_1=0.5, double downgrading_ratio_of_type_2=0.03 ) {
    
    double ratio_liquid_asset_to_obligation ;
    double ratio_equity_asset ;
    
    ratio_liquid_asset_to_obligation= (b.cash+b.ineterbankLoans)/(b.deposit+b.interbankBorrowing+b.centraleBankBorrowing);
    ratio_equity_asset = b.equity / (b.cash+b.ineterbankLoans+b.loans);
    
    if (ratio_liquid_asset_to_obligation<downgrading_ratio_of_type_1 "ou" <downgrading_ratio_of_type_2){
        
        Rating* rating_for_ban_b = new Rating(ta() , 1);
        
    }
    
    else {
        Rating * rating_for_ban_b = new Rating(t );}
    
    return rating_for_bank_b ;
    
}



void::Bank takeDepositAction(Loan l) {
    
    desposit.insert(l);
    
 }

void acceptLoanApplication (Loan l) {
    






