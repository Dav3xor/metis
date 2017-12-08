* Hello!
*
* This is a simple metis assembler file to 
* test the functionality of the Metis Assembler

* save a label of the start of our code...
LOC start
LOC init
* store some values in the A and B registers
STOREI REGA 1
STOREI REGB 2

* push some registers on the stack
PUSHR ( REGA REGB )

* and pop them back
POPR ( REGA REGB )


* a matrix...
MATRIX stuff 3 3
  1.1 2.1 3.1
  1.2 2.2 3.2
  1.3 2.3 3.3

* an identity matrix...
IMATRIX identity 4 4

MATRIX matrixa 3 3
  1.1 2.1 3.1
  1.2 2.2 3.2
  1.3 2.3 3.3

MATRIX matrixb 3 3
  2.1 2.1 3.1
  2.2 2.2 3.2
  2.3 2.3 3.3

MATRIX matrixc 3 3
  0.0 0.0 0.0   
  0.0 0.0 0.0   
  0.0 0.0 0.0   

STOREI REGA matrixa
STOREI REGB matrixb
STOREI REGC matrixc

MMUL REGA REGB matrixc
MADD REGC REGA matrixb
VDOT REGB REGA matrixc
VCROSS REGC REGA matrixb
* a couple of no-ops
NOOP
NOOP

* the end!
LOC mainloop
END
