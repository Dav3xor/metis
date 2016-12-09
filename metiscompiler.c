#include "../mpc/mpc.h"
#include "grammar.h"

#define PARSER(name, symbol) mpc_parser_t *name      = mpc_new("symbol");

int main(int argc, char **argv) {
  mpc_result_t r;

  PARSER(Unsigned,unsigned);
  PARSER(Float,   float);
  PARSER(Label,   label);
  PARSER(Vector,  vector);
  PARSER(Matrix,  matrix);

  mpca_lang(MPCA_LANG_PREDICTIVE, grammar_txt,
            Unsigned, Float, Label, Vector, Matrix, NULL);

}
