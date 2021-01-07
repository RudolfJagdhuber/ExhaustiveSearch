
#include "EvaluationThread.h"

// For thread safety when printing and updating status logs
std::mutex m;

void ExhaustiveThread(const size_t& threadID, GLM Model,
  const Combination& Comb, const size_t& nResults, StatusLog* SLptr,
  bool quietly, std::promise<ranking>&& p) {

  // Read the start and stop limits of this task.
  std::vector<uint> currentComb = Comb.getBatchLimits()[threadID - 1];
  std::vector<uint> stoppingComb = Comb.getBatchLimits()[threadID];


  // Store results into a priority queue of pairs (performance, combination),
  // which is ordered by the first element desc. (The worst (highest) is first
  // in the queue). "ranking" is an alias for this structure (see globals.h)
  ranking result;

  size_t iteration = 0;
  size_t printAfter  = (Model.getFamily() == "gaussian") ? 500000 : 2000;
  size_t updateAfter = (Model.getFamily() == "gaussian") ? 50000  : 500;

  // Continue, as long as the stopping combination was not evaluated
  while (currentComb != stoppingComb) {

    // Function is found in Combination.h
    setNextCombination(currentComb, Comb.getN());

    iteration++;

    // Compute the Model for the current combination
    Model.setFeatureCombination(currentComb);
    Model.fit();
    double perfResult = Model.getPerformance();

    // Add this combination to our ranking if either there is room, or if it
    // among the top nResults.
    if (result.size() < nResults || perfResult < result.top().first) {

      result.push(std::make_pair(perfResult, currentComb));

      // Is the queue now too large? -> remove the first element (the worst)
      if (result.size() > nResults) result.pop();
    }

    // Update the StatusLog object
    if (iteration % updateAfter == 0) {
      m.lock();
      SLptr->addIters(updateAfter);
      if (SLptr->getCurIters() % printAfter == 0 && !quietly)
        Rcpp::Rcout << SLptr->status() << std::endl;
      m.unlock();
    }
  }

  // Final update of StatusLog Object
  m.lock();
  SLptr->addIters(iteration % updateAfter);
  m.unlock();

  p.set_value(result);
}
