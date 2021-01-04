
#include "Combination.h"


// A simple function to compute N over k
size_t NoverK(uint N, uint k) {

    if (k == 0) return 1;

    // If k is larger than N-k its easier to use N-k for the computation
    uint K = k > N / 2 ? N - k : k;
    size_t nCombs = 1;
    for (uint i = 1; i <= K; i++) {
        nCombs *= N - i + 1;
        nCombs /= i;
    }
    return nCombs;
}


// Compute the number of combinations of up to k parameters
size_t computeCombinations(uint N, uint k){

    size_t nCombs = 0;
    for (uint K = 1; K <= k; K++) nCombs += NoverK(N, K);
    return nCombs;
}


Combination::Combination(uint N, uint k, size_t nBatches) :
    m_N(N), m_k(k), m_nBatches(nBatches) {

    // Define initial value for current combination with is just "1"
    m_currentCombination.push_back(1);

    // Compute the total number of existing combinations with this setup.
    m_nCombinations = computeCombinations(m_N, m_k);

    // The following computes a set of starting combinations that split all
    // combinations into m_nBatches approx equal parts (a vector of vectors)
    size_t targetSize = ceil(((float)m_nCombinations) / m_nBatches);

    // Initial limit needs to be "(0)" as the evaluation calls nextCombination()
    // as a first step, which thus results in the true first element "(1)"
    std::vector<uint> element{0};
    m_batchLimits.emplace_back(element);

    // The current idea is to iteratively add all combs of k elements with a
    // fixed first digit until the batch is approx of the intended size.
    // This is not optimal and sometimes results in a smaller number of batches
    // than intended, but is good enough for most practical purposes.
    uint firstDigit = 0;
    uint curK = 1;
    for (size_t j = 0; j < m_nBatches; j++) {

        // Continue to add combs until the batch size is large enough
        size_t curBatchSize = 0;
        while (curBatchSize < targetSize) {
            if (firstDigit < m_N - curK + 1) firstDigit++;
            else if (curK < m_k) {// Start with next larger set of combinations
                curK++;
                firstDigit = 1;
            } else break; // Last combination reached

            curBatchSize += NoverK(m_N - firstDigit, curK - 1);
        }

        // Found starting number firstDigit of a combination of curK. -> Add it
        element.clear();
        for (uint i = 0; i < curK; i++) element.emplace_back(firstDigit + i);
        m_batchLimits.emplace_back(element);

        // Also add the batch size to m_batchSizes
        m_batchSizes.emplace_back(curBatchSize);

        // If all combs are partitioned, stop and return with fewer batches
        if (curK == m_k && firstDigit == m_N - curK + 1) {
            m_nBatches = m_batchSizes.size();
            break;
        }
    }
}


// Compute and set the next combination from the current one
// returns false if it already was the last combination and true otherwise
bool Combination::nextCombination() {

    // Have we reached the early stopping combination? Then do nothing.
    if (m_currentCombination == m_stopCombination) return false;

    // Get the current length of the combination
    uint k = m_currentCombination.size();

    // We want to find the index of the rightmost element in
    // m_currentCombination that can be increased.  We call its index 'pivot'.
    int pivot = k - 1;
    while (pivot >= 0 && m_currentCombination[pivot] == m_N + 1 - k + pivot)
        --pivot;

    // pivot will be -1 if the final combination of (N, k) is reached.
    if (pivot == -1) {
        // If we are at the max size of our analyzed combinations, we are done
        if (k == m_k) return false;
        else { // Otherwise set the first combination of k+1 elements
            m_currentCombination.clear();
            m_currentCombination.reserve(k + 1);
            for (uint i = 1; i <= k + 1; i++) m_currentCombination.push_back(i);
            return true;
        }
    } else {
        // We increment at pivot and set the following positions accordingly
        ++m_currentCombination[pivot];
        for (uint i = pivot + 1; i < k; ++i)
            m_currentCombination[i] = m_currentCombination[pivot] + i - pivot;
        return true;
    }


}
