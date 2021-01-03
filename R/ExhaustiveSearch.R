

#' Exhaustive feature selection function
#'
#' Performs an exhaustive feature selection. `ExhaustiveSearch()` is a fast and
#' scalable implementation of an exhaustive feature selection framework. It is
#' particularly suited for huge tasks, which would typically not be possible due
#' to memory limitations.  of candidate The current version allows to compute
#' linear and logistic regression models and compare them with respect to AIC
#' or MSE.
#'
#' @details
#' TODO
#'
#' @param formula An object of class '[formula]' (or one that can be coerced to
#'   that class): a symbolic description of the model to be fitted.
#' @param data A data frame (or object coercible by `as.data.frame` to a data
#'   frame) containing the variables in the model.
#' @param family A character string naming the family function similar to the
#'   parameter in [glm()]. Currently options are 'gaussian' or 'binomial'.
#' @param performanceMeasure A character string naming the performance measure
#'   to compare models by. Current options are 'AIC' or 'MSE'.
#' @param combsUpTo An integer of length 1 to set an upper limit to the number
#'   of features in a combination. This can be useful to drastically reduce the
#'   total number of combinations to a feasible size.
#' @param nResults An integer of length 1 to define the size of the final
#'   ranking list. The default (1000) provides a good trade-off of memory usage
#'   and result size. Set this value to `Inf` to store all models.
#' @param nThreads Number of threads to use. The default is number of CPUs
#'   available.
#' @param testSetIDs A vector of row indices of data, which define the test set
#'   partition. If this parameter is `NULL` (default), models are trained and
#'   evaluated on the full data set. If it is set, models are trained on
#'   `data[-testSetIDs,]` and tested on `data[testSetIDs,]`.
#' @param errorVal A numeric value defining what performance result is returned
#'   if the model could not be fitted. The default (-1) makes those models
#'   appear at the top of the result ranking.
#' @param quietly logical. If set to TRUE (default), status and runtime updates
#'   are printed to the console.
#'
#' @return Object of class `ExhaustiveSearch` with elements
#'   \item{nModels}{The total number of evaluated models.}
#'   \item{runtimeSec}{The total runtime of the exhaustive search in seconds.}
#'   \item{ranking}{A list of the performance values and the featureIDs. The
#'   i-th element of both correspond. The featureIDs refer to the elements of
#'   `featureNames`. Formatted results of these rankings can e.g. be obtained
#'   with [getFeatures()].}
#'   \item{featureNames}{The feature names in the given data. `featureIDs` in
#'   the ranking element refer to this vector.}
#'   \item{setup}{A list of imput parameters from the function call.}
#'
#' @author Rudolf Jagdhuber
#'
#' @export
ExhaustiveSearch = function(formula, data, family = NULL,
  performanceMeasure = NULL, combsUpTo = NULL, nResults = 1000, nThreads = NULL,
  testSetIDs = NULL, errorVal = -1, allowHugeStorage = FALSE, quietly = FALSE) {

  formula = formula(formula)
  if (!inherits(formula, "formula")) stop("\n\nInvalid formula.")

  data = as.data.frame(data)
  if (!inherits(data, "data.frame")) stop("\n\nInvalid data object.")

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

  ## Split into training and testing partitions
  if (length(testSetIDs) == 0) {
    XTest = X[NULL, ]
    yTest = y[NULL]
  } else if (!is.numeric(testSetIDs) | any(testSetIDs > nrow(X)) |
      length(testSetIDs) >= nrow(X)) {
    stop(paste0(
      "\nThe given testSetIDs need to be numeric and cannot exceed the data\n",
      "dimension.\n\n"))
  } else {
    XTest = X[testSetIDs, ]
    yTest = y[testSetIDs]
    X = X[-testSetIDs,]
    y = y[-testSetIDs]
  }

  ## Check if the family parameter was set correctly
  if (is.null(family)) {
    family = ifelse(length(unique(y)) == 2, "binomial", "gaussian")
    warning(paste0("\n\n",
      "'family' not specified! From the given response data, I assume\n'",
      family, "' and continue.\n\n"))
  } else if (family == "gaussian") {
    if (length(unique(y)) == 2) warning(paste0("\n\n",
      "'family' was set to 'gaussian', but the response appears to be\n",
      "binary, are you sure this is intended?\n\n"))
  } else if (family == "gaussian") {
    if (length(unique(y)) != 2) stop(paste0("\n\n",
      "'family' was set to 'binomial', but the response is not binary.\n\n"))
  }

  ## Check the performanceMeasure parameter and the validity of each case
  if (is.null(performanceMeasure)) {
    if (nrow(XTest) == 0) performanceMeasure = "AIC"
    else performanceMeasure = "MSE"
  } else if (performanceMeasure == "AIC") {
    if (nrow(XTest) != 0) stop(paste0("\n\n",
      "A TestSet was defined, but 'performanceMeasure' is set to 'AIC'.\n\n"))
  } else if (performanceMeasure == "MSE") {
    if (nrow(XTest) == 0) warning(paste0("\n\n",
      "No TestSet was defined and performanceMeasure set to 'MSE'.\n",
      "Comparing MSE values on training data will always prefer the higher\n",
      "dimensional model in nested setups and is thus not recommended for\n",
      "feature selection tasks.\n\n"))
  } else stop(paste0("\n\n",
    "Unsupported performanceMeasure! Please check the help file for a list\n",
    "of all available options.\n\n"))

  ## Check combUpTo parameter
  if (is.null(combsUpTo)) combsUpTo = length(feats)
  if (!is.numeric(combsUpTo) | length(combsUpTo) != 1 | any(combsUpTo <= 0))
    stop("\n\ncombsUpTo needs to be a single numeric value > 0\n\n")
  if (combsUpTo == Inf) combsUpTo = length(feats)


  ## Check nResults parameter and safety-check if the user requests huge memory
  nCombs = sum(choose(length(feats), seq(combsUpTo)))
  if (is.null(nResults)) nResults = nCombs
  if (!is.numeric(nResults) | length(nResults) != 1 | any(nResults <= 0))
    stop("\nnResults needs to be a single numeric value > 0")
  if (nResults > nCombs) nResults = nCombs

  # Check nThreads parameter, if not set detect later in C++
  if (is.null(nThreads)) nThreads = 0
  else if (!is.numeric(nThreads) | length(nThreads) != 1 | nThreads %% 1 != 0)
    stop("\n\nnThreads needs to be a single integer value\n\n")

  ## Check errorVal parameter
  if (is.null(errorVal)) stop("\nerrorVal parameter cannot be NULL\n\n")
  if (!is.numeric(errorVal) | length(errorVal) != 1)
    stop("\n\nerrorVal needs to be a single numeric value\n\n")

  if (!quietly) cat("\nStarting the exhaustive evaluation.\n\n")

  ## The main C++ function call
  cppOutput = ExhaustiveSearchCpp(
    XInput = X,
    yInput = y,
    XTestSet = XTest,
    yTestSet = yTest,
    family = family,
    performanceMeasure = performanceMeasure,
    intercept = intercept,
    combsUpTo = combsUpTo,
    nResults = nResults,
    nThreads = nThreads,
    errorVal = errorVal,
    quietly = quietly)

  if (length(cppOutput) == 0) stop("\n\nAn internal error occured.\n\n")

  ## Preparation of the return value
  result = list()
  class(result) = "ExhaustiveSearch"

  result$nModels = cppOutput[[4]]
  result$runtimeSec = cppOutput[[1]]
  result$ranking = list(
    performance = cppOutput[[2]],
    featureIDs = cppOutput[[3]])
  result$featureNames = feats
  result$setup = list(call = match.call(), family = family,
    performanceMeasure = performanceMeasure, intercept = intercept,
    combsUpTo = combsUpTo, nResults = nResults, nThreads = nThreads,
    testSetIDs = testSetIDs, nTrain = nrow(X), nTest = nrow(XTest))

  if (!quietly) {
    if (cppOutput[[4]] == nCombs) cat("\nEvaluation finished successfully.\n\n")
    else warning("\nEvaluation Incomplete! Not all models were evaluated!\n\n")
  }

  return(result)
}
