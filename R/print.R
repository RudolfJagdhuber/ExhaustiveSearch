

formatSecTime = function(sec) {
  days = sec / 60 / 60 / 24
  hour = (sec / 60 / 60) %% 24
  min = (sec / 60) %% 60
  osec = sec %% 60
  paste(paste0(sprintf("%02d", floor(c(days, hour, min, osec))),
               c("d ", "h ", "m ", "s")), collapse = "")
}


#' @export
print.ExhaustiveSearch = function(x) {

  evalOn = ifelse(x$setup$nTest == 0,
    paste0("training set (n = ", format(x$setup$nTrain, big.mark = ","), ")\n"),
    paste0("test set (n = ", format(x$setup$nTest, big.mark = ","), ")\n"))

  cat("\n+-------------------------------------------------+")
  cat("\n|            Exhaustive Search Results            |")
  cat("\n+-------------------------------------------------+\n")
  cat("Model family:         ", x$setup$family, "\n")
  cat("Intercept:            ", x$setup$intercept, "\n")
  cat("Performance measure:  ", x$setup$performanceMeasure, "\n")
  cat("Models fitted on:     ", " training set (n = ", x$setup$nTrain, ")\n",
    sep = "")
  cat("Models evaluated on:  ", evalOn)
  cat("Models evaluated:     ", format(x$nModels, big.mark = ","),
    ifelse(x$evaluatedModels != x$nModels, " (Incomplete!)", ""), "\n")
  cat("Models saved:         ", format(x$setup$nResults, big.mark = ","), "\n")
  cat("Total runtime:        ", formatSecTime(x$runtimeSec), "\n")
  cat("Threads used:         ", x$setup$nBatches, "of", x$setup$nThreads, "\n")
  cat("\n+-------------------------------------------------+")
  cat("\n|                Top Feature Sets                 |")
  cat("\n+-------------------------------------------------+\n")
  cat(paste(capture.output(resultTable(x, 5, "  ")), collapse = "\n"), "\n\n")
}
