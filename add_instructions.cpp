#include "metis.hpp"

#define RETURN_NEXT() return (uint64_t)instruction-(uint64_t)code_start;

#define CHECK_INSTRUCTION(instruction_length) \
  if ((uint8_t *)(registers[REGIP].ulong + instruction_length) > code_end) { \
    throw MetisException("attempt to add instruction past address space",__LINE__,__FILE__); \
  } 

#define CHECK_LOCATION(location) \
  if (location > (uint64_t)(code_end-code_start-1)) { \
    throw MetisException("attempt to use location outside address space",__LINE__,__FILE__); \
  }

#define CHECK_POINTER(pointer) \
  if (pointer == NULL) { \
    throw MetisException("null pointer",__LINE__,__FILE__); \
  }

ostringstream MetisException::cnvt;
ostringstream MasmException::cnvt;

uint64_t MetisVM::add_noop(void) {
  CHECK_INSTRUCTION(INS_NOOP_SIZE);

  MetisInstruction *instruction            = (MetisInstruction *)registers[REGIP].ulong;
  instruction->type                        = INS_NOOP;      
  registers[REGIP].ulong += INS_NOOP_SIZE;
  RETURN_NEXT();
}

uint64_t MetisVM::add_end(void) {
  CHECK_INSTRUCTION(INS_END_SIZE);

  MetisInstruction *instruction            = (MetisInstruction *)registers[REGIP].ulong;
  instruction->type                        = INS_END;      
  registers[REGIP].ulong += INS_END_SIZE;
  RETURN_NEXT();
}

uint64_t MetisVM::add_error(void) {
  CHECK_INSTRUCTION(INS_ERROR_SIZE);

  MetisInstruction *instruction            = (MetisInstruction *)registers[REGIP].ulong;
  instruction->type                        = INS_ERROR;      
  registers[REGIP].ulong += INS_ERROR_SIZE;
  RETURN_NEXT();
}

uint64_t MetisVM::add_atan2(address_mode x, address_mode y, address_mode result) {
  CHECK_INSTRUCTION(INS_ATAN2_SIZE);

  MetisInstruction *instruction            = (MetisInstruction *)registers[REGIP].ulong;
  instruction->type                        = INS_ATAN2;      
  instruction->commands.extended2.addr_mode1 = BUILD_ADDR(x,y);
  instruction->commands.extended2.addr_mode2 = BUILD_ADDR(0,result);
  registers[REGIP].ulong += INS_ATAN2_SIZE;
  RETURN_NEXT();
}

uint64_t MetisVM::add_jump(address_mode src) {
  CHECK_INSTRUCTION(INS_JUMP_SIZE);

  MetisInstruction *instruction            = (MetisInstruction *)registers[REGIP].ulong;
  instruction->type                        = INS_JUMP;      
  instruction->commands.extended.addr_mode = BUILD_ADDR(src, 0);
  registers[REGIP].ulong += INS_JUMP_SIZE;
  RETURN_NEXT();
}

uint64_t MetisVM::add_jumpi(uint64_t location) {
  CHECK_INSTRUCTION(INS_JUMPI_SIZE);
  CHECK_LOCATION(location); 

  MetisInstruction *instruction                 = (MetisInstruction *)registers[REGIP].ulong;
  instruction->type                             = INS_JUMPI;      
  instruction->commands.jumpi.value = location;
  registers[REGIP].ulong += INS_JUMPI_SIZE;
  RETURN_NEXT();
}

uint64_t MetisVM::add_jizz(address_mode src, address_mode dest) {
  CHECK_INSTRUCTION(INS_JIZZ_SIZE);

  MetisInstruction *instruction            = (MetisInstruction *)registers[REGIP].ulong;
  instruction->type                        = INS_JIZZ;      
  instruction->commands.extended.addr_mode = BUILD_ADDR(src, dest);
  registers[REGIP].ulong += INS_JIZZ_SIZE;
  RETURN_NEXT();
} 

uint64_t MetisVM::add_jnz(address_mode src, address_mode dest) {
  CHECK_INSTRUCTION(INS_JNZ_SIZE);

  MetisInstruction *instruction            = (MetisInstruction *)registers[REGIP].ulong;
  instruction->type                        = INS_JNZ;      
  instruction->commands.extended.addr_mode = BUILD_ADDR(src, dest);
  registers[REGIP].ulong += INS_JNZ_SIZE;
  RETURN_NEXT();
} 

uint64_t MetisVM::add_jne(address_mode src, address_mode dest, uint64_t location) {
  CHECK_INSTRUCTION(INS_JNE_SIZE);
  CHECK_LOCATION(location); 

  MetisInstruction *instruction            = (MetisInstruction *)registers[REGIP].ulong;
  instruction->type                        = INS_JNE;      
  instruction->commands.extended.addr_mode = BUILD_ADDR(src, dest);
  instruction->commands.extended.ext.jne.value = location;
  registers[REGIP].ulong += INS_JNE_SIZE;
  RETURN_NEXT();
}

uint64_t MetisVM::add_jmpe(address_mode src, address_mode dest, uint64_t location) {
  CHECK_INSTRUCTION(INS_JMPE_SIZE);
  CHECK_LOCATION(location); 

  MetisInstruction *instruction              = (MetisInstruction *)registers[REGIP].ulong;
  instruction->type                          = INS_JMPE;      
  instruction->commands.extended.addr_mode   = BUILD_ADDR(src, dest);
  instruction->commands.extended.ext.jmpe.value  = location;
  registers[REGIP].ulong += INS_JMPE_SIZE;
  RETURN_NEXT();
}

uint64_t MetisVM::add_store(address_mode src, address_mode dest) {
  CHECK_INSTRUCTION(INS_STORE_SIZE);

  MetisInstruction *instruction            = (MetisInstruction *)registers[REGIP].ulong;
  instruction->type                        = INS_STORE;      
  instruction->commands.extended.addr_mode = BUILD_ADDR(src, dest);
  registers[REGIP].ulong += INS_STORE_SIZE;
  RETURN_NEXT();
} 

