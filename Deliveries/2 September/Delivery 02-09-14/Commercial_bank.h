#ifndef _Commercial_bank_h_
#define _Commercial_bank_h_
#include "adevs.h"
#include "Agent.h"
#include <cassert>
#include <deque>
#include <algorithm>	// Needed for sort() method
#include <map>

class Commercial_bank: public adevs::Atomic<Loan>, public Agent
{
	public:
		Commercial_bank(double ProbabilityDefault);
		void delta_int();
		void delta_ext(double e, const adevs::Bag<Loan>& xb);
		void delta_conf(const adevs::Bag<Loan>& xb);
		void output_func(adevs::Bag<Loan>& yb);
		double ta();
		bool model_transition();
		unsigned int getDepositQueueSize();
		unsigned int getLoanQueueSize();
		void gc_output(adevs::Bag<Loan>&);
		double value();
		void reduce_equity(double shock);
		bool checkInsolvent();
		//rate for deposits at CB, stored as int and static because these rates are the same for all banks
		static int RateDepositCB;
		//rate for loan quoted by Central Bank
		static int RateLoanCB;
		//set deposit offered for deposits
		void SetDepositRate(int r);
		//set rate asked for loans
		void SetLoanRate(int r);
		//set the range for possible rates
		void SetDepositRateRange(double a, double b);

		void SetLoanRateRange(double a, double b);

		void IncreaseEquity(double e);

		void SetEquityRange(double a, double b);

		int GetDepositRate();

		int GetLoanRate();

		void SetEquity(double e);

		double GetEquity();

		int GetNumberStatesVisited();

		double LastStateActionValue();

		int GetAR();

		void SetAR(int r);

		void StopUpdatingProcess();

		void StartUpdatingProcess();

		void SetNPartners(int n);

		int GetAllowedNPartners();

		void AddPartner(Commercial_bank* p);

		int GetNPartners();


	private:
		static bool UpdatingProcess;
		//Number of partners
		int partners;
		//Set of partners
		std::set<Commercial_bank*> PartnersSet;
		//Agency rate
		int AR;
		//map with pointers to partners, the borrowing rates and agency rates as keys (at the same rate, select the partner with best agency rate)
		std::map< int, std::map< int, Commercial_bank* > > PartnersVectorBR;
		//map with pointers to partners, the borrowing rates and agency rates as keys (at the same rate, select the partner with best agency rate)
		std::map< int, std::map< int, Commercial_bank* > > PartnersVectorLR;
		// Queue with deposits and loans, useful when looking for the next internal event
		std::deque<Loan> qt;
		// Queue for deposits
		std::deque<Loan> qd;
		// Queue for loans
		std::deque<Loan> ql;
		// Queue for loans that will go on default
		// Time remaining on the deposit
		double tleft; 
	    adevs::rv def;
	    unsigned int defaults;
	    // State var, equity level
		double equity;
		double Pdefault;
		// Function to sort deposits or loans
		static bool NextExpiring(const Loan& l1 , const Loan& l2);
		// This is used to see if a loan will default, reflecting the default probability of loans associated with the bank (not loan)

		bool WillItDefault();

		double profits;
		// All possible rates that can used for deposits
		std::vector<double> DepositRates;
		// All possible rates that can be used for loans
		std::vector<double> LoanRates;
		// Possible equity states
		//std::vector<double> Equities;
		//Keep in memory last state visited
		std::vector<int> LastState;
		//Keep in memory last action
		std::vector<int> LastAction;
		//Vector of actions
		std::vector< std::vector<int> > ACustomerLoan;
		std::vector< std::vector<int> > ACustomerDeposit;
		std::vector< std::vector<int> > AIBLoan;
		std::vector< std::vector<int> > AIBDeposit;

		vector<int> accept;
		vector<int> reject;
		vector<int> SendToBacklog;
		vector<int> UseCR;
		vector<int> GoToInterbankMarket;
		vector<int> GoToCB;

		// State variable current deposit rate
		int DepositRate;
		// State variable, current loan rate
		int LoanRate;

		// State vector and action vector as key, value as value
		static std::map<std::vector<int>, std::map< double, std::vector<int> > > ValueFunction;
		static std::map< double,std::vector<int> > ValueFunction_data;

		static std::map<std::vector<int>, std::map< std::vector<int>, double > > ValueFunction2;
		static std::map< std::vector<int>, double > ValueFunction2_data;

		static std::map<std::vector<int>, std::map< std::vector<int>, double > > InitVal;
		static std::map< std::vector<int>, double >	InitVal_data;
		// Counter for algorithm, number of iterations that state has been visited
		static std::map< std::vector<int>, std::map<vector<int>,int > > h;
		static std::map<vector<int>,int > h_data;

		// Function to set initial value of actions
		void SetInitialValue(std::vector<int> state, std::vector<int> action, double v);
		//Function to set continuation value (w)
		void UpdateW(std::vector<int> state, std::vector<int> action, double v);








};

#endif
