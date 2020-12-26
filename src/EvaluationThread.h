#pragma once

#include <mutex>

#include "globals.h"
#include "DataSet.h"
#include "Combination.h"
#include "GLM.h"
#include "StatusLog.h"




// This function does the main exhaustive execution.
// Note that LogReg/Comb are copies, not references -> individual per thread!
// Comb object includes the search range of this thread.
ranking ExhaustiveThread(size_t threadID, GLM Model, Combination Comb,
  size_t nResults, StatusLog* SLptr, bool quietly);

