* Hello!
*
* This is a simple metis assembler file to 
* test the functionality of the Metis Assembler

* save a label of the start of our code...
LOC start

* store some values in the A and B registers
STOREI REGA 1
STOREI REGB 2

* a couple of no-ops
NOOP
NOOP

* the end!
END
