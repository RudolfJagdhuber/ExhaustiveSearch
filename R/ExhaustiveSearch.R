



ExhaustiveSearch = function(formula, data, family = NULL, combsUpTo = NULL,
  nResults = NULL, nThreads = 4, allowHugeCalls = FALSE,
  allowHugeStorage = FALSE) {

  formula = formula(formula)
  if (!inherits(formula, "formula")) stop("Error: Invalid formula.")

  ## Extract the design matrix with dims, response vector and feature names
  X = stats::model.matrix(formula, data)
  XRows = nrow(X)
  XCols = ncol(X)
  feats = colnames(X)[!colnames(X) == "(Intercept)"]
  y = model.response(model.frame(formula, data))

  ## Format X and y for the C++ function input
  XInput = c(t(X))
  yInput = as.numeric(y)

  ## Check if the family parameter was set correctly
  if (is.null(family)) {
    family = ifelse(length(unique(yInput)) == 2, "binomial", "gaussian")
    warning(paste0("Warning: family not specified! From the given response ",
      "data, I assume '", family, "' and continue."))
  } else if (family == "gaussian") {
    if (length(unique(yInput)) == 2) {
      warning(paste0("Warning: You set family to 'gaussian', but the response ",
        "appears to be binary, are you sure this is intended?"))
    }
  } else if (family == "gaussian") {
    if (length(unique(yInput)) != 2) {
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

  ## Safety-check if the user requests something huge
  nCombs = sum(choose(length(feats), seq(combsUpTo)))
  if (nCombs > 100000 * nThreads & !allowHugeCalls) {
    stop(paste0("You are requesting a very large task of ", nCombs,
      "models!\n\nPlease consider defining a suitable combsUpTo value.\n\n",
      "If you want to continue anyway, set 'allowHugeCalls = TRUE' in the ",
      "function call."))
  }

  ## Check nResults parameter and safety-check if the user requests huge memory
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


  ## The main C++ function call
  cppOutput = ExhaustiveSearchCpp(
    XInput = XInput,
    yInput = yInput,
    XRows =  XRows,
    XCols =  XCols,
    combsUpTo = combsUpTo,
    nResults = nResults,
    nThreads = nThreads)

  if (length(cppOutput) == 0) stop("User interrupt or internal error.")

  ## Preparation of the return value
  result = list()
  class(result) = "ExhaustiveSearch"

  result$nModels = nCombs
  result$nModels2 = cppOutput[[1]]
  result$runtimeSec = cppOutput[[2]]
  result$ranking = list(aic = cppOutput[[3]], featureIDs = cppOutput[[4]])
  result$featureNames = feats
  result$batchSizes = cppOutput[[5]]
  result$setup = list(family = family, combsUpTo = combsUpTo,
    nResults = nResults, nThreads = nThreads)

  return(result)
}
