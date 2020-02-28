#ifndef _Bank_h_
#define _Bank_h_
#include "adevs.h"
#include "Loan.h"
#include <cassert>
#include <deque>
#include <algorithm>	// Needed for sort() method
#include <map>
#include <fstream>

class Bank: public adevs::Atomic<Loan>, public Agent

{
	public:
		Bank(double ProbabilityDefault);
		~Bank();
		void delta_int();
		void delta_ext(double e, const adevs::Bag<Loan>& xb);
		void delta_conf(const adevs::Bag<Loan>& xb);
		void output_func(adevs::Bag<Loan>& yb);
		double ta();
		bool model_transition();
		unsigned int getDepositQueueSize();
		unsigned int getLoanQueueSize();
		unsigned int getIBDepositRequestQueueSize();
		unsigned int getIBDepositAcceptedQueueSize();

		void gc_output(adevs::Bag<Loan>&);
		//double value();
		void reduce_equity(double shock);
		bool checkInsolvent();

		//set deposit offered for customer deposits
		static void SetCDepositRate(int r);
		//set rate asked for customer loans
		static void SetCLoanRate(int r);

//		static int GetCDepositRate();
//
//		static int GetCLoanRate();

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

		void AddPartner(Bank* p);

		void UpdatePartnerInfo();

		int GetNPartners();

		void SendRating();

		void SetLqBnch();

		string printPartners();

		void updateState(int type, Loan* loan);

		static int GetCDepositRate();

		static int GetCLoanRate();

		int getID();

		int getReplacesBank();

		void setReplacesBank(int bank);

		bool operator ==  (Bank a) const;

		double GetProbDef();

		int GetAssets();

		void Liquidate();

		static int getBankCounter();



	private:

		static int bankCounter;

		std::ofstream log;

		int ID;

		int ReplacesBank;

		static bool UpdatingProcess;
		//Number of partners
		int partners;
		//Set of partners
		std::set<Bank*> PartnersVector;

		static std::map< Agent*, int > ActualAgencyRatings;

		static std::map< Agent*, int > LaggedAgencyRatings;

		//map with pointers to partners, the deposit rates as keys
		std::map< int, Bank* > PartnersVectorDR;
		//map with pointers to partners, the lending rates as keys
		std::map< int, Bank* > PartnersVectorLR;

		std::map<Bank*, int > PartnersVectorDRderef;

		std::map<Bank*, int > PartnersVectorLRderef;

		//Backlogs
		std::map< int , std::deque<Loan*> > UMLoans;

		std::map< int , std::deque<Loan*> > UMDeposits;

		// Queue with deposits and loans, useful when looking for the next internal event
		std::deque<Loan> qt;

		// LiquidityBenchmark
		double LqBnch;

		// Liabilities
		// Queue for customer deposits
		//std::deque<Loan> qd;
		int qd;
		// Queue for interbank deposits accepted
		//std::deque<Loan> qida;
		int qida;
		// Queue for CB borrowings, LF
		//std::deque<Loan> qcblf;
		int qcblf;
		// State var, equity level
		int equity;

		// Queue for interbank loans requested
		std::deque<Loan> qilrQueue;
		int qilr;

		// Assets
		//Cash level
		int CSH;
		// Queue for customer loans
		//std::deque<Loan> ql;
		int ql;
		// Queue for interbank loans accepted
		//std::deque<Loan> qila;
		int qila;

		// Queue for interbank deposits requested
		std::deque<Loan> qidrQueue;
		int qidr;

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

		bool sendingIBDepositRequest;
		bool sendingIBLoanRequest;



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
		//std::vector<int> LastState;
		State LastState;


		std::set<Action> actionsSet;

		std::set<Action*> actionsSetPointers;

		//I tried with pointers directly but I got some errors, so I continued
		//Keep in memory last action
		//std::vector<int> LastAction;
		Action LastAction;
		//Vector of actions
//		std::vector< std::vector<int> > ACustomerLoan;
//		std::vector< std::vector<int> > ACustomerDeposit;
//		std::vector< std::vector<int> > AIBLoan;
//		std::vector< std::vector<int> > AIBDeposit;

//		vector<int> accept;
//		vector<int> reject;
//		vector<int> SendToBacklog;
//		vector<int> UseCR;
//		vector<int> GoToInterbankMarket;
//		vector<int> GoToCB;

		// current customer deposit rate
		static int CDepositRate;
		// current customer loan rate
		static int CLoanRate;
		// current IB deposit rate
		int IBDepositRate;
		// current IB loan rate
		int IBLoanRate;

		// State vector and action vector as key, value as value
//		static std::map<std::vector<int>, std::map< double, std::vector<int> > > ValueFunction;
//		static std::map< double,std::vector<int> > ValueFunction_data;

		static std::map< State, std::map< double, Action > > ValueFunction;
		static std::map< double, Action > ValueFunction_data;

		//static std::map< Strategy, double > ValueFunction2;
		//static std::map<std::vector<int>, std::map< std::vector<int>, double > > ValueFunction2;
		//static std::map< std::vector<int>, double > ValueFunction2_data;

		static std::map< State, std::map< Action, double > > ValueFunction2;
		static std::map< Action, double > ValueFunction2_data;


		//static std::map< Strategy, double > InitVal;
//		static std::map<std::vector<int>, std::map< std::vector<int>, double > > InitVal;
//		static std::map< std::vector<int>, double >	InitVal_data;

		static std::map<State, std::map< Action, double > > InitVal;
		static std::map< Action, double >	InitVal_data;

		// Counter for algorithm, number of iterations that state has been visited
		static std::map< State, std::map< Action ,int > > h;
		static std::map< Action ,int > h_data;


//		static std::map< std::vector<int>, std::map<vector<int>,int > > h;
//		static std::map<vector<int>,int > h_data;

		// Function to set initial value of actions
		void SetInitialValue(State state, Action action, double v);
		//Function to set continuation value (w)
		void UpdateW(State state, Action action, double v);

		bool CheckCashReserveRequirements();
		//Update partners set depending on whether the last action involves a deposit or loan, a = 1 for deposit, a = 2, for loan
		void UpdatePartnersSet(int a);

		void ReplaceBankruptPartner(Bank* bank);

		std::vector<Loan*> lbank;

		//The following functions are part of the re-entry process
		//void CancelLoan();
		//void ResolveCancelledLoan();



};

#endif
