#include "mpc.h"
#include "grammar.h"

#define PARSER(name, symbol) mpc_parser_t *name      = mpc_new(symbol);

int main(int argc, char **argv) {
  mpc_result_t r;

  PARSER(String,     "string");
  PARSER(Label,      "label");
  PARSER(Unsigned,   "unsigned");
  PARSER(Integer,    "integer");
  PARSER(Float,      "float");
  PARSER(Val,        "val");
  PARSER(Metis,      "metis");

  printf((char *)grammar_txt);

  mpca_lang(MPCA_LANG_DEFAULT, (char *)grammar_txt,
            String,  Label, Unsigned, Integer, Float, Val, Metis, NULL);

  mpc_print(String);
  mpc_print(Label);
  mpc_print(Unsigned);
  mpc_print(Integer);
  mpc_print(Float);
  mpc_print(Val);
  mpc_print(Metis);
  
  if(mpc_parse_contents("test.m", Metis, &r)) {
    mpc_ast_print(r.output);
    mpc_ast_delete(r.output);
  } else {
    mpc_err_print(r.error);
    mpc_err_delete(r.error);
  }
}

