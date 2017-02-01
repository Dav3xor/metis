#include "mpc.h"
#include "uthash.h"
#include "grammar.h"
#include "stdbool.h"

#define PARSER(name, symbol) mpc_parser_t *name      = mpc_new(symbol);

typedef struct parser_state_t {
  char *last_string;
  char *last_label;
  char *last_integer;
  char *last_float;
}parser_state;

typedef void (*grammar_handler)(parser_state *, mpc_ast_trav_t *traveller);

typedef struct handler_t {
  char            handle[128];
  grammar_handler handler;
  UT_hash_handle  hh;
} handler;

handler *handlers = NULL;

void handle_start(parser_state *state, mpc_ast_trav_t *contents) {
  mpc_ast_t *ast_next = mpc_ast_traverse_next(&contents);
  handler   *cur;
  while (ast_next) {
    HASH_FIND_STR(handlers, ast_next->tag, cur);
    if (cur) {
      cur->handler(&state, contents);
    }
    ast_next = mpc_ast_traverse_next(&contents);
  }
  //ast_next = mpc_ast_traverse_next(&contents);
  //ast_next = mpc_ast_traverse_next(&contents);
  //printf("%s\n", ast_next->contents);
  //printf("Tag: %s -- %d -- %s\n", ast_next->tag, ast_next->state, ast_next->contents);
}
void handle_comment(parser_state *state, mpc_ast_trav_t *contents) {
  // pass
}

void handle_block(parser_state *state, mpc_ast_trav_t *contents) {
  handler   *cur;
  // consume the def/if/while/for/type/etc...  string.
  mpc_ast_t *ast_next = mpc_ast_traverse_next(&contents);
  printf ("%s\n", ast_next->contents);
  // now get the actual function/if/while block...
  ast_next = mpc_ast_traverse_next(&contents);

  if(ast_next) {
    HASH_FIND_STR(handlers, ast_next->tag, cur);
    if (cur) {
      cur->handler(&state, contents);
    }
  }  
  // pass
}

void handle_function(parser_state *state, mpc_ast_trav_t *contents) {
  uint64_t  num_arguments = 0;
  bool      run           = true;

  // consume the function name.
  mpc_ast_t *ast_next = mpc_ast_traverse_next(&contents);
  printf ("LABEL %s\n",ast_next->contents);

  // now get the args
  ast_next = mpc_ast_traverse_next(&contents);
  if(!(strcmp(ast_next->tag, "args|>"))) {
    while(run) {
      ast_next = mpc_ast_traverse_next(&contents);
      if(!(strcmp(ast_next->tag, "typeident|>"))) {
        ast_next = mpc_ast_traverse_next(&contents);
        char *type = ast_next->contents;
        ast_next = mpc_ast_traverse_next(&contents);
        char *var  = ast_next->contents;
        printf("ARG: %s %s\n",type, var);
        num_arguments += 1;
      }
      ast_next = mpc_ast_traverse_next(&contents);
      if (strcmp(ast_next->content, ",")) {
        run = false;
      }
    }
  }
}
void handle_label(parser_state *state, mpc_ast_trav_t *contents) {
  //state->last_label = contents;
  //printf("%s\n", contents);
}

void handle_string(parser_state *state, mpc_ast_trav_t *contents) {
  //state->last_string = contents;
  //printf("%s\n", contents);
}

void handle_integer(parser_state *state, mpc_ast_trav_t *contents) {
  //state->last_integer = contents;
  //printf("%s\n", contents);
}

void handle_float(parser_state *state, mpc_ast_trav_t *contents) {
  //state->last_float = contents;
  //printf("%s\n", contents);
}
void handle_fcall(parser_state *state, mpc_ast_trav_t *contents) {
  //printf("fcall start\n");
}

handler handler_defs[] = { {"bs|comment|longcomment|regex",   &handle_comment},
                           {"bs|comment|shortcomment|regex",  &handle_comment},
                           {"bs|block|>",                     &handle_block},
                           {"function|>",                  &handle_function},
                           {"label|regex",                    &handle_label},
                           {"lexp|term|factor|integer|regex", &handle_integer},
                           {"lexp|term|factor|float|regex",   &handle_float},
                           {"fcall|>",                        &handle_fcall},
                           {"string",                         &handle_string}
                         };
unsigned int num_handlers = sizeof(handler_defs)/sizeof(handler);


int main(int argc, char **argv) {
  mpc_result_t r;
  mpc_ast_t *ast;
  mpc_ast_trav_t *traveller;

  parser_state state;

  PARSER(Label,        "label");
  PARSER(String,       "string");
  PARSER(File,         "file");
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
            String,  Label, File, Unsigned, Integer, 
            Float, Vector, Matrix, Fcall,
            Term, Lexp, Type, 
            Typeident, Comparator, 
            Exp, Assignment, Shortcomment, Longcomment, Comment, Args, 
            Bs, Trait, Traitident, Function, 
            Block, Stmt, Factor, Metis, NULL);

  mpc_print(Label);
  mpc_print(String);
  mpc_print(File);
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
    mpc_cleanup(18, Label, String, File, Unsigned, Integer, Float, Vector,
                    Matrix, Fcall, Term, Lexp, Type, Typeident, Comparator,
                    Exp, Assignment, Comment, Shortcomment, Args, Bs, Block, Stmt, Function, Trait, 
                    Traitident, Factor, Metis);
    return EXIT_FAILURE;
  }



  traveller = mpc_ast_traverse_start(ast, mpc_ast_trav_order_pre);
  handle_start(&state, traveller);

}

