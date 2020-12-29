#pragma once

#include <chrono>
#include <string>
#include <sstream>
#include <iomanip>
#include <cmath>

#include "globals.h"


class StatusLog {

	size_t m_totalIters;
	size_t m_curIters;
  std::chrono::time_point<std::chrono::high_resolution_clock> m_start;
  // The number of digits m_totalIters has (used for formatting)
  uint m_dig;
  size_t m_totalTimeSecs;

public:
	StatusLog(size_t total) : m_totalIters(total), m_curIters(0),
	  m_start(std::chrono::high_resolution_clock::now()),
	  m_dig(m_totalIters > 0 ? (int)(log10((double)m_totalIters)) + 1 : 1),
	  m_totalTimeSecs(0) {};
  size_t getCurIters() {return m_curIters;}
  size_t getTotalTimeSecs() {return m_totalTimeSecs;}
  void addIters(size_t add) {	m_curIters += add; }
  void finalize() {
    m_curIters = m_totalIters;
    m_totalTimeSecs = (size_t)((std::chrono::duration<float>)(
      std::chrono::high_resolution_clock::now() - m_start)).count();
  }
	std::string status();
	std::string header();
	std::string footer();
};
