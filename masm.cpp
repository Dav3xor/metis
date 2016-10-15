#include "metis.hpp"

void MetisASM::assemble(const string &filename) {

};

MetisASM::MetisASM() : 
  handlers({
    {"ERROR",               [](MetisVM &m, char *s) -> void {  m.add_error(); } },
    {"END",                 [](MetisVM &m, char *s) -> void {  m.add_end(); } }, 
    {"NOOP",                [](MetisVM &m, char *s) -> void {  m.add_noop(); } }, 
    {"JUMP",                [](MetisVM &m, char *s) -> void {  m.add_noop(); } } 
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
