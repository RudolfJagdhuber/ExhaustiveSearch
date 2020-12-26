#pragma once

#include <vector>
#include <limits>
#include <math.h>
#include <string.h>

#include "DataSet.h"
#include "./lbfgs/lbfgs.h"

class LogisticRegression {

	DataSet m_D;
	std::vector<ushort> m_featureComb;

	// Used in the optimization process
	uint m_nBeta;
	double* m_beta;
	double m_negloglik;

public:

	LogisticRegression(const DataSet& D);
	std::vector<double> getBeta();
	double getNegLogLik() { return m_negloglik; }
	// return and store as float to save memory
	float getAIC() { return 2 * ((float)m_negloglik + m_nBeta); }
	void setFeatureCombination(const std::vector<ushort>& new_comb);
	int fit();

	// The target function to be optimized in the form that lbfgs takes it
	static double _evaluate(void* instance, const double* beta, double* g,
		const int n, const double step)	{
		return reinterpret_cast<LogisticRegression*>(instance)->evaluate(beta,
			g, n, step);
	}

	// The main lbfgs function. Sets the gradient values at pointer.
	// Returns target function value.
	double evaluate(const double *beta, double *g, const size_t n,
		const double step);

};