uint64_t MetisVM::add_storei(address_mode dest, uint64_t value) {
  CHECK_INSTRUCTION(INS_STOREI_SIZE);

  MetisInstruction *instruction                 = (MetisInstruction *)registers[REGIP].ulong;
  instruction->type                             = INS_STOREI;      
  instruction->commands.extended.addr_mode = BUILD_ADDR(0, dest);
  instruction->commands.extended.ext.storei.value.ulong = value;
  registers[REGIP].ulong += INS_STOREI_SIZE;
  RETURN_NEXT();
}

uint64_t MetisVM::add_storei_double(address_mode dest, double value) {
  CHECK_INSTRUCTION(INS_STOREI_SIZE);

  MetisInstruction *instruction                 = (MetisInstruction *)registers[REGIP].ulong;
  instruction->type                             = INS_STOREI;      
  instruction->commands.extended.addr_mode = BUILD_ADDR(0, dest);
  instruction->commands.extended.ext.storei.value.whole_double = value;
  registers[REGIP].ulong += INS_STOREI_SIZE;
  RETURN_NEXT();
}

uint64_t MetisVM::add_store_sr(address_mode src, uint64_t offset) {
  CHECK_INSTRUCTION(INS_STORE_SR_SIZE);

  MetisInstruction *instruction                 = (MetisInstruction *)registers[REGIP].ulong;
  instruction->type                             = INS_STORE_SR;      
  instruction->commands.extended.addr_mode = BUILD_ADDR(src, 0);
  instruction->commands.extended.ext.store_sr.offset = offset+1;
  registers[REGIP].ulong += INS_STORE_SR_SIZE;
  RETURN_NEXT();
}

uint64_t MetisVM::add_pushr(uint16_t reg_set) {
  CHECK_INSTRUCTION(INS_PUSHR_SIZE);

  MetisInstruction *instruction                 = (MetisInstruction *)registers[REGIP].ulong;
  instruction->type                             = INS_PUSHR;      
  instruction->commands.pushr.registers = reg_set;
  registers[REGIP].ulong += INS_PUSHR_SIZE;
  RETURN_NEXT();
}
uint64_t MetisVM::add_popr(uint16_t reg_set) {
  CHECK_INSTRUCTION(INS_POPR_SIZE);

  MetisInstruction *instruction                 = (MetisInstruction *)registers[REGIP].ulong;
  instruction->type                             = INS_POPR;      
  instruction->commands.popr.registers = reg_set;
  registers[REGIP].ulong += INS_POPR_SIZE;
  RETURN_NEXT();
}
uint64_t MetisVM::add_stack_adj(uint64_t amount) {
  CHECK_INSTRUCTION(INS_STACK_ADJ_SIZE);

  MetisInstruction *instruction                 = (MetisInstruction *)registers[REGIP].ulong;
  instruction->type                             = INS_STACK_ADJ;      
  instruction->commands.stack_adj.amount = amount;
  registers[REGIP].ulong += INS_STACK_ADJ_SIZE;
  RETURN_NEXT();
}

uint64_t MetisVM::add_load_sr(uint64_t offset, address_mode dest) {
  CHECK_INSTRUCTION(INS_LOAD_SR_SIZE);

  MetisInstruction *instruction                 = (MetisInstruction *)registers[REGIP].ulong;
  instruction->type                             = INS_LOAD_SR;      
  instruction->commands.extended.addr_mode = BUILD_ADDR(0, dest);
  instruction->commands.extended.ext.load_sr.offset = offset+1;
  registers[REGIP].ulong += INS_LOAD_SR_SIZE;
  RETURN_NEXT();
}
uint64_t MetisVM::add_label_ip(const char *label) {
  CHECK_POINTER(label);
 
  // add a label pointing at the current IP Register value
  // not really an instruction, but it basically acts like one...
  TypedCell new_loc;

  new_loc.cell.ulong = (registers[REGIP].ulong-(uint64_t)code_start);
  new_loc.type  = TypedCell::TYPE_ULONG;
  labels[label] = new_loc;
  return new_loc.cell.ulong;
}

uint64_t MetisVM::add_label_val(const char *label, uint64_t val) {
  CHECK_POINTER(label);
 
  // add a label pointing at the current IP Register value
  // not really an instruction, but it basically acts like one...
  TypedCell new_loc;

  new_loc.cell.ulong = val;
  new_loc.type  = TypedCell::TYPE_ULONG;
  labels[label] = new_loc;
  return val;
}

uint64_t MetisVM::add_label_float(const char *label, float val) {
  CHECK_POINTER(label);
 
  // add a label pointing at the current IP Register value
  // not really an instruction, but it basically acts like one...
  TypedCell new_loc;

  new_loc.cell.floats[0] = val;
  new_loc.type  = TypedCell::TYPE_FLOAT;
  labels[label] = new_loc;
  return val;
}

uint64_t MetisVM::add_data(const uint8_t *data, const uint64_t length, const char *label) {
  CHECK_INSTRUCTION(INS_DATA_SIZE);
  CHECK_POINTER(data);
  CHECK_POINTER(label);

  MetisInstruction *instruction     = (MetisInstruction *)registers[REGIP].ulong;
  instruction->type                 = INS_DATA;      
  instruction->commands.data.length = length;
  if (registers[REGIP].ulong + length > (uint64_t)code_end) {
    throw MetisException(string("out of memory -- (add_data) label = ") + label,__LINE__,__FILE__);
  }
  registers[REGIP].ulong += INS_DATA_SIZE;
  add_label_ip(label);

  memcpy((void *)registers[REGIP].ulong,data,length);
  registers[REGIP].ulong += length;
  RETURN_NEXT();
}


