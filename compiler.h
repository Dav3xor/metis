#include "mpc.h"
#include "uthash.h"
#include "stdbool.h"

#define NUM_CONTEXTS 256

#define MAX_LABEL_SIZE 128
#define MAX_HANDLE_SIZE 128

#define PARSER(name, symbol) mpc_parser_t *name      = mpc_new(symbol);

#define FILL_HASH {NULL, NULL, NULL, NULL, NULL, NULL, 0, 0}

#define CMP(a,b) (!(strcmp(a, b)))

#define OPERATOR_MULTIPLY 1
#define OPERATOR_DIVIDE   2
#define OPERATOR_MODULUS  3
#define OPERATOR_DOT      4
#define OPERATOR_CROSS    5

enum operator {MULTIPLY, DIVIDE, MODULUS, DOT, CROSS};

typedef struct label_t {
  char label[MAX_LABEL_SIZE];
  uint64_t value;
  UT_hash_handle hh;
} label;

typedef struct parser_state_t {
  char *last_string;
  char *last_label;
  char *last_integer;
  char *last_float;

  label *label_contexts[NUM_CONTEXTS];
  uint64_t cur_context;
  
}parser_state;

typedef void (*grammar_handler)(parser_state *, mpc_ast_trav_t *traveller);

typedef struct handler_t {
  char            handle[MAX_HANDLE_SIZE];
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
void handle_factor(parser_state *state, mpc_ast_trav_t *contents);
void handle_term(parser_state *state, mpc_ast_trav_t *contents);
void handle_label(parser_state *state, mpc_ast_trav_t *contents);
void handle_float(parser_state *state, mpc_ast_trav_t *contents);



void     push_label_context(parser_state *state);
void     pop_label_context(parser_state *state);
void     add_label(parser_state *state, char *label_name, uint64_t value);
uint64_t find_label(parser_state *state, char *label_name);



