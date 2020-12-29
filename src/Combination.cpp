
#include "Combination.h"


// A simple function to compute N over k
size_t computeCombinations(uint N, uint k){
    // If k is larger than N-k its easier to use N-k for the computation
    unsigned short K = k > N / 2 ? N - k : k;
    unsigned long ncombs = 1;
    for (unsigned int i = 1; i <= K; i++) {
        ncombs *= N - i + 1;
        ncombs /= i;
    }
    return ncombs;
}


Combination::Combination(uint N, uint k, uint nBatches) :
    m_N(N), m_k(k), m_nBatches(nBatches) {

    // Define initial value for current combination (1, 2, 3, ...)
    m_currentCombination.reserve(m_k);
    for (uint i = 1; i <= m_k; i++) m_currentCombination.push_back(i);

    // Compute the total number of existing combinations with this setup.
    m_nCombinations = computeCombinations(m_N, m_k);

    // The following computes a set of starting combinations that split all
    // combinations into m_nBatches approx equal parts (a vector of vectors)
    float targetSize = m_nCombinations / m_nBatches;
    std::vector<uint> element;
    // First limit is 1, 2, 3, ...
    for (size_t i = 1; i <= m_k; i++) element.emplace_back(i);
    m_batchLimits.emplace_back(element);
    uint firstNum = 0;
    for (size_t j = 0; j < m_nBatches; j++) {
        // Search for next start num to make the batch size at least targetsize
        size_t curBatchSize = 0;
        while (curBatchSize < targetSize && firstNum < m_N - m_k + 1) {
            firstNum++;
            curBatchSize += computeCombinations(m_N - firstNum, m_k - 1);
        }
        // Found starting number. Now add it to result
        element.clear();
        for (size_t i = 0; i < m_k; i++) element.emplace_back(firstNum + i);
        m_batchLimits.emplace_back(element);
        // Also add the batch size to m_batchSizes
        m_batchSizes.emplace_back(curBatchSize);
    }
}


// Compute and set the next combination from the current one
// returns false if it already was the last combination and true otherwise
bool Combination::nextCombination() {

    // Have we reached the early stopping combination? Then do nothing.
    if (m_currentCombination == m_stopCombination) return false;

    // We want to find the index of the least significant element
    // in v that can be increased.  Let's call that index 'pivot'.
    short pivot = m_k - 1;
    while (pivot >= 0 && m_currentCombination[pivot] == m_N + 1 - m_k + pivot)
        --pivot;

    // pivot will be -1 if v == {N - k, N - k + 1, ..., N - 1},
    // in which case, there is no next combination and we are done.
    if (pivot == -1) return false;

    // We increment at pivot and set the following positions accordingly
    ++m_currentCombination[pivot];
    for (uint i = pivot + 1; i < m_k; ++i)
        m_currentCombination[i] = m_currentCombination[pivot] + i - pivot;

    return true;
}
