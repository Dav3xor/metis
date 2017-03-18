#include <inttypes.h>
#include "compiler.h"


void push_label_context(parser_state *state)
{
  if (state->cur_context > NUM_CONTEXTS-1) {
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

void add_label(parser_state *state, char *label_name, uint64_t value)
{
  printf("add label: %s %" PRId64 "\n",label_name,value);
  label *cur_context = state->label_contexts[state->cur_context];
  label *new_label;
  new_label        = malloc(sizeof(label));
  strncpy(new_label->label, label_name, MAX_LABEL_SIZE);
  new_label->value = value;
   
  HASH_ADD_STR(cur_context,label,new_label);
}

uint64_t find_label(parser_state *state, char *label_name)
{
  label *cur;
  for(int64_t i = state->cur_context; i>=0; i--) {
    printf("%" PRId64 "\n",i);
    HASH_FIND_STR(state->label_contexts[i], label_name, cur);
    if(cur) {
      return cur->value;
    }
  }
  printf("could not find label: %s\n",label_name);
  exit(1);
}
  
