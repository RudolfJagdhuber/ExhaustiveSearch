
#include <thread>
#include <future>

#include "globals.h"
#include "DataSet.h"
#include "GLM.h"
#include "Combination.h"
#include "EvaluationThread.h"
#include "StatusLog.h"


// [[Rcpp::export]]
Rcpp::List ExhaustiveSearchCpp(
    const arma::mat& XInput, // Design Matrix (with intercept column!)
    const std::vector<double>& yInput,
    std::string family,
    bool intercept,
    size_t combsUpTo,
    size_t nResults,
    size_t nThreads,
    double errorVal,
    bool quietly) {
  Rcpp::List result;

  // Take the given Data and create a custom object (DataSet.h) from it
  DataSet Data(XInput, yInput);

  // Initialize the Logistic Regression Object (not fitted and not subsetted)
  GLM Model(Data, family, intercept, errorVal);

  // Initialize the Combination Object (first column reserved for the intercept)
  Combination Comb(XInput.n_cols - 1, combsUpTo, nThreads);

  // Initialize the status logging Object and print the header
  StatusLog* SL =  new StatusLog(Comb.getNCombinations());
  if (!quietly) Rcpp::Rcout << (*SL).header() << std::endl;;

  // Create the threads with their respective batches of combinations
  std::vector<std::future<ranking>> futures;
  for (size_t i = 0; i < nThreads; i++) {
    Comb.setCurrentComb(Comb.getBatchLimits()[i]);
    Comb.setStopComb(Comb.getBatchLimits()[i + 1]);
    futures.push_back(std::async(&ExhaustiveThread, i + 1, Model, Comb,
      nResults, SL, quietly));
  }

  // Collect all std::future objects (i.e. wait for threads to finish)
  std::vector<ranking> allres;
  for (size_t i = 0; i < nThreads; i++) allres.push_back(futures[i].get());

  // Finalize the StatusLog object and print the footer of the table
  (*SL).finalize();
  if (!quietly) Rcpp::Rcout << (*SL).footer() << std::endl;;

  // If I cut off the ranking in a single thread, I can no longer guarantee
  // that there is no nearly similar performing subsequent model. Therefore all
  // models worse than this worst value are no longer legit. So i need to find
  // the best of these worst values in all threads, which did not store every
  // single model and remove all results worse than it from all threads.
  float topworst = std::numeric_limits<float>::infinity();
  for (size_t i = 0; i < nThreads; i++)
    if (allres[i].top().first < topworst && Comb.getBatchSizes()[i] > nResults)
      topworst = allres[i].top().first;

  // Combine the elements of all rankings, which are better than topworst
  ranking finaltop;
  for (size_t i = 0; i < nThreads; i++) while (!allres[i].empty()) {
    if (allres[i].top().first <= topworst) finaltop.push(allres[i].top());
    allres[i].pop();
    if (finaltop.size() > nResults) finaltop.pop();
  }

  // Write the final priority queue in reverse order into the result List
  Rcpp::NumericVector AicList;
  Rcpp::List CombList;
  while (!finaltop.empty()) {
    AicList.push_front(finaltop.top().first);
    CombList.push_front(finaltop.top().second);
    finaltop.pop();
  }


  // Filling up the return object
  result.push_back((*SL).getTotalTimeSecs());
  result.push_back(AicList);
  result.push_back(CombList);
  result.push_back(Comb.getBatchSizes());

  return result;
}
