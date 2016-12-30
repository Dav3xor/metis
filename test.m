
def square float x <- float:
  <- x * x
end

def is_positive float x <- bool:
  if x > 0
    <- true
  else
    <- false
  end
end

def is_zero float x <- bool:
  if x > 0
    <- false
  else if x < 0
    <- false
  else
    <- true
  end
end

def divide float x, float y <- float:
  if -is_zero y = true
    <-| divide_by_zero_error
  end

  float result := x/y.
  <- result
end


float x := -square 5, 2.

if x > 1
  print "x > 1.5!".
end


