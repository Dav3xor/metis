#include "metis.hpp"

void MetisASM::assemble(const string &filename) {

};

address_mode MetisASM::get_addr_mode(void) {
  return addr_modes.at(strtok(0," "));
}
MetisASM::MetisASM() : 
  handlers({
    {"ERROR",               [](MetisVM &m, char *s) -> void {  m.add_error(); } },
    {"END",                 [](MetisVM &m, char *s) -> void {  m.add_end(); } }, 
    {"NOOP",                [](MetisVM &m, char *s) -> void {  m.add_noop(); } }, 
    {"JUMP",                [this](MetisVM &m, char *s) -> void {  m.add_jump(this->get_addr_mode()); } } 
  }),
  addr_modes({
    {"REGA",        REGA},
    {"REGB",        REGB},
    {"REGC",        REGC},
    {"REGD",        REGD},
    {"REGSP",       REGSP},
    {"REGIP",       REGIP},
    {"REGBP",       REGBP},
    {"REGERR",      REGERR},
    {"STACK_PUSH",  STACK_PUSH},
    {"STACK_POP",   STACK_POP}
  }) {
};
