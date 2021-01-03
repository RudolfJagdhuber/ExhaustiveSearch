#pragma once

#include "globals.h"

// A simple struct to hold pointers to a training/testing data set combination
struct DataSet {

    const arma::mat * XTrain;
    const std::vector<double> * yTrain;
    const arma::mat * XTest;
    const std::vector<double> * yTest;

    DataSet(const arma::mat*& XTrain, const std::vector<double>*& yTrain,
        const arma::mat*& XTest, const std::vector<double>*& yTest)
    : XTrain(XTrain), yTrain(yTrain), XTest(XTest), yTest(yTest) {}

    bool noTestSet() { return XTrain == XTest && yTrain == yTest; }
};
