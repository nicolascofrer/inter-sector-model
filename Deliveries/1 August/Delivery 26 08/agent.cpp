

#include "agent.h"

void assign_all_rate() {
    
    
    
    for(int i=0 ; i<=2*P;++i){
        
      all_rate.push_front(InterestRate(deposit_facility+(i*(lending_facility-deposit_facility)/(2*P))));
    }
    all_rate.push_front( InterestRate (customer_deposit_interest_rate));
    all_rate.push_front (InterestRate( customer_loan_interest_rate));
    
}