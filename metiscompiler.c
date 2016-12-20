#include "mpc.h"
#include "grammar.h"

#define PARSER(name, symbol) mpc_parser_t *name      = mpc_new(symbol);

int main(int argc, char **argv) {
  mpc_result_t r;

  PARSER(Label,      "label");
  PARSER(String,     "string");
  PARSER(Unsigned,   "unsigned");
  PARSER(Integer,    "integer");
  PARSER(Float,      "float");
  PARSER(Vector,     "vector");
  PARSER(Matrix,     "matrix");
  PARSER(Term,       "term");
  PARSER(Lexp,       "lexp");
  PARSER(Typeident,  "typeident");
  PARSER(Args,       "args");
  PARSER(Factor,     "factor");
  PARSER(Metis,      "metis");

  printf((char *)grammar_txt);

  mpca_lang(MPCA_LANG_DEFAULT, (char *)grammar_txt,
            String,  Label, Unsigned, Integer, Float, Vector, Matrix, Term, Lexp, Typeident, Args, Factor, Metis, NULL);

  mpc_print(Label);
  mpc_print(String);
  mpc_print(Unsigned);
  mpc_print(Integer);
  mpc_print(Float);
  mpc_print(Vector);
  mpc_print(Matrix);
  mpc_print(Term);
  mpc_print(Lexp);
  mpc_print(Typeident);
  mpc_print(Args);
  mpc_print(Factor);
  mpc_print(Metis);
  
  if(mpc_parse_contents("test.m", Metis, &r)) {
    mpc_ast_print(r.output);
    mpc_ast_delete(r.output);
  } else {
    mpc_err_print(r.error);
    mpc_err_delete(r.error);
  }
}

