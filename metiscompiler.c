#include "mpc.h"

#define PARSER(name, symbol) mpc_parser_t *name      = mpc_new("symbol");
int main(int argc, char **argv) {
  int fd;
  fd
  mpc_result_t r;

  PARSER(Unsigned,unsigned);
  PARSER(Integer, integer);
  PARSER(Float,   float);
  PARSER(Label,   label);
  PARSER(Vector,  vector);
  PARSER(Matrix,  matrix);