uint64_t MetisVM::add_identity_matrix(const uint8_t width, const uint8_t height, 
                       const char *label) {
  CHECK_INSTRUCTION(INS_DATA_SIZE);
  CHECK_POINTER(label);

  uint64_t length = width*height*4;
  MetisInstruction *instruction     = (MetisInstruction *)registers[REGIP].ulong;
  instruction->type                 = INS_DATA;      
  instruction->commands.data.length = length + sizeof(MetisMatrixHeader);
  if (registers[REGIP].ulong + length > (uint64_t)code_end) {
    throw MetisException(string("matrix doesn't fit (add_matrix) label = ") + label,__LINE__,__FILE__);
  }
  registers[REGIP].ulong += INS_DATA_SIZE;
  // set width/height
  instruction->commands.data.contents.matrix.width = width;
  instruction->commands.data.contents.matrix.height = height;
  registers[REGIP].ulong += sizeof(MetisMatrixHeader);
  if(label) {
    add_label_val(label, (uint64_t)((uint64_t)(&instruction->commands.data.contents.matrix))-(uint64_t)code_start);
  }

  float *matrix = (float *)registers[REGIP].ulong;
  
  for(int i=0; i<height; i++) {
    for(int j=0; j<width; j++) {
      if(i==j) {
        matrix[i*height+j] = 1.0;
      } else {
        matrix[i*height+j] = 0.0;
      }
    }
  }
        
  registers[REGIP].ulong += length;
  RETURN_NEXT();
}
uint64_t MetisVM::add_matrix(const uint8_t width, const uint8_t height, 
                       const uint8_t *data, 
                       const char *label) {
  CHECK_INSTRUCTION(INS_DATA_SIZE);
  CHECK_POINTER(data);
  CHECK_POINTER(label);

  uint64_t length = width*height*4;
  MetisInstruction *instruction     = (MetisInstruction *)registers[REGIP].ulong;
  instruction->type                 = INS_DATA;      
  instruction->commands.data.length = length + sizeof(MetisMatrixHeader);
  if (registers[REGIP].ulong + length > (uint64_t)code_end) {
    throw MetisException(string("matrix doesn't fit (add_matrix) label = ") + label,__LINE__,__FILE__);
  }
  registers[REGIP].ulong += INS_DATA_SIZE;
  // set width/height
  instruction->commands.data.contents.matrix.width = width;
  instruction->commands.data.contents.matrix.height = height;
  registers[REGIP].ulong += sizeof(MetisMatrixHeader);
  if(label) {
    add_label_val(label, (uint64_t)((uint64_t)(&instruction->commands.data.contents.matrix))-(uint64_t)code_start);
  }

  memcpy((void *)registers[REGIP].ulong,data,length);
  registers[REGIP].ulong += length;
  RETURN_NEXT();
}

uint64_t MetisVM::add_push_matrix(uint64_t location) {
  CHECK_INSTRUCTION(INS_PUSH_MATRIX);
  MetisInstruction *instruction             = (MetisInstruction *)registers[REGIP].ulong;
  instruction->type                         = INS_PUSH_MATRIX;      
  instruction->commands.pushmatrix.location = location;
  registers[REGIP].ulong += INS_PUSH_MATRIX_SIZE;
  RETURN_NEXT();
}

uint64_t MetisVM::add_matrix_multiply(address_mode src1, address_mode src2, uint64_t dest) {
  CHECK_INSTRUCTION(INS_MATRIX_MULTIPLY);

  MetisInstruction *instruction            = (MetisInstruction *)registers[REGIP].ulong;
  instruction->type                        = INS_MATRIX_MULTIPLY;      
  instruction->commands.extended.addr_mode = BUILD_ADDR(src1, src2);
  instruction->commands.extended.ext.matrix_multiply.destination = dest;
  registers[REGIP].ulong += INS_MATRIX_MULTIPLY_SIZE;
  RETURN_NEXT();
} 

uint64_t MetisVM::add_matrix_add(address_mode src1, address_mode src2, uint64_t dest) {
  CHECK_INSTRUCTION(INS_MATRIX_ADD);

  MetisInstruction *instruction            = (MetisInstruction *)registers[REGIP].ulong;
  instruction->type                        = INS_MATRIX_ADD;      
  instruction->commands.extended.addr_mode = BUILD_ADDR(src1, src2);
  instruction->commands.extended.ext.matrix_add.destination = dest;
  registers[REGIP].ulong += INS_MATRIX_ADD_SIZE;
  RETURN_NEXT();
}
  
uint64_t MetisVM::add_vector_dot(address_mode src1, address_mode src2, uint64_t dest) {
  CHECK_INSTRUCTION(INS_VECTOR_DOT);

  MetisInstruction *instruction            = (MetisInstruction *)registers[REGIP].ulong;
  instruction->type                        = INS_VECTOR_DOT;      
  instruction->commands.extended.addr_mode = BUILD_ADDR(src1, src2);
  instruction->commands.extended.ext.vector_dot.destination = dest;
  registers[REGIP].ulong += INS_VECTOR_DOT_SIZE;
  RETURN_NEXT();
}

uint64_t MetisVM::add_vector_cross(address_mode src1, address_mode src2, uint64_t dest) {
  CHECK_INSTRUCTION(INS_VECTOR_CROSS);

  MetisInstruction *instruction            = (MetisInstruction *)registers[REGIP].ulong;
  instruction->type                        = INS_VECTOR_CROSS;      
  instruction->commands.extended.addr_mode = BUILD_ADDR(src1, src2);
  instruction->commands.extended.ext.vector_cross.destination = dest;
  registers[REGIP].ulong += INS_VECTOR_CROSS_SIZE;
  RETURN_NEXT();
}

uint64_t MetisVM::add_wait(address_mode src) {
  CHECK_INSTRUCTION(INS_WAIT_SIZE);

  MetisInstruction *instruction            = (MetisInstruction *)registers[REGIP].ulong;
  instruction->type                        = INS_WAIT;      
  instruction->commands.extended.addr_mode = BUILD_ADDR(src, 0);
  registers[REGIP].ulong += INS_WAIT_SIZE;
  RETURN_NEXT();
} 

