
#include <math.h>

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

    // Compute the total number of existing combinations with this setup.
    m_nCombinations = computeCombinations(m_N, m_k);

    // The following algorithm computes a set of starting combinations that
    // split all combinations into almost equal sized parts and returns
    // a vector of the combinations that divide the batches

    // The intended minimal size of each batch. Use ceil(), if it would create
    // enough batches, if not, use floor() (= ceil() - 1).
    size_t targetSize = ceil(((float)m_nCombinations) / m_nBatches);
    if (targetSize * (m_nBatches - 1) >= m_nCombinations) targetSize--;

    // Initial limit needs to be "(0)" as the evaluation calls nextCombination()
    // as a first step, which thus results in the true first element "(1)"
    std::vector<uint> element{0};
    m_batchLimits.emplace_back(element);

    // The current position within 'element' under evaluation
    int indent = 0;
    // The current length 'element', basically just: element.size();
    size_t curLength = element.size();
    // The current value under inspection, basically just: element[indent];
    uint digit = 0;
    // Both values are necessary copies, as element gets changed repeatedly

    // One iteration per batch limit to be identified
    for (size_t j = 0; j < m_nBatches; j++) {

        // The final iteration is trivial so avoid unnecessary computations here
        if (j == m_nBatches - 1) {
            // Insert the last combination
            element.clear();
            for (uint i = m_N - m_k + 1; i <= m_N ; i++)
                element.emplace_back(i);
            m_batchLimits.push_back(element);

            // Insert the size of the remaining combinations
            size_t combsNow = 0;
            for (size_t& n : m_batchSizes) combsNow += n;
            m_batchSizes.push_back(m_nCombinations - combsNow);

            break;
        }

        // Continue to add combs until the batch size is reached
        size_t curBatchSize = 0;
        size_t projectedSize = 0;
        while (curBatchSize < targetSize) {

            // Identify at which position (indent) I can increase without
            // overshooting the targetSize. There is always the option to just
            // add 1 combination, so this loop will terminate eventually.
            do {
                // What would an increase add to curBatchSize?
                projectedSize = curBatchSize +
                    NoverK(m_N - element[indent], curLength - indent - 1);

                // If it would overshoot, I will retry with a higher indent
                if (projectedSize > targetSize) indent++;
            } while (projectedSize > targetSize);

            // I have now decided which step to make, so lets make it

            // First find the indent at which it is possible to increase
            while (element[indent] >= m_N - curLength + indent + 1) {
                indent--;
                if (indent == -1) break;
            }

            // Is there still a way to increase the current combination?
            if (indent != -1) {
                // Increase at indent position and set all following accordingly
                digit = element[indent];
                for (size_t i = 0; i < (curLength - indent); i++)
                    element[indent + i] =  digit + i + 1;

            // indent == -1 means no increase possible at current length
            // Try to start with the next larger length if possible.
            } else if (curLength < m_k) {
                curLength++;
                indent = 0;
                element.clear();
                for (uint i = 1; i <= curLength ; i++) element.emplace_back(i);

            // If not possible, the last combination was reached and its done.
            // Note: this should never be reached, due to the shortcut taken in
            // the final iteration
            } else break;

            // The step was made and the projected size is now reality.
            curBatchSize = projectedSize;
        }

        // A combination was found -> Add it.
        m_batchLimits.emplace_back(element);

        // Also add the resulting batch size to m_batchSizes
        m_batchSizes.emplace_back(curBatchSize);

        // This line just ensures that the algorithm finishes after the last
        // combination, even if for some reason the combinations were split into
        // less than m_nBatches.
        if (curLength == m_k && element[0] == m_N - m_k + 1) {
            m_nBatches = m_batchSizes.size();
            break;
        }
    }
}


// A free function that can compute the next combination from a given one
void setNextCombination(std::vector<uint>& comb, const size_t& N) {

    // Get the current length of the combination
    uint k = comb.size();

    // We want to find the index of the rightmost element in comb that can be
    // increased.  We call its index 'indent'.
    int indent = k - 1;
    while (indent >= 0 && comb[indent] == N + 1 - k + indent) indent--;

    // indent will be -1 if the final combination of size k is reached.
    if (indent == -1) {
        // Otherwise set the first combination of k+1 elements
        comb.clear();
        comb.reserve(k + 1);
        for (uint i = 1; i <= k + 1; i++) comb.push_back(i);
    } else {
        // We increment at indent and set the following positions accordingly
        comb[indent]++;
        for (uint i = indent + 1; i < k; i++)
            comb[i] = comb[indent] + i - indent;
    }
}
