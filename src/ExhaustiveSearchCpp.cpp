
#include "SearchTask.h"


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
  const arma::mat * X = &XInput;
  const std::vector<double> * y = &yInput;
  const arma::mat * XT = &XTestSet;
  const std::vector<double> * yT = &yTestSet;

  // If a TestSet was specified use it, otherwise repeat the training pointers
  DataSet D(X, y, XTestSet.n_rows > 0 ? XT : X, XTestSet.n_rows > 0 ? yT : y);

  // Initialize the modelling task object
  GLM Model(D, family, performanceMeasure, intercept, errorVal);
  GLM* ModelPtr = &Model;

  // Initialize the Combination Object (ncols - 1 because of the Intercept col).
  // If nThreads was not specified, set it to the number of available threads.
  Combination Comb(XInput.n_cols - 1, combsUpTo,
    nThreads > 0 ? nThreads: std::thread::hardware_concurrency());
  Combination* CombPtr = &Comb;

  // The SearchTask handles the (multithreaded) execution and saves the results
  SearchTask ST(ModelPtr, CombPtr, nResults, quietly);
  ST.run();

  // Write the final ranking in reverse order into a formatted result (List)
  Rcpp::NumericVector AicList;
  Rcpp::List CombList;
  while (!ST.rankingEmpty()) {
    AicList.push_front(ST.rankingTop().first);
    CombList.push_front(ST.rankingTop().second);
    ST.popRanking();
  }

  // Fill up the result object
  Rcpp::List result;
  result.push_back(ST.getTotalRuntimeSec());
  result.push_back(AicList);
  result.push_back(CombList);
  result.push_back(ST.getProgress());
  result.push_back(Comb.getNBatches());
  result.push_back(Comb.getBatchSizes());
  result.push_back(Comb.getBatchLimits());

  return result;
}
