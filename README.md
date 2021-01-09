
# ExhaustiveSearch <img src="man/figures/logo.svg" align="right" height = 87/>

<!-- badges: start -->
[![](https://www.r-pkg.org/badges/version/ExhaustiveSearch?color=blue)](https://cran.r-project.org/package=ExhaustiveSearch)
[![Travis build status](https://travis-ci.com/RudolfJagdhuber/ExhaustiveSearch.svg?branch=master)](https://travis-ci.com/RudolfJagdhuber/ExhaustiveSearch)
[![R-CMD-check](https://github.com/RudolfJagdhuber/ExhaustiveSearch/workflows/R-CMD-check/badge.svg)](https://github.com/RudolfJagdhuber/ExhaustiveSearch/actions)
<!-- badges: end -->

The aim of this R package is to provide an easy to use, fast and and scalable 
exhaustive search framework. An exhaustive feature selection can require
a very large number of models to be fitted and evaluated. Execution speed and 
memory management are thus crucial factors for such tasks. This package solves 
both issues by using a multi-threaded C++ backend. Memory usage is kept constant
by only storing the best results. This allows evaluations of enormous tasks 
that would normally be seen as unfeasible in standard setups.

## Installation

This package is not yet available on CRAN. However, I intend to publish it as 
soon as possible.

Until then you can use the current development version from GitHub:
``` r
devtools::install_github("RudolfJagdhuber/ExhaustiveSearch")
```

## Usage

The main function `ExhaustiveSearch()` uses the typical `formula` and `data`
structure, which might be familiar to you from functions like `lm()` or `glm()`. 

As a simple example, an exhaustive linear regression task of the mtcars data set
is analyzed. We evaluate all possible feature combinations to predict the 
response variable 'mpg' (miles per gallon).
``` r
library(ExhaustiveSearch)

data(mtcars)
ES <- ExhaustiveSearch(mpg ~ ., data = mtcars, family = "gaussian")
```
```
Starting the exhaustive evaluation.

 Runtime          |  Completed  |  Status
------------------------------------------
 00d 00h 00m 00s  |  1023/1023  |  100%
------------------------------------------

Evaluation finished successfully.
```
As you can see, the evaluation of 1023 models only took a fraction of a second. 
Printing the result object to the console creates an overview with further 
details to the task.
``` r
print(ES)
```
```
+-------------------------------------------------+
|            Exhaustive Search Results            |
+-------------------------------------------------+
Model family:          gaussian 
Intercept:             TRUE 
Performance measure:   AIC 
Models fitted on:      training set (n = 32)
Models evaluated on:   training set (n = 32)
Models evaluated:      1,023  
Models saved:          1,023 
Total runtime:         00d 00h 00m 00s 
Number of threads:     16 

+-------------------------------------------------+
|                Top Feature Sets                 |
+-------------------------------------------------+
       AIC                  Combination
1 154.1194               wt + qsec + am
2 154.3274          hp + wt + qsec + am
3 154.5631        wt + qsec + am + carb
4 154.9740   disp + hp + wt + qsec + am
5 155.4766                cyl + hp + wt 
```
There are a lot more options to the exhaustive search function, which are 
documented in the help files (see `?ExhaustiveSearch()`). These include:

* logistic regression models for classification
* other performance measures
* limiting the size of combinations (e.g. only up to 5 features)
* defining separate training and testing partitions
* ...


## Performance
The example above is an easy task for this framework and could of course also
be managed by less sophisticated frameworks without greater problems. For larger
tasks however, performance may be an issue. In this section, I will give a crude
estimate of what run-times to expect for different tasks.

The main influencing factors to the run-time are the modeling task, the number
of available threads for parallel execution, the data size and the CPU speed. 

A local benchmark on an AMD Ryzen 7 1700X on a data set of 500 observations
resulted in the following performances:
```
Linear Regression:    45000 models/threadsec
Logistic Regression:   2500 models/threadsec
```
Therefore, on a typical home-PC setup (16 threads), one could expect to be able
to evaluate 2,592,000,000 linear regression models in one hour. For logistic
regression, this number would be 120,000,000 models per hour.

## More about this Package and Exhaustive Searches
In feature and model selection application, exhaustive searches are often 
referred to as *optimal* search strategies, as they test each setup and 
therefore ensure to find the best solution. The main downside of this approach 
is the possibly enormous computational complexity of the task. This package 
provides an easy to use and efficient framework for such problems.

Its main characteristics are:

* Combinations are iteratively generated on the fly,
* Model fitting and evalution is performed multi-threaded in C++,
* Only a fixed amount of models are stored to keep memory usage small.

Currently, ordinary linear regression models similar to `lm()` and logistic 
regression models similar to `glm()` (with parameter family = "binomial") can 
be fitted. All model results of the C++ backend are identical to what would be 
obtained by `glm()` or `lm()`. For that, the logistic regression uses the same 
L-BFGS optimizer as `glm()`.

To assess the quality of a model, the performanceMeasure options 'AIC' (Akaike's
An Information Criterion) and 'MSE' (Mean Squared Error) are implemented.

While this framework is able to handle very large amounts of combinations, an 
exhaustive search of every theoretical combination can still be unfeasible. 
However, a possible way to drastically limit the total number of combinations 
is to define an upper bound for the size of a combination. For example, 
evaluating all combinations of 500 features ($3.3\cdot 10^{150}$) is obviously 
impossible. But if we only consider combinations of up to 3 features, this 
number reduces to around 21 million, which could easily be evaluated by this 
framework in less than a minute on 16 threads. Setting an upper limit is thus a 
very powerful option to enable high dimensional analyses. It is implemented by 
the 'combsUpTo' parameter.

## Further Development

The official version includes a consistent and ready-to-use framework. 
Nevertheless, a lot more can be done. Further development is managed in this 
GitHub repository. To submit further ideas, please open an issue.
