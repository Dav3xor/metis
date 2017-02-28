#include "mpc.h"
#include "uthash.h"
#include "stdbool.h"

#define PARSER(name, symbol) mpc_parser_t *name      = mpc_new(symbol);

typedef struct parser_state_t {
  char *last_string;
  char *last_label;
  char *last_integer;
  char *last_float;
  
}parser_state;

typedef void (*grammar_handler)(parser_state *, mpc_ast_trav_t *traveller);

typedef struct label_t {
  char label[256];
  UT_hash_handle hh;
} label;

typedef struct handler_t {
  char            handle[128];
  grammar_handler handler;
  UT_hash_handle  hh;
} handler;


void handle_start(parser_state *state, mpc_ast_trav_t *contents);
void handle_comment(parser_state *state, mpc_ast_trav_t *contents);
void handle_bs(parser_state *state, mpc_ast_trav_t *contents);
void handle_block(parser_state *state, mpc_ast_trav_t *contents);
void handle_stmt(parser_state *state, mpc_ast_trav_t *contents);
void handle_return(parser_state *state, mpc_ast_trav_t *contents);
void handle_type(parser_state *state, mpc_ast_trav_t *contents);
void handle_if(parser_state *state, mpc_ast_trav_t *contents);
void handle_include(parser_state *state, mpc_ast_trav_t *contents);
void handle_while(parser_state *state, mpc_ast_trav_t *contents);
void handle_for(parser_state *state, mpc_ast_trav_t *contents);
void handle_def(parser_state *state, mpc_ast_trav_t *contents);
void handle_function(parser_state *state, mpc_ast_trav_t *contents);
void handle_label(parser_state *state, mpc_ast_trav_t *contents);
void handle_string(parser_state *state, mpc_ast_trav_t *contents);
void handle_integer(parser_state *state, mpc_ast_trav_t *contents);
void handle_float(parser_state *state, mpc_ast_trav_t *contents);
void handle_fcall(parser_state *state, mpc_ast_trav_t *contents);
void handle_lexp(parser_state *state, mpc_ast_trav_t *contents);





