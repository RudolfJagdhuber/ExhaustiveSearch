
#include "GLM.h"


void GLM::setFeatureCombination(const std::vector<uint>& new_comb) {

  // Extract the size of the feature combination and allocate the betas
  m_nBeta = new_comb.size() + (m_intercept ? 1 : 0);
  m_beta = (double*)malloc(sizeof(double) * m_nBeta);
  for (size_t i = 0; i < m_nBeta; i++) m_beta[i] = 0.0;

  // Set the new feature combination
  m_featureComb.clear();
  m_featureComb.reserve(m_nBeta);
  if (m_intercept) m_featureComb.push_back(0);
  for (uint elem : new_comb) m_featureComb.push_back(elem);

  // Reset the negative log-Likelihood
  m_negloglik = 0.0;
}


void GLM::fit() {

  if (m_beta == nullptr) return;
  int ret = 1;
  if (m_family == "gaussian") {
    // Use simple matrix algebra for optimization
    ret = computeOLS();
  } else if (m_family == "binomial") {
    // Execute the LBFGS optimizer and compute the betas
    lbfgs_parameter_t param;
    lbfgs_parameter_init(&param);
    ret = lbfgs(m_nBeta, m_beta, &m_negloglik, _evalLogReg, NULL, this, &param);
  }
  // Model could not be fitted
  if (ret == 1) m_negloglik = m_errorVal;
}


int GLM::computeOLS() {

  // Use the standard OLS formula to compute the regression coefficients
  arma::mat betas;
  bool success = arma::solve(betas, getX(), getY());
  if (success) {
    double sse = arma::as_scalar(
      (getY() - getX() * betas).t() * (getY() - getX() * betas));
    double n = m_D.getY().size();
    m_negloglik = n/2 * (log(2 * M_PI * sse / n) + 1);
    return 0;
  } else return 1;
}


// Helper function that computes the negLogLik for a given set of betas.
// It is used by the optimizer to find the regression coefficients.
double GLM::evalLogReg(const double* beta, double* g, const size_t n,
  const double step) {

  // reset space of gradient vector
  memset(g, 0, sizeof(double) * n);

  // Iterate over observations i and sum up the negative log-likelihoods
  double logL = 0.0;
  for (size_t i = 0; i < m_D.getY().size(); i++) {

    // Iterate over data columns to compute eta_i = x_i %*% beta
    double eta_i = 0.0;
    for (size_t j = 0; j < m_featureComb.size(); j++)
      eta_i += getX()(i, j) * beta[j];

    // Compute prediction of observation i
    double y_ihat =  1.0 / (1.0 + exp(-eta_i));

    // If predictions exact 0 or 1 use small number to allow log.
    if (y_ihat == 0.0) y_ihat = std::numeric_limits<double>::epsilon();
    if (y_ihat == 1.0) y_ihat = 1 - std::numeric_limits<double>::epsilon();

    // Also compute partial derivative of each beta_j and sum it up:
    // sum( (y_i - y_ihat) * x_{ij} )
    for (size_t j = 0; j < m_nBeta; j++)
      g[j] += (m_D.getY()[i] - y_ihat) * getX()(i, j);

    // Compute negative log likelihood of observation i and sum up
    logL +=  m_D.getY()[i] * log(y_ihat) +
      (1 - m_D.getY()[i]) * log(1 - y_ihat);

    // for reference on the used formulas see:
    // https://web.stanford.edu/class/archive/cs/cs109/cs109.1178/lectureHandouts/220-logistic-regression.pdf
    // page 2 bottom and page 3.
  }

  // The gradient of the negative function should be negative
  for (size_t i = 0; i < n; i++) g[i] = -g[i];

  // Return the negative log Likelihood (which is to be minimized)
  return -logL;
}
