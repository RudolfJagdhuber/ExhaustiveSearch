#include "LogisticRegression.h"


// Only define the data set for the Object, and update the feature combination
// separately. This makes an exhaustive evaluation easier to implement.
LogisticRegression::LogisticRegression(const DataSet& D)
    : m_D(D), m_nBeta(D.numCols()), m_negloglik(0) {

    // Define initial feature subset (use all columns)
    m_featureComb.reserve(D.numCols());
    for (ushort i = 1; i <= D.numCols(); i++) m_featureComb.push_back(i);

    // Allocate and initialize the betas
    m_beta = lbfgs_malloc(m_nBeta);
    for (size_t i = 0; i < m_nBeta; i++) m_beta[i] = 0.0;
}


void LogisticRegression::setFeatureCombination(
    const std::vector<ushort>& new_comb) {

    // Set the new feature combination
	  m_featureComb = new_comb;
    m_nBeta = m_featureComb.size();

    // Allocate and initialize the betas
    m_beta = lbfgs_malloc(m_nBeta);
    for (size_t i = 0; i < m_nBeta; i++) m_beta[i] = 0.0;

    // Reset negative log-Likelihood
    m_negloglik = 0.0;
}


// TODO: Revise why so complicated
std::vector<double> LogisticRegression::getBeta() {

	std::vector<double> beta;
	beta.reserve(m_nBeta);
	for (size_t i = 0; i < m_nBeta; i++)
		beta.emplace_back(m_beta[i]);
	return beta;
}


// Use the LBFGS optimizer to fit the model coefficients
int LogisticRegression::fit() {

    if (m_beta == nullptr) return 1;

    // Execute the LBFGS optimizer and compute the betas
    lbfgs_parameter_t param;
    lbfgs_parameter_init(&param);
    int ret = lbfgs(m_nBeta, m_beta, &m_negloglik, _evaluate, NULL, this,
      &param);

    return ret;
}


// Helper function that computes the negLogLik for a given set of betas.
// It is used by the optimizer to find the regression coefficients.
double LogisticRegression::evaluate(const double* beta, double* g,
  const size_t n, const double step) {

    // reset space of gradient vector
    memset(g, 0, sizeof(double) * n);

    // Iterate over observations i and sum up the negative log-likelihoods
    double logL = 0.0;
    for (size_t i = 0; i < m_D.numRows(); i++) {

        // Iterate over data columns to compute eta_i = x_i %*% beta
        double eta_i = 0.0;
        for (size_t j = 0; j < m_featureComb.size(); j++)
            eta_i += m_D.getXij(i, j, m_featureComb) * beta[j];

        // Compute prediction of observation i
        double y_ihat =  1.0 / (1.0 + exp(-eta_i));

        // If predictions exact 0 or 1 use small number to allow log.
        if (y_ihat == 0.0) y_ihat = std::numeric_limits<double>::epsilon();
        if (y_ihat == 1.0) y_ihat = 1 - std::numeric_limits<double>::epsilon();

        // Also compute partial derivative of each beta_j and sum it up:
        // sum( (y_i - y_ihat) * x_{ij} )
        for (size_t j = 0; j < m_featureComb.size(); j++)
            g[j] += (m_D.getY()[i] - y_ihat) * m_D.getXij(i, j, m_featureComb);

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
