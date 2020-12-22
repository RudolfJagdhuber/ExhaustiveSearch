#pragma once

#include <vector>
#include <stdexcept>
#include <numeric>

#include "globals.h"


// A Combination object holds the task to iterate over all possible combinations
// for a given N and k. It starts from the first combination and has a function
// to compute the next combination from it.
class Combination
{
	ushort m_N;
	ushort m_k;
	std::vector<ushort> m_currentCombination;
	size_t m_nCombinations;
	// A combination to stop at. Used when splitting combs into multiple threads
	std::vector<ushort> m_stopCombination;
  // Splitting the total number of combination into batches
  ushort m_nBatches;
  std::vector<std::vector<ushort>> m_batchLimits;
  std::vector<size_t> m_batchSizes;

public:
	Combination(ushort N, ushort k, ushort nBatches);
	ushort getN() { return m_N; }
	ushort getK() { return m_k; }
	std::vector<ushort> getCurrentComb()	{ return m_currentCombination; }
	std::vector<ushort> getCurrentCombWith0() {
		std::vector<ushort> current0;
		current0.reserve(m_k + 1);
		current0.emplace_back(0);
		for (auto i : m_currentCombination) current0.emplace_back(i);
		return current0;
	}
	size_t getNCombinations() { return m_nCombinations; }
	std::vector<ushort> getStopComb() { return m_stopCombination;	}
	ushort getNBatches() { return m_nBatches; }
	std::vector<std::vector<ushort>> getBatchLimits() {
	  return m_batchLimits;
	}
	std::vector<size_t> getBatchSizes() { return m_batchSizes; }
	void setCurrentComb(const std::vector<ushort>& curComb)	{
		m_currentCombination = curComb;
	}
	void setStopComb(const std::vector<ushort>& lastComb) {
		m_stopCombination = lastComb;
	}
	bool nextCombination();
};
