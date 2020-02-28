

#include "sqlite3.h"

#include <boost/assert.hpp>
#include <sstream>
#include <stdexcept>

extern const char* DB_CONNECTION_URI;

struct ExternalBankParameters
{
  Rating rating;
  RateLevel_t l_rate;
  RateLevel_t b_rate;
};

/**
 * SqliteObservableState
 */
class SqliteObservableState : public ObservableState
{
template <typename Time_t> friend class SqliteDecisionTable;
public:
  ObservableState* with_new_rate(const RateLevel_t new_rate)
  {
    BOOST_ASSERT_MSG(new_rate <= MAX_RATE_IDX_NUM(number_of_rate_levels), "New incoming rate > CB_L");
    BOOST_ASSERT_MSG(new_rate >= 0 || (CUSTOMER_DEPOSIT_RATE_LEVEL == new_rate) ||
      (CUSTOMER_LOAN_RATE_LEVEL == new_rate), "New incoming rate < CB_D and is not from customers");
    this->new_rate = new_rate;
    return this;
  }

  ObservableState* with_liquidity_benchmark_level(const CashToLiquidityBenchmarkLevel alpha_level)
  {
    BOOST_ASSERT_MSG((alpha_level == -1 || alpha_level == 1), "Invalid liquidity benchmark level");
    this->liquidity_benchmark_level = alpha_level;
    return this;
  }

  ObservableState* with_unmatched_deposits_backlog(const RateLevel_t lending_rate,
      const size_t cnt_at_lending_rate,
      const size_t cnt_lending_others)
  {
    this->lending_rate = lending_rate;
    this->cnt_at_lending_rate = cnt_at_lending_rate;
    this->cnt_lending_others = cnt_lending_others;
    return this;
  }

  ObservableState* with_unmatched_loans_backlog(const RateLevel_t borrowing_rate,
      const size_t cnt_at_borrowing_rate,
      const size_t cnt_borrowing_others)
  {
    this->borrowing_rate = borrowing_rate;
    this->cnt_at_borrowing_rate = cnt_at_borrowing_rate;
    this->cnt_borrowing_others = cnt_borrowing_others;
    return this;
  }

  ObservableState* with_own_credit_rating(const Rating rating)
  {
    own_rating = rating;
    return this;
  }

  ObservableState* with_external_bank(const int ith, const RateLevel_t l_rate, const RateLevel_t b_rate, const Rating rating)
  {
    BOOST_ASSERT_MSG(0 <= ith && ith < ext_banks_num, "Invalid index of the external bank");
    external_banks[ith].rating = rating;
    external_banks[ith].b_rate = b_rate;
    external_banks[ith].l_rate = l_rate;
    return this;
  }

  ObservableState* with_counterpart(const Rating cp_credit_rating, const RateLevel_t cp_borrowing_rate, const RateLevel_t cp_lending_rate)
  {
    this->cp_credit_rating = cp_credit_rating;
    this->cp_borrowing_rate = cp_borrowing_rate;
    this->cp_lending_rate = cp_lending_rate;
    return this;
  }

 ~SqliteObservableState() {}
private: // Constructor
  SqliteObservableState(const int number_of_rate_levels, const int ext_banks_num)
  : ObservableState(number_of_rate_levels, ext_banks_num)
  {
    external_banks.insert(external_banks.begin(), ext_banks_num, ExternalBankParameters());
  }
private: // State variables
  RateLevel_t new_rate;
  CashToLiquidityBenchmarkLevel liquidity_benchmark_level;
  Rating own_rating;
  RateLevel_t borrowing_rate;
  size_t cnt_at_borrowing_rate;
  size_t cnt_borrowing_others;
  RateLevel_t lending_rate;
  size_t cnt_at_lending_rate;
  size_t cnt_lending_others;
  std::vector<ExternalBankParameters> external_banks;
  Rating cp_credit_rating;
  RateLevel_t cp_borrowing_rate;
  RateLevel_t cp_lending_rate;
};

