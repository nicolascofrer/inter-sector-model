
#include "Agent.hpp"
#include "adevs.h"
#include "genDeposit.hpp"
#include "commercial_bank.hpp"
#include <list>

Bank::Bank(double equity =10, double deposits=0, double loans =0 , double interbankBorrowing=0 ,double cash =0 ,
double centralebankBorrowing =0, double liquidity_benchmark  , int depositRate  ):
Atomic<IO_Type>(),
double equity( ,double deposits,double loans ,double interbankLoan,double interbankBorrowing ,double cash ,
double centalebankBorrowing ,double liquidity_benchmark ,
int depositRate ,int lendingRate ,Rating laggedRate,Rating creditRate,double reviewFrequency ,
Stategy &depositRateStrategy  ,Stategy &loanRateStrategy ,static int alpha ,
static int costLink  {};   // a revoir




static const int Bank::customer_loan_arrival_port = 0;
static const int Bank::customer_deposit_arrival_port = 1;

/// Model output port.
static const int Bank::arrive=2;


/// Internal transition function.
void Bank::delta_int(){}

/// External transition function.
void Bank::delta_ext(double e, const adevs::Bag<IO_Type>& xb){
              switch (port_num)
              {

            case customer_deposit_arrival_port:
            bank_model->process_event(static_cast<cdep_arrival<Time_t>&>(event));
            break;

          case customer_loan_arrival_port:
            bank_model->process_event(static_cast<cloan_arrival<Time_t>&>(event));
            break;
          case customer_loan_maturity_port:
            bank_model->process_event(static_cast<cloan_matures<Time_t>&>(event));
            break;

              }

}

void Bank::delta_conf(const adevs::Bag<IO_Type>& xb) {}

double ta();
/// Output function.
void output_func(adevs::Bag<IO_Type>& yb);
/// Output value garbage collection.
void gc_output(adevs::Bag<IO_Type>& g);
/// Destructor.
~Bank();

bool Bank::checkInsolvent() {

if ( equity <0  ) return true ;
else return false ;



}

bool Bank::chekReserveRequirement (double reserveRequirement){

if (cash >=reserveRequirement ) return true ;
else return false ;
}


bool Bank::checkLiquidityDefault() {




}


void Bank::liquidate() { // liquidation  process

}








bool acceptLoanApplication ( Borrower & potentialBorrower );
bool acceptInterbankLoanApplication (Bank &   potentialBorrower, Loan & potentialLoan  );
bool Bank::takeDepositAction ( Loan &){ // puisque le depot n'est jamais refusé crééer un loan des son arrivée
loans.insert(Loan);


}
bool takeLoanActtion (Loan & );
void repayLoan( Loan& );
void reserveActions() ;
void resolveCancelledLoan(Loan &);
void loanCancelled (Loan & );





