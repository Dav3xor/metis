#include "metis.hpp"

ostringstream MetisException::cnvt;

uint64_t MetisVM::add_noop(void) {
  CHECK_INSTRUCTION(INS_NOOP_SIZE);

  MetisInstruction *instruction            = (MetisInstruction *)registers[REGIP];
  instruction->type                        = INS_NOOP;      
  registers[REGIP] += INS_NOOP_SIZE;
  RETURN_NEXT();
};

uint64_t MetisVM::add_end(void) {
  CHECK_INSTRUCTION(INS_END_SIZE);

  MetisInstruction *instruction            = (MetisInstruction *)registers[REGIP];
  instruction->type                        = INS_END;      
  registers[REGIP] += INS_END_SIZE;
  RETURN_NEXT();
};

uint64_t MetisVM::add_jump(address_mode src) {
  CHECK_INSTRUCTION(INS_JUMP_SIZE);

  MetisInstruction *instruction            = (MetisInstruction *)registers[REGIP];
  instruction->type                        = INS_JUMP;      
  instruction->commands.extended.addr_mode = BUILD_ADDR(src, 0);
  registers[REGIP] += INS_JUMP_SIZE;
  RETURN_NEXT();
};

uint64_t MetisVM::add_jumpi(uint64_t location) {
  CHECK_INSTRUCTION(INS_JUMPI_SIZE);
  CHECK_LOCATION(location); 

  MetisInstruction *instruction                 = (MetisInstruction *)registers[REGIP];
  instruction->type                             = INS_JUMPI;      
  instruction->commands.jumpi.value = location;
  registers[REGIP] += INS_JUMPI_SIZE;
  RETURN_NEXT();
};

uint64_t MetisVM::add_jizz(address_mode src, address_mode dest) {
  CHECK_INSTRUCTION(INS_JIZZ_SIZE);

  MetisInstruction *instruction            = (MetisInstruction *)registers[REGIP];
  instruction->type                        = INS_JIZZ;      
  instruction->commands.extended.addr_mode = BUILD_ADDR(src, dest);
  registers[REGIP] += INS_JIZZ_SIZE;
  RETURN_NEXT();
}; 

uint64_t MetisVM::add_jnz(address_mode src, address_mode dest) {
  CHECK_INSTRUCTION(INS_JNZ_SIZE);

  MetisInstruction *instruction            = (MetisInstruction *)registers[REGIP];
  instruction->type                        = INS_JNZ;      
  instruction->commands.extended.addr_mode = BUILD_ADDR(src, dest);
  registers[REGIP] += INS_JNZ_SIZE;
  RETURN_NEXT();
}; 

uint64_t MetisVM::add_jne(address_mode src, address_mode dest, uint64_t location) {
  CHECK_INSTRUCTION(INS_JNE_SIZE);
  CHECK_LOCATION(location); 

  MetisInstruction *instruction            = (MetisInstruction *)registers[REGIP];
  instruction->type                        = INS_JNE;      
  instruction->commands.extended.addr_mode = BUILD_ADDR(src, dest);
  instruction->commands.extended.ext.jne.value = location;
  registers[REGIP] += INS_JNE_SIZE;
  RETURN_NEXT();
}

uint64_t MetisVM::add_jmpe(address_mode src, address_mode dest, uint64_t location) {
  CHECK_INSTRUCTION(INS_JMPE_SIZE);
  CHECK_LOCATION(location); 

  MetisInstruction *instruction              = (MetisInstruction *)registers[REGIP];
  instruction->type                          = INS_JMPE;      
  instruction->commands.extended.addr_mode   = BUILD_ADDR(src, dest);
  instruction->commands.extended.ext.jmpe.value  = location;
  registers[REGIP] += INS_JMPE_SIZE;
  RETURN_NEXT();
}

uint64_t MetisVM::add_store(address_mode src, address_mode dest) {
  CHECK_INSTRUCTION(INS_STORE_SIZE);

  MetisInstruction *instruction            = (MetisInstruction *)registers[REGIP];
  instruction->type                        = INS_STORE;      
  instruction->commands.extended.addr_mode = BUILD_ADDR(src, dest);
  registers[REGIP] += INS_STORE_SIZE;
  RETURN_NEXT();
}; 
uint64_t MetisVM::add_storei(address_mode dest, uint64_t value) {
  CHECK_INSTRUCTION(INS_STOREI_SIZE);

  MetisInstruction *instruction                 = (MetisInstruction *)registers[REGIP];
  instruction->type                             = INS_STOREI;      
  instruction->commands.extended.addr_mode = BUILD_ADDR(0, dest);
  instruction->commands.extended.ext.storei.value = value;
  registers[REGIP] += INS_STOREI_SIZE;
  RETURN_NEXT();
};