template <typename Time_t>
class SqliteDecisionTable : public IDecisionTable<Time_t>
{
public:
  SqliteDecisionTable(ISimulationParameters<Time_t>* const simulation_params, std::string table_name);
  ActionInState get_best_matching_rate(ObservableState* const state) const;
  ObservableState* get_observable_state_instance() const;
  Money_t get_expected_return_on_investment(ObservableState* const state, const RateLevel_t) const;
  void update_expected_roi_in_state(ObservableState* const state, const Money_t realized_return);
  void dump_to_file(std::string& destination_file_name) const;
  std::string get_table_name() const { return table_name; } // Return a copy.
  std::string get_idx_suffix() const { return "Idx"; }
  ~SqliteDecisionTable();
public: // Variables
  sqlite3 *db_connection;
private: // Member functions
  std::string table_name;
  void execute_ddl_statement(std::string& statement);
  std::string get_decisions_table_ddl() const;
  std::string get_decisions_index_ddl() const;
  inline std::stringstream& ext_lending_rate_col_name(std::stringstream& ss, const int ith) const;
  inline std::stringstream& ext_borrowing_rate_col_name(std::stringstream& ss, const int ith) const;
  inline std::stringstream& ext_balance_sheet_col_name(std::stringstream& ss, const int ith) const;
};

template <typename Time_t>
SqliteDecisionTable<Time_t>::SqliteDecisionTable(ISimulationParameters<Time_t>* const simulation_params,
                                                 std::string table_name)
: IDecisionTable<Time_t>(simulation_params), table_name(table_name)
{
  BOOST_ASSERT_MSG(!table_name.empty(), "Table name could not be empty");
  // DOC config options : http://www.sqlite.org/c3ref/c_config_covering_index_scan.html
  int rc = SQLITE_OK;// sqlite3_config(SQLITE_CONFIG_SINGLETHREAD);
  if (SQLITE_OK != rc)
  {
    std::stringstream ss;
    ss << "DB Configuration failed with error #" << rc;
    throw std::invalid_argument(ss.str());
  }
  // DOC open parameters : http://www.sqlite.org/c3ref/open.html
  // DOC in-memory db : http://www.sqlite.org/inmemorydb.html
  rc = sqlite3_open_v2(DB_CONNECTION_URI, &db_connection,
    SQLITE_OPEN_MEMORY | SQLITE_OPEN_CREATE | SQLITE_OPEN_READWRITE | SQLITE_OPEN_SHAREDCACHE,
    NULL);
  if (SQLITE_OK != rc)
  {
    std::stringstream ss;
    ss << "DB Connection failed to open with error #" << rc;
    throw std::invalid_argument(ss.str());
  }
  rc = sqlite3_extended_result_codes(db_connection, 1);
  // Set up database settings.
  std::string turn_off_synchronous_mode("PRAGMA synchronous=OFF;");
  execute_ddl_statement(turn_off_synchronous_mode);
  // Journalling is already in OFF or MEMORY because the DB was opened in MEMORY mode.
  // std::string turn_off_journal_mode("PRAGMA journal_mode=MEMORY;");
  std::string set_temp_store_mode("PRAGMA temp_store=MEMORY;");
  execute_ddl_statement(set_temp_store_mode);
  // Create relative tables.
  std::string decisions_table_ddl = get_decisions_table_ddl();
  execute_ddl_statement(decisions_table_ddl);
  std::string decisions_index_ddl = get_decisions_index_ddl();
  execute_ddl_statement(decisions_index_ddl);
}

template <typename Time_t>
void SqliteDecisionTable<Time_t>::execute_ddl_statement(std::string& statement)
{
  int rc = SQLITE_OK;
  sqlite3_stmt *stmt = NULL;
  rc = sqlite3_prepare_v2(db_connection, statement.data(), -1, &stmt, NULL);
  BOOST_ASSERT_MSG(SQLITE_OK == rc, "Deposit decision initial DDL Prepared statement failed");
  rc = sqlite3_step(stmt);
  BOOST_ASSERT_MSG(SQLITE_DONE == rc, "Deposit decision initial DDL Statement execution failed");
  rc = sqlite3_finalize(stmt);
}

