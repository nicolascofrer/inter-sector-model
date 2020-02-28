//
//  centraleBank.hpp
//  banksim
//
//  Created by Bakar AHMEDOU  on 12/08/2014.
//  Copyright (c) 2014 bakar. All rights reserved.
//

#ifndef banksim_centraleBank_hpp
#define banksim_centraleBank_hpp

#include "bank.hpp"
#include <list>


class central_bank : public Bank {
    
    central_bank(double depoRate =0.15, double lendRate = 0.35, double proba=0.95): depositRate(depoRate) , lendingRate(lendRate) {} ;
    //bool issueCentraleBankLoan ( Bank& potentialkBorrower , double quantity )
    void addLoan (Bank & borrower , double quantity )
    
    void repayLoan ( Loan & toRepay )
    ~central_bank() ;
    private :
    
    list<Loan> loans ;
    double  depositRate ;
    double lendingRate ;
    double probabilityLending ;
    
    
    
};

#include "centrale_bank.hpp""
#include<list>
#include "proba.hpp"



void central_bank ::addLoan (Bank & borrower , double quantity=1 ) {
    
    if (bernoulli(probabilityLending)==0)  {
        
        cout << "Loan was rejected" << endl;
        
        else {
            
            Loan * loan= new Loan (central_bank, borrower ,quantity,**time issued ,**time maturity, lendingRate,**liquidity reward ,**desired_rate  );
            
            
            loans.insert(loan);
            
        }
    }
    
    
    
    
}


void central_bank::repayLoan ( Loan & toRepay ){
    
    
    
    
    
    
}



central_bank::~central_bank()
{
    
	list<Loan>::iterator i;
	for (i = loans.begin(); i != loans.end(); i++)
	{
		delete *i;
	}
}



#endif
