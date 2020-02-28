#include "adevs.h"
#include "Agent.h"
#define BorrowerMaturityParameter	10/4
#define DepositorMaturityParameter	10/4

	void Loan::setID(int a) {ID = a;}

	void Loan::set_borrower(Agent* b) {borrower = b;}

	void Loan::set_borrower(int b){IDborrower = b;}

	Agent* Loan::get_borrower() const {return borrower;}

	int Loan::getID() const {return ID;}

	int Loan::get_IDborrower() const {return IDborrower;}

	void Loan::set_lender(Agent* l) {lender = l;}

	void Loan::set_lender(int l) {IDlender = l;}

	Agent* Loan::get_lender() const {return lender;}

	int Loan::get_IDlender() const {return IDlender;}

	double Loan::get_t_to_maturity() const {return t_to_maturity;}

	double Loan::GetQuantity() {return quantity;}



//DEPOSITOR
	Depositor::Depositor(): Loan::Loan(){quantity = 0.05; ID = 1; set_time_to_maturity();};
	void Depositor::set_time_to_maturity(){t_to_maturity = d.exponential( DepositorMaturityParameter); }

//BORROWER
	Borrower::Borrower(): Loan::Loan(){quantity=20; ID = 2; set_time_to_maturity();}

	void Borrower::set_time_to_maturity(){t_to_maturity = b.exponential( BorrowerMaturityParameter ); }




