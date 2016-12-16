#include "mpc.h"
#include "grammar.h"

#define PARSER(name, symbol) mpc_parser_t *name      = mpc_new("symbol");

int main(int argc, char **argv) {
  mpc_result_t r;

  PARSER(String,  string);
  PARSER(Unsigned,xunsigned);
  PARSER(Integer, integer);
  PARSER(Float,   float);
  PARSER(Label,   label);
  PARSER(Vector,  vector);
  PARSER(Matrix,  matrix);
  PARSER(Factor,  factor);
  PARSER(Term,  term);
  PARSER(Lexp,  lexp);
  PARSER(Typeident,  typeident);
  PARSER(Args,  args);
  PARSER(Function,  function);
  PARSER(Stmt,  stmt);
  PARSER(Metis,  metis);

  mpca_lang(MPCA_LANG_DEFAULT, grammar_txt,
            String, Unsigned, Integer, Float, Label, Vector, Matrix, 
            Factor, Term, Lexp, Typeident, Args, Function, Stmt, Metis, NULL);

  mpc_print(Unsigned);
  mpc_print(Float);
  mpc_print(Label);
  mpc_print(Vector);
  mpc_print(Matrix);
  if(mpc_parse_contents("test.m", Metis, &r)) {
    mpc_ast_print(r.output);
    mpc_ast_delete(r.output);
  } else {
    mpc_err_print(r.error);
    mpc_err_delete(r.error);
  }
}

