* Hello!
*
* This is a simple metis assembler file to 
* test the functionality of the Metis Assembler

* save a label of the start of our code...
LOC start

* store some values in the A and B registers
STOREI REGA 1
STOREI REGB 2

* a matrix...
MATRIX stuff 3 3
  1.1 2.1 3.1
  1.2 2.2 3.2
  1.3 2.3 3.3

* an identity matrix...
IMATRIX identity 4 4

* a couple of no-ops
NOOP
NOOP

* the end!
END
