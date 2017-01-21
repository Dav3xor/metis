#include "mpc.h"
#include "uthash.h"
#include "grammar.h"

#define PARSER(name, symbol) mpc_parser_t *name      = mpc_new(symbol);

typedef struct parser_state_t {
  char *last_string;
  char *last_label;
  char *last_integer;
}parser_state;

typedef void (*grammar_handler)(parser_state *, char *);

typedef struct handler_t {
  char            handle[128];
  grammar_handler handler;
  UT_hash_handle  hh;
} handler;
  
void handle_comment(parser_state *state, char *contents) {
  // pass
  printf("%s\n", contents);
}

void handle_string(parser_state *state, char *contents) {
  state->last_string = contents;
  printf("%s\n", contents);
}

void handle_integer(parser_state *state, char * contents) {
  state->last_integer = contents;
  printf("%s\n", contents);
}

handler handler_defs[] = { {"bs|comment|longcomment|regex",  &handle_comment},
                           {"bs|comment|shortcomment|regex", &handle_comment},
                           {"string",                        &handle_string}
                         };
unsigned int num_handlers = sizeof(handler_defs)/sizeof(handler);


int main(int argc, char **argv) {
  mpc_result_t r;
  mpc_ast_t *ast;
  mpc_ast_t *child;
  mpc_ast_t *tree;
  mpc_ast_t *ast_next;
  mpc_ast_trav_t *traveller;

  handler *handlers = NULL;
  parser_state state;

  int index;


  PARSER(Label,        "label");
  PARSER(String,       "string");
  PARSER(Unsigned,     "unsigned");
  PARSER(Integer,      "integer");
  PARSER(Float,        "float");
  PARSER(Vector,       "vector");
  PARSER(Matrix,       "matrix");
  PARSER(Fcall,        "fcall");
  PARSER(Term,         "term");
  PARSER(Lexp,         "lexp");
  PARSER(Type,         "type");
  PARSER(Typeident,    "typeident");
  PARSER(Comparator,   "comparator");
  PARSER(Exp,          "exp");
  PARSER(Assignment,   "assignment");
  PARSER(Shortcomment, "shortcomment");
  PARSER(Longcomment,  "longcomment");
  PARSER(Comment,      "comment");
  PARSER(Args,         "args");
  PARSER(Bs,           "bs");
  PARSER(Function,     "function");
  PARSER(Trait,        "trait");
  PARSER(Traitident,   "traitident");
  PARSER(Block,        "block");
  PARSER(Stmt,         "stmt");
  PARSER(Factor,       "factor");
  PARSER(Metis,        "metis");


  for(int i=0; i<num_handlers; i++) {
    handler *cur = &(handler_defs[i]);
    HASH_ADD_STR(handlers, handle, cur);
  }

  mpca_lang(MPCA_LANG_DEFAULT, (char *)grammar_txt,
            String,  Label, Unsigned, Integer, 
            Float, Vector, Matrix, Fcall,
            Term, Lexp, Type, 
            Typeident, Comparator, 
            Exp, Assignment, Shortcomment, Longcomment, Comment, Args, 
            Bs, Trait, Traitident, Function, 
            Block, Stmt, Factor, Metis, NULL);

  mpc_print(Label);
  mpc_print(String);
  mpc_print(Unsigned);
  mpc_print(Integer);
  mpc_print(Float);
  mpc_print(Vector);
  mpc_print(Matrix);
  mpc_print(Fcall);
  mpc_print(Term);
  mpc_print(Lexp);
  mpc_print(Type);
  mpc_print(Typeident);
  mpc_print(Comparator);
  mpc_print(Exp);
  mpc_print(Assignment);
  mpc_print(Shortcomment);
  mpc_print(Longcomment);
  mpc_print(Comment);
  mpc_print(Args);
  mpc_print(Bs);
  mpc_print(Block);
  mpc_print(Stmt);
  mpc_print(Function);
  mpc_print(Trait);
  mpc_print(Traitident);
  mpc_print(Factor);
  mpc_print(Metis);
  
  if(mpc_parse_contents("test.m", Metis, &r)) {
    ast = r.output;
    mpc_ast_print(r.output);
    //mpc_ast_delete(r.output);
  } else {
    mpc_err_print(r.error);
    mpc_err_delete(r.error);
    mpc_cleanup(18, Label, String, Unsigned, Integer, Float, Vector,
                    Matrix, Fcall, Term, Lexp, Type, Typeident, Comparator,
                    Exp, Assignment, Comment, Shortcomment, Args, Bs, Block, Stmt, Function, Trait, 
                    Traitident, Factor, Metis);
    return EXIT_FAILURE;
  }



  traveller = mpc_ast_traverse_start(ast, mpc_ast_trav_order_pre);
  ast_next  = mpc_ast_traverse_next(&traveller);
  while (ast_next) {
    handler *cur;
    HASH_FIND_STR(handlers, ast_next->tag, cur);
    if (cur) {
      cur->handler(&state, ast_next->contents);
    }
    printf("Tag: %s -- %d -- %s\n", ast_next->tag, ast_next->state, ast_next->contents);

    ast_next = mpc_ast_traverse_next(&traveller);
  }

}