uint64_t MetisVM::add_curtime(address_mode dest) {
  CHECK_INSTRUCTION(INS_WAIT_SIZE);

  MetisInstruction *instruction            = (MetisInstruction *)registers[REGIP].ulong;
  instruction->type                        = INS_CURTIME;      
  instruction->commands.extended.addr_mode = BUILD_ADDR(0, dest);
  registers[REGIP].ulong += INS_CURTIME_SIZE;
  RETURN_NEXT();
} 

uint64_t MetisVM::add_open(address_mode src, address_mode dest) {
  CHECK_INSTRUCTION(INS_OPEN_SIZE);

  MetisInstruction *instruction            = (MetisInstruction *)registers[REGIP].ulong;
  instruction->type                        = INS_OPEN;      
  instruction->commands.extended.addr_mode = BUILD_ADDR(src, dest);

  registers[REGIP].ulong += INS_OPEN_SIZE;
  RETURN_NEXT();
} 

uint64_t MetisVM::add_close(address_mode file) {
  CHECK_INSTRUCTION(INS_CLOSE_SIZE);

  MetisInstruction *instruction            = (MetisInstruction *)registers[REGIP].ulong;
  instruction->type                        = INS_CLOSE;      
  instruction->commands.extended.addr_mode = BUILD_ADDR(file, 0);

  registers[REGIP].ulong += INS_CLOSE_SIZE;
  RETURN_NEXT();
} 

uint64_t MetisVM::add_read(address_mode file, address_mode dest, uint64_t max_bytes) {
  CHECK_INSTRUCTION(INS_READ_SIZE);

  MetisInstruction *instruction            = (MetisInstruction *)registers[REGIP].ulong;
  instruction->type                        = INS_READ;      
  instruction->commands.extended.addr_mode = BUILD_ADDR(file, dest);
  instruction->commands.extended.ext.read.max_bytes = max_bytes;

  registers[REGIP].ulong += INS_READ_SIZE;
  RETURN_NEXT();
} 

uint64_t MetisVM::add_write(address_mode file, address_mode dest, uint64_t num_bytes) {
  CHECK_INSTRUCTION(INS_WRITE_SIZE);

  MetisInstruction *instruction            = (MetisInstruction *)registers[REGIP].ulong;
  instruction->type                        = INS_WRITE;      
  instruction->commands.extended.addr_mode = BUILD_ADDR(file, dest);
  instruction->commands.extended.ext.write.num_bytes = num_bytes;

  registers[REGIP].ulong += INS_WRITE_SIZE;
  RETURN_NEXT();
} 

uint64_t MetisVM::add_seek(address_mode file, address_mode src_seek) {
  CHECK_INSTRUCTION(INS_SEEK_SIZE);

  MetisInstruction *instruction            = (MetisInstruction *)registers[REGIP].ulong;
  instruction->type                        = INS_SEEK;      
  instruction->commands.extended.addr_mode = BUILD_ADDR(file,src_seek);

  registers[REGIP].ulong += INS_SEEK_SIZE;
  RETURN_NEXT();
} 

uint64_t MetisVM::add_select(address_mode src_select) {
  CHECK_INSTRUCTION(INS_SELECT_SIZE);

  MetisInstruction *instruction            = (MetisInstruction *)registers[REGIP].ulong;
  instruction->type                        = INS_SELECT;      
  instruction->commands.extended.addr_mode = BUILD_ADDR(src_select,0);

  registers[REGIP].ulong += INS_SELECT_SIZE;
  RETURN_NEXT();
} 

uint64_t MetisVM::add_remove(address_mode file, address_mode result) {
  CHECK_INSTRUCTION(INS_REMOVE_SIZE);

  MetisInstruction *instruction            = (MetisInstruction *)registers[REGIP].ulong;
  instruction->type                        = INS_REMOVE;      
  instruction->commands.extended.addr_mode = BUILD_ADDR(file,result);

  registers[REGIP].ulong += INS_REMOVE_SIZE;
  RETURN_NEXT();
} 

uint64_t MetisVM::add_exists(address_mode file, address_mode result) {
  CHECK_INSTRUCTION(INS_EXISTS_SIZE);

  MetisInstruction *instruction            = (MetisInstruction *)registers[REGIP].ulong;
  instruction->type                        = INS_EXISTS;      
  instruction->commands.extended.addr_mode = BUILD_ADDR(file, result);

  registers[REGIP].ulong += INS_EXISTS_SIZE;
  RETURN_NEXT();
} 
  
uint64_t MetisVM::add_malloc(address_mode size, address_mode result) {
  CHECK_INSTRUCTION(INS_MALLOC_SIZE);

  MetisInstruction *instruction            = (MetisInstruction *)registers[REGIP].ulong;
  instruction->type                        = INS_MALLOC;      
  instruction->commands.extended.addr_mode = BUILD_ADDR(size, result);

  registers[REGIP].ulong += INS_MALLOC_SIZE;
  RETURN_NEXT();
} 

uint64_t MetisVM::add_free(address_mode src) {
  CHECK_INSTRUCTION(INS_FREE_SIZE);

  MetisInstruction *instruction            = (MetisInstruction *)registers[REGIP].ulong;
  instruction->type                        = INS_FREE;      
  instruction->commands.extended.addr_mode = BUILD_ADDR(src, 0);

  registers[REGIP].ulong += INS_FREE_SIZE;
  RETURN_NEXT();
} 

uint64_t MetisVM::add_buffer(const uint8_t *new_buffer, const uint64_t length, const char *label) {
  CHECK_POINTER(new_buffer);
  CHECK_POINTER(label);

  if ((uint64_t)((buffer_end + length) - buffer) > buffer_size) {
    throw MetisException(string("buffer blob doesn't fit (add_buffer) label = ") + label,__LINE__,__FILE__);
  }
  memcpy((void *)buffer_end, new_buffer, length);
  add_label_val(label, buffer_end-buffer);
  buffer_end+=length;
  return buffer_end-length-buffer;
}



