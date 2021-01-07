#pragma once

#include <vector>


typedef unsigned int  uint;

// A Combination object holds a setup of combinations for given N and k. It also
// includes a static function to compute the next combination from a given one.
class Combination {

  // The number of elements to choose from
	uint m_N;
  // The upper limit of elements per combination
	uint m_k;
	size_t m_nCombinations;
  // The total set of combination is split into equal sized batches for threads
  size_t m_nBatches;
  std::vector<std::vector<uint>> m_batchLimits;
  std::vector<size_t> m_batchSizes;

public:
	Combination(uint N, uint k, size_t nBatches);
	uint getN() { return m_N; }
	uint getK() { return m_k; }
	size_t getNCombinations() { return m_nCombinations; }
	size_t getNBatches() { return m_nBatches; }
	std::vector<std::vector<uint>> getBatchLimits() { return m_batchLimits; }
	std::vector<size_t> getBatchSizes() { return m_batchSizes; }
	static bool setNextCombination(std::vector<uint>& comb);
};
