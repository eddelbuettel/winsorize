
library(microbenchmark)
library(robustHD)
library(winsorize)


set.seed(42)

x <- rnorm(1000)
ind <- sample(1:1000,10)
x[ind] <- x[ind] * 10

summary(x)
summary(robustHD::winsorize(x))
summary(winsorize::winsorize(x))

res <- microbenchmark(robustHD::winsorize(x), winsorize::winsorize(x), times=10000)
print(res)