uint64_t MetisVM::add_not(address_mode src, address_mode dest) {
  CHECK_INSTRUCTION(INS_MATH_SIZE);

  MetisInstruction *instruction            = (MetisInstruction *)registers[REGIP].ulong;
  instruction->type                        = INS_NOT;      
  instruction->commands.extended.addr_mode = BUILD_ADDR(src, dest);
  registers[REGIP].ulong += INS_MATH_SIZE;
  RETURN_NEXT();
}

uint64_t MetisVM::add_gldrawelements(GLenum mode, GLsizei count, 
                                 GLenum type, uint64_t indices) {
  CHECK_INSTRUCTION(INS_GLDRAWELEMENTS_SIZE);

  MetisInstruction *instruction            = (MetisInstruction *)registers[REGIP].ulong;
  instruction->type                        = INS_GLDRAWELEMENTS;      
  instruction->commands.gldrawelements.mode = mode;
  instruction->commands.gldrawelements.count = count;
  instruction->commands.gldrawelements.type = type;
  instruction->commands.gldrawelements.indices = indices;
  registers[REGIP].ulong += INS_GLDRAWELEMENTS_SIZE;
  RETURN_NEXT();
} 

uint64_t MetisVM::add_gldrawarrays(GLenum mode, GLint first, GLsizei count) {
  CHECK_INSTRUCTION(INS_GLDRAWARRAYS_SIZE);

  MetisInstruction *instruction            = (MetisInstruction *)registers[REGIP].ulong;
  instruction->type                        = INS_GLDRAWARRAYS;      
  instruction->commands.gldrawarrays.mode = mode;
  instruction->commands.gldrawarrays.first = first;
  instruction->commands.gldrawarrays.count = count;
  registers[REGIP].ulong += INS_GLDRAWARRAYS_SIZE;
  RETURN_NEXT();
}

uint64_t MetisVM::add_glgenbuffers(GLsizei n, metisgl_identifier start_index) {
  CHECK_INSTRUCTION(INS_GLGENBUFFERS_SIZE);

  MetisInstruction *instruction                  = (MetisInstruction *)registers[REGIP].ulong;
  instruction->type                              = INS_GLGENBUFFERS;      
  instruction->commands.glgenbuffers.num_identifiers = n;
  instruction->commands.glgenbuffers.start_index = start_index;
  registers[REGIP].ulong += INS_GLGENBUFFERS_SIZE;
  RETURN_NEXT();
}

uint64_t MetisVM::add_glgenvertexarrays(GLsizei num_identifiers, metisgl_identifier start_index) {
  CHECK_INSTRUCTION(INS_GLGENVERTEXARRAYS_SIZE);

  MetisInstruction *instruction                       = (MetisInstruction *)registers[REGIP].ulong;
  instruction->type                                   = INS_GLGENVERTEXARRAYS;      
  instruction->commands.glgenvertexarrays.num_identifiers = num_identifiers;
  instruction->commands.glgenvertexarrays.start_index = start_index;
  registers[REGIP].ulong += INS_GLGENVERTEXARRAYS_SIZE;
  RETURN_NEXT();
}

uint64_t MetisVM::add_glbindvertexarray(metisgl_identifier array_index) {
  CHECK_INSTRUCTION(INS_GLBINDVERTEXARRAY_SIZE);

  MetisInstruction *instruction                       = (MetisInstruction *)registers[REGIP].ulong;
  instruction->type                                   = INS_GLBINDVERTEXARRAY;      
  instruction->commands.glbindvertexarray.array_index = array_index;
  registers[REGIP].ulong += INS_GLBINDVERTEXARRAY_SIZE;
  RETURN_NEXT();
}


uint64_t MetisVM::add_glbindbuffer(GLenum target, metisgl_identifier buffer_index) {
  CHECK_INSTRUCTION(INS_GLBINDBUFFER_SIZE);

  MetisInstruction *instruction             = (MetisInstruction *)registers[REGIP].ulong;
  instruction->type                         = INS_GLBINDBUFFER;      
  instruction->commands.glbindbuffer.target = target;
  instruction->commands.glbindbuffer.buffer_index = buffer_index;
  registers[REGIP].ulong += INS_GLBINDBUFFER_SIZE;
  RETURN_NEXT();
}

uint64_t MetisVM::add_glbufferdata(GLenum target, GLsizeiptr size, uint64_t data_index, GLenum usage) {
  CHECK_INSTRUCTION(INS_GLBUFFERDATA_SIZE);

  MetisInstruction *instruction                   = (MetisInstruction *)registers[REGIP].ulong;
  instruction->type                               = INS_GLBUFFERDATA;      
  instruction->commands.glbufferdata.target       = target;
  instruction->commands.glbufferdata.size         = size;
  instruction->commands.glbufferdata.data_index   = data_index;
  instruction->commands.glbufferdata.usage        = usage;
  registers[REGIP].ulong +=  INS_GLBUFFERDATA_SIZE;  
  RETURN_NEXT();
}


uint64_t MetisVM::add_glenablevertexattribarray(GLuint index) {
  CHECK_INSTRUCTION(INS_GLENABLEVERTEXATTRIBARRAY_SIZE);

  MetisInstruction *instruction             = (MetisInstruction *)registers[REGIP].ulong;
  instruction->type                         = INS_GLENABLEVERTEXATTRIBARRAY;      
  instruction->commands.glenablevertexattribarray.index = index;
  registers[REGIP].ulong += INS_GLENABLEVERTEXATTRIBARRAY_SIZE;
  RETURN_NEXT();
}

uint64_t MetisVM::add_glvertexattribpointer(GLuint index, GLint size, 
                             GLenum type, GLboolean normalized, 
                             GLsizei stride, uint64_t pointer) {
  CHECK_INSTRUCTION(INS_GLVERTEXATTRIBPOINTER_SIZE);
  //CHECK_POINTER(pointer);

  MetisInstruction *instruction             = (MetisInstruction *)registers[REGIP].ulong;
  instruction->type                         = INS_GLVERTEXATTRIBPOINTER;      
  instruction->commands.glvertexattribpointer.index = index;
  instruction->commands.glvertexattribpointer.size = size;
  instruction->commands.glvertexattribpointer.type = type;
  instruction->commands.glvertexattribpointer.normalized = normalized;
  instruction->commands.glvertexattribpointer.stride = stride;
  instruction->commands.glvertexattribpointer.pointer = pointer;
  registers[REGIP].ulong += INS_GLVERTEXATTRIBPOINTER_SIZE;
  RETURN_NEXT();
}

