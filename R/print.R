

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
  cat("\n+---------------------------------+")
  cat("\n|    Exhaustive Search Results    |")
  cat("\n+---------------------------------+\n")
  cat("Model family:      ", x$setup$family, "\n")
  cat("Models evaluated:  ", format(x$nModels, big.mark = ","), "\n")
  cat("Total runtime:     ", formatSecTime(x$runtimeSec), "\n")
  cat("\n+---------------------------------+")
  cat("\n|        Top Feature Sets         |")
  cat("\n+---------------------------------+\n")
  cat(paste(capture.output(resultTable(x, 5, "  ")), collapse = "\n"), "\n\n")
}

