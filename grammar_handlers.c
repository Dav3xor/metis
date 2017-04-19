#include <inttypes.h>
#include "compiler.h"
extern handler *handlers;
extern handler *termhandlers;
extern handler *lexphandlers;
extern handler *bshandlers;
extern handler *blockhandlers;
extern handler *stmthandlers;

Operator get_operator(char *operator) {
  Operator operator_type = NONE;
  if(CMP(operator,"*")) operator_type = MULTIPLY;
  else if (CMP(operator,"/")) operator_type = DIVIDE;
  else if (CMP(operator,"%")) operator_type = MODULUS;
  else if (CMP(operator,"dot")) operator_type = DOT;
  else if (CMP(operator,"cross")) operator_type = CROSS;
  return operator_type;
}


void handle_start(parser_state *state, mpc_ast_trav_t *contents) {
  mpc_ast_t *ast_next;
  PARSER_NEXT(contents, ast_next);

  handler   *cur;
  while (ast_next) {
    DISPATCH(handlers, ast_next->tag, cur);
    PARSER_NEXT(contents, ast_next);
  }
}


void handle_comment(parser_state *state, mpc_ast_trav_t *contents) {
  (void)state;
  (void)contents;
  // pass
}


void handle_bs(parser_state *state, mpc_ast_trav_t *contents) {
  mpc_ast_t *ast_next;
  handler   *cur;
  PARSER_NEXT(contents, ast_next);
  DISPATCH(bshandlers, ast_next->tag, cur);
}


void handle_block(parser_state *state, mpc_ast_trav_t *contents) {
  handler   *cur;
  // consume the def/if/while/for/type/etc...  string.
  mpc_ast_t *ast_next;
  PARSER_NEXT(contents, ast_next);

  if(ast_next) {
    DISPATCH(blockhandlers, ast_next->tag, cur);
  }  
  // pass
}


void handle_stmt(parser_state *state, mpc_ast_trav_t *contents) {
  handler   *cur;
  // consume the def/if/while/for/type/etc...  string.
  mpc_ast_t *ast_next;
  PARSER_NEXT(contents, ast_next);

  if(ast_next) {
    DISPATCH(stmthandlers, ast_next->tag, cur);
  }  
  // consume the '.' at the end of the stmt.
  printf("--\n");
  PARSER_NEXT(contents, ast_next);

  printf("--\n");
}


void handle_return(parser_state *state, mpc_ast_trav_t *contents) {
  // consume <-
  mpc_ast_t *ast_next;
  PARSER_NEXT(contents, ast_next);

  handle_lexp(state, contents);
  // pass
}


void handle_lexp(parser_state *state, mpc_ast_trav_t *contents) {
  handler   *cur;
  mpc_ast_t *ast_next;
  PARSER_NEXT(contents, ast_next);
  HASH_FIND_STR(lexphandlers, ast_next->tag, cur);
  if (cur) {
    cur->handler(state, contents);
  }
}


void do_label(parser_state *state, char *destination, char *label) {
  uint64_t stack_offset = find_label(state, label);
  printf("LOADSR %" PRIu64 ", %s\n", stack_offset, destination);
}


void handle_factor(parser_state *state, mpc_ast_trav_t *contents) {
  mpc_ast_t *ast_next;
  PARSER_NEXT(contents, ast_next);
  if(CMP(ast_next->tag, "factor|label|regex")) {
    do_label(state, "REGA", ast_next->contents);
  } else if (CMP(ast_next->tag, "factor|float|regex")) {
    printf ("STOREI REGA %s\n", ast_next->contents);
  }
  // pass
}


