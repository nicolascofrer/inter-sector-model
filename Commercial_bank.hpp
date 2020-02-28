
#ifndef _Commercial_bank_hpp_
#define _Commercial_bank_hpp_

#include "Agent.hpp"
#include "adevs.h"
#include "genDeposit.hpp"
#include <list>
class Bank:public Agent, public adevs::Atomic<IO_Type>{

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
static const int interbank_loan_arrival_port  ;
static const int interbank_deposit_arrival_port ;
/// Model output port.
static const int arrive;



bool checkInsolvent();
bool chekReserveRequirement (double reserveRequirement);
bool checkLiquidityDefault() ;
void liquidate ;
bool acceptLoanApplication ( Borrower & potentialBorrower );
bool acceptInterbankLoanApplication (Bank &   potentialBorrower, Loan & potentialLoan  );
bool takeDepositAction ( Loan &);
bool takeLoanActtion (Loan & );
void repayLoan( Loan& );
void reserveActions() ;
void resolveCancelledLoan(Loan &);
void loanCancelled (Loan & );

void updatePotentialPartners(list <bank> & allBank );





private:
std::list <Loan>centralbankDeposit ;
std::list<Loan&>centralbankBorrowing;
std::list<Loan&> interbankBorrowinglist<Loan> interbankLoan;
std ::list<Loan&> deposits;
std ::list<Loan&> loans ;
std::list<Loan >unmatchedloans;
std::list <Loan>unmatchedDeposit;
std::map<Loan&,Loan&> matchings;
double equity ;
double deposits;
double loans ;
double interbankLoan;
double interbankBorrowing ;
double cash ;
double centralebankBorrowing ;
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



} ;


























#endif
