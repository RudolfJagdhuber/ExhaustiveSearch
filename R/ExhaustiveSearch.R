

#' Exhaustive feature selection
#'
#' Performs an exhaustive feature selection. `ExhaustiveSearch()` is a fast and
#' scalable implementation of an exhaustive feature selection framework. It is
#' particularly suited for huge tasks, which would typically not be possible due
#' to memory limitations. The current version allows to compute linear and
#' logistic regression models and compare them with respect to AIC or MSE.
#'
#' @details
#' An exhaustive search evaluates all setups of a combinatorial task. In feature
#' and model selection application, exhaustive searches are often referred to as
#' *optimal* search strategies, as they test each setup and therefore ensure to
#' find the best solution. The main downside of this approach is the possibly
#' enormous computational complexity of the task. `ExhaustiveSearch()` provides
#' an easy to use and efficient framework for these tasks.
#'
#' Its main characteristics are:
#' - Combinations are iteratively generated on the fly,
#' - Model fitting and evalution is performed multi-threaded in C++,
#' - Only a fixed amount of models are stored to keep memory usage small.
#'
#' Therefore, the framework of this package is able to evaluate huge tasks of
#' billions of models, while only being limited by run-time.
#'
#' Currently, ordinary linear regression models similar to [lm()] and logistic
#' regression models similar to [glm()] (with parameter `family = "binomial"`)
#' can be fitted. The model type is specified via the `family` parameter. All
#' model results of the C++ backend are identical to what would be obtained by
#' [glm()] or [lm()]. For that, the logistic regression also uses the same
#' \href{https://en.wikipedia.org/wiki/Limited-memory_BFGS}{L-BFGS} optimizer
#' as [glm()].
#'
#' To assess the quality of a model, the `performanceMeasure` options 'AIC'
#' (Akaike's An Information Criterion) and 'MSE' (Mean Squared Error) are
#' implemented. Note that the AIC can only be computed on the training data,
#' while it is recommended for the MSE to be computed on independent test data.
#' If `performanceMeasure` is not set, it will be decided according to the
#' definition of a test data set.
#'
#' While this framework is able to handle very large amounts of combinations, an
#' exhaustive search of every theoretical combination can still be unfeasible.
#' However, a possible way to drastically limit the total number of combinations
#' is to define an upper bound for the size of a combination. For example,
#' evaluating all combinations of 500 features (3.3e150) is obviously
#' impossible. But if we only consider combinations of up to 3 features, this
#' number reduces to around 21 million, which could easily be evaluated by this
#' framework in less than a minute (16 threads). Setting an upper limit is thus
#' a very powerful option to enable high dimensional analyses. It is implemented
#' by the parameter `combsUpTo`.
#'
#' A core element of why this framework does not require more memory if tasks
#' get larger is that at any point the best models are stored in a list of
#' fixed size. Therefore, sub-optimal models are not saved and do not take space
#' and time to be handled. The parameter defining the size of the models, which
#' are actively stored is `nResults`. Large values here can impair performance
#' or even cause errors, if the system memory runs out and should always be set
#' with care. The function will however warn you beforehand if you set a very
#' large value here.
#'
#' The parameter `testSetIDs` can be used to split the data into a training and
#' testing partition. If it is not set, all models will be trained and tested on
#' the full data set. If it is set, the data will be split beforehand into
#' `data[testSetIDs,]` and `data[-testSetIDs,]`.
#'
#'
#'
#' The development version of this package can be found at
#' \url{https://github.com/RudolfJagdhuber/ExhaustiveSearch}. Issues or requests
#' are handled on this page.
#'
#' @param formula An object of class [formula] (or one that can be coerced to
#'   that class): a symbolic description of the model to be fitted.
#' @param data A [data.frame] (or object coercible by [as.data.frame()] to a
#'   [data.frame]) containing the variables in the model.
#' @param family A [character] string naming the family function similar to the
#'   parameter in [glm()]. Currently options are 'gaussian' or 'binomial'. If
#'   not specified, the function tries to guess it from the response variable.
#' @param performanceMeasure A [character] string naming the performance measure
#'   to compare models by. Currently available options are 'AIC' (Akaike's An
#'   Information Criterion) or 'MSE' (Mean Squared Error).
#' @param combsUpTo An integer of length 1 to set an upper limit to the number
#'   of features in a combination. This can be useful to drastically reduce the
#'   total number of combinations to a feasible size.
#' @param nResults An integer of length 1 to define the size of the final
#'   ranking list. The default (5000) provides a good trade-off of memory usage
#'   and result size. Set this value to `Inf` to store all models.
#' @param nThreads Number of threads to use. The default is to detect the
#'   available number of threads automatically.
#' @param testSetIDs A vector of row indices of data, which define the test set
#'   partition. If this parameter is `NULL` (default), models are trained and
#'   evaluated on the full data set. If it is set, models are trained on
#'   `data[-testSetIDs,]` and tested on `data[testSetIDs,]`.
#' @param errorVal A numeric value defining what performance result is returned
#'   if the model could not be fitted. The default (-1) makes those models
#'   appear at the top of the result ranking.
#' @param quietly [logical]. If set to `TRUE` (default), status and runtime
#'   updates are printed to the console.
#' @param checkLarge [logical]. Very large calls get stopped by a safety net.
#'   This parameter can be used to execute these calls anyway.
#'
#' @return Object of class `ExhaustiveSearch` with elements
#'   \item{nModels}{The total number of evaluated models.}
#'   \item{runtimeSec}{The total runtime of the exhaustive search in seconds.}
#'   \item{ranking}{A list of the performance values and the featureIDs. The
#'     i-th element of both correspond. The featureIDs refer to the elements of
#'     `featureNames`. Formatted results of these rankings can e.g. be obtained
#'     with [getFeatures()], or [resultTable()].}
#'   \item{featureNames&#160;&#160;}{The feature names in the given data.
#'     `featureIDs` in the ranking element refer to this vector.}
#'   \item{batchInfo}{A list of information on the batches, into which the
#'     total task has been partitioned. List elements are the number of batches,
#'     the number of elements per batch, and the combination boundaries that
#'     define the batches.}
#'   \item{setup}{A list of input parameters from the function call.}
#'
#' @examples
#' ## Linear Regression on mtcars data
#' data(mtcars)
#'
#' ##  Exhaustive search of 1023 models compared by AIC
#' ES <- ExhaustiveSearch(mpg ~ ., data = mtcars, family = "gaussian",
#'   performanceMeasure = "AIC")
#' print(ES)
#'
#' ## Same setup, but compared by MSE on a test set partition
#' testIDs <- sample(nrow(mtcars), round(1/3 * nrow(mtcars)))
#' ES2 <- ExhaustiveSearch(mpg ~ ., data = mtcars, family = "gaussian",
#'   performanceMeasure = "MSE", testSetIDs = testIDs)
#' print(ES2)
#'
#'
#' ## Logistic Regression on Ionosphere Data
#' data("Ionosphere", package = "mlbench")
#'
#' ## Only combinations of up to 3 features! -> 5488 models instead of 4 billion
#' ES3 <- ExhaustiveSearch((Class == "good") ~ ., data = Ionosphere[,-c(1,2)],
#'   family = "binomial", combsUpTo = 3)
#' print(ES3)
#'
#' @author Rudolf Jagdhuber
#'
#' @seealso [resultTable()], [getFeatures()]
#'
#' @importFrom Rcpp evalCpp
#' @import stats
#' @export
ExhaustiveSearch = function(formula, data, family = NULL,
  performanceMeasure = NULL, combsUpTo = NULL, nResults = 5000, nThreads = NULL,
  testSetIDs = NULL, errorVal = -1, quietly = FALSE, checkLarge = TRUE) {

  formula = formula(formula)
  if (!inherits(formula, "formula")) stop("\nInvalid formula.")

  data = as.data.frame(data)
  if (!inherits(data, "data.frame")) stop("\nInvalid data object.")

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
    if (length(unique(y)) != 2) stop(paste0("\n",
      "'family' was set to 'binomial', but the response is not binary.\n\n"))
  }

  ## Check the performanceMeasure parameter and the validity of each case
  if (is.null(performanceMeasure)) {
    if (nrow(XTest) == 0) performanceMeasure = "AIC"
    else performanceMeasure = "MSE"
  } else if (performanceMeasure == "AIC") {
    if (nrow(XTest) != 0) stop(paste0("\n",
      "A TestSet was defined, but 'performanceMeasure' is set to 'AIC'.\n\n"))
  } else if (performanceMeasure == "MSE") {
    if (nrow(XTest) == 0) warning(paste0("\n\n",
      "No TestSet was defined and performanceMeasure set to 'MSE'.\n",
      "Comparing MSE values on training data will always prefer the higher\n",
      "dimensional model in nested setups and is thus not recommended for\n",
      "feature selection tasks.\n\n"))
  } else stop(paste0("\n",
    "Unsupported performanceMeasure! Please check the help file for a list\n",
    "of all available options.\n\n"))

  ## Check combUpTo parameter
  if (is.null(combsUpTo)) combsUpTo = length(feats)
  if (!is.numeric(combsUpTo) | length(combsUpTo) != 1 | any(combsUpTo <= 0))
    stop("\ncombsUpTo needs to be a single numeric value > 0\n\n")
  if (combsUpTo == Inf) combsUpTo = length(feats)


  ## Safety-check if the user requests a huge task
  nCombs = sum(choose(length(feats), seq(combsUpTo)))
  if (nCombs > 1e8 & checkLarge) stop(paste0(
    "\nThe requested task needs to evaluate a huge number of combinations:\n\n",
    "  -> ", format(nCombs, big.mark = ",", scientific = FALSE)," models.\n\n",
    "Consider reducing the total number of combinations with 'combsUpTo'.\n\n",
    "To continue with this setup, set the parameter 'checkLarge = FALSE'.\n\n"))

  ## Check nResults parameter
  if (is.null(nResults)) nResults = nCombs
  if (!is.numeric(nResults) | length(nResults) != 1 | any(nResults <= 0))
    stop("\nnResults needs to be a single numeric value > 0")
  if (nResults > nCombs) nResults = nCombs

  # Check nThreads parameter, if not set detect later in C++
  if (is.null(nThreads)) nThreads = 0
  else if (!is.numeric(nThreads) | length(nThreads) != 1 | nThreads %% 1 != 0)
    stop("\nnThreads needs to be a single integer value\n\n")

  ## Check errorVal parameter
  if (is.null(errorVal)) stop("\nerrorVal parameter cannot be NULL\n\n")
  if (!is.numeric(errorVal) | length(errorVal) != 1)
    stop("\nerrorVal needs to be a single numeric value\n\n")

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
  result$batchInfo = list(nBatches = cppOutput[[6]],
    batchSizes = cppOutput[[7]], batchLimits = cppOutput[[8]])
  result$setup = list(call = match.call(), family = family,
    performanceMeasure = performanceMeasure, intercept = intercept,
    combsUpTo = combsUpTo, nResults = nResults, nThreads = cppOutput[[5]],
    testSetIDs = testSetIDs, nTrain = nrow(X), nTest = nrow(XTest))

  if (!quietly) {
    if (cppOutput[[4]] == nCombs) cat("\nEvaluation finished successfully.\n\n")
    else warning("\n\nEvaluation Incomplete! Not all models were evaluated!\n")
  }

  return(result)
}
