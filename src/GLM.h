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
  const DataSet& m_D;
  std::vector<uint> m_featureComb;
  std::string m_family;
  bool m_intercept;
  double m_errorVal;
  size_t m_nBeta;
  double* m_beta;
  double m_negloglik;

public:
  // Initializer only defines the modeling setup. A feature combination needs
  // to be set separately before any further evaluation.
  GLM(const DataSet& D, std::string family, bool intercept, double errorVal)
    : m_D(D), m_family(family), m_intercept(intercept), m_errorVal(errorVal),
      m_nBeta(D.getX().n_cols), m_negloglik(0) {}
  std::string getFamily() { return m_family;};
  bool hasIntercept() { return m_intercept; }
  arma::mat getXsubset() {
    return m_D.getX().cols(arma::Col<uint>(m_featureComb));
  }
  float getAIC() {
    if (m_negloglik == m_errorVal) return m_errorVal;
    else return 2 * ((float) m_negloglik + m_nBeta +
      (m_family == "gaussian" ? 1 : 0));
  }
  void setFeatureCombination(const std::vector<uint>& new_comb);
  void fit();
  // Multiple Linear Regression
  int computeOLS();
  // Logistic Regression
  // The target function to be optimized in the form that lbfgs takes it
  static double _evalLogReg(void* instance, const double* betaPtr, double* g,
    const int n, const double step)	{
    return reinterpret_cast<GLM*>(instance)->evalLogReg(betaPtr, g, n, step);
  }
  // The main lbfgs function. Sets the gradient values at pointer.
  // Returns target function value.
  double evalLogReg(const double *betaPtr, double *g, const size_t n,
    const double step);

};
