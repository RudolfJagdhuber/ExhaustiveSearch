

#' @export
resultTable = function(ESResult, n = Inf, insertStart = "") {
  n = min(n, length(ESResult$ranking$aic))
  data.frame("AIC" = ESResult$ranking$aic[seq_len(n)],
    "Combination" = sapply(ESResult$ranking$featureIDs[seq_len(n)],
      function(ids) paste0(insertStart,
        paste(ESResult$featureNames[ids], collapse = " + "))))
}
