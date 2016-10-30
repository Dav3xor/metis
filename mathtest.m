* Hello!
*
* This is a simple metis assembler file to 
* test the functionality of the Metis Assembler

* save a label of the start of our code...
LOC start

* store some values in the A and B registers
STOREI REGA 1
STOREI REGB 2

ADD REGA REGB  * B = 3
INC REGB REGB  * B = 4
DEC REGB REGB  * B = 3
SUB REGB REGA  * A = 2
MUL REGA REGB  * B = 6
DIV REGB REGA  * A = 3
INC REGB REGB  * B = 7
MOD REGB REGA  * A = 1
AND REGA REGB  * B = 1
OR  REGB REGA  * A = 1
XOR REGA REGB  * B = 0

END


