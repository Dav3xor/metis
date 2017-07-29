# this is a comment

# longer
# comment

include 
 - filea
 - fileb
fin

def square float x <- float:
  print "hello".
  matrix m := | 1  2  3  4  |
              | 5  6  7  8  |
              | 9  10 11 12 |
              | 13 14 15 16 |.
  <- x * x.
fin

trait size <- float.

type Ship
  float xpos.
  float ypos.
  float xcomp.
  float ycomp.
  float weight.
  :size
    <- 5.
  fin
  
  def speed <- float:
    <- { sqrt (xcomp*xcomp + ycomp*ycomp) }.
  fin
fin

def complex float x, float y <- float:
  if x = 0
    print "1".
    <-! "x can't be zero!".
  else
    print "2".
  fin

  foreach i in x
    print i.
  fin

  while x > 5
    print "3".
    print true.
    print false.
    x := x - 1.
  fin
  <- x*(x+y)*{ square 5 }+5.05+7.777.
fin

