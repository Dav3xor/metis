#include "metis.hpp"

void MetisASM::assemble(const string &filename) {

};

address_mode MetisASM::get_addr_mode(void) {
  return addr_modes.at(strtok(0," "));
}

uint64_t MetisASM::get_uint64(void) {
  return strtoull(strtok(0," "),NULL,0);
}

MetisASM::MetisASM() : 
  handlers({
    {"ERROR",               [this](MetisVM &m, char *s) -> void {  m.add_error      (); } },
    {"END",                 [this](MetisVM &m, char *s) -> void {  m.add_end        (); } }, 
    {"NOOP",                [this](MetisVM &m, char *s) -> void {  m.add_noop       (); } }, 
    {"JUMP",                [this](MetisVM &m, char *s) -> void {  m.add_jump       (this->get_addr_mode()); } },
    {"JUMPI",               [this](MetisVM &m, char *s) -> void {  m.add_jumpi      (this->get_uint64());   } },
    {"JIZ",                 [this](MetisVM &m, char *s) -> void {  m.add_jizz       (this->get_addr_mode(),
                                                                                     this->get_addr_mode()); } }, 
    {"JNZ",                 [this](MetisVM &m, char *s) -> void {  m.add_jnz        (this->get_addr_mode(),
                                                                                     this->get_addr_mode()); } },
    {"JNE",                 [this](MetisVM &m, char *s) -> void {  m.add_jne        (this->get_addr_mode(),
                                                                                     this->get_addr_mode(),
                                                                                     this->get_uint64()); } }, 
    {"JMPE",                [this](MetisVM &m, char *s) -> void {  m.add_jmpe       (this->get_addr_mode(),
                                                                                     this->get_addr_mode(),
                                                                                     this->get_uint64()); } },
    {"STO",                 [this](MetisVM &m, char *s) -> void {  m.add_store      (this->get_addr_mode(),
                                                                                     this->get_addr_mode()); } },
    {"STI",                 [this](MetisVM &m, char *s) -> void {  m.add_storei     (this->get_addr_mode(),
                                                                                     this->get_uint64()); } } 
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
