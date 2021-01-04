
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
    const arma::mat& XTestSet,
    const std::vector<double>& yTestSet,
    std::string family,
    std::string performanceMeasure,
    bool intercept,
    size_t combsUpTo,
    size_t nResults,
    size_t nThreads,
    double errorVal,
    bool quietly) {

  // The data shall not be copied from now on, so only work with pointers stored
  // in a DataSet object (DataSet.h) for structure.
  // If a TestSet is specified use it, otherwise repeat the training data ptr
  const arma::mat * X = &XInput;
  const std::vector<double> * y = &yInput;
  const arma::mat * XT = &XTestSet;
  const std::vector<double> * yT = &yTestSet;
  bool hasTestSet = XTestSet.n_rows > 0;
  DataSet Data(X, y, hasTestSet ? XT : X, hasTestSet ? yT : y);

  // Initialize the Logistic Regression Object (not fitted and not subsetted)
  GLM Model(Data, family, performanceMeasure, intercept, errorVal);

  // set nThreads to the number of available Threads if it was not set
  if (nThreads == 0) nThreads =  std::thread::hardware_concurrency();;

  // Initialize the Combination Object (ncols - 1 because of the Intercept col)
  Combination Comb(XInput.n_cols - 1, combsUpTo, nThreads);

  // Initialize the status logging Object and print the header
  StatusLog* SL =  new StatusLog(Comb.getNCombinations());
  if (!quietly) Rcpp::Rcout << (*SL).header() << std::endl;;

  // Create threads that each define a ranking object as future. I avoid the
  // simpler std::async to use RcppThread::Thread for interrupt management.
  std::vector<std::future<ranking>> futures;
  // std::vector<RcppThread::Thread> threads;
  std::vector<std::thread> threads;
  for (size_t i = 0; i <  Comb.getNBatches(); i++) {
    Comb.setCurrentComb(Comb.getBatchLimits()[i]);
    Comb.setStopComb(Comb.getBatchLimits()[i + 1]);
    std::promise<ranking> p;
    futures.push_back(p.get_future());
    threads.push_back(std::thread(&ExhaustiveThread, i + 1,
      Model, Comb, nResults, SL, quietly, std::move(p)));
  }

  // Join threads and collect all std::future objects
  std::vector<ranking> allres;
  for (size_t i = 0; i <  Comb.getNBatches(); i++) {
    threads[i].join();
    allres.push_back(futures[i].get());
  }

  size_t evaluatedModels = (*SL).getCurIters();

  // Finalize the StatusLog object and print the footer of the table
  (*SL).finalize();
  if (!quietly) Rcpp::Rcout << (*SL).footer() << std::endl;;

  // If I cut off the ranking in a single thread, I can no longer guarantee
  // that there is no nearly similar performing subsequent model. Therefore all
  // models worse than this worst value are no longer legit. So i need to find
  // the best of these worst values in all threads, which did not store every
  // single model and remove all results worse than it from all threads.
  float topworst = std::numeric_limits<float>::infinity();
  for (size_t i = 0; i <  Comb.getNBatches(); i++)
    if (allres[i].top().first < topworst && Comb.getBatchSizes()[i] > nResults)
      topworst = allres[i].top().first;

  // Combine the elements of all rankings, which are better than topworst
  ranking finaltop;
  for (size_t i = 0; i <  Comb.getNBatches(); i++) while (!allres[i].empty()) {
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

  // Filling up the result object
  Rcpp::List result;
  result.push_back((*SL).getTotalTimeSecs());
  result.push_back(AicList);
  result.push_back(CombList);
  result.push_back(evaluatedModels);
  result.push_back(nThreads);
  result.push_back(Comb.getNBatches());

  return result;
}
