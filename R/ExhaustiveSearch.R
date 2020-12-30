

#' @export
ExhaustiveSearch = function(formula, data, family = NULL, combsUpTo = NULL,
  nResults = NULL, nThreads = 4, errorVal = -1, allowHugeStorage = FALSE,
  quietly = FALSE) {

  formula = formula(formula)
  if (!inherits(formula, "formula")) stop("Error: Invalid formula.")

  ## Extract the design matrix with dims, response vector and feature names
  X = stats::model.matrix(formula, data)

  ## Does the setup include an intercept
  intercept = attr(terms(formula, data = data), "intercept") == 1

  ## Features combinations are stored as 1, 2, 3,... in C++, 0 is reserved for
  ## the intercept, which is the first columnof the DataSet (Data[,0]). To keep
  ## the C++ subsetting via Column[,index] consistent for setups with and
  ## without intercepts, an unused dummy column needs to be added at setups
  ## without intercept. This makes Data[,i] always refer to the same feature i.
  if (!intercept) X = cbind("NotUsed" = 0, X)

  feats = colnames(X)[-1]
  y = as.numeric(model.response(model.frame(formula, data)))

  ## Check if the family parameter was set correctly
  if (is.null(family)) {
    family = ifelse(length(unique(y)) == 2, "binomial", "gaussian")
    warning(paste0("Warning: family not specified! From the given response ",
      "data, I assume '", family, "' and continue."))
  } else if (family == "gaussian") {
    if (length(unique(y)) == 2) {
      warning(paste0("Warning: You set family to 'gaussian', but the response ",
        "appears to be binary, are you sure this is intended?"))
    }
  } else if (family == "gaussian") {
    if (length(unique(y)) != 2) {
      stop(paste0("Error: You set family to 'binomial', but the response is ",
        "not binary."))
    }
  }

  ## Check combUpTo parameter
  if (is.null(combsUpTo)) combsUpTo = length(feats)
  if (!is.numeric(combsUpTo) | length(combsUpTo) != 1 | any(combsUpTo <= 0)) {
    stop("Error: combsUpTo needs to be a single numeric value > 0")
  }
  if (combsUpTo == Inf) combsUpTo = length(feats)


  ## Check nResults parameter and safety-check if the user requests huge memory
  nCombs = sum(choose(length(feats), seq(combsUpTo)))
  if (is.null(nResults)) nResults = nCombs
  if (!is.numeric(nResults) | length(nResults) != 1 | any(nResults <= 0)) {
    stop("Error: nResults needs to be a single numeric value > 0")
  }
  if (nResults > 100000 & !allowHugeStorage) {
    stop(paste0("Saving the ", nResults, " best models would take a very ",
      "large amount of memory!\n\nPlease consider setting nResults <= 100000.",
      "\n\nIf you want to continue anyway, set 'allowHugeStorage = TRUE' in ",
      " the function call."))
  }

  # Check nThreads parameter
  if (is.null(nThreads)) stop("Error: nThreads parameter cannot be NULL")
  if (!is.numeric(nThreads) | length(nThreads) != 1 | nThreads %% 1 != 0) {
    stop("Error: nThreads needs to be a single integer value")
  }

  ## Check errorVal parameter
  if (is.null(errorVal)) stop("Error: errorVal parameter cannot be NULL")
  if (!is.numeric(errorVal) | length(errorVal) != 1) {
    stop("Error: errorVal needs to be a single numeric value")
  }

  if (!quietly) cat("\nStarting the exhaustive evaluation.\n\n")

  ## The main C++ function call
  cppOutput = ExhaustiveSearchCpp(
    XInput = X,
    yInput = y,
    family = family,
    intercept = intercept,
    combsUpTo = combsUpTo,
    nResults = nResults,
    nThreads = nThreads,
    errorVal = errorVal,
    quietly = quietly)

  if (length(cppOutput) == 0) stop("User interrupt or internal error.")

  ## Preparation of the return value
  result = list()
  class(result) = "ExhaustiveSearch"

  result$nModels = nCombs
  result$runtimeSec = cppOutput[[1]]
  result$ranking = list(aic = cppOutput[[2]], featureIDs = cppOutput[[3]])
  result$featureNames = feats
  result$batchSizes = cppOutput[[4]]
  result$setup = list(call = match.call(), family = family,
    intercept = intercept, combsUpTo = combsUpTo, nResults = nResults,
    nThreads = nThreads)

  if (!quietly) cat("\nEvaluation finished successfully.\n")

  return(result)
}
