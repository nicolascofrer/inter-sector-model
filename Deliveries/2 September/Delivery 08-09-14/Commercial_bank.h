#ifndef _Commercial_bank_h_
#define _Commercial_bank_h_
#include "adevs.h"
#include "Loan.h"
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

		//set deposit offered for customer deposits
		void SetCDepositRate(int r);
		//set rate asked for customer loans
		void SetCLoanRate(int r);

		int GetCDepositRate();

		int GetCLoanRate();
		//set deposit offered for IB deposits
		void SetIBDepositRate(int r);
		//set rate asked for IB loans
		void SetIBLoanRate(int r);

		int GetIBDepositRate();

		int GetIBLoanRate();

		static void SetCBDepositRate(int r);
		static void SetCBLoanRate(int r);
		//set the range for possible rates
		static void SetDepositRateRange(double a, double b);

		static void SetLoanRateRange(double a, double b);

		static void SetEquityRange(double a, double b);

		void IncreaseEquity(int e);

		void SetEquity(int e);

		int GetEquity();

		int GetNumberStatesVisited();

		double LastStateActionValue();

		int GetAR();

		void SetAR(int r);

		void StopUpdatingProcess();

		void StartUpdatingProcess();

		void SetNPartners(int n);

		int GetAllowedNPartners();

		void AddPartner(Commercial_bank* p);

		void UpdatePartnerInfo();

		int GetNPartners();

		void SendRating();

		void SetLqBnch();


	private:
		static bool UpdatingProcess;
		//Number of partners
		int partners;
		//Set of partners
		std::set<Commercial_bank*> PartnersVector;

		static std::map< Commercial_bank*, int > ActualAgencyRatings;

		static std::map< Commercial_bank*, int > LaggedAgencyRatings;

		//map with pointers to partners, the deposit rates as keys
		std::map< int, Commercial_bank* > PartnersVectorDR;
		//map with pointers to partners, the lending rates as keys
		std::map< int, Commercial_bank* > PartnersVectorLR;

		std::map<Commercial_bank*, int > PartnersVectorDRderef;

		std::map<Commercial_bank*, int > PartnersVectorLRderef;

		//Backlogs
		std::map< int , Loan* > UMLoans;
		std::map< int , Loan* > UMDeposits;

		// Queue with deposits and loans, useful when looking for the next internal event
		std::deque<Loan> qt;

		// LiquidityBenchmark
		int LqBnch;

		// Liabilities
		// Queue for customer deposits
		std::deque<Loan> qd;
		// Queue for interbank deposits accepted
		std::deque<Loan> qida;
		// Queue for CB borrowings, LF
		std::deque<Loan> qcblf;
		// State var, equity level
		int equity;

		// Queue for interbank loans requested
		std::deque<Loan> qilr;


		// Assets
		//Cash level
		int CSH;
		// Queue for customer loans
		std::deque<Loan> ql;
		// Queue for interbank loans accepted
		std::deque<Loan> qila;

		// Queue for interbank deposits requested
		std::deque<Loan> qidr;


		// Time remaining on the deposit
		double tleft; 
	    adevs::rv def;
	    adevs::rv a;
	    unsigned int defaults;
		double Pdefault;
		// Function to sort deposits or loans
		static bool NextExpiring(const Loan& l1 , const Loan& l2);
		// This is used to see if a loan will default, reflecting the default probability of loans associated with the bank (not loan)

		bool WillItDefault();

		double profits;
		//rate for deposits at CB, stored as int and static because these rates are the same for all banks
		static int RateDepositCB;
		//rate for loan quoted by Central Bank
		static int RateLoanCB;
		// All possible rates that can used for deposits
		static std::vector<double> DepositRates;
		// All possible rates that can be used for loans
		static std::vector<double> LoanRates;
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

		// current customer deposit rate
		int CDepositRate;
		// current customer loan rate
		int CLoanRate;
		// current IB deposit rate
		int IBDepositRate;
		// current IB loan rate
		int IBLoanRate;

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

		bool CheckCashReserveRequirements();
		//Update partners set depending on whether the last action involves a deposit or loan, a = 1 for deposit, a = 2, for loan
		void UpdatePartnersSet(int a);

		//The following functions are part of the re-entry process
		//void CancelLoan();
		//void ResolveCancelledLoan();



};

#endif
