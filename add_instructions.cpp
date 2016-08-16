#include "metis.hpp"

ostringstream MetisException::cnvt;

void MetisVM::add_end(void) {
  MetisInstruction *instruction            = (MetisInstruction *)registers[REGIP];
  instruction->type                        = INS_END;      
  registers[REGIP] += INS_END_SIZE;   
};

void MetisVM::add_jump(address_mode src) {
  MetisInstruction *instruction            = (MetisInstruction *)registers[REGIP];
  instruction->type                        = INS_JUMP;      
  instruction->commands.extended.addr_mode = BUILD_ADDR(src, 0);
  registers[REGIP] += INS_JUMP_SIZE;
};

void MetisVM::add_jumpi(uint64_t location) {
  MetisInstruction *instruction                 = (MetisInstruction *)registers[REGIP];
  instruction->type                             = INS_JUMPI;      
  instruction->commands.jumpi.value = location;
  registers[REGIP] += INS_JUMPI_SIZE;
};

void MetisVM::add_jizz(address_mode src, address_mode dest) {
  MetisInstruction *instruction            = (MetisInstruction *)registers[REGIP];
  instruction->type                        = INS_JIZZ;      
  instruction->commands.extended.addr_mode = BUILD_ADDR(src, dest);
  registers[REGIP] += INS_JIZZ_SIZE;
}; 

void MetisVM::add_jnz(address_mode src, address_mode dest) {
  MetisInstruction *instruction            = (MetisInstruction *)registers[REGIP];
  instruction->type                        = INS_JNZ;      
  instruction->commands.extended.addr_mode = BUILD_ADDR(src, dest);
  registers[REGIP] += INS_JNZ_SIZE;
}; 

void MetisVM::add_jne(address_mode src, address_mode dest, uint64_t location) {
  MetisInstruction *instruction            = (MetisInstruction *)registers[REGIP];
  instruction->type                        = INS_JNE;      
  instruction->commands.extended.addr_mode = BUILD_ADDR(src, dest);
  instruction->commands.extended.ext.jne.value = location;
  registers[REGIP] += INS_JNE_SIZE;
}

void MetisVM::add_jmpe(address_mode src, address_mode dest, uint64_t location) {
  MetisInstruction *instruction              = (MetisInstruction *)registers[REGIP];
  instruction->type                          = INS_JMPE;      
  instruction->commands.extended.addr_mode   = BUILD_ADDR(src, dest);
  instruction->commands.extended.ext.jmpe.value  = location;
  registers[REGIP] += INS_JMPE_SIZE;
}

void MetisVM::add_store(address_mode src, address_mode dest) {
  MetisInstruction *instruction            = (MetisInstruction *)registers[REGIP];
  instruction->type                        = INS_STORE;      
  instruction->commands.extended.addr_mode = BUILD_ADDR(src, dest);
  registers[REGIP] += INS_STORE_SIZE;
}; 
void MetisVM::add_storei(address_mode dest, uint64_t value) {
  MetisInstruction *instruction                 = (MetisInstruction *)registers[REGIP];
  instruction->type                             = INS_STOREI;      
  instruction->commands.extended.addr_mode = BUILD_ADDR(0, dest);
  instruction->commands.extended.ext.storei.value = value;
  registers[REGIP] += INS_STOREI_SIZE;
};

uint64_t MetisVM::add_label_ip(const char *label) {
  // add a label pointing at the current IP Register value
  // not really an instruction, but it basically acts like one...
  uint64_t new_loc = (registers[REGIP]-(uint64_t)start);
  labels[label] = new_loc;
  return new_loc;
}
uint64_t MetisVM::add_label_val(const char *label, uint64_t val) {
  // add a label pointing at the current IP Register value
  // not really an instruction, but it basically acts like one...
  labels[label] = val;
  return val;
}
void MetisVM::add_data(const uint8_t *data, const uint64_t length, const char *label) {
  MetisInstruction *instruction     = (MetisInstruction *)registers[REGIP];
  instruction->type                 = INS_DATA;      
  instruction->commands.data.length = length;
  if (registers[REGIP] + length > (uint64_t)end) {
    throw MetisException("data blob doesn't fit (add_data)");
  }
  registers[REGIP] += INS_DATA_SIZE;
  add_label_ip(label);

  memcpy((void *)registers[REGIP],data,length);
  registers[REGIP] += length;
}

