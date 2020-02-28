#ifndef _Bank_h_
#define _Bank_h_
#include "adevs.h"
#include "Loan.h"
#include <cassert>
#include <deque>
#include <algorithm>	// Needed for sort() method#include <map>
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
	unsigned int getIBLoanRequestQueueSize();
	unsigned int getIBLoanAcceptedQueueSize();
	int getCash();
	int getCBLendingFacility();

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
	static void setRateRange(int p);

	static void SetEquityRange(double a, double b);

	void IncreaseEquity(int e);

	void SetEquity(int e);

	int GetEquity();

	int GetNumberStatesVisited();

	double LastStateActionValue();

	int GetAR();

	void SetAR(int r);

	static void StopUpdatingProcess();

	static void StartUpdatingProcess();

	void SetNPartners(int n);

	int GetAllowedNPartners();

	void AddPartner(Bank* p);

	void UpdatePartnerInfo();

	int GetNPartners();

	void SendRating();

	void SetLqBnch();

	string printPartners();

	void updateState(int type, Loan loan);

	static int GetCDepositRate();

	static int GetCLoanRate();

	int getID();

	int getReplacesBank();

	void setReplacesBank(int bank);

	bool operator ==(Bank a) const;

	double GetProbDef();

	int GetAssets();

	void Liquidate();

	static int getBankCounter();

	bool isPartner(Bank* bank);

	int getIDCanceledLoan();

	int getSenderCanceledRequest();

	void setCancelingLoanToFalse();

	void setRejectingLoanToFalse();

	bool isCancelingLoan();

	bool isRejectingIBLRequest();

	void resolveCanceledLoan(int IDCanceledLoan);

	int getLF();

	int getCD();

	int getIBD();

	int getCSH();

	int getIBL();

	// We use this function to send the IDs of IB deposits held, so the network decides which will be paid to depositors.
	std::vector<int> sendIBLiabilitiesIDs();

	static void setTime(double t);

	static double getWdV();

	static void fillActionsVector();
private:

	// max absolute value of the difference between successive Valuefunctions
	static double WdV;

	double linkcost;

	int sender;

	bool rejectingIBloanrequest;

	static double probTrembling;

	bool isTrembling;

	static int bankCounter;

	static double time; // Absolute time set from the simulator

	std::ofstream log;

	std::ofstream variablesDebugging;


	int ID;

	int ReplacesBank;

	static bool UpdatingProcess;
	//Number of partners
	int partners;
	//Set of partners
	std::set<Bank*> PartnersVector;

	static std::map<Agent*, int> ActualAgencyRatings;

	static std::map<Agent*, int> LaggedAgencyRatings;

	//map with pointers to partners, the deposit rates as keys
	std::map<int, std::vector<Bank*> > PartnersVectorDR;
	//map with pointers to partners, the lending rates as keys
	std::map<int, std::vector<Bank*> > PartnersVectorLR;

	std::map<Bank*, int> PartnersVectorDRderef;

	std::map<Bank*, int> PartnersVectorLRderef;

	//Backlogs
	std::map<int, std::deque<Loan> > UMLoans;

	std::map<int, std::deque<Loan> > UMDeposits;

	// Map with value associated to matched IB loan
	std::map<int, Loan*> valueMatchedIBLoans;
	// Map with matched items, two maps to make a bidirectional relation
	std::map<Loan, Loan> matchMap1;
	std::map<Loan, Loan> matchMap2;

	// Map with loan and strategy used
	static std::map<int, Strategy*> loanStrategyMap;
	// Map with cumulative value (we are adding the value of each math for each object)
	static std::map<int, double> cumulativeObjectValue;

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
	// Cash level
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
	adevs::rv centralBank;
	adevs::rv randomMatch;
	adevs::rv maturityOfIBDepositRequest;
	adevs::rv tremblingRandomVariable;
	unsigned int defaults;
	double Pdefault;
	// Function to sort deposits or loans
	static bool NextExpiring(const Loan& l1, const Loan& l2);
	// This is used to see if a loan will default, reflecting the default probability of loans associated with the bank (not loan)

	bool WillItDefault();

	bool sendingIBDepositRequest;
	bool sendingIBLoanRequest;
	bool cancelingLoan;

	int canceledLoan;

	double profits;
	//rate for deposits at CB, stored as int and static because these rates are the same for all banks
	static int RateDepositCB;
	//rate for loan quoted by Central Bank
	static int RateLoanCB;
	// All possible rates that can used for deposits
	static std::vector<double> Rates;

	// Possible equity states
	//std::vector<double> Equities;
	//Keep in memory last state visited
	//std::vector<int> LastState;
	State LastState;

	std::set<Action> actionsSet;

	std::set<Action*> actionsSetPointers;

	std::vector<Action*> actionsVector;


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

	static std::map<State, std::map<double, Action> > ValueFunction;
	static std::map<double, Action> ValueFunction_data;

	//static std::map< Strategy, double > ValueFunction2;
	//static std::map<std::vector<int>, std::map< std::vector<int>, double > > ValueFunction2;
	//static std::map< std::vector<int>, double > ValueFunction2_data;

	static std::map<State, std::map<Action, double> > ValueFunction2;
	static std::map<Action, double> ValueFunction2_data;

	//static std::map< Strategy, double > InitVal;
//		static std::map<std::vector<int>, std::map< std::vector<int>, double > > InitVal;
//		static std::map< std::vector<int>, double >	InitVal_data;

	static std::map<State, std::map<Action, double> > InitVal;
	static std::map<Action, double> InitVal_data;

	// Counter for algorithm, number of iterations that state has been visited
	static std::map<State, std::map<Action, int> > h;
	static std::map<Action, int> h_data;

//		static std::map< std::vector<int>, std::map<vector<int>,int > > h;
//		static std::map<vector<int>,int > h_data;

	// Function to set initial value of actions
	void SetInitialValue(State state, Action action, double v);
	//Function to set continuation value (w)
	void UpdateW(Strategy* S, double v);

	bool CheckCashReserveRequirements();
	//Update partners set depending on whether the last action involves a deposit or loan, a = 1 for deposit, a = 2, for loan
	void UpdatePartnersSet(int a);

	void ReplaceBankruptPartner(Bank* bank);

	std::vector<Loan*> lbank;

	// Last option when the bank need cash
	void askCentralBank();

	void initializeActionSet();
	// This functions are used when the bank need to close positions in order to increase CSH
	void cancelLoan(int IDCanceledLoan);
	void resolveIlliquidity();
	// Reject an IB Loan request (Almost the same that cancelLoan)
	void rejectLoanRequest(int IDCanceledLoan, int requestsender);

	void updateBacklogs();

	void newMatch(Loan l1,Loan l2);

	void saveToStrategyMap(int ID);

	void updateCumulativeValueObject(Loan l,bool isThereDefault);

};

#endif
