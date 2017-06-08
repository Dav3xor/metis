# this is a comment

# longer
# comment

include 
 - filea
 - fileb
fin

def square float x <- float:
  print "hello".
  <- x * x.
fin


def complex float x, float y <- float:
  if x = 0
    print "1".
  else
    print "2".
  fin
  while x > 5
    print "3".
    x := x - 1.
  fin
  <- x*(x+y)*{ square 5 }+5.05+7.777.
fin