template <typename Time_t>
std::string SqliteDecisionTable<Time_t>::get_decisions_table_ddl() const
{
  // DOC table creation : http://www.sqlite.org/lang_createtable.html
  // DOC consider a limit of 2000 to the total number of columns : http://www.sqlite.org/limits.html#max_column
  std::stringstream decisions_table_ddl;
  decisions_table_ddl
    << "CREATE TABLE "
    << table_name
    << " ("
    <<   " InRate INT"
    <<   ",LiqBenchLvl INT"
    <<   ",BorrowingRate INT"
    <<   ",CntAtBorrowingRate INT"
    <<   ",CntBorrowingOthers INT"
    <<   ",LendingRate INT"
    <<   ",CntAtLendingRate INT"
    <<   ",CntLendingOthers INT"
    <<   ",OwnCreditRating INT"
    <<   ",CpCreditRating INT"
    <<   ",CpBorrowingRate INT"
    <<   ",CpLendingRate INT";
  // Iterate over External Neighbourhood
  for (int ith = 0; ith < this->simulation_parameters->get_external_bank_neighborhood_number(); ith++)
  {
    decisions_table_ddl << ",";
    ext_lending_rate_col_name(decisions_table_ddl, ith);
    decisions_table_ddl << " INT,";
    ext_borrowing_rate_col_name(decisions_table_ddl, ith);
    decisions_table_ddl << " INT,";
    ext_balance_sheet_col_name(decisions_table_ddl, ith);
    decisions_table_ddl << " INT";
  }
  decisions_table_ddl
    << ",PrevBkLogPosition INT" // -1 for a new Deposit, 0 for a matched, >0 for being in BackLog.
    << ",PrevDesiredRate INT"   // NULL'able if it is a new Deposit.
    << ",DesiredRate INT"
    << ",ExpectedPayout DOUBLE"
    << ",HitCount INT"
    << ");";
  return decisions_table_ddl.str();
}

template <typename Time_t>
std::string SqliteDecisionTable<Time_t>::get_decisions_index_ddl() const
{
  // DOC index creation : http://www.sqlite.org/lang_createindex.html
  std::stringstream decisions_index_ddl;
  decisions_index_ddl
    << "CREATE INDEX "
    << table_name
    << get_idx_suffix()
    << " ON "
    << table_name
    << " ("
    <<   " InRate"
    <<   ",LiqBenchLvl"
    <<   ",BorrowingRate"
    <<   ",CntAtBorrowingRate"
    <<   ",CntBorrowingOthers"
    <<   ",LendingRate"
    <<   ",CntAtLendingRate"
    <<   ",CntLendingOthers"
    <<   ",OwnCreditRating"
    <<   ",CpCreditRating"
    <<   ",CpBorrowingRate"
    <<   ",CpLendingRate";
  // Iterate over External Neighbourhood
  for (int ith = 0; ith < this->simulation_parameters->get_external_bank_neighborhood_number(); ith++)
  {
    decisions_index_ddl << ",";
    ext_lending_rate_col_name(decisions_index_ddl, ith);
    decisions_index_ddl << ",";
    ext_borrowing_rate_col_name(decisions_index_ddl, ith);
    decisions_index_ddl << ",";
    ext_balance_sheet_col_name(decisions_index_ddl, ith);
  }
  decisions_index_ddl
    << ",PrevBkLogPosition" // -1 for a new Deposit, 0 for a matched, >0 for being in BackLog.
    << ",PrevDesiredRate"   // NULL'able if it is a new Deposit.
    << ");";
  return decisions_index_ddl.str();
}

