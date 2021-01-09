
#' Overview of the top exhaustive search results
#'
#' Extract the top `n` results of an exhaustive search and present them as a
#' `data.frame` object.
#'
#' @details
#' The result of an exhaustive search is given by an object of class
#' ExhaustiveSearch, which is a list of encoded feature combinations and
#' performance values. This function decodes the feature combinations and
#' presents them in a `data.frame` together with the respective performance
#' values
#'
#' @param ESResult a result object from an exhaustive search.
#' @param n number of results to be returned. The default (`Inf`) returns every
#'   result available in `ESResult`.
#' @param insertStart used for additional spacing when printing. The value of
#'   `insertStart` gets printed in front of every feature combination to
#'   increase the space to the printed performance measure.
#'
#' @return A `data.frame` with two columns. The first one shows the performance
#'   values and the second shows the decoded feature set collapsed with plus
#'   signs.
#'
#' @examples
#' \dontrun{
#' ## Exhaustive search on the mtcars data
#' data(mtcars)
#' ES <- ExhaustiveSearch(mpg ~ ., data = mtcars, family = "gaussian")
#'
#' ## Summary data.frame of the top 5 models
#' resultTable(ES, 5)
#'
#' ## Return a data.frame of all stored models
#' res <- resultTable(ES)
#' str(res)
#'
#' ## Add custom characters for printing
#' resultTable(ES, 1, "  <->  ")
#' }
#'
#' @author Rudolf Jagdhuber
#'
#' @seealso [ExhaustiveSearch()]
#'
#' @export
resultTable = function(ESResult, n = Inf, insertStart = "") {
  n = min(n, length(ESResult$ranking$performance))
  ret = data.frame(ESResult$ranking$performance[seq_len(n)],
    sapply(ESResult$ranking$featureIDs[seq_len(n)],
      function(ids) paste0(insertStart,
        paste(ESResult$featureNames[ids], collapse = " + "))))
  colnames(ret) = c(ESResult$setup$performanceMeasure, "Combination")
  return(ret)
}


#' Extract the feature sets from an ExhaustiveSearch object
#'
#' A simple function to get a vector of feature names for one or more elements
#' of an ExhaustiveSearch object.
#'
#' @param ESResult a result object from an exhaustive search.
#' @param ranks a numeric value or vector defining which elements should be
#'   returned.
#'
#' @return If `ranks` is a single value, a vector of feature names is returned.
#'   If an intercept is included, the first element of this vector is "1". If
#'   `ranks` includes multiple values, a list of such vectors is returned.
#'
#' @examples
#' \dontrun{
#' ## Exhaustive search on the mtcars data
#' data(mtcars)
#' ES <- ExhaustiveSearch(mpg ~ ., data = mtcars, family = "gaussian")
#'
#' ## Get the feature combinations of the top 3 models
#' getFeatures(ES, 1:3)
#'
#' ## Get the feature combination of the 531th best model
#' getFeatures(ES, 531)
#' }
#'
#' @author Rudolf Jagdhuber
#'
#' @seealso [ExhaustiveSearch()]
#'
#' @export
getFeatures = function(ESResult, ranks) {
  sapply(ranks, function(x) c(ifelse(ESResult$setup$intercept, "1", NULL),
    ESResult$featureNames[ESResult$ranking$featureIDs[[x]]]))
}
