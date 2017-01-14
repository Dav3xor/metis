;; square: 
   a function for squaring a number

   arguments: x (the number to square ;;

def square float x <- float:
  <- x * x.
fin

def is_positive float x <- bool:
  if x > 0
    <- true.
  else
    <- false.
  fin
fin

def is_zero float x <- bool:
  if x > 0
    <- false.
  else if x < 0
    <- false.
  else
    <- true.
  fin
fin

def divide float x, float y <- float:
  if {is_zero y} = true
    <-| divide_by_zero_error.
  fin

  float result := x/y.
  <- result.
fin


float x := {square 5, 2}.

if x > 1
  print "x > 1.5!".
fin

matrix y := | 1.0 -2 -3.0 |
            | 4    5  6 |
            | 7    8  9 |.

float z := (x+y)*6 + y.
float z1 := 5.
float z2 := 5.0.
float z3 := -5.
float z1 := -5.0.
float z1 := 5.


string s := "quick brown fox.".
unsigned u := 5.
integer i := -5.

for integer x := 0 ; x < 5 ; 1
  print "x".
fin

:magnitude <- float.
:print.

type point
  float x.
  float y.

  :magnitude
    <- {sqrt x*x + y*y}.
  fin

  :print
    print "x={x} y={y}".
  fin
fin
