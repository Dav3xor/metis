#include "compiler.h"


void push_label_context(parser_state *state)
{
  if (state->cur_context < NUM_CONTEXTS-1) {
    printf("out of label contexts\n");
    exit(1);
  }
  state->cur_context += 1;
}

void pop_label_context(parser_state *state)
{
  label *cur, *temp;
  if(state->cur_context == 0) {
    printf("attempt to pop context 0\n");
    exit(1);
  }
  HASH_ITER(hh, state->label_contexts[state->cur_context], cur, temp) {
    HASH_DEL(state->label_contexts[state->cur_context], cur);
    free(cur);
  }
  state->cur_context -= 1;
} 