void handle_term(parser_state *state, mpc_ast_trav_t *contents) {
  handle_factor(state, contents);

  mpc_ast_t *ast_next;
  PARSER_NEXT(contents, ast_next);
  // if we get an operator, do another factor
  if(CMP(ast_next->tag, "operator|string")) {
    Operator operator;
    printf("STORE REGA, REGC\n");
    operator = get_operator(ast_next->contents);

    handle_factor(state, contents);
    switch (operator) {
      case NONE:
        printf("No operator?!?\n");
        break;
      case MULTIPLY:
        printf("MUL REGA, REGC\n");
        break;
      case DIVIDE:
        printf("DIV REGA, REGC\n");
        break;
      case MODULUS:
        printf("MOD REGA, REGC\n");
        break;
      case DOT:
        printf("DOT REGA, REGC\n");
        break;
      case CROSS:
        printf("CROSS REGA, REGC\n");
        break;
    } 
  }
}


void handle_label(parser_state *state, mpc_ast_trav_t *contents) {
  uint64_t location;
  mpc_ast_t *ast_next;
  PARSER_NEXT(contents, ast_next);
  location = find_label(state, ast_next->contents);
  printf ("%" PRIu64 "\n",location);
  // pass
}


void handle_float(parser_state *state, mpc_ast_trav_t *contents) {
  (void)state;
  (void)contents;
  // pass
}


void handle_type(parser_state *state, mpc_ast_trav_t *contents) {
  (void)state;
  (void)contents;
  // pass
}


void handle_if(parser_state *state, mpc_ast_trav_t *contents) {
  (void)state;
  (void)contents;
  // pass
}


void handle_include(parser_state *state, mpc_ast_trav_t *contents) {
  (void)state;
  (void)contents;
  // pass
}


void handle_while(parser_state *state, mpc_ast_trav_t *contents) {
  (void)state;
  (void)contents;
  // pass
}


void handle_for(parser_state *state, mpc_ast_trav_t *contents) {
  (void)state;
  (void)contents;
  // pass
}


void handle_def(parser_state *state, mpc_ast_trav_t *contents) {
  (void)state;
  (void)contents;
  // pass
}


void handle_function(parser_state *state, mpc_ast_trav_t *contents) {
  uint64_t  num_arguments = 0;
  bool      run           = true;

  push_label_context(state);
  // consume the function name.
  printf("y\n");
  mpc_ast_t *ast_next;
  PARSER_NEXT(contents, ast_next);
  printf ("LABEL %s\n",ast_next->contents);

  // now get the args
  PARSER_NEXT(contents, ast_next);
  if(CMP(ast_next->tag, "args|>")) {
    while(run) {
      printf("x\n");
      PARSER_NEXT(contents, ast_next);
      if(CMP(ast_next->tag, "typeident|>")) {
        PARSER_NEXT(contents, ast_next);
        PARSER_NEXT(contents, ast_next);
        char *var  = ast_next->contents;
        add_label(state, var, num_arguments);
        num_arguments += 1;
      }
      PARSER_NEXT(contents, ast_next);
      if (!CMP(ast_next->contents, ",")) {
        run = false;
      }
    }
  } else if (CMP(ast_next->tag, "args|typeident|>")) {
    PARSER_NEXT(contents, ast_next);
    PARSER_NEXT(contents, ast_next);
    char *var  = ast_next->contents;
    add_label(state, var, num_arguments);
    PARSER_NEXT(contents, ast_next);
    num_arguments += 1;
  }
  if(CMP(ast_next->contents, "<-")) {
    PARSER_NEXT(contents, ast_next);
  }
  // next token must be the :, so consume it.
  PARSER_NEXT(contents, ast_next);

  while(!CMP(ast_next->tag, "endblock|string")) {
    printf("1\n");
    handle_bs(state, contents);
    printf("3\n");
    PARSER_NEXT(contents, ast_next);
    printf("2\n");
  }
  pop_label_context(state);
  printf("STACK_ADJ %" PRIu64 "\n", num_arguments);
}


void handle_string(parser_state *state, mpc_ast_trav_t *contents) {
  (void)state;
  (void)contents;
}


void handle_integer(parser_state *state, mpc_ast_trav_t *contents) {
  (void)state;
  (void)contents;
}


void handle_fcall(parser_state *state, mpc_ast_trav_t *contents) {
  (void)state;
  (void)contents;
}

