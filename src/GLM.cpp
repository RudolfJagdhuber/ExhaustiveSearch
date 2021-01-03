
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

  int ret = 0;
  if (m_family == "gaussian") {
    // Use simple matrix algebra for optimization
    ret = computeOLS();
  } else if (m_family == "binomial") {
    // Execute the LBFGS optimizer and compute the betas
    lbfgs_parameter_t param;
    lbfgs_parameter_init(&param);
    ret = lbfgs(m_nBeta, m_beta, &m_negloglik, _evalLogReg, NULL, this,
      &param);

    // Lbfgs has many error codes (negative ret), which are not all real errors.
    // Unfortunately, I do not know which are still OK, so I assume, that if the
    // likelihood was set, it is somewhat acceptable (-> room for improvement).
    if (ret < 0 && m_negloglik !=0) ret = 123;
  }
  // Model could not be fitted
  if (ret < 0) m_negloglik = m_errorVal;
}


double GLM::getMSE() {

  // Model could not be fitted
  if (m_negloglik == m_errorVal) return m_errorVal;

  // double!! because otherwise (2/n -> 0), which has cost me hours to find...
  double n = (*m_D.XTest).n_rows;

  // shortcut for gaussian training set mse
  if (m_D.noTestSet() && m_family == "gaussian")
    return exp(2/n * m_negloglik - 1) / (2 * M_PI);

  double sse = 0;
  double eta, yHat;
  for (size_t i = 0; i < (*m_D.XTest).n_rows; i++) {
    eta = 0;
    for (size_t j = 0; j < m_nBeta; j++)
      eta += (*m_D.XTest)(i, m_featureComb[j]) * m_beta[j];

    if (m_family == "gaussian") yHat = eta;
    else if (m_family == "binomial") yHat = 1.0 / (1.0 + exp(-eta));
    else return m_errorVal;

    sse += pow((*m_D.yTest)[i] - yHat, 2);
  }
  return sse / n;
}


int GLM::computeOLS() {

  arma::vec beta;
  arma::mat X = getXTrainSubset();
  arma::vec y = arma::vec(*m_D.yTrain);

  // Use the standard OLS formula to compute the regression coefficients
  bool success = arma::solve(beta, X, y);
  if (success) {
    for (size_t i = 0; i < m_nBeta; i++) m_beta[i] = beta[i];
    double sse = arma::as_scalar((y - X * beta).t() * (y - X * beta));
    double n = y.n_rows;
    m_negloglik = n/2 * (log(2 * M_PI * sse / n) + 1);
    return 0;
  } else return -1;
}


// Helper function that computes the negLogLik for a given set of betas. It is
// used by the optimizer to optimize the regression coefficients. For lbfgs to
// work, it also needs to set the gradient vector to the memory address "g".
// For further reference on the used formulas for nll and the gradients see:
// https://web.stanford.edu/class/archive/cs/cs109/cs109.1178/lectureHandouts/220-logistic-regression.pdf
// page 2 bottom and page 3.
double GLM::evalLogReg(const double* betaPtr, double* g, const size_t n,
  const double step) {

  // References for shorter code and better readability
  const arma::mat& X = *m_D.XTrain;
  const std::vector<double>& y = *m_D.yTrain;

  // reset space of gradient vector
  memset(g, 0, sizeof(double) * n);

  // Iterate over observations i and sum up the negative log-likelihoods
  double nll = 0.0;
  for (size_t i = 0; i < X.n_rows; i++) {

    // Iterate over data columns to compute eta_i = x_i %*% beta
    double eta_i = 0.0;
    for (size_t j = 0; j < m_nBeta; j++)
      eta_i += X(i, m_featureComb[j]) * betaPtr[j];

    // Compute prediction of observation i
    double y_ihat =  1.0 / (1.0 + exp(-eta_i));

    // If predictions exact 0 or 1 use small number to allow log.
    if (y_ihat == 0.0) y_ihat = std::numeric_limits<double>::epsilon();
    if (y_ihat == 1.0) y_ihat = 1 - std::numeric_limits<double>::epsilon();

    // Also compute partial derivative of each beta_j and sum it up:
    for (size_t j = 0; j < m_nBeta; j++)
      g[j] -= (y[i] - y_ihat) * X(i, m_featureComb[j]);

    // Compute negative log likelihood of observation i and sum up
    nll -=  y[i] * log(y_ihat) + (1 - y[i]) * log(1 - y_ihat);
  }

  // Return the negative log Likelihood (which is to be minimized)
  return nll;
}







// // arma version of LogReg is > 2.5x slower (I suspect data copies)
// double GLM::evalLogReg(const double* betaPtr, double* g, const size_t n,
//   const double step) {
//
//   // create arma::vec from the memory address of the optimized betas
//   arma::vec beta(betaPtr, m_nBeta);
//   const arma::mat& X = (*m_D.XTrain).cols(arma::Col<uint>(m_featureComb));
//   const arma::vec& y = arma::vec(*m_D.yTrain);
//
//   // Compute the vector of predictions
//   arma::vec yHat = 1.0 / (1.0 + exp(-(X * beta)));
//
//   // If predictions exact 0 or 1 change by epsilon to allow log(y) and log(1-y).
//   yHat.elem(arma::find(yHat == 0)).fill(std::numeric_limits<double>::epsilon());
//   yHat.elem(arma::find(yHat == 1)).fill(
//     1 - std::numeric_limits<double>::epsilon());
//
//   // Compute the gradient vector of the coefficients and write to memory
//   arma::rowvec grd = -(y - yHat).t() * X;
//   for (size_t i = 0; i < m_nBeta; i++) g[i] = grd(i);
//
//   // Compute and return the negative log likelihood
//   return -arma::as_scalar(y.t() * log(yHat) + (1 - y).t() * log(1 - yHat));
// }