void MetisVM::add_buffer(const uint8_t *buffer, const uint64_t length, const char *label) {
  if ((buffer_end + length) - buffer > buffer_size) {
    throw MetisException("buffer blob doesn't fit (add_buffer)");
  }
  memcpy((void *)buffer_end, buffer, length);
  buffer_end+=length;
  //add_label_ip(label);
}



void MetisVM::add_not(address_mode src, address_mode dest) {
  MetisInstruction *instruction            = (MetisInstruction *)registers[REGIP];
  instruction->type                        = INS_NOT;      
  instruction->commands.extended.addr_mode = BUILD_ADDR(src, dest);
  registers[REGIP] += INS_MATH_SIZE;
};

void MetisVM::add_gldrawelements(GLenum mode, GLsizei count, 
                                 GLenum type, GLvoid *indices) {
  MetisInstruction *instruction            = (MetisInstruction *)registers[REGIP];
  instruction->type                        = INS_GLDRAWELEMENTS;      
  instruction->commands.gldrawelements.mode = mode;
  instruction->commands.gldrawelements.count = count;
  instruction->commands.gldrawelements.type = type;
  instruction->commands.gldrawelements.indices = indices;
  registers[REGIP] += INS_GLDRAWELEMENTS_SIZE;
}; 

void MetisVM::add_gldrawarrays(GLenum mode, GLint first, GLsizei count) {
  MetisInstruction *instruction            = (MetisInstruction *)registers[REGIP];
  instruction->type                        = INS_GLDRAWARRAYS;      
  instruction->commands.gldrawarrays.mode = mode;
  instruction->commands.gldrawarrays.first = first;
  instruction->commands.gldrawarrays.count = count;
  registers[REGIP] += INS_GLDRAWARRAYS_SIZE;
};

void MetisVM::add_glgenbuffers(GLsizei n, GLuint start_index) {
  MetisInstruction *instruction                  = (MetisInstruction *)registers[REGIP];
  instruction->type                              = INS_GLGENBUFFERS;      
  instruction->commands.glgenbuffers.num_buffers = n;
  instruction->commands.glgenbuffers.start_index = start_index;
  registers[REGIP] += INS_GLGENBUFFERS_SIZE;
};

void MetisVM::add_glbindbuffer(GLenum target, GLuint buffer_index) {
  MetisInstruction *instruction             = (MetisInstruction *)registers[REGIP];
  instruction->type                         = INS_GLBINDBUFFER;      
  instruction->commands.glbindbuffer.target = target;
  instruction->commands.glbindbuffer.buffer_index = buffer_index;
  registers[REGIP] += INS_GLBINDBUFFER_SIZE;
};

void MetisVM::add_glbufferdata(GLenum target, GLsizeiptr size, GLvoid *data, GLenum usage) {
  MetisInstruction *instruction             = (MetisInstruction *)registers[REGIP];
  instruction->type                         = INS_GLBUFFERDATA;      
  instruction->commands.glbufferdata.target = target;
  instruction->commands.glbufferdata.size   = size;
  instruction->commands.glbufferdata.data   = data;
  instruction->commands.glbufferdata.usage  = usage;
  registers[REGIP] +=  INS_GLBUFFERDATA_SIZE;
};


void MetisVM::add_glenablevertexattribarray(GLuint index) {
  MetisInstruction *instruction             = (MetisInstruction *)registers[REGIP];
  instruction->type                         = INS_GLENABLEVERTEXATTRIBARRAY;      
  instruction->commands.glenablevertexattribarray.index = index;
  registers[REGIP] += INS_GLENABLEVERTEXATTRIBARRAY_SIZE;
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
  registers[REGIP] += INS_GLVERTEXATTRIBPOINTER_SIZE;
};

void MetisVM::add_gldisablevertexattribarray(GLuint index) {
  MetisInstruction *instruction             = (MetisInstruction *)registers[REGIP];
  instruction->type                         = INS_GLDISABLEVERTEXATTRIBARRAY;      
  instruction->commands.glenablevertexattribarray.index = index;
  registers[REGIP] += INS_GLDISABLEVERTEXATTRIBARRAY_SIZE;
};


