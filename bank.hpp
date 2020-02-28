

#include <algorithm>
#include <bitset>
#include <boost/assert.hpp>
#include <boost/function.hpp>
#include <boost/multi_index_container.hpp>
#include <boost/multi_index/composite_key.hpp>
#include <boost/multi_index/hashed_index.hpp>
#include <boost/multi_index/member.hpp>
#include <boost/multi_index/mem_fun.hpp>
#include <boost/multi_index/ordered_index.hpp>
#include <boost/tuple/tuple.hpp>
#include <boost/unordered_map.hpp>
#include <queue>
#include <stdexcept>
#include <utility>

template <typename Time_t>
class RandomBankGeneratorLecuyer : public IRandomBankGenerator<Time_t>
{
public:
  RandomBankGeneratorLecuyer(::std::vector<ICommercialBankModelBase<Time_t>*>* const banks)
  : IRandomBankGenerator<Time_t>(banks) {}
  virtual ~RandomBankGeneratorLecuyer() {}
  ICommercialBankModelBase<Time_t>* const next()
  {
    int next_bank_num = rng_stream.RandInt(0, this->banks->size()-1);
    return this->banks->at(next_bank_num);
  }
private:
  RngStream rng_stream; // Streams are advanced automatically in constructor.
};

/**
 * DepositBacklogEntry
 */
template <typename Time_t, typename Entry_t>
struct BacklogEntry
{
  explicit BacklogEntry(RateLevel_t desired_rate, Entry_t* const entry,
               Time_t entry_time)
  : entry(entry),
    entry_time(entry_time),
    desired_rate(desired_rate),
    id(entry->id) {};
  Entry_t* const entry;
  Time_t entry_time;
  RateLevel_t desired_rate;
  TransactionsID_t id;
};

/**
 * tags.
 */
struct by_id {};
struct by_rate_by_time {};
struct by_lender_rate {};
struct by_depositor_rate {};

/**
 * CommercialBankModel
 */
template <typename Time_t>
class CommercialBankModel : public ICommercialBankModelBase<Time_t>
{
public:
  typedef typename ::boost::function<CommercialBankModel<Time_t>* (
      ISimulationParameters<Time_t>* const simulation_params,
      IDecisionTable<Time_t>* const deposit_decision_table,
      IDecisionTable<Time_t>* const loan_decision_table,
      ICentralBankModel<Time_t>* const central_bank,
      IRatingAgencyModel<Time_t>* const rating_agency,
      IRandomBankGenerator<Time_t>* const random_bank_generator) > constructor_factory;
public: // Type definitions.
  template <typename Entry_t>
  struct Backlog
  {
    typedef typename ::boost::multi_index_container<
      BacklogEntry<Time_t, Entry_t>,
      ::boost::multi_index::indexed_by<
        ::boost::multi_index::hashed_unique<
          ::boost::multi_index::tag<by_id>,
          ::boost::multi_index::member<BacklogEntry<Time_t, Entry_t>, const TransactionsID_t, &BacklogEntry<Time_t, Entry_t>::id>
        >,
        ::boost::multi_index::ordered_unique<
          ::boost::multi_index::tag<by_rate_by_time>,
          ::boost::multi_index::composite_key<
            BacklogEntry<Time_t, Entry_t>,
            ::boost::multi_index::member<BacklogEntry<Time_t, Entry_t>, const RateLevel_t, &BacklogEntry<Time_t, Entry_t>::desired_rate>,
            ::boost::multi_index::member<BacklogEntry<Time_t, Entry_t>, const Time_t, &BacklogEntry<Time_t, Entry_t>::entry_time>
          >,
          ::boost::multi_index::composite_key_compare<
            ::std::less<RateLevel_t>,
            ::std::less<Time_t>
          >
        >
      >
    > unmatched_entries_container_t;
    typedef typename ::boost::unordered_map<TransactionsID_t, Entry_t*> matched_entries_container_t;
    typedef typename ::boost::unordered_map<TransactionsID_t, ::boost::tuple<Entry_t*, Entry_t*> > entries_waiting_to_be_terminated_t;
  };
public: // Constructor-Destructor.
  CommercialBankModel(
      ISimulationParameters<Time_t>* const simulation_params,
      IDecisionTable<Time_t>* const deposit_decision_table,
      IDecisionTable<Time_t>* const loan_decision_table,
      ICentralBankModel<Time_t>* const central_bank,
      IRatingAgencyModel<Time_t>* const rating_agency,
      IRandomBankGenerator<Time_t>* const random_bank_generator)
  : ICommercialBankModelBase<Time_t>(
      simulation_params,
      deposit_decision_table,
      loan_decision_table,
      central_bank,
      rating_agency,
      random_bank_generator)
  {
    // Put initial equity into CB DF.
    Money_t initial_cash = simulation_params->get_initial_liquidity();
    if (initial_cash > 0)
    {
      central_bank->put_cash_to_depository_facility(adevs_zero<Time_t>(), initial_cash);
    }
  }

  virtual ~CommercialBankModel() {}

public: // ICommercialBankModel interface.

  bool add_external_bank_into_neighborhood(ICommercialBankModelBase<Time_t>* const external_bank)
  {
    // Can not add a bank itself to its neighbourhood.
    if (external_bank->get_id() == this->get_id()) return false;
    // Can not add a bank to a neighborhood if it is already there.
    if (0 != bank_neighborhood.get<by_id>().count(external_bank->get_id())) return false;
    // Add it in any other case.
    bank_neighborhood.get<by_id>().insert(external_bank);
    return true;
  }

  double get_liquid_assets_to_obligations_ratio() const
  {
    // TODO: it is a stub.
    return .1;
  }

  double get_equity_to_risky_assets_ratio() const
  {
    // TODO: it is a stub.
    return .1;
  }

  bool has_outstanding_ibloans() const
  {
    if (matched_loans.empty()) return false;
    auto it = matched_loans.begin();
    while (it != matched_loans.end())
    {
      if ((*it).second->counterpart_bank_id != CUSTOMER_ID)
      {
        return true;
      }
      ++it;
    }
    return false;
  }

  ILoan<Time_t>* get_ibloan_with_lowest_expected_return()
  {
    if (matched_loans.empty()) return NULL;
    // TODO write search for minimumal expected outcome.
    auto it = matched_loans.begin();
    while (it != matched_loans.end())
    {
      if ((*it).second->counterpart_bank_id != CUSTOMER_ID)
      {
        return (*it).second;
      }
      ++it;
    }
    return NULL;
  }


};

#endif /* COMMERCIAL_BANK_HPP_ */

