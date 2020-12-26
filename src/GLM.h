#pragma once

#include <vector>
#include <limits>
#include <math.h>
#include <string>
#include <string.h>

#include "DataSet.h"
#include "lbfgs.h"

class GLM {

protected:
  DataSet m_D;
  std::vector<uint> m_featureComb;
  std::string m_family;
  uint m_nBeta;
  double* m_beta;
  double m_negloglik;

public:
  GLM(const DataSet& D, std::string family);
  arma::mat getX() {
    return (Rcpp::as<arma::mat>(m_D.getX())).cols(
        arma::Col<uint>(m_featureComb));
  }
  arma::colvec getY() { return arma::colvec(m_D.getY()); }
  std::vector<double> getBeta();
  double getNegLogLik() { return m_negloglik; }
  float getAIC() { return 2 * ((float)m_negloglik + m_nBeta); }
  void setFeatureCombination(const std::vector<uint>& new_comb);
  int fit();

  // Additional functions for individual families

  // Multiple Linear Regression

  // void computeOLS();





  // Logistic Regression

  // The target function to be optimized in the form that lbfgs takes it
  static double _evalLogReg(void* instance, const double* beta, double* g,
    const int n, const double step)	{
    return reinterpret_cast<GLM*>(instance)->evalLogReg(beta, g, n, step);
  }

  // The main lbfgs function. Sets the gradient values at pointer.
  // Returns target function value.
  double evalLogReg(const double *beta, double *g, const size_t n,
    const double step);

};