uint64_t MetisVM::add_gldisablevertexattribarray(GLuint index) {
  CHECK_INSTRUCTION(INS_GLDISABLEVERTEXATTRIBARRAY_SIZE);

  MetisInstruction *instruction             = (MetisInstruction *)registers[REGIP].ulong;
  instruction->type                         = INS_GLDISABLEVERTEXATTRIBARRAY;      
  instruction->commands.glenablevertexattribarray.index = index;
  registers[REGIP].ulong += INS_GLDISABLEVERTEXATTRIBARRAY_SIZE;
  RETURN_NEXT();
}


uint64_t MetisVM::add_glenable(GLenum capability) {
  CHECK_INSTRUCTION(INS_GLENABLE_SIZE);

  MetisInstruction *instruction             = (MetisInstruction *)registers[REGIP].ulong;
  instruction->type                         = INS_GLENABLE;
  instruction->commands.glenable.capability = capability;
  registers[REGIP].ulong += INS_GLENABLE_SIZE;
  RETURN_NEXT();
}

uint64_t MetisVM::add_gldepthfunc(GLenum function) {
  CHECK_INSTRUCTION(INS_GLDEPTHFUNC_SIZE);

  MetisInstruction *instruction             = (MetisInstruction *)registers[REGIP].ulong;
  instruction->type                         = INS_GLDEPTHFUNC;
  instruction->commands.gldepthfunc.function   = function;
  registers[REGIP].ulong += INS_GLDEPTHFUNC_SIZE;
  RETURN_NEXT();
}


uint64_t MetisVM::add_glcreateshader(GLenum type, metisgl_identifier start_index) {
  CHECK_INSTRUCTION(INS_GLCREATESHADER_SIZE);

  MetisInstruction *instruction                    = (MetisInstruction *)registers[REGIP].ulong;
  instruction->type                                = INS_GLCREATESHADER;
  instruction->commands.glcreateshader.type        = type;
  instruction->commands.glcreateshader.start_index = start_index;
  registers[REGIP].ulong += INS_GLCREATESHADER_SIZE;
  RETURN_NEXT();
}

uint64_t MetisVM::add_glshadersource(GLuint shader, metisgl_identifier source_index) {
  CHECK_INSTRUCTION(INS_GLSHADERSOURCE_SIZE);

  MetisInstruction *instruction                     = (MetisInstruction *)registers[REGIP].ulong;
  instruction->type                                 = INS_GLSHADERSOURCE;
  instruction->commands.glshadersource.shader       = shader;
  instruction->commands.glshadersource.source_index = source_index;
  registers[REGIP].ulong += INS_GLSHADERSOURCE_SIZE;
  RETURN_NEXT();
}
uint64_t MetisVM::add_glcompileshader(metisgl_identifier index) {
  CHECK_INSTRUCTION(INS_GLCOMPILESHADER_SIZE);

  MetisInstruction *instruction                      = (MetisInstruction *)registers[REGIP].ulong;
  instruction->type                                  = INS_GLCOMPILESHADER;
  instruction->commands.glcompileshader.shader_index = index;
  registers[REGIP].ulong += INS_GLCOMPILESHADER_SIZE;
  RETURN_NEXT();
}

uint64_t MetisVM::add_glcreateprogram(metisgl_identifier program_index) {
  CHECK_INSTRUCTION(INS_GLCREATEPROGRAM_SIZE);

  MetisInstruction *instruction                       = (MetisInstruction *)registers[REGIP].ulong;
  instruction->type                                   = INS_GLCREATEPROGRAM;
  instruction->commands.glcreateprogram.program_index = program_index;
  registers[REGIP].ulong += INS_GLCREATEPROGRAM_SIZE;
  RETURN_NEXT();
}



uint64_t MetisVM::add_glattachshader(metisgl_identifier program_index,
                                     metisgl_identifier shader_index) {
  CHECK_INSTRUCTION(INS_GLATTACHSHADER_SIZE);

  MetisInstruction *instruction                      = (MetisInstruction *)registers[REGIP].ulong;
  instruction->type                                  = INS_GLATTACHSHADER;
  instruction->commands.glattachshader.program_index = program_index;
  instruction->commands.glattachshader.shader_index  = shader_index;
  registers[REGIP].ulong += INS_GLATTACHSHADER_SIZE;
  RETURN_NEXT();
}

uint64_t MetisVM::add_gllinkprogram(metisgl_identifier program_index) {
  CHECK_INSTRUCTION(INS_GLLINKPROGRAM_SIZE);

  MetisInstruction *instruction                       = (MetisInstruction *)registers[REGIP].ulong;
  instruction->type                                   = INS_GLLINKPROGRAM;
  instruction->commands.gllinkprogram.program_index   = program_index;
  registers[REGIP].ulong += INS_GLLINKPROGRAM_SIZE;
  RETURN_NEXT();
}

uint64_t MetisVM::add_gldetachshader(metisgl_identifier program_index,
                                     metisgl_identifier shader_index) {
  CHECK_INSTRUCTION(INS_GLDETACHSHADER_SIZE);

  MetisInstruction *instruction                      = (MetisInstruction *)registers[REGIP].ulong;
  instruction->type                                  = INS_GLDETACHSHADER;
  instruction->commands.gldetachshader.program_index = program_index;
  instruction->commands.gldetachshader.shader_index  = shader_index;
  registers[REGIP].ulong += INS_GLDETACHSHADER_SIZE;
  RETURN_NEXT();
}

