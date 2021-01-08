
#include <chrono>

#include "SearchTask.h"

SearchTask::SearchTask(GLM*& ModelPtr, Combination*& CombPtr, size_t& nResults,
  bool& quietly) :
  m_ModelPtr(ModelPtr), m_CombPtr(CombPtr), m_nResults(nResults),
  m_quietly(quietly), m_aborted(false), m_abortedThreads(0), m_progress(0),
  m_totalIterations(0), m_totalRuntimeSec(0) {

  for(size_t n : CombPtr->getBatchSizes()) m_totalIterations += n;
}


void SearchTask::run() {

  std::vector<std::thread> threads;
  threads.reserve(m_CombPtr->getNBatches());
  for (size_t i = 0; i < m_CombPtr->getNBatches(); i++)
    threads.emplace_back(&SearchTask::threadComputation, this, i);

  // The main thread is held in a loop, iteratively printing progress updates
  // and checking for interrupts until the threads are finished.
  trackStatus();

  for (std::thread &thread : threads) thread.join();

  if (m_abortedThreads > 0)
    throw std::runtime_error("Execution aborted by the user.");
}


void SearchTask::threadComputation(size_t threadID) {

  size_t n = m_CombPtr->getN();

  // Read the start and stop limits of this task.
  std::vector<uint> currentComb = m_CombPtr->getBatchLimits()[threadID];
  std::vector<uint> stoppingComb = m_CombPtr->getBatchLimits()[threadID + 1];

  // Thread creates a copy of the GLM object to fit it without worries
  GLM Model = *m_ModelPtr;

  // Continue, as long as the stopping combination was not evaluated yet
  while (currentComb != stoppingComb) {

    // Function is found in Combination.h
    setNextCombination(currentComb, n);

    // Compute the Model for the current combination
    Model.setFeatureCombination(currentComb);
    Model.fit();
    double perfResult = Model.getPerformance();

    // This code chunk has a local lock_guard to secure r/w on shared data.
    // Could this be a possible bottleneck? The alternative would be to build up
    // separate results per thread and join them afterwards on main thread. This
    // would remove the shared data, but would need nThreads-times more memory.
    {
      std::lock_guard<std::mutex> lockGuard(mtx);

      if (m_result.size() < m_nResults || perfResult < m_result.top().first) {

        m_result.push(std::make_pair(perfResult, currentComb));

        // Is the queue now too large? -> remove the first element (the worst)
        if (m_result.size() > m_nResults) m_result.pop();
      }
      m_progress++;

      // Check for user interrupts
      if (m_aborted) {
        m_abortedThreads++;
        break;
      }

      // allows trackStatus() to run every once in a while
      if (m_progress % M_NOTIFY_INTERVAL == 0) condVar.notify_one();
    }
  }
  // If the function ends, trackStatus() may run
  condVar.notify_one();
}


void SearchTask::trackStatus() {

  auto startTime = std::chrono::high_resolution_clock::now();
  auto timeLastPrint = startTime;
  size_t elapsedTimeSec = 0;

  // The number of digits m_totalIterations has (used for formatting)
  uint m_dig = m_totalIterations > 0 ? (int)(log10(
    (double)m_totalIterations)) + 1 : 1;

  // Print the output header
  if (!m_quietly) {
    Rcpp::Rcout << " Runtime          |  Completed"
    << std::string(2 * (m_dig - 4), ' ') << "  |  Status\n"
    << std::string(34 + 2 * m_dig, '-') << std::endl;
  }

  std::unique_lock<std::mutex> lock(mtx);
  while (m_progress < m_totalIterations) {

    // Wait for a thread to allow a run
    condVar.wait(lock);

    // Check for user interrupts
    if (checkInterrupt()) {
      m_aborted = true;
      return;
    }

    if (!m_quietly) {
      m_totalRuntimeSec = (size_t)((std::chrono::duration<float>)(
        std::chrono::high_resolution_clock::now() - startTime)).count();
      // Have enough seconds passed for an update to the console?
      elapsedTimeSec = (size_t)((std::chrono::duration<float>)(
        std::chrono::high_resolution_clock::now() - timeLastPrint)).count();
      if (elapsedTimeSec >= M_PRINT_INTERVAL_SEC ||
        m_progress == m_totalIterations) {
        // Format into (dd:hh:mm:ss)
        uint days = m_totalRuntimeSec / 60 / 60 / 24;
        uint hour = (m_totalRuntimeSec / 60 / 60) % 24;
        uint min = (m_totalRuntimeSec / 60) % 60;
        uint sec = m_totalRuntimeSec % 60;

        Rcpp::Rcout << " "
        << std::setw(2) << std::setfill('0') << days << "d "
        << std::setw(2) << std::setfill('0') << hour << "h "
        << std::setw(2) << std::setfill('0') << min << "m "
        << std::setw(2) << std::setfill('0') << sec << "s  |  "
        << std::setw(m_dig) << m_progress << "/" << m_totalIterations << "  |  "
        << (uint)(100 * m_progress / m_totalIterations) << "%" << std::endl;

        timeLastPrint = std::chrono::high_resolution_clock::now();
      }
    }
  }
  // Make a final update on the total runtime in seconds
  m_totalRuntimeSec = (size_t)((std::chrono::duration<float>)(
    std::chrono::high_resolution_clock::now() - startTime)).count();

  // Print the output footer
  if (!m_quietly) Rcpp::Rcout << std::string(34 + 2 * m_dig, '-') << std::endl;
}
