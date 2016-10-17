#include "metis.hpp"

#define HANDLED_BY [this](MetisVM &m, ifstream &s) -> void
void MetisASM::assemble(const string &filename, MetisVM &vm) {
  infile.open(filename);
  string opcode;
  while(!(infile.eof())) {
    infile >> opcode;
    handlers.at(opcode)(vm, infile);
  }
};

address_mode MetisASM::get_addr_mode(void) {
  return addr_modes.at(strtok(0," "));
}

uint64_t MetisASM::get_uint64(void) {
  return strtoull(strtok(0," "),NULL,0);
}

char *MetisASM::get_string(void) {
  return strtok(0," ");
}

MetisASM::MetisASM() : 
  handlers({
    {"ERROR",               HANDLED_BY {  m.add_error      (); } },
    {"END",                 HANDLED_BY {  m.add_end        (); } }, 
    {"NOOP",                HANDLED_BY {  m.add_noop       (); } }, 
    {"JUMP",                HANDLED_BY {  m.add_jump       (this->get_addr_mode()); } },
    {"JUMPI",               HANDLED_BY {  m.add_jumpi      (this->get_uint64());   } },
    {"JIZZ",                HANDLED_BY {  m.add_jizz       (this->get_addr_mode(),
                                                            this->get_addr_mode()); } }, 
    {"JNZ",                 HANDLED_BY {  m.add_jnz        (this->get_addr_mode(),
                                                           this->get_addr_mode()); } },
    {"JNE",                 HANDLED_BY {  m.add_jne        (this->get_addr_mode(),
                                                            this->get_addr_mode(),
                                                            this->get_uint64()); } }, 
    {"JMPE",                HANDLED_BY {  m.add_jmpe       (this->get_addr_mode(),
                                                            this->get_addr_mode(),
                                                            this->get_uint64()); } },
    {"STORE",               HANDLED_BY {  m.add_store      (this->get_addr_mode(),
                                                            this->get_addr_mode()); } },
    {"STOREI",              HANDLED_BY {  m.add_storei     (this->get_addr_mode(),
                                                            this->get_uint64()); } },

    {"LOC",                 HANDLED_BY {  m.add_label_ip   (this->get_string()); } },
    {"LABEL",               HANDLED_BY {  m.add_label_val  (this->get_string(),
                                                            this->get_uint64()); } },
  
    
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