template <typename Time_t>
ActionInState SqliteDecisionTable<Time_t>::get_best_matching_rate(ObservableState* uncasted_state) const
{
  SqliteObservableState* state = static_cast<SqliteObservableState*>(uncasted_state);
  // DOC rows selection : http://www.sqlite.org/lang_select.html
  std::stringstream best_matching_rate_selector_dml;
  best_matching_rate_selector_dml << "SELECT DesiredRate,ExpectedPayout FROM "
    << table_name
    << " WHERE "
    <<   "     (InRate='" << state->new_rate << "')"
    <<   " AND (LiqBenchLvl='" << state->liquidity_benchmark_level << "')"
    <<   " AND (BorrowingRate='" << state->borrowing_rate << "')"
    <<   " AND (CntAtBorrowingRate='" << state->cnt_at_borrowing_rate << "')"
    <<   " AND (CntBorrowingOthers='" << state->cnt_borrowing_others << "')"
    <<   " AND (LendingRate='" << state->lending_rate << "')"
    <<   " AND (CntAtLendingRate='" << state->cnt_at_lending_rate << "')"
    <<   " AND (CntLendingOthers='" << state->cnt_lending_others << "')"
    <<   " AND (OwnCreditRating='" << state->own_rating << "')"
    <<   " AND (CpCreditRating='" << state->cp_credit_rating << "')"
    <<   " AND (CpBorrowingRate='" << state->cp_borrowing_rate << "')"
    <<   " AND (CpLendingRate='" << state->cp_lending_rate << "')";
  // Iterate over External Neighbourhood
  for (int ith = 0; ith < state->ext_banks_num; ith++)
  {
    best_matching_rate_selector_dml
    <<   " AND (";
    ext_lending_rate_col_name(best_matching_rate_selector_dml, ith);
    best_matching_rate_selector_dml
    <<   "='" << state->external_banks[ith].l_rate << "')"
    <<   " AND (";
    ext_borrowing_rate_col_name(best_matching_rate_selector_dml, ith);
    best_matching_rate_selector_dml
    <<   "='" << state->external_banks[ith].b_rate << "')"
    <<   " AND (";
    ext_balance_sheet_col_name(best_matching_rate_selector_dml, ith);
    best_matching_rate_selector_dml
    <<   "='" << state->external_banks[ith].rating << "')";
  }
  // TODO:
  // "PrevBkLogPosition" // -1 for a new Deposit, 0 for a matched, >0 for being in BackLog.
  // "PrevDesiredRate"   // NULL'able if it is a new Deposit.
  best_matching_rate_selector_dml << " ORDER BY DesiredRate ASC;";

  int rc = SQLITE_OK;
  sqlite3_stmt *stmt = NULL;
  rc = sqlite3_prepare_v2(db_connection, best_matching_rate_selector_dml.str().data(), -1, &stmt, NULL);
  BOOST_ASSERT_MSG(SQLITE_OK == rc, "Decision best rate selection failed");
  std::vector<ActionPayout_t> payouts;
  payouts.insert(payouts.begin(), MAX_RATE_IDX_NUM(this->simulation_parameters->get_rate_levels_number()), ActionPayout_t());
  while (SQLITE_ROW == (rc = sqlite3_step(stmt)))
  {
    // Read a row of data.
    RateLevel_t rate = sqlite3_column_int(stmt, 0);
    payouts[rate] = sqlite3_column_double(stmt, 1);
  }
  BOOST_ASSERT_MSG(SQLITE_DONE == rc, "Decision best rate query failed");
  rc = sqlite3_finalize(stmt);
  auto argmax_pos_it = std::max_element(payouts.begin(), payouts.end());
  BOOST_ASSERT_MSG(argmax_pos_it != payouts.end(), "Empty range while looking for an argmax");
  auto action = ActionInState(argmax_pos_it - payouts.begin());
  return action;
}

template <typename Time_t>
void SqliteDecisionTable<Time_t>::update_expected_roi_in_state(ObservableState* const state, const Money_t realized_return)
{
  BOOST_ASSERT_MSG(false, "TODO : implement");
}

template <typename Time_t>
void SqliteDecisionTable<Time_t>::dump_to_file(std::string& destination_file_name) const
{
  // DOC online backup : http://www.sqlite.org/backup.html
  // DOC API : http://www.sqlite.org/c3ref/backup_finish.html
  // TODO aoboturov : implement
  BOOST_ASSERT_MSG(false, "TODO : implement");
}

