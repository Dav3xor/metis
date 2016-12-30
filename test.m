
def square float x <- float:
  <- x * x.
.

def is_positive float x <- bool:
  if x > 0
    <- true.
  else
    <- false.
  .
.

def is_zero float x <- bool:
  if x > 0
    <- false.
  else if x < 0
    <- false.
  else
    <- true.
  .
.

def divide float x, float y <- float:
  if -is_zero y = true
    <-| divide_by_zero_error.
  .

  float result := x/y.
  <- result.
.


float x := -square 5, 2.

if x > 1
  print "x > 1.5!".
.