uint64_t MetisVM::add_gldeleteshader(metisgl_identifier shader_index) {
  CHECK_INSTRUCTION(INS_GLDELETESHADER_SIZE);

  MetisInstruction *instruction                       = (MetisInstruction *)registers[REGIP].ulong;
  instruction->type                                   = INS_GLDELETESHADER;
  instruction->commands.gldeleteshader.shader_index   = shader_index;
  registers[REGIP].ulong += INS_GLDELETESHADER_SIZE;
  RETURN_NEXT();
}


uint64_t MetisVM::add_gluseprogram(metisgl_identifier program_index) {
  CHECK_INSTRUCTION(INS_GLUSEPROGRAM_SIZE);

  MetisInstruction *instruction                       = (MetisInstruction *)registers[REGIP].ulong;
  instruction->type                                   = INS_GLUSEPROGRAM;
  instruction->commands.gluseprogram.program_index   = program_index;
  registers[REGIP].ulong += INS_GLUSEPROGRAM_SIZE;
  RETURN_NEXT();
}


uint64_t MetisVM::add_gluniformfv(address_mode src, metisgl_identifier uniform_index) {
  CHECK_INSTRUCTION(INS_GLUNIFORMFV_SIZE);

  MetisInstruction *instruction                                = (MetisInstruction *)registers[REGIP].ulong;
  instruction->type                                            = INS_GLUNIFORMFV;
  instruction->commands.extended.addr_mode                     = BUILD_ADDR(src, 0);
  instruction->commands.extended.ext.gluniformfv.uniform_index      = uniform_index;
  registers[REGIP].ulong += INS_GLUNIFORMFV_SIZE;
  RETURN_NEXT();
}

uint64_t MetisVM::add_gluniformiv(address_mode src, metisgl_identifier uniform_index) {
  CHECK_INSTRUCTION(INS_GLUNIFORMIV_SIZE);

  MetisInstruction *instruction                                = (MetisInstruction *)registers[REGIP].ulong;
  instruction->type                                            = INS_GLUNIFORMIV;
  instruction->commands.extended.addr_mode                     = BUILD_ADDR(src, 0);
  instruction->commands.extended.ext.gluniformiv.uniform_index          = uniform_index;
  registers[REGIP].ulong += INS_GLUNIFORMIV_SIZE;
  RETURN_NEXT();
}

uint64_t MetisVM::add_gluniformuiv(address_mode src, metisgl_identifier uniform_index) {
  CHECK_INSTRUCTION(INS_GLUNIFORMUIV_SIZE);

  MetisInstruction *instruction                                = (MetisInstruction *)registers[REGIP].ulong;
  instruction->type                                            = INS_GLUNIFORMUIV;
  instruction->commands.extended.addr_mode                     = BUILD_ADDR(src, 0);
  instruction->commands.extended.ext.gluniformuiv.uniform_index         = uniform_index;
  registers[REGIP].ulong += INS_GLUNIFORMUIV_SIZE;
  RETURN_NEXT();
}

uint64_t MetisVM::add_gluniformmatrixfv(address_mode src, metisgl_identifier uniform_index) {
  CHECK_INSTRUCTION(INS_GLUNIFORMMATRIXFV_SIZE);

  MetisInstruction *instruction                                = (MetisInstruction *)registers[REGIP].ulong;
  instruction->type                                            = INS_GLUNIFORMMATRIXFV;
  instruction->commands.extended.addr_mode                     = BUILD_ADDR(src, 0);
  instruction->commands.extended.ext.gluniformmatrixfv.uniform_index    = uniform_index;
  registers[REGIP].ulong += INS_GLUNIFORMMATRIXFV_SIZE;
  RETURN_NEXT();
}

uint64_t MetisVM::add_glgetuniformlocation(metisgl_identifier program_index,
                                           metisgl_identifier uniform_index, 
                                           const char *uniform_name) {
  CHECK_INSTRUCTION(INS_GLGETUNIFORMLOCATION_SIZE);
  CHECK_POINTER(uniform_name);

  uint64_t length = strlen(uniform_name)+1;

  if(length>255) {
    throw MetisException(string("uniform location too big (255 byte string max) uniform = ")+uniform_name,__LINE__,__FILE__);
  }
    
  MetisInstruction *instruction             = (MetisInstruction *)registers[REGIP].ulong;
  instruction->type                         = INS_GLGETUNIFORMLOCATION;      
  instruction->commands.glgetuniformlocation.program_index = program_index;
  instruction->commands.glgetuniformlocation.uniform_index = uniform_index;
  instruction->commands.glgetuniformlocation.id_length = length;
  if (registers[REGIP].ulong + length > (uint64_t)code_end) {
    throw MetisException(string("out of memory -- (add_glgetuniform) uniform = ") + uniform_name,__LINE__,__FILE__);
  }
  registers[REGIP].ulong += INS_GLGETUNIFORMLOCATION_SIZE;

  memcpy((void *)registers[REGIP].ulong,uniform_name,length);
  registers[REGIP].ulong += length;

  RETURN_NEXT();
}


uint64_t MetisVM::add_glgentextures(GLsizei num_identifiers, 
                                    metisgl_identifier start_index) {
  CHECK_INSTRUCTION(INS_GLGENTEXTURES_SIZE);

  MetisInstruction *instruction                         = (MetisInstruction *)registers[REGIP].ulong;
  instruction->type                                     = INS_GLGENTEXTURES;
  instruction->commands.glgentextures.num_identifiers    = num_identifiers;
  instruction->commands.glgentextures.start_index        = start_index;
  registers[REGIP].ulong += INS_GLGENTEXTURES_SIZE;
  RETURN_NEXT();
}

uint64_t  MetisVM::add_glbindtexture(GLenum target, 
                           metisgl_identifier texture_index) {
  CHECK_INSTRUCTION(INS_GLBINDTEXTURE_SIZE);

  MetisInstruction *instruction                          = (MetisInstruction *)registers[REGIP].ulong;
  instruction->type                                      = INS_GLBINDTEXTURE;
  instruction->commands.glbindtexture.target             = target;
  instruction->commands.glbindtexture.texture_index      = texture_index;
  registers[REGIP].ulong += INS_GLBINDTEXTURE_SIZE;
  RETURN_NEXT();
}