template <typename Time_t>
Money_t SqliteDecisionTable<Time_t>::get_expected_return_on_investment(ObservableState* const uncasted_state, const RateLevel_t matched_rate) const
{
  SqliteObservableState* state = static_cast<SqliteObservableState*>(uncasted_state);
  // DOC rows selection : http://www.sqlite.org/lang_select.html
  std::stringstream expected_payout_selector_dml;
  expected_payout_selector_dml << "SELECT ExpectedPayout FROM "
    << table_name
    << " WHERE "
    <<   "     (InRate='" << state->new_rate << "')"
    <<   " AND (LiqBenchLvl='" << state->liquidity_benchmark_level << "')"
    <<   " AND (BorrowingRate='" << state->borrowing_rate << "')"
    <<   " AND (CntAtBorrowingRate='" << state->cnt_at_borrowing_rate << "')"
    <<   " AND (CntBorrowingOthers='" << state->cnt_borrowing_others << "')"
    <<   " AND (LendingRate='" << state->lending_rate << "')"
    <<   " AND (CntAtLendingRate='" << state->cnt_at_lending_rate << "')"
    <<   " AND (CntLendingOthers='" << state->cnt_lending_others << "')"
    <<   " AND (OwnCreditRating='" << state->own_rating << "')"
    <<   " AND (CpCreditRating='" << state->cp_credit_rating << "')"
    <<   " AND (CpBorrowingRate='" << state->cp_borrowing_rate << "')"
    <<   " AND (CpLendingRate='" << state->cp_lending_rate << "')";
  // Iterate over External Neighbourhood
  for (int ith = 0; ith < state->ext_banks_num; ith++)
  {
    expected_payout_selector_dml
    <<   " AND (";
    ext_lending_rate_col_name(expected_payout_selector_dml, ith);
    expected_payout_selector_dml
    <<   "='" << state->external_banks[ith].l_rate << "')"
    <<   " AND (";
    ext_borrowing_rate_col_name(expected_payout_selector_dml, ith);
    expected_payout_selector_dml
    <<   "='" << state->external_banks[ith].b_rate << "')"
    <<   " AND (";
    ext_balance_sheet_col_name(expected_payout_selector_dml, ith);
    expected_payout_selector_dml
    <<   "='" << state->external_banks[ith].rating << "')";
  }
  expected_payout_selector_dml
    <<   " AND (PrevBkLogPosition='-1')" // -1 for a new Deposit, 0 for a matched, >0 for being in BackLog.
    <<   " AND (PrevDesiredRate IS NULL)" // NULL'able if it is a new Deposit.
    <<   " AND (DesiredRate='" << matched_rate << "');";

  int rc = SQLITE_OK;
  sqlite3_stmt *stmt = NULL;
  rc = sqlite3_prepare_v2(db_connection, expected_payout_selector_dml.str().data(), -1, &stmt, NULL);
  BOOST_ASSERT_MSG(SQLITE_OK == rc, "Expected payout selection failed");

  int results_produced = 0;
  Money_t expected_payout;
  while (SQLITE_ROW == (rc = sqlite3_step(stmt)))
  {
    results_produced++;
    // Read a row of data.
    expected_payout = sqlite3_column_double(stmt, 0);
  }
  BOOST_ASSERT_MSG(results_produced <= 1, "At most one rate should be returned");
  BOOST_ASSERT_MSG(SQLITE_DONE == rc, "Expected payout selection failed");
  rc = sqlite3_finalize(stmt);
  if (0 == results_produced)
  {
    // TODO: Generate some initial value.
    expected_payout = .0;
  }
  return expected_payout;
}

template <typename Time_t>
ObservableState* SqliteDecisionTable<Time_t>::get_observable_state_instance() const
{
  return new SqliteObservableState(this->simulation_parameters->get_rate_levels_number(), this->simulation_parameters->get_external_bank_neighborhood_number());
}

template <typename Time_t>
SqliteDecisionTable<Time_t>::~SqliteDecisionTable()
{
  sqlite3_close(db_connection);
}

template <typename Time_t>
inline std::stringstream& SqliteDecisionTable<Time_t>::ext_lending_rate_col_name(std::stringstream& ss, const int ith) const
{
  ss << "ExtLendingRate_" << ith;
  return ss;
}

template <typename Time_t>
inline std::stringstream& SqliteDecisionTable<Time_t>::ext_borrowing_rate_col_name(std::stringstream& ss, const int ith) const
{
  ss << "ExtBorrowingRate_" << ith;
  return ss;
}

template <typename Time_t>
inline std::stringstream& SqliteDecisionTable<Time_t>::ext_balance_sheet_col_name(std::stringstream& ss, const int ith) const
{
  ss << "ExtBalanceSheet_" << ith;
  return ss;
}

template <typename Time_t>
class SqliteDepositDecisionTable : public SqliteDecisionTable<Time_t>
{
public:
  SqliteDepositDecisionTable(ISimulationParameters<Time_t>* const simulation_params)
    : SqliteDecisionTable<Time_t>(simulation_params, std::string("DepositDecisionTable")) {}
  virtual ~SqliteDepositDecisionTable() {}
};

template <typename Time_t>
class SqliteLoanDecisionTable : public SqliteDecisionTable<Time_t>
{
public:
  SqliteLoanDecisionTable(ISimulationParameters<Time_t>* const simulation_params)
    : SqliteDecisionTable<Time_t>(simulation_params, std::string("LoanDecisionTable")) {}
  virtual ~SqliteLoanDecisionTable() {}
};


