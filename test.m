
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