uint64_t MetisVM::add_label_ip(const char *label) {
  CHECK_POINTER(label);
 
  // add a label pointing at the current IP Register value
  // not really an instruction, but it basically acts like one...
  uint64_t new_loc = (registers[REGIP]-(uint64_t)start);
  labels[label] = new_loc;
  return new_loc;
}
uint64_t MetisVM::add_label_val(const char *label, uint64_t val) {
  CHECK_POINTER(label);
 
  // add a label pointing at the current IP Register value
  // not really an instruction, but it basically acts like one...
  labels[label] = val;
  return val;
}
uint64_t MetisVM::add_data(const uint8_t *data, const uint64_t length, const char *label) {
  CHECK_INSTRUCTION(INS_DATA_SIZE);
  CHECK_POINTER(data);
  CHECK_POINTER(label);

  MetisInstruction *instruction     = (MetisInstruction *)registers[REGIP];
  instruction->type                 = INS_DATA;      
  instruction->commands.data.length = length;
  if (registers[REGIP] + length > (uint64_t)end) {
    throw MetisException("data blob doesn't fit (add_data)",__LINE__,__FILE__);
  }
  registers[REGIP] += INS_DATA_SIZE;
  add_label_ip(label);

  memcpy((void *)registers[REGIP],data,length);
  registers[REGIP] += length;
  RETURN_NEXT();
}


uint64_t MetisVM::add_matrix(const uint8_t width, const uint8_t height, 
                       const uint8_t *data, 
                       const char *label) {
  CHECK_INSTRUCTION(INS_DATA_SIZE);
  CHECK_POINTER(data);
  CHECK_POINTER(label);

  uint64_t length = width*height*4;
  MetisInstruction *instruction     = (MetisInstruction *)registers[REGIP];
  instruction->type                 = INS_DATA;      
  instruction->commands.data.length = length + sizeof(MetisMatrixHeader);
  if (registers[REGIP] + length > (uint64_t)end) {
    throw MetisException("matrix doesn't fit (add_matrix)",__LINE__,__FILE__);
  }
  registers[REGIP] += INS_DATA_SIZE;
  // set width/height
  instruction->commands.data.contents.matrix.width = width;
  instruction->commands.data.contents.matrix.height = height;
  registers[REGIP] += sizeof(MetisMatrixHeader);
  if(label) {
    add_label_val(label, (uint64_t)((uint64_t)(&instruction->commands.data.contents.matrix))-(uint64_t)start);
  }

  memcpy((void *)registers[REGIP],data,length);
  registers[REGIP] += length;
  RETURN_NEXT();
}

uint64_t MetisVM::add_push_matrix(uint64_t location) {
  CHECK_INSTRUCTION(INS_PUSH_MATRIX);
  MetisInstruction *instruction             = (MetisInstruction *)registers[REGIP];
  instruction->type                         = INS_PUSH_MATRIX;      
  instruction->commands.pushmatrix.location = location;
  registers[REGIP] += INS_PUSH_MATRIX_SIZE;
  RETURN_NEXT();
}

uint64_t MetisVM::add_matrix_multiply( address_mode src1, address_mode src2, uint64_t dest) {
  CHECK_INSTRUCTION(INS_MATRIX_MULTIPLY);

  MetisInstruction *instruction            = (MetisInstruction *)registers[REGIP];
  instruction->type                        = INS_MATRIX_MULTIPLY;      
  instruction->commands.extended.addr_mode = BUILD_ADDR(src1, src2);
  instruction->commands.extended.ext.matrix_multiply.destination = dest;
  registers[REGIP] += INS_MATRIX_MULTIPLY_SIZE;
  RETURN_NEXT();
}; 

void MetisVM::add_buffer(const uint8_t *new_buffer, const uint64_t length, const char *label) {
  CHECK_POINTER(new_buffer);
  CHECK_POINTER(label);

  if ((uint64_t)((buffer_end + length) - buffer) > buffer_size) {
    throw MetisException("buffer blob doesn't fit (add_buffer)",__LINE__,__FILE__);
  }
  memcpy((void *)buffer_end, new_buffer, length);
  add_label_val(label, buffer_end-buffer);
  buffer_end+=length;
}



uint64_t MetisVM::add_not(address_mode src, address_mode dest) {
  CHECK_INSTRUCTION(INS_MATH_SIZE);

  MetisInstruction *instruction            = (MetisInstruction *)registers[REGIP];
  instruction->type                        = INS_NOT;      
  instruction->commands.extended.addr_mode = BUILD_ADDR(src, dest);
  registers[REGIP] += INS_MATH_SIZE;
  RETURN_NEXT();
};

uint64_t MetisVM::add_gldrawelements(GLenum mode, GLsizei count, 
                                 GLenum type, GLvoid *indices) {
  CHECK_INSTRUCTION(INS_GLDRAWELEMENTS_SIZE);
  CHECK_POINTER(indices);

  MetisInstruction *instruction            = (MetisInstruction *)registers[REGIP];
  instruction->type                        = INS_GLDRAWELEMENTS;      
  instruction->commands.gldrawelements.mode = mode;
  instruction->commands.gldrawelements.count = count;
  instruction->commands.gldrawelements.type = type;
  instruction->commands.gldrawelements.indices = indices;
  registers[REGIP] += INS_GLDRAWELEMENTS_SIZE;
  RETURN_NEXT();
}; 

