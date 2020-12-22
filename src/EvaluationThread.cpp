
#include "EvaluationThread.h"

std::mutex m;

ranking ExhaustiveThread(size_t threadID, LogisticRegression Model,
  Combination Comb, size_t nResults, StatusLog* SLptr) {

  // Store results into a priority queue of pairs (AIC, combination), which is
  // ordered by the first element desc. (The worst is first in the queue).
  // "ranking" is an alias for this structure (defined in globals.h)
  ranking result;

  size_t iteration = 1;
  do {

    // Compute the Model for the current combination
    Model.setFeatureCombination(Comb.getCurrentCombWith0());
    Model.fit();

    // Add this combination to our toplist if either there is room, or if it
    // among the top nResults.
    if (result.size() < nResults || Model.getAIC() < result.top().first) {

      result.push(std::make_pair(Model.getAIC(), Comb.getCurrentComb()));

      // Is the queue now too large? -> remove the first element (the worst)
      if (result.size() > nResults) result.pop();
    }

    // Update the StatusLog object
    if (iteration % 2000 == 0) {
      m.lock();
      (*SLptr).addIters(2000);
      if ((*SLptr).getCurIters() % 10000 == 0)
        Rcpp::Rcout << (*SLptr).status() << std::endl;
      m.unlock();
    }

    iteration++;
  } while (Comb.nextCombination()); // As long as there is a next combination

  return result;
}
