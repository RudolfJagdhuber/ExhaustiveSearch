#pragma once

#include <mutex>
#include <future>

#include "globals.h"
#include "Combination.h"
#include "GLM.h"
#include "StatusLog.h"


// This function does the main exhaustive execution.
// Note that Model/Comb are copies, not references -> individual per thread!
// Comb object includes the search range of this thread.
void ExhaustiveThread(const size_t& threadID, GLM Model,
  const Combination& Comb, const size_t& nResults, StatusLog* SLptr,
  bool quietly, std::promise<ranking>&& p);

