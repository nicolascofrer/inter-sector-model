#include "adevs.h"
#include "Loan.h"
#include "Parameters.h"



	//This ID will tell us what type of loan object it is. Interbank loan, customer deposit, etc...
	void Loan::setID(int a) {ID = a;}

	void Loan::SetLinkedAgent(Agent* b){linked = b;}

	Agent* Loan::GetLinkedAgent() const {return linked;}

	void Loan::SetLinkedAgent(int b){IDlinked = b;}

	int Loan::GetLinkedAgentID() const {return IDlinked;}

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

	void Loan::SetAsExpired(){expired = true;}

	bool Loan::IsExpired() const {return expired;}



//DEPOSITOR
	Depositor::Depositor(): Loan::Loan(){quantity = 0.05; ID = 1; set_time_to_maturity(); expired = false;};
	void Depositor::set_time_to_maturity(){t_to_maturity = d.exponential( DepositorMaturityParameter); }

//BORROWER
	Borrower::Borrower(): Loan::Loan(){quantity=20; ID = 2; set_time_to_maturity(); expired = false;}

	void Borrower::set_time_to_maturity(){t_to_maturity = b.exponential( BorrowerMaturityParameter ); }




