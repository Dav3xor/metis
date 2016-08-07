#include "metis.hpp"

ostringstream MetisException::cnvt;

void MetisVM::add_end(void) {
  MetisInstruction *instruction            = (MetisInstruction *)registers[REGIP];
  instruction->type                        = INS_END;      
  registers[REGIP] += ADVANCE(0, 0);
};

void MetisVM::add_jump(address_mode src) {
  MetisInstruction *instruction            = (MetisInstruction *)registers[REGIP];
  instruction->type                        = INS_JUMP;      
  instruction->commands.extended.addr_mode = BUILD_ADDR(src, 0);
  registers[REGIP] += ADVANCE(1, 0);
};

void MetisVM::add_jumpi(uint64_t location) {
  MetisInstruction *instruction                 = (MetisInstruction *)registers[REGIP];
  instruction->type                             = INS_JUMPI;      
  instruction->commands.jumpi.value = location;
  registers[REGIP] += ADVANCE(0, sizeof(ext_jumpi_t));
};

void MetisVM::add_jizz(address_mode src, address_mode dest) {
  MetisInstruction *instruction            = (MetisInstruction *)registers[REGIP];
  instruction->type                        = INS_JIZZ;      
  instruction->commands.extended.addr_mode = BUILD_ADDR(src, dest);
  registers[REGIP] += ADVANCE(1, 0);
}; 

void MetisVM::add_jnz(address_mode src, address_mode dest) {
  MetisInstruction *instruction            = (MetisInstruction *)registers[REGIP];
  instruction->type                        = INS_JNZ;      
  instruction->commands.extended.addr_mode = BUILD_ADDR(src, dest);
  registers[REGIP] += ADVANCE(1, 0);
}; 

void MetisVM::add_jne(address_mode src, address_mode dest, uint64_t location) {
  MetisInstruction *instruction            = (MetisInstruction *)registers[REGIP];
  instruction->type                        = INS_JNE;      
  instruction->commands.extended.addr_mode = BUILD_ADDR(src, dest);
  instruction->commands.extended.ext.jne.value = location;
  registers[REGIP] += ADVANCE(1, sizeof(ext_jne_t));
}

void MetisVM::add_jmpe(address_mode src, address_mode dest, uint64_t location) {
  MetisInstruction *instruction              = (MetisInstruction *)registers[REGIP];
  instruction->type                          = INS_JMPE;      
  instruction->commands.extended.addr_mode   = BUILD_ADDR(src, dest);
  instruction->commands.extended.ext.jmpe.value  = location;
  registers[REGIP] += ADVANCE(1, sizeof(ext_jmpe_t));
}

void MetisVM::add_store(address_mode src, address_mode dest) {
  MetisInstruction *instruction            = (MetisInstruction *)registers[REGIP];
  instruction->type                        = INS_STORE;      
  instruction->commands.extended.addr_mode = BUILD_ADDR(src, dest);
  registers[REGIP] += ADVANCE(1, 0);
}; 
void MetisVM::add_storei(address_mode dest, uint64_t value) {
  MetisInstruction *instruction                 = (MetisInstruction *)registers[REGIP];
  instruction->type                             = INS_STOREI;      
  instruction->commands.extended.addr_mode = BUILD_ADDR(0, dest);
  instruction->commands.extended.ext.storei.value = value;
  registers[REGIP] += ADVANCE(1, sizeof(ext_storei_t));
};

uint64_t MetisVM::add_label(const char *label) {
  // not really an instruction, but it basically acts like one...
  uint64_t new_loc = (registers[REGIP]-(uint64_t)start);
  labels[label] = new_loc;
  return new_loc;
}
void MetisVM::add_data(const uint8_t *data, const uint64_t length, const char *label) {
  MetisInstruction *instruction     = (MetisInstruction *)registers[REGIP];
  instruction->type                 = INS_DATA;      
  instruction->commands.data.length = length; 
  if (registers[REGIP] + length > (uint64_t)end) {
    throw MetisException("data blob doesn't fit (add_data)");
  }
  registers[REGIP] += ADVANCE(0, sizeof(data_t));
  add_label(label);

  memcpy((void *)registers[REGIP],data,length);
  registers[REGIP] += length;
}

void MetisVM::add_not(address_mode src, address_mode dest) {
  MetisInstruction *instruction            = (MetisInstruction *)registers[REGIP];
  instruction->type                        = INS_NOT;      
  instruction->commands.extended.addr_mode = BUILD_ADDR(src, dest);
  registers[REGIP] += ADVANCE(1, 0);
};

void MetisVM::add_gldrawelements(GLenum mode, GLsizei count, 
                                 GLenum type, GLvoid *indices) {
  MetisInstruction *instruction            = (MetisInstruction *)registers[REGIP];
  instruction->type                        = INS_GLDRAWELEMENTS;      
  instruction->commands.gldrawelements.mode = mode;
  instruction->commands.gldrawelements.count = count;
  instruction->commands.gldrawelements.type = type;
  instruction->commands.gldrawelements.indices = indices;
  registers[REGIP] += ADVANCE(0, sizeof(gldrawelements_t));
}; 

void MetisVM::add_gldrawarrays(GLenum mode, GLint first, GLsizei count) {
  MetisInstruction *instruction            = (MetisInstruction *)registers[REGIP];
  instruction->type                        = INS_GLDRAWARRAYS;      
  instruction->commands.gldrawarrays.mode = mode;
  instruction->commands.gldrawarrays.first = first;
  instruction->commands.gldrawarrays.count = count;
  registers[REGIP] += ADVANCE(0, sizeof(gldrawarrays_t));
}; 
