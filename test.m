# this is a comment

# longer
# comment

include - filea
        - fileb

def square float x <- float:
  print "hello".
  <- x * x.
fin


def complex float x, float y <- float:
  <- x*(x+y)*{ square 5 }+5.05+7.777.
fin

