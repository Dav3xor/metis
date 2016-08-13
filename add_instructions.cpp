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
  registers[REGIP] += ADVANCE(0, sizeof(instruction->commands.extended.ext.jumpi));
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
  registers[REGIP] += ADVANCE(1, sizeof(instruction->commands.extended.ext.jne));
}

void MetisVM::add_jmpe(address_mode src, address_mode dest, uint64_t location) {
  MetisInstruction *instruction              = (MetisInstruction *)registers[REGIP];
  instruction->type                          = INS_JMPE;      
  instruction->commands.extended.addr_mode   = BUILD_ADDR(src, dest);
  instruction->commands.extended.ext.jmpe.value  = location;
  registers[REGIP] += ADVANCE(1, sizeof(instruction->commands.extended.ext.jmpe));
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
  registers[REGIP] += ADVANCE(1, sizeof(instruction->commands.extended.ext.storei));
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
  registers[REGIP] += ADVANCE(0, sizeof(instruction->commands.data));
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
  registers[REGIP] += ADVANCE(0, sizeof(instruction->commands.gldrawelements));
}; 

void MetisVM::add_gldrawarrays(GLenum mode, GLint first, GLsizei count) {
  MetisInstruction *instruction            = (MetisInstruction *)registers[REGIP];
  instruction->type                        = INS_GLDRAWARRAYS;      
  instruction->commands.gldrawarrays.mode = mode;
  instruction->commands.gldrawarrays.first = first;
  instruction->commands.gldrawarrays.count = count;
  registers[REGIP] += ADVANCE(0, sizeof(instruction->commands.gldrawarrays));
};

void MetisVM::add_glgenbuffers(GLsizei n, GLuint start_index) {
  MetisInstruction *instruction                  = (MetisInstruction *)registers[REGIP];
  instruction->type                              = INS_GLGENBUFFERS;      
  instruction->commands.glgenbuffers.num_buffers = n;
  instruction->commands.glgenbuffers.start_index = start_index;
  registers[REGIP] += ADVANCE(0, sizeof(instruction->commands.glgenbuffers));
};

void MetisVM::add_glbindbuffer(GLenum target, GLuint buffer_index) {
  MetisInstruction *instruction             = (MetisInstruction *)registers[REGIP];
  instruction->type                         = INS_GLBINDBUFFER;      
  instruction->commands.glbindbuffer.target = target;
  instruction->commands.glbindbuffer.buffer_index = buffer_index;
  registers[REGIP] += ADVANCE(0, sizeof(instruction->commands.glbindbuffer));
};

void MetisVM::add_glbufferdata(GLenum target, GLsizeiptr size, GLvoid *data, GLenum usage) {
  MetisInstruction *instruction             = (MetisInstruction *)registers[REGIP];
  instruction->type                         = INS_GLBUFFERDATA;      
  instruction->commands.glbufferdata.target = target;
  instruction->commands.glbufferdata.size   = size;
  instruction->commands.glbufferdata.data   = data;
  instruction->commands.glbufferdata.usage  = usage;
  registers[REGIP] += ADVANCE(0, sizeof(instruction->commands.gldrawarrays));
};


void MetisVM::add_glenablevertexattribarray(GLuint index) {
  MetisInstruction *instruction             = (MetisInstruction *)registers[REGIP];
  instruction->type                         = INS_GLENABLEVERTEXATTRIBARRAY;      
  instruction->commands.glenablevertexattribarray.index = index;
  registers[REGIP] += ADVANCE(0, sizeof(instruction->commands.glenablevertexattribarray));
};

void MetisVM::add_glvertexattribpointer(GLuint index, GLint size, 
                             GLenum type, GLboolean normalized, 
                             GLsizei stride, GLvoid *pointer) {
  MetisInstruction *instruction             = (MetisInstruction *)registers[REGIP];
  instruction->type                         = INS_GLVERTEXATTRIBPOINTER;      
  instruction->commands.glvertexattribpointer.index = index;
  instruction->commands.glvertexattribpointer.size = size;
  instruction->commands.glvertexattribpointer.type = type;
  instruction->commands.glvertexattribpointer.normalized = normalized;
  instruction->commands.glvertexattribpointer.stride = stride;
  instruction->commands.glvertexattribpointer.pointer = pointer;
  registers[REGIP] += ADVANCE(0, sizeof(instruction->commands.glvertexattribpointer));
};

void MetisVM::add_gldisablevertexattribarray(GLuint index) {
  MetisInstruction *instruction             = (MetisInstruction *)registers[REGIP];
  instruction->type                         = INS_GLDISABLEVERTEXATTRIBARRAY;      
  instruction->commands.glenablevertexattribarray.index = index;
  registers[REGIP] += ADVANCE(0, sizeof(instruction->commands.glenablevertexattribarray));
};


