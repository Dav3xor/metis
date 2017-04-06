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
  mpc_ast_t *ast_next = mpc_ast_traverse_next(&contents);

  handler   *cur;
  while (ast_next) {
    HASH_FIND_STR(handlers, ast_next->tag, cur);
    if (cur) {
      cur->handler(state, contents);
    }
    ast_next = mpc_ast_traverse_next(&contents);
  }
}


void handle_comment(parser_state *state, mpc_ast_trav_t *contents) {
  (void)state;
  (void)contents;
  // pass
}


void handle_bs(parser_state *state, mpc_ast_trav_t *contents) {
  mpc_ast_t *ast_next = mpc_ast_traverse_next(&contents);
  handler   *cur;
  while (ast_next) {
    HASH_FIND_STR(bshandlers, ast_next->tag, cur);
    if (cur) {
      cur->handler(state, contents);
    } 
  }
}


void handle_block(parser_state *state, mpc_ast_trav_t *contents) {
  handler   *cur;
  // consume the def/if/while/for/type/etc...  string.
  mpc_ast_t *ast_next = mpc_ast_traverse_next(&contents);

  if(ast_next) {
    HASH_FIND_STR(blockhandlers, ast_next->tag, cur);
    if (cur) {
      cur->handler(state, contents);
    }
  }  
  // pass
}


void handle_stmt(parser_state *state, mpc_ast_trav_t *contents) {
  handler   *cur;
  // consume the def/if/while/for/type/etc...  string.
  mpc_ast_t *ast_next = mpc_ast_traverse_next(&contents);

  if(ast_next) {
    HASH_FIND_STR(stmthandlers, ast_next->tag, cur);
    if (cur) {
      cur->handler(state, contents);
    }
  }  
}


void handle_return(parser_state *state, mpc_ast_trav_t *contents) {
  // consume <-
  mpc_ast_traverse_next(&contents);

  handle_lexp(state, contents);
  // pass
}


void handle_lexp(parser_state *state, mpc_ast_trav_t *contents) {
  handler   *cur;
  mpc_ast_t *ast_next = mpc_ast_traverse_next(&contents);
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
  mpc_ast_t *ast_next = mpc_ast_traverse_next(&contents);
  if(CMP(ast_next->tag, "factor|label|regex")) {
    do_label(state, "REGA", ast_next->contents);
  } else if (CMP(ast_next->tag, "factor|float|regex")) {
    printf ("STOREI REGA %s\n", ast_next->contents);
  }
  // pass
}


void handle_term(parser_state *state, mpc_ast_trav_t *contents) {
  handle_factor(state, contents);

  mpc_ast_t *ast_next = mpc_ast_traverse_next(&contents);
  // if we get an operator, do another factor
  if(CMP(ast_next->tag, "operator|string")) {
    uint64_t operator;
    printf("STORE REGA, REGC\n");
    operator = get_operator(ast_next->contents);

    handle_factor(state, contents);
    switch (operator) {
      case OPERATOR_MULTIPLY:
        printf("MUL REGA, REGC\n");
        break;
      case OPERATOR_DIVIDE:
        printf("DIV REGA, REGC\n");
        break;
      case OPERATOR_MODULUS:
        printf("MOD REGA, REGC\n");
        break;
      case OPERATOR_DOT:
        printf("DOT REGA, REGC\n");
        break;
      case OPERATOR_CROSS:
        printf("CROSS REGA, REGC\n");
        break;
    } 
  }
}


void handle_label(parser_state *state, mpc_ast_trav_t *contents) {
  handler   *cur;
  uint64_t location;
  mpc_ast_t *ast_next = mpc_ast_traverse_next(&contents);
  location = find_label(state, ast_next->contents);
  printf ("%" PRIu64 "\n",location);
  HASH_FIND_STR(lexphandlers, ast_next->tag, cur);
  if (cur) {
    cur->handler(state, contents);
  }
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
  mpc_ast_t *ast_next = mpc_ast_traverse_next(&contents);
  printf ("LABEL %s\n",ast_next->contents);

  // now get the args
  ast_next = mpc_ast_traverse_next(&contents);
  if(CMP(ast_next->tag, "args|>")) {
    while(run) {
      ast_next = mpc_ast_traverse_next(&contents);
      if(CMP(ast_next->tag, "typeident|>")) {
        ast_next = mpc_ast_traverse_next(&contents);
        ast_next = mpc_ast_traverse_next(&contents);
        char *var  = ast_next->contents;
        add_label(state, var, num_arguments);
        num_arguments += 1;
      }
      ast_next = mpc_ast_traverse_next(&contents);
      if (!CMP(ast_next->contents, ",")) {
        run = false;
      }
    }
  } else if (CMP(ast_next->tag, "args|typeident|>")) {
    ast_next = mpc_ast_traverse_next(&contents);
    ast_next = mpc_ast_traverse_next(&contents);
    char *var  = ast_next->contents;
    add_label(state, var, num_arguments);
    ast_next = mpc_ast_traverse_next(&contents);
    num_arguments += 1;
  }
  if(CMP(ast_next->contents, "<-")) {
    ast_next = mpc_ast_traverse_next(&contents);
  }
  // next token must be the :, so consume it.
  ast_next = mpc_ast_traverse_next(&contents);
  while(!CMP(ast_next->contents, "fin")) {
    handle_bs(state, contents);
    ast_next = mpc_ast_traverse_next(&contents);
  }
  pop_label_context(state);
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

