#pragma once

#include <vector>
#include <stdexcept>
#include <numeric>

#include "globals.h"


// A Combination object holds the task to iterate over all possible combinations
// for a given N and k. It starts from the first combination and has a function
// to compute the next combination from it.
class Combination {

  // The number of elements to choose from
	uint m_N;
  // The upper limit of elements per combination
	uint m_k;
	std::vector<uint> m_currentCombination;
	size_t m_nCombinations;
	// A combination to stop at. Used when splitting combs into multiple threads
	std::vector<uint> m_stopCombination;
  // Splitting the total number of combination into batches
  size_t m_nBatches;
  std::vector<std::vector<uint>> m_batchLimits;
  std::vector<size_t> m_batchSizes;

public:
	Combination(uint N, uint k, size_t nBatches);
	uint getN() { return m_N; }
	uint getK() { return m_k; }
	std::vector<uint> getCurrentComb()	{ return m_currentCombination; }
	size_t getNCombinations() { return m_nCombinations; }
	std::vector<uint> getStopComb() { return m_stopCombination;	}
	size_t getNBatches() { return m_nBatches; }
	std::vector<std::vector<uint>> getBatchLimits() { return m_batchLimits; }
	std::vector<size_t> getBatchSizes() { return m_batchSizes; }
	void setCurrentComb(const std::vector<uint>& curComb)	{
		m_currentCombination = curComb;
	}
	void setStopComb(const std::vector<uint>& lastComb) {
		m_stopCombination = lastComb;
	}
	bool nextCombination();
};
