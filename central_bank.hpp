
#ifndef CENTRAL_BANK_HPP_
#define CENTRAL_BANK_HPP_


#include <cmath>
#include <boost/function.hpp>
#include <boost/math/distributions/binomial.hpp>
#include <stdexcept>

/**
 * Central Bank model is instantiated in an aggregation relationship in the Commercial Bank.
 * This approach will save us 2 roundtrips of events from CB to Commercial Bank per each request
 * to the DF or LF. Overall monitoring of different instance could be provided via common
 * monitor instance.
 */
template <typename Time_t>
class ICentralBankModel
{
public:
  typedef typename ::boost::function<ICentralBankModel* (
      ISimulationParameters<Time_t>* const simulation_parameters,
      UniformRVGenerator* const binomial_inversion_proba_generator) > constructor_factory;
public:
  ICentralBankModel(ISimulationParameters<Time_t>* const simulation_parameters,
                    UniformRVGenerator* const binomial_inversion_proba_generator)
  : simulation_parameters(simulation_parameters)
  , binomial_inversion_proba_generator(binomial_inversion_proba_generator) {}
  virtual ~ICentralBankModel() {}
public: // Interface definition.
  // Handle cash.
  virtual bool is_reserves_requirement_satisfied(const Time_t now, const Money_t outstanding_obligations) const = 0;
  virtual Money_t get_present_value_of_cash_in_depository_facility(const Time_t now) const = 0;
  virtual Money_t put_cash_to_depository_facility(const Time_t now, const Money_t additional_cash_amount) = 0;
  virtual Money_t get_cash_from_depository_facility(const Time_t now, const Money_t requested_cash_amount) = 0;
  virtual Money_t get_cash_amount_of_minimal_required_reserves(const Time_t now, const Money_t outstanding_obligations) const = 0;
  virtual bool can_grant_loan(const Money_t amount) const = 0;
protected:
  ISimulationParameters<Time_t>* const simulation_parameters;
  UniformRVGenerator* const binomial_inversion_proba_generator;
};

template <typename Time_t>
class CentralBankModel : public ICentralBankModel<Time_t>
{
public:
  CentralBankModel(ISimulationParameters<Time_t>* const simulation_parameters,
                   UniformRVGenerator* const binomial_inversion_proba_generator)
  : ICentralBankModel<Time_t>(simulation_parameters, binomial_inversion_proba_generator)
  , cash_deposit_from_time(adevs_zero<Time_t>())
  , cash_deposit_amount(0) {}
  virtual ~CentralBankModel() {}
public: // Interface implementation.
  bool is_reserves_requirement_satisfied(const Time_t now, const Money_t outstanding_obligations) const
  {
    return (get_present_value_of_cash_in_depository_facility(now) >=
            this->simulation_parameters->get_minimum_reserve_requirements()*outstanding_obligations);
  }

  Money_t get_present_value_of_cash_in_depository_facility(const Time_t now) const
  {
    Money_t present_value_of_cash_deposit = compute_future_value_of_cash<Time_t>(
        cash_deposit_amount, cash_deposit_from_time, now, this->simulation_parameters->get_central_bank_depository_facility_rate());
    return present_value_of_cash_deposit;
  }

  Money_t put_cash_to_depository_facility(const Time_t now, const Money_t additional_cash_amount)
  {
    cash_deposit_amount = get_present_value_of_cash_in_depository_facility(now) + additional_cash_amount;
    cash_deposit_from_time = now;
    return cash_deposit_amount;
  }

  Money_t get_cash_from_depository_facility(const Time_t now, const Money_t requested_cash_amount)
  {
    Money_t present_value_of_cash_account = get_present_value_of_cash_in_depository_facility(now);
    if (present_value_of_cash_account < requested_cash_amount) throw ::std::invalid_argument("Requested more money than available in cash account");

    cash_deposit_from_time = now;
    cash_deposit_amount = present_value_of_cash_account - requested_cash_amount;
    return cash_deposit_amount;
  }

  Money_t get_cash_amount_of_minimal_required_reserves(const Time_t now, const Money_t outstanding_obligations) const
  {
    return this->simulation_parameters->get_minimum_reserve_requirements()*outstanding_obligations;
  }

  bool can_grant_loan(const Money_t amount) const
  {
    // Here we consider a loan as a sequence of loans of a one unit given by the CB with specified rejection proba.
    int binomial_coeff_support = std::ceil(amount);
    try
    {
      double proba_accum = 0.;
      double proba_granted = 1. - this->simulation_parameters->get_loan_rejection_proba_from_lending_facility();
      auto binomial_distr_rv = ::boost::math::binomial(binomial_coeff_support, proba_granted);
      // This is just an inverse algorithm to generate a binomial rv.
      for (int i = 0; i < binomial_coeff_support; ++i)
      {
        proba_accum += ::boost::math::pdf(binomial_distr_rv, i);
      }
      return proba_accum <= this->binomial_inversion_proba_generator->next();
    }
    catch (...)
    {
      return false;
    }
  }
private:
  Time_t cash_deposit_from_time;
  Money_t cash_deposit_amount;
};

#endif /* CENTRAL_BANK_HPP_ */
