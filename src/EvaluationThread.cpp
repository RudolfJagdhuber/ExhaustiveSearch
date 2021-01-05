
#include "EvaluationThread.h"

// For thread safety when printing and updating status logs
std::mutex m;

void ExhaustiveThread(size_t threadID, GLM Model, Combination Comb,
  size_t nResults, StatusLog* SLptr, bool quietly, std::promise<ranking>&& p) {

  // Store results into a priority queue of pairs (performance, combination),
  // which is ordered by the first element desc. (The worst (highest) is first
  // in the queue). "ranking" is an alias for this structure (see globals.h)
  ranking result;

  size_t iteration = 0;
  size_t printAfter  = (Model.getFamily() == "gaussian") ? 500000 : 2000;
  size_t updateAfter = (Model.getFamily() == "gaussian") ? 50000  : 500;

  // Step to the next combination and evaluate it, as long as there is one
  while (Comb.nextCombination()) {

    iteration++;

    // Compute the Model for the current combination
    Model.setFeatureCombination(Comb.getCurrentComb());
    Model.fit();
    double perfResult = Model.getPerformance();

    // Add this combination to our toplist if either there is room, or if it
    // among the top nResults.
    if (result.size() < nResults || perfResult < result.top().first) {

      result.push(std::make_pair(perfResult, Comb.getCurrentComb()));

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
