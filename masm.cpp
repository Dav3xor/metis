#include "metis.hpp"

#define HANDLED_BY [this](MetisVM &m, ifstream &s) -> void
void MetisASM::assemble(const string &filename, MetisVM &vm) {
  infile.open(filename);
  string opcode;
  while(!(infile.eof())) {
    infile >> opcode;
    //printf("%s\n",opcode.c_str());
    handlers.at(opcode)(vm, infile);
  }
};

address_mode MetisASM::get_addr_mode(void) {
  string mode;
  infile >> mode;
  //printf(" - %s\n", mode.c_str());
  return addr_modes.at(mode);
}

uint64_t MetisASM::get_uint64(void) {
  uint64_t val;
  infile >> val;
  printf(" - %ju\n", val);
  return val;
}

string MetisASM::get_string(void) {
  string val; 
  infile >> val;
  //printf(" - %s\n", val.c_str());
  return val;
}

MetisASM::MetisASM() : 
  handlers({
    {"ERROR",               HANDLED_BY {  m.add_error      (); } },
    {"END",                 HANDLED_BY {  m.add_end        (); } }, 
    {"NOOP",                HANDLED_BY {  m.add_noop       (); } }, 
    {"JUMP",                HANDLED_BY {  m.add_jump       (this->get_addr_mode()); } },
    {"JUMPI",               HANDLED_BY {  m.add_jumpi      (this->get_uint64());   } },
    {"JIZZ",                HANDLED_BY {  address_mode src  = this->get_addr_mode();
                                          address_mode dest = this->get_addr_mode();
                                          m.add_jizz       (src,dest); } }, 
    {"JNZ",                 HANDLED_BY {  address_mode src  = this->get_addr_mode();
                                          address_mode dest = this->get_addr_mode();
                                          m.add_jnz        (src,dest); } },
    {"JNE",                 HANDLED_BY {  address_mode src  = this->get_addr_mode();
                                          address_mode dest = this->get_addr_mode();
                                          uint64_t val      = this->get_uint64();
                                          m.add_jne        (src, dest, val); } }, 
    {"JMPE",                HANDLED_BY {  address_mode src  = this->get_addr_mode();
                                          address_mode dest = this->get_addr_mode();
                                          uint64_t val      = this->get_uint64();
                                          m.add_jmpe       (src, dest, val); } },
    {"STORE",               HANDLED_BY {  address_mode src  = this->get_addr_mode();
                                          address_mode dest = this->get_addr_mode();
                                          m.add_store      (src, dest); } },
    {"STOREI",              HANDLED_BY {  address_mode src  = this->get_addr_mode();
                                          uint64_t     val  = this->get_uint64();
                                          m.add_storei     (src, val); } },

    {"LOC",                 HANDLED_BY {  m.add_label_ip   (this->get_string().c_str()); } },
    {"LABEL",               HANDLED_BY {  string label = this->get_string();
                                          uint64_t val = this->get_uint64();
                                          m.add_label_val  (label.c_str(), val); } },
  
    
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