uint64_t  MetisVM::add_gltexparameteri(GLenum target, GLenum pname, GLint param) {
  CHECK_INSTRUCTION(INS_GLTEXPARAMETERI_SIZE);

  MetisInstruction *instruction                          = (MetisInstruction *)registers[REGIP].ulong;
  instruction->type                                      = INS_GLTEXPARAMETERI;
  instruction->commands.gltexparameteri.target           = target;
  instruction->commands.gltexparameteri.pname            = pname;
  instruction->commands.gltexparameteri.param            = param;
  registers[REGIP].ulong += INS_GLTEXPARAMETERI_SIZE;
  RETURN_NEXT();
}
uint64_t  MetisVM::add_gltexparameterfv(address_mode src, GLenum target, GLenum pname) {
  CHECK_INSTRUCTION(INS_GLTEXPARAMETERFV_SIZE);

  MetisInstruction *instruction                              = (MetisInstruction *)registers[REGIP].ulong;
  instruction->type                                          = INS_GLTEXPARAMETERFV;
  instruction->commands.extended.addr_mode                   = BUILD_ADDR(src, 0);
  instruction->commands.extended.ext.gltexparameterfv.target = target;
  instruction->commands.extended.ext.gltexparameterfv.pname  = pname;
  registers[REGIP].ulong += INS_GLTEXPARAMETERFV_SIZE;
  RETURN_NEXT();
}
uint64_t  MetisVM::add_glgeneratemipmap(GLenum target) {
  CHECK_INSTRUCTION(INS_GLGENERATEMIPMAP_SIZE);

  MetisInstruction *instruction                 = (MetisInstruction *)registers[REGIP].ulong;
  instruction->type                             = INS_GLGENERATEMIPMAP;
  instruction->commands.glgeneratemipmap.target = target;
  registers[REGIP].ulong += INS_GLGENERATEMIPMAP_SIZE;
  RETURN_NEXT();
}

uint64_t  MetisVM::add_glteximage2d(GLenum target, GLint level, GLint internal_format,
                          GLsizei width, GLsizei height, GLint border,
                          GLenum format, GLenum type, uint64_t data_index) {
  CHECK_INSTRUCTION(INS_GLTEXIMAGE2D_SIZE);

  MetisInstruction *instruction                        = (MetisInstruction *)registers[REGIP].ulong;
  instruction->type                                    = INS_GLTEXIMAGE2D;
  instruction->commands.glteximage2d.target            = target;
  instruction->commands.glteximage2d.level             = level;
  instruction->commands.glteximage2d.internal_format   = internal_format;
  instruction->commands.glteximage2d.width             = width;
  instruction->commands.glteximage2d.height            = height;
  instruction->commands.glteximage2d.border            = border;
  instruction->commands.glteximage2d.format            = format;
  instruction->commands.glteximage2d.type              = type;
  instruction->commands.glteximage2d.data_index        = data_index;
  registers[REGIP].ulong += INS_GLTEXIMAGE2D_SIZE;
  RETURN_NEXT();
}

uint64_t  MetisVM::add_glgetattriblocation(metisgl_identifier attrib_index, const char *attrib_name) {
  CHECK_INSTRUCTION(INS_GLGETATTRIBLOCATION_SIZE);
  CHECK_POINTER(attrib_name);

  uint64_t length = strlen(attrib_name)+1;

  if(length>255) {
    throw MetisException(string("attribute location too big (255 byte string max) uniform = ")+attrib_name,__LINE__,__FILE__);
  }
    
  MetisInstruction *instruction             = (MetisInstruction *)registers[REGIP].ulong;
  instruction->type                         = INS_GLGETATTRIBLOCATION;      
  instruction->commands.glgetattriblocation.attrib_index = attrib_index;
  instruction->commands.glgetattriblocation.id_length = length;
  if (registers[REGIP].ulong + length > (uint64_t)code_end) {
    throw MetisException(string("out of memory -- (add_glgetattriblocation) uniform = ") + attrib_name,__LINE__,__FILE__);
  }
  registers[REGIP].ulong += INS_GLGETATTRIBLOCATION_SIZE;

  memcpy((void *)registers[REGIP].ulong,attrib_name,length);
  registers[REGIP].ulong += length;

  RETURN_NEXT();

}

uint64_t  MetisVM::add_glactivetexture(GLenum texture) {
  CHECK_INSTRUCTION(INS_GLACTIVETEXTURE_SIZE);

  MetisInstruction *instruction                 = (MetisInstruction *)registers[REGIP].ulong;
  instruction->type                             = INS_GLACTIVETEXTURE;
  instruction->commands.glactivetexture.texture = texture;
  registers[REGIP].ulong += INS_GLACTIVETEXTURE_SIZE;
  RETURN_NEXT();
}


uint64_t  MetisVM::add_glclear(GLbitfield flags) {
  CHECK_INSTRUCTION(INS_GLCLEAR_SIZE);

  MetisInstruction *instruction                 = (MetisInstruction *)registers[REGIP].ulong;
  instruction->type                             = INS_GLCLEAR;
  instruction->commands.glclear.flags           = flags;
  registers[REGIP].ulong += INS_GLCLEAR_SIZE;
  RETURN_NEXT();
}
uint64_t  MetisVM::add_glclearcolor(GLclampf r, GLclampf g, GLclampf b,GLclampf a) {
  CHECK_INSTRUCTION(INS_GLCLEARCOLOR_SIZE);

  MetisInstruction *instruction                 = (MetisInstruction *)registers[REGIP].ulong;
  instruction->type                             = INS_GLCLEARCOLOR;
  instruction->commands.glclearcolor.r = r;
  instruction->commands.glclearcolor.g = g;
  instruction->commands.glclearcolor.b = b;
  instruction->commands.glclearcolor.a = a;
  registers[REGIP].ulong += INS_GLCLEARCOLOR_SIZE;
  RETURN_NEXT();
}
