
#include <thread>
#include <queue>
#include <mutex>
#include <condition_variable>

#include "GLM.h"
#include "Combination.h"


typedef std::priority_queue<std::pair<double, std::vector<uint>>> ranking;

const size_t M_NOTIFY_INTERVAL = 100;
const size_t M_PRINT_INTERVAL_SEC = 5;

class SearchTask {

  // Input
  GLM* m_ModelPtr;
  Combination* m_CombPtr;
  size_t m_nResults;
  bool m_quietly;

  // Execution
  std::mutex mtx;
  std::condition_variable condVar;
  bool m_aborted;
  size_t m_abortedThreads;
  size_t m_progress;
  size_t m_totalIterations;
  size_t m_totalRuntimeSec;

  // Output
  ranking m_result;



public:
  SearchTask(GLM*& ModelPtr, Combination*& CombPtr, size_t& nResults,
    bool& quietly);

  size_t getProgress() { return m_progress; }
  void popRanking() { m_result.pop(); }
  bool rankingEmpty() { return m_result.empty(); }
  std::pair<double, std::vector<uint>> rankingTop() { return m_result.top(); }
  size_t getTotalRuntimeSec() { return m_totalRuntimeSec; }

  void run();
  void threadComputation(size_t threadID);
  void trackStatus();

  // User interrupt checks that are ensured to be Toplevel
  static void chkIntFn(void *dummy) { R_CheckUserInterrupt(); }
  inline bool checkInterrupt() {
    return (R_ToplevelExec(chkIntFn, NULL) == FALSE);
  }

};
