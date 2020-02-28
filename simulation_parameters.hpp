#ifndef IB_MARKET_SIM
#define IB_MARKET_SIM

#include <istream>
#include <list>
#include <stdexcept>

template <typename Time_t>
class SimulationParameters : public ISimulationParameters<Time_t>
{
public:
  SimulationParameters() 
  : ibmarket_transaction_fee(.0003)
  , model_maximal_runtime(0)
  , rate_levels_number(2)
  , external_bank_neighborhood_number(3)
  , number_of_days_in_year(252)
  , central_bank_depository_facility_rate(Percent_t(.015))
  , central_bank_lending_facility_rate(Percent_t(.035))
  , minimum_reserve_requirements(Percent_t(.02))
  , loan_rejection_proba_from_lending_facility(Percent_t(.05))
  , periodicity(1./252.)
  , lag_days_number(21)
  , downgrading_ratio_of_type_1(Percent_t(.5))
  , downgrading_ratio_of_type_2(Percent_t(.03))
  , customer_loan_interest_rate(Percent_t(.055))
  , customer_loan_non_repayment_probability(Percent_t(.15))
  , customer_loans_arrival_rate(.1)
  , customer_loans_maturity_parameter(1./252.)
  , customer_deposit_interest_rate(Percent_t(.025))
  , customer_deposit_arrival_rate(.1)
  , customer_deposit_maturity_parameter(1./252.)
  , number_of_commercial_banks(10)
  , reentry_rate(1./504.)
  , reappearance_delay(0)
  , bankruptcy_cost(Percent_t(.2))
  , initial_liquidity(2)
  , initial_rating(GOOD)
  , alpha_liquidity_level(Percent_t(.0003))
  , beta_liquidity_level(Percent_t(.04))
  , liquidity_penalty_rate(.003) {}
  virtual ~SimulationParameters() {}
private:
  // SimulationParameters(const SimulationParameters<Time_t>& that) {};
public: // Interface definition
  void initialize(::std::istream& istream)
  {
    std::string new_line;
    auto skip_end_of_line = [&] () {  std::getline(istream, new_line); };
    // IB Model parameters
    istream >> model_maximal_runtime; skip_end_of_line();
    if (adevs_zero<Time_t>() == model_maximal_runtime) model_maximal_runtime = adevs_inf<Time_t>();
    if (model_maximal_runtime < adevs_zero<Time_t>()) throw ::std::invalid_argument("Model maximal runtime < 0");

    istream >> ibmarket_transaction_fee; skip_end_of_line();
    if (0 >= ibmarket_transaction_fee || 1 <= ibmarket_transaction_fee) throw ::std::invalid_argument("CLink is outside of ]0;1[");

    istream >> rate_levels_number; skip_end_of_line();
    if (0 == rate_levels_number) throw ::std::invalid_argument("# rate levels > 0");

    istream >> external_bank_neighborhood_number; skip_end_of_line();
    if (0 == external_bank_neighborhood_number) throw ::std::invalid_argument("# external bank neighborhood > 0"); 

    istream >> number_of_days_in_year; skip_end_of_line();
    // End of IB Model parameters
    skip_end_of_line();

    // Central Bank
    double central_bank_depository_facility_rate_param;
    istream >> central_bank_depository_facility_rate_param; skip_end_of_line();
    central_bank_depository_facility_rate = Percent_t(central_bank_depository_facility_rate_param);

    double central_bank_lending_facility_rate_param;
    istream >> central_bank_lending_facility_rate_param; skip_end_of_line();
    central_bank_lending_facility_rate = Percent_t(central_bank_lending_facility_rate_param);

    double minimum_reserve_requirements_param;
    istream >> minimum_reserve_requirements_param; skip_end_of_line();
    minimum_reserve_requirements = Percent_t(minimum_reserve_requirements_param);

    double loan_rejection_proba_from_lending_facility_param;
    istream >> loan_rejection_proba_from_lending_facility_param; skip_end_of_line();
    loan_rejection_proba_from_lending_facility = Percent_t(loan_rejection_proba_from_lending_facility_param);
    // End of Central Bank
    skip_end_of_line();
    
    // Rating Agency
    istream >> periodicity; skip_end_of_line();
    if (adevs_zero<Time_t>() >= periodicity) throw ::std::invalid_argument("Periodicity <= 0");

    istream >> lag_days_number; skip_end_of_line();
    if (adevs_zero<Time_t>() >= lag_days_number) throw ::std::invalid_argument("Lag days <= 0");
    
    double downgrading_ratio_of_type_1_param;
    istream >> downgrading_ratio_of_type_1_param; skip_end_of_line();
    downgrading_ratio_of_type_1 = Percent_t(downgrading_ratio_of_type_1_param);

    double downgrading_ratio_of_type_2_param;
    istream >> downgrading_ratio_of_type_2_param; skip_end_of_line();
    downgrading_ratio_of_type_2 = Percent_t(downgrading_ratio_of_type_2_param);
    // End of Rating Agency
    skip_end_of_line();

    // Customer Loans
    double customer_loan_interest_rate_param;
    istream >> customer_loan_interest_rate_param; skip_end_of_line();
    customer_loan_interest_rate = Percent_t(customer_loan_interest_rate_param);

    double customer_loan_non_repayment_probability_param;
    istream >> customer_loan_non_repayment_probability_param; skip_end_of_line();
    customer_loan_non_repayment_probability = Percent_t(customer_loan_non_repayment_probability_param);

    istream >> customer_loans_arrival_rate; skip_end_of_line();
    if (0 >= customer_loans_arrival_rate) throw ::std::invalid_argument("Customer loans arrival rate <= 0");

    istream >> customer_loans_maturity_parameter; skip_end_of_line();
    if (0 >= customer_loans_maturity_parameter) throw ::std::invalid_argument("Customer loans maturity parameter <= 0");
    // End of Customer Loans
    skip_end_of_line();

    // Customer Deposits
    double customer_deposit_interest_rate_param;
    istream >> customer_deposit_interest_rate_param; skip_end_of_line();
    customer_deposit_interest_rate_param = Percent_t(customer_deposit_interest_rate_param);

    istream >> customer_deposit_arrival_rate; skip_end_of_line();
    if (0 >= customer_deposit_arrival_rate) throw ::std::invalid_argument("Customer deposit arrival rate <= 0");

    istream >> customer_deposit_maturity_parameter; skip_end_of_line();
    if (0 >= customer_deposit_maturity_parameter) throw ::std::invalid_argument("Customer deposit maturity parameter <= 0");
    // End of Customer Deposits
    skip_end_of_line();
    
    // Commercial Bank
    istream >> number_of_commercial_banks; skip_end_of_line();
    if (0 >= number_of_commercial_banks) throw ::std::invalid_argument("# commercial banks <= 0");

    istream >> reentry_rate; skip_end_of_line();
    if (0 >= reentry_rate) throw ::std::invalid_argument("Reentry rate <= 0");

    istream >> reappearance_delay; skip_end_of_line();
    if (adevs_zero<Time_t>() > reappearance_delay) throw ::std::invalid_argument("Reappearance delay < 0"); 

    double bankruptcy_cost_param;
    istream >> bankruptcy_cost_param; skip_end_of_line();
    bankruptcy_cost = Percent_t(bankruptcy_cost_param);

    istream >> initial_liquidity; skip_end_of_line();
    if (0 > initial_liquidity) throw ::std::invalid_argument("Initial liquidity is negative");
  
    int initial_rating_param;
    istream >> initial_rating_param; skip_end_of_line();
    initial_rating = static_cast<Rating>(initial_rating_param);

    double alpha_liquidity_level_param;
    istream >> alpha_liquidity_level_param; skip_end_of_line();
    alpha_liquidity_level = Percent_t(alpha_liquidity_level_param);

    double beta_liquidity_level_param;
    istream >> beta_liquidity_level_param; skip_end_of_line();
    beta_liquidity_level = Percent_t(beta_liquidity_level_param);

    double liquidity_penalty_rate_param;
    istream >> liquidity_penalty_rate_param; skip_end_of_line();
    liquidity_penalty_rate = liquidity_penalty_rate_param;
    // End of Commercial Bank
    skip_end_of_line();
  }
  // Utility functions
  Percent_t rate_level_to_real_rate(RateLevel_t level) const
  {
    if (CUSTOMER_DEPOSIT_RATE_LEVEL == level) return get_customer_deposit_interest_rate();
    if (CUSTOMER_LOAN_RATE_LEVEL == level) return get_customer_loan_interest_rate();
    auto P = get_rate_levels_number();
    double dp = (get_central_bank_lending_facility_rate()-get_central_bank_depository_facility_rate())/(2*P);
    double real_rate = get_central_bank_depository_facility_rate()+dp*level;
    return Percent_t(real_rate);
  }
  // IB Model parameters
  Time_t get_model_maximal_runtime() const { return model_maximal_runtime; }
  double get_ibmarket_transaction_fee() const { return ibmarket_transaction_fee; }
  unsigned int get_rate_levels_number() const { return rate_levels_number; }
  unsigned int get_external_bank_neighborhood_number() const { return external_bank_neighborhood_number; }
  unsigned int get_number_of_days_in_year() const { return number_of_days_in_year; }
  // Central Bank
  Percent_t get_central_bank_depository_facility_rate() const { return central_bank_depository_facility_rate; }
  Percent_t get_central_bank_lending_facility_rate() const { return central_bank_lending_facility_rate; }
  Percent_t get_minimum_reserve_requirements() const { return minimum_reserve_requirements; }
  Percent_t get_loan_rejection_proba_from_lending_facility() const { return loan_rejection_proba_from_lending_facility; }
  // Rating Agency
  Time_t get_rating_periodicity() const { return periodicity; }
  unsigned int get_lag_days_number() const { return lag_days_number; }
  Percent_t get_downgrading_ratio_of_type_1() const { return downgrading_ratio_of_type_1; }
  Percent_t get_downgrading_ratio_of_type_2() const { return downgrading_ratio_of_type_2; }
  // Customer Loans
  Percent_t get_customer_loan_interest_rate() const { return customer_loan_interest_rate; }
  Percent_t get_customer_loan_non_repayment_probability() const { return customer_loan_non_repayment_probability; }
  double get_customer_loans_arrival_rate() const { return customer_loans_arrival_rate; }
  double get_customer_loans_maturity_parameter() const { return customer_loans_maturity_parameter; }
  // Customer Deposits
  Percent_t get_customer_deposit_interest_rate() const { return customer_deposit_interest_rate; }
  double get_customer_deposit_arrival_rate() const { return customer_deposit_arrival_rate; }
  double get_customer_deposit_maturity_parameter() const { return customer_deposit_maturity_parameter; }
  // Commercial Bank
  unsigned int get_number_of_commercial_banks() const { return number_of_commercial_banks; }
  double get_reentry_rate() const { return reentry_rate; }
  Time_t get_reappearance_delay() const { return reappearance_delay; }
  Percent_t get_bankruptcy_cost() const { return bankruptcy_cost; }
  double get_initial_liquidity() const { return initial_liquidity; }
  Rating get_initial_rating() const { return initial_rating; }
  Percent_t get_alpha_liquidity_level() const { return alpha_liquidity_level; }
  Percent_t get_beta_liquidity_level() const { return beta_liquidity_level; }
  Money_t get_liquidity_penalty() const { return liquidity_penalty_rate; }
private:
  double ibmarket_transaction_fee;
  Time_t model_maximal_runtime;
  unsigned int rate_levels_number;
  unsigned int external_bank_neighborhood_number;
  unsigned int number_of_days_in_year;
  Percent_t central_bank_depository_facility_rate;
  Percent_t central_bank_lending_facility_rate;
  Percent_t minimum_reserve_requirements;
  Percent_t loan_rejection_proba_from_lending_facility;
  Time_t periodicity;
  unsigned int lag_days_number;
  Percent_t downgrading_ratio_of_type_1;
  Percent_t downgrading_ratio_of_type_2;
  Percent_t customer_loan_interest_rate;
  Percent_t customer_loan_non_repayment_probability;
  double customer_loans_arrival_rate;
  double customer_loans_maturity_parameter;
  Percent_t customer_deposit_interest_rate;
  double customer_deposit_arrival_rate;
  double customer_deposit_maturity_parameter;
  unsigned int number_of_commercial_banks;
  double reentry_rate;
  Time_t reappearance_delay;
  Percent_t bankruptcy_cost;
  double initial_liquidity;
  Rating initial_rating;
  Percent_t alpha_liquidity_level;
  Percent_t beta_liquidity_level;
  Money_t liquidity_penalty_rate;
};

#endif
