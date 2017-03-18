#include <inttypes.h>
#include "compiler.h"
extern handler *handlers;
extern handler *termhandlers;
extern handler *lexphandlers;
extern handler *bshandlers;
extern handler *blockhandlers;
extern handler *stmthandlers;

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
  //ast_next = mpc_ast_traverse_next(&contents);
  //ast_next = mpc_ast_traverse_next(&contents);
  //printf("%s\n", ast_next->contents);
  //printf("Tag: %s -- %d -- %s\n", ast_next->tag, ast_next->state, ast_next->contents);
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
    printf("zz %s - %s\n", ast_next->tag, ast_next->contents);
    HASH_FIND_STR(bshandlers, ast_next->tag, cur);
    if (cur) {
      printf("zzz\n");
      cur->handler(state, contents);
    } else {
      printf("zzz?\n");
    }
    ast_next = mpc_ast_traverse_next(&contents);
  }
}

void handle_block(parser_state *state, mpc_ast_trav_t *contents) {
  handler   *cur;
  // consume the def/if/while/for/type/etc...  string.
  mpc_ast_t *ast_next = mpc_ast_traverse_next(&contents);
  printf ("block type: %s\n", ast_next->contents);

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
    printf ("stmt type: %s\n", ast_next->tag);
    HASH_FIND_STR(stmthandlers, ast_next->tag, cur);
    if (cur) {
      cur->handler(state, contents);
    }
  }  
  printf("(return contents of REGA)\n");
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
  printf ("lexp type: %s - %s\n", ast_next->tag, ast_next->contents);
  HASH_FIND_STR(lexphandlers, ast_next->tag, cur);
  if (cur) {
    cur->handler(state, contents);
  }
}

void handle_factor(parser_state *state, mpc_ast_trav_t *contents) {
  (void)state;
  (void)contents;
  // pass
}

void do_label(parser_state *state, char *destination, char *label) {
  uint64_t stack_offset = find_label(state, label);
  printf("LOADSR %" PRIu64 ", %s\n", stack_offset, destination);
}

void handle_term(parser_state *state, mpc_ast_trav_t *contents) {
  handler   *cur;
  mpc_ast_t *ast_next = mpc_ast_traverse_next(&contents);
  printf ("term: %s - %s\n", ast_next->tag, ast_next->contents);
  if(CMP(ast_next->tag, "factor|label|regex")) {
    printf("got factor label\n");
    do_label(state, "REGA", ast_next->contents);
  } else {
    HASH_FIND_STR(termhandlers, ast_next->tag, cur);
    if (cur) {
      cur->handler(state, contents);
    }
  }
  // pass
}

void handle_label(parser_state *state, mpc_ast_trav_t *contents) {
  handler   *cur;
  uint64_t location;
  mpc_ast_t *ast_next = mpc_ast_traverse_next(&contents);
  printf ("label: %s - %s\n", ast_next->tag, ast_next->contents);
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
  char *return_type       = NULL;
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
        char *type = ast_next->contents;
        ast_next = mpc_ast_traverse_next(&contents);
        char *var  = ast_next->contents;
        printf("ARG: %s %s\n",type, var);
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
    char *type = ast_next->contents;
    ast_next = mpc_ast_traverse_next(&contents);
    char *var  = ast_next->contents;
    printf("ARG: %s %s\n",type, var);
    add_label(state, var, num_arguments);
    ast_next = mpc_ast_traverse_next(&contents);
    num_arguments += 1;
  }
  //printf("??? %s %s\n",ast_next->tag,ast_next->contents);
  if(CMP(ast_next->contents, "<-")) {
    ast_next = mpc_ast_traverse_next(&contents);
    return_type = ast_next->contents;
    printf("RETURN: %s\n", return_type);
  }
  // next token must be the :, so consume it.
  ast_next = mpc_ast_traverse_next(&contents);
  while(!CMP(ast_next->contents, "fin")) {
    handle_bs(state, contents);
    ast_next = mpc_ast_traverse_next(&contents);
    printf("y: %s\n", ast_next->contents);
  }
  pop_label_context(state);
}

void handle_string(parser_state *state, mpc_ast_trav_t *contents) {
  (void)state;
  (void)contents;
  //state->last_string = contents;
  //printf("%s\n", contents);
}

void handle_integer(parser_state *state, mpc_ast_trav_t *contents) {
  (void)state;
  (void)contents;
  //state->last_integer = contents;
  //printf("%s\n", contents);
}

void handle_fcall(parser_state *state, mpc_ast_trav_t *contents) {
  (void)state;
  (void)contents;
  //printf("fcall start\n");
}

