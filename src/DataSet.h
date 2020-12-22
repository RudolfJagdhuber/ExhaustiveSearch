#pragma once

#include "globals.h"

// A simple and sparse class to hold all data information (response and feature
// matrix). The features are stored as a double vector with known dimensions.
class DataSet {
    std::vector<double> m_X;
    std::vector<double> m_y;
    uint m_Xrows;
    uint m_Xcols;

public:
    DataSet(std::vector<double>& X, std::vector<double>& y, uint Xrows,
        uint Xcols) : m_X(X), m_y(y), m_Xrows(Xrows), m_Xcols(Xcols) {}

    const uint& numRows() const {return m_Xrows;}
    const uint& numCols() const {return m_Xcols;}
    const std::vector<double>& getX() const { return m_X;}
    const std::vector<double>& getY() const { return m_y; }

    // Select ij-element from full data, or from a given subset (all start at 0)
    const double& getXij(uint i, uint j, std::vector<ushort> col_subset) const {
        return m_X[col_subset[j] + i * m_Xcols];
    }
};
