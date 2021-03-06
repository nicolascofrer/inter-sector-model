#ifndef __Parameters_h_
#define __Parameters_h_

#define seed 1;
//Parameters from
//1. # of Commercial Banks,							10
#define NBANKS   10 //Number of banks at the beginning
//4. # of external banks in the neighborhood,							3
#define NNeighborhood	3
#define cash0	100.0
#define equity0	100.0
// Time space for illiquid asset process
#define	dt 0.0001
//2. Customer Loans non repayment probability in ]0,1[,							.15
#define PDefaul1Loan1	0.0375	//Prob default loans that arrive at first group
#define PDefaultLoan2	0.0375	//Prob default loans that arrive at second group
//Deposit loans param
//4. Customer Loans maturity param.							.003968254, 1 day
#define BorrowerMaturityParameter	1.0
//3. Customer Deposits maturity parameter.							.003968254, 1 day
#define DepositorMaturityParameter	1.0
//Generator param
//2. Customer Deposits arrival rate,							.1
#define Df 10.0
//3. Customer Loans arrival rate,							.1
#define Lf 10.0
//Central Bank Parameters
//1. Depository facility rate in ]0,1[,							.015
#define CBDFR (0.025/252)
//2. Lending facility rate in ]0,1[,							.035
#define CBLFR (0.045/252)
//3. Minimum reserve requirements in ]0,1[,							.02
#define beta1 0.02
//4. Loan to LF rejection probability in ]0,1[.							.05
#define phiCB 0.05

//1. Periodicity of rating assignment as a proportion of base time unit,
#define delta 0.0205
//2. A Lag period for observations from Balance Sheets of Commercial Banks by a Rating Agency,
#define lag 21.0
//3. Downgrade ratio of type 1 in ]0,1[,
#define DowngradeRatio1 0.5
//4. Downgrade ratio of type 2 in ]0,1[.
#define DowngradeRatio2 0.03


//Interbank Model Parameters
//1. Model maximal run time (=0 considered as Inf),							.5
//2. IB market transaction fee,							.0003
#define Clink 0.0003
//3. P, # of rates levels, 2p+1
#define pRatesLevels 3.0
//5. # of days in a year.							252
//Rating Agency Parameters

//Customer Loans Parameters
//1. Customer Loans interest rate in ]0,1[,							.055
#define customerLoanInterestRate (0.055/252)
//Customer Deposit Parameters
//1. Customer Deposits interest rate in ]0,1[,							.025
#define customerDepositInterestRate	(0.015/252)
//Commercial Bank Parameters
//2. Reentry rate,							.001984127
//3. Reappearance delay after failure,							0
//4. Bankruptcy cost in ]0,1[,							.2
#define BankruptcyCost	0.2
//5. Initial liquidity,							2
//7. Alpha liquidity level in ]0,1[,							.0003
#define alphaLiquidity 0.0025
//8. Beta liquidity level in ]0,1[,							.04
#define beta2 0.1
//9. Amount of liquidity penalty, w.							1.19047619047619E-006
#define liquidityPenaltyParameter 0.00000119047619047619

#endif




