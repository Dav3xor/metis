#include "metis.hpp"

#define HANDLED_BY [this](MetisVM &m, ifstream &s) -> void


#define MATH_INSTRUCTION(instruction, add_method) \
    {instruction,           HANDLED_BY {  address_mode src = this->get_addr_mode(); \
                                          address_mode dest = this->get_addr_mode(); \
                                          m.add_method(src, dest); } }

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
  //printf(" - %ju\n", val);
  return val;
}

uint8_t MetisASM::get_uint8(void) {
  uint32_t val;
  infile >> val;
  if( val > 255) {
    throw MetisException("1 byte assembler value out of bounds", __LINE__, __FILE__);
  }
  //printf(" - %cu\n", val);
  return val;
}

float MetisASM::get_float(void) {
  float val;
  infile >> val;
  //printf(" - %f\n", val);
  return val;
}

string MetisASM::get_string(void) {
  string val; 
  infile >> val;
  //printf(" - %s\n", val.c_str());
  return val;
}
uint64_t MetisASM::get_addr(MetisVM &m) {
  string val;
  infile >> val;
  if((val[0] >= '0')&&(val[0] <= '9')) {
    return stoull(val, 0, 10);
  } else {
    return m.get_label(val.c_str());
  }
}
    
string MetisASM::get_comment(void) {
  string comment;
  getline(infile, comment);
  return comment;
}

MetisASM::MetisASM() : 
  handlers({
    {"*",                   HANDLED_BY {  get_comment(); } }, 
    {"ERROR",               HANDLED_BY {  m.add_error      (); } },
    {"END",                 HANDLED_BY {  m.add_end        (); } }, 
    {"NOOP",                HANDLED_BY {  m.add_noop       (); } }, 
    {"JUMP",                HANDLED_BY {  m.add_jump       (this->get_addr_mode()); } },
    {"JUMPI",               HANDLED_BY {  m.add_jumpi      (this->get_addr(m));   } },
    {"JIZZ",                HANDLED_BY {  address_mode src  = this->get_addr_mode();
                                          address_mode dest = this->get_addr_mode();
                                          m.add_jizz       (src,dest); } }, 
    {"JNZ",                 HANDLED_BY {  address_mode src  = this->get_addr_mode();
                                          address_mode dest = this->get_addr_mode();
                                          m.add_jnz        (src,dest); } },
    {"JNE",                 HANDLED_BY {  address_mode src  = this->get_addr_mode();
                                          address_mode dest = this->get_addr_mode();
                                          uint64_t val      = this->get_addr(m);
                                          m.add_jne        (src, dest, val); } }, 
    {"JMPE",                HANDLED_BY {  address_mode src  = this->get_addr_mode();
                                          address_mode dest = this->get_addr_mode();
                                          uint64_t val      = this->get_uint64();
                                          m.add_jmpe       (src, dest, val); } },
    {"STORE",               HANDLED_BY {  address_mode src  = this->get_addr_mode();
                                          address_mode dest = this->get_addr_mode();
                                          m.add_store      (src, dest); } },
    {"STOREI",              HANDLED_BY {  address_mode dest = this->get_addr_mode();
                                          uint64_t     val  = this->get_addr(m);
                                          m.add_storei     (dest, val); } },

    {"LOC",                 HANDLED_BY {  m.add_label_ip   (this->get_string().c_str()); } },
    {"LABEL",               HANDLED_BY {  string label = this->get_string();
                                          uint64_t val = this->get_uint64();
                                          m.add_label_val  (label.c_str(), val); } },
  
    {"MATRIX",              HANDLED_BY {  string  label   = this->get_string();
                                          // have to use uint32_t to get iostream
                                          // to store as integer, not ascii...
                                          uint32_t width  = this->get_uint8(); 
                                          uint32_t height = this->get_uint8(); 
                                          uint32_t size   = width*height;
                                          float *mat      = new float[size];
                                          for(uint32_t i=0; i<size; i++) {
                                            mat[i] = this->get_float();
                                          }
                                          m.add_matrix(width, height, (uint8_t *)mat, label.c_str());
                                          delete[] mat; } },
    {"IMATRIX",             HANDLED_BY {  string  label   = this->get_string();
                                          uint32_t width  = this->get_uint8(); 
                                          uint32_t height = this->get_uint8(); 
                                          m.add_identity_matrix(width,height,label.c_str()); } },
    {"MPUSH",               HANDLED_BY {  uint64_t loc    = this->get_addr(m);
                                          m.add_push_matrix(loc); } },
    {"MMUL",                HANDLED_BY {  address_mode src1 = this->get_addr_mode();
                                          address_mode src2 = this->get_addr_mode();
                                          uint64_t     dest = this->get_addr(m);
                                          m.add_matrix_multiply(src1, src2, dest); } },
    {"MADD",                HANDLED_BY {  address_mode src1 = this->get_addr_mode();
                                          address_mode src2 = this->get_addr_mode();
                                          uint64_t     dest = this->get_addr(m);
                                          m.add_matrix_add(src1, src2, dest); } },
    {"VDOT",                HANDLED_BY {  address_mode src1 = this->get_addr_mode();
                                          address_mode src2 = this->get_addr_mode();
                                          uint64_t     dest = this->get_addr(m);
                                          m.add_vector_dot(src1, src2, dest); } },
    {"VCROSS",              HANDLED_BY {  address_mode src1 = this->get_addr_mode();
                                          address_mode src2 = this->get_addr_mode();
                                          uint64_t     dest = this->get_addr(m);
                                          m.add_vector_cross(src1, src2, dest); } },
    {"BUFFER",              HANDLED_BY {  string  label   = this->get_string();
                                          uint32_t size   = this->get_uint64(); 
                                          float *buffer   = new float[size];
                                          for(uint32_t i=0; i<size; i++) {
                                            buffer[i] = this->get_float();
                                          }
                                          m.add_buffer((uint8_t *)buffer, size, label.c_str());
                                          delete[] buffer; } },
    MATH_INSTRUCTION("NOT", add_not),
    MATH_INSTRUCTION("INC", add_inc), 
    MATH_INSTRUCTION("DEC", add_dec),
    MATH_INSTRUCTION("ADD", add_add), 
    MATH_INSTRUCTION("SUB", add_sub), 
    MATH_INSTRUCTION("MUL", add_mul), 
    MATH_INSTRUCTION("DIV", add_div), 
    MATH_INSTRUCTION("MOD", add_mod), 
    MATH_INSTRUCTION("AND", add_and), 
    MATH_INSTRUCTION("OR",  add_or), 
    MATH_INSTRUCTION("XOR", add_xor),
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
