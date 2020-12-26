#pragma once

#include "globals.h"

// A simple and sparse class to hold all data information (response and feature
// matrix). The features are stored as a double vector with known dimensions.
class DataSet {
    matrix m_X;
    std::vector<double> m_y;

public:
    DataSet(matrix X, std::vector<double>& y) : m_X(X), m_y(y) {}
    const matrix& getX() const { return m_X;}
    const std::vector<double>& getY() const { return m_y; }
};
