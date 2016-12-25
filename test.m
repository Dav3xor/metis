
def square float x <- float:
  <- x * x
end

def is_positive float x <- bool:
  if x > 0
    <- true
  end
  <- false
end

def is_zero float x <- bool:
  if x
    <- false
  end
  <- true
end