uint64_t MetisVM::add_gldrawarrays(GLenum mode, GLint first, GLsizei count) {
  CHECK_INSTRUCTION(INS_GLDRAWARRAYS_SIZE);

  MetisInstruction *instruction            = (MetisInstruction *)registers[REGIP];
  instruction->type                        = INS_GLDRAWARRAYS;      
  instruction->commands.gldrawarrays.mode = mode;
  instruction->commands.gldrawarrays.first = first;
  instruction->commands.gldrawarrays.count = count;
  registers[REGIP] += INS_GLDRAWARRAYS_SIZE;
  RETURN_NEXT();
};

uint64_t MetisVM::add_glgenbuffers(GLsizei n, GLuint start_index) {
  CHECK_INSTRUCTION(INS_GLGENBUFFERS_SIZE);

  MetisInstruction *instruction                  = (MetisInstruction *)registers[REGIP];
  instruction->type                              = INS_GLGENBUFFERS;      
  instruction->commands.glgenbuffers.num_buffers = n;
  instruction->commands.glgenbuffers.start_index = start_index;
  registers[REGIP] += INS_GLGENBUFFERS_SIZE;
  RETURN_NEXT();
};

uint64_t MetisVM::add_glbindbuffer(GLenum target, GLuint buffer_index) {
  CHECK_INSTRUCTION(INS_GLBINDBUFFER_SIZE);

  MetisInstruction *instruction             = (MetisInstruction *)registers[REGIP];
  instruction->type                         = INS_GLBINDBUFFER;      
  instruction->commands.glbindbuffer.target = target;
  instruction->commands.glbindbuffer.buffer_index = buffer_index;
  registers[REGIP] += INS_GLBINDBUFFER_SIZE;
  RETURN_NEXT();
};

uint64_t MetisVM::add_glbufferdata(GLenum target, GLsizeiptr size, GLvoid *data, GLenum usage) {
  CHECK_INSTRUCTION(INS_GLBUFFERDATA_SIZE);
  CHECK_POINTER(data);

  MetisInstruction *instruction             = (MetisInstruction *)registers[REGIP];
  instruction->type                         = INS_GLBUFFERDATA;      
  instruction->commands.glbufferdata.target = target;
  instruction->commands.glbufferdata.size   = size;
  instruction->commands.glbufferdata.data   = data;
  instruction->commands.glbufferdata.usage  = usage;
  registers[REGIP] +=  INS_GLBUFFERDATA_SIZE;
  RETURN_NEXT();
};


uint64_t MetisVM::add_glenablevertexattribarray(GLuint index) {
  CHECK_INSTRUCTION(INS_GLENABLEVERTEXATTRIBARRAY_SIZE);

  MetisInstruction *instruction             = (MetisInstruction *)registers[REGIP];
  instruction->type                         = INS_GLENABLEVERTEXATTRIBARRAY;      
  instruction->commands.glenablevertexattribarray.index = index;
  registers[REGIP] += INS_GLENABLEVERTEXATTRIBARRAY_SIZE;
  RETURN_NEXT();
};

uint64_t MetisVM::add_glvertexattribpointer(GLuint index, GLint size, 
                             GLenum type, GLboolean normalized, 
                             GLsizei stride, GLvoid *pointer) {
  CHECK_INSTRUCTION(INS_GLVERTEXATTRIBPOINTER_SIZE);
  CHECK_POINTER(pointer);

  MetisInstruction *instruction             = (MetisInstruction *)registers[REGIP];
  instruction->type                         = INS_GLVERTEXATTRIBPOINTER;      
  instruction->commands.glvertexattribpointer.index = index;
  instruction->commands.glvertexattribpointer.size = size;
  instruction->commands.glvertexattribpointer.type = type;
  instruction->commands.glvertexattribpointer.normalized = normalized;
  instruction->commands.glvertexattribpointer.stride = stride;
  instruction->commands.glvertexattribpointer.pointer = pointer;
  registers[REGIP] += INS_GLVERTEXATTRIBPOINTER_SIZE;
  RETURN_NEXT();
};

uint64_t MetisVM::add_gldisablevertexattribarray(GLuint index) {
  CHECK_INSTRUCTION(INS_GLDISABLEVERTEXATTRIBARRAY_SIZE);

  MetisInstruction *instruction             = (MetisInstruction *)registers[REGIP];
  instruction->type                         = INS_GLDISABLEVERTEXATTRIBARRAY;      
  instruction->commands.glenablevertexattribarray.index = index;
  registers[REGIP] += INS_GLDISABLEVERTEXATTRIBARRAY_SIZE;
  RETURN_NEXT();
};


