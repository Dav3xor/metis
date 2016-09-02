#include "metis.hpp"

bool MetisVM::eval() {
  reset();
  uint64_t advance;
  MetisMatrixHeader *source1_matrix;
  //MetisMatrixHeader *source2_matrix;
  //MetisMatrixHeader *destination_matrix;
  float             *a;
  float             *b;
  float             *d;
  uint8_t            i,j,k;
  uint32_t           num_bytes;
  while(registers[REGIP] <= (uint64_t)end) {
    MetisInstruction *instruction = (MetisInstruction *)registers[REGIP];
    //printf("--> %u\n", instruction->type);
    switch (instruction->type) {
      // instruction index and stack instructions
      case INS_JUMP:
        registers[REGIP] = (uint64_t)start + get_val(ADDR_MODES);
        break;
      case INS_JUMPI:
        registers[REGIP] = (uint64_t)start + instruction->commands.jumpi.value;
        break;
      case INS_JNE:
        if(get_val(ADDR_MODES) != get_dest_val(ADDR_MODES)) {
          registers[REGIP] = (uint64_t)start + instruction->commands.extended.ext.jne.value;
        } else {
          registers[REGIP] += INS_JNE_SIZE;
        }
        break; 
      case INS_JMPE:
        if(get_val(ADDR_MODES) == get_dest_val(ADDR_MODES)) {
          registers[REGIP] = (uint64_t)start + instruction->commands.extended.ext.jmpe.value;
        } else {
          registers[REGIP] += INS_JMPE_SIZE;
        }
        break; 
      case INS_JIZZ:
        if (get_val(ADDR_MODES)==0) {
          registers[REGIP] = (uint64_t)start + get_dest_val(ADDR_MODES);
        } else {
          registers[REGIP] += INS_JIZZ_SIZE;
        }
        break;
      case INS_JNZ:
        if (get_val(ADDR_MODES)!=0) {
          registers[REGIP] = (uint64_t)start + get_dest_val(ADDR_MODES);
        } else {
          registers[REGIP] += INS_JNZ_SIZE;
        }
        break;
      case INS_STORE:
        set_val(ADDR_MODES,
                get_val(ADDR_MODES));
        registers[REGIP] += INS_STORE_SIZE;
        break;
      case INS_STOREI:
        set_val(ADDR_MODES,
                instruction->commands.extended.ext.storei.value);
        registers[REGIP] += INS_STOREI_SIZE;
        break;
      
      // math instructions
      case INS_INC:
        set_val(ADDR_MODES,
                get_val(ADDR_MODES)+1);
        registers[REGIP] += INS_MATH_SIZE;
        break;
      case INS_DEC:
        set_val(ADDR_MODES,
                get_val(ADDR_MODES)-1);
        registers[REGIP] += INS_MATH_SIZE;
        break;
      case INS_ADD:
        MATH_OPERATION(+);
        break;
      case INS_SUB:
        MATH_OPERATION(-);
        break;
      case INS_MUL:
        MATH_OPERATION(*);
        break;
      case INS_DIV:
        MATH_OPERATION(/);
        break;
      case INS_MOD:
        MATH_OPERATION(%);
        break;
      
      // logical operation          
      case INS_AND:
        MATH_OPERATION(&);
        break;
      case INS_OR:
        MATH_OPERATION(|);
        break;
      case INS_XOR:
        MATH_OPERATION(^);
        break;
      case INS_NOT:
        set_val(ADDR_MODES, ~get_val(ADDR_MODES));
        registers[REGIP] += INS_MATH_SIZE;
        break;

      case INS_PUSH_MATRIX:
        source1_matrix = (MetisMatrixHeader *)((uint64_t)start + instruction->commands.pushmatrix.location);
        num_bytes     = source1_matrix->width * source1_matrix->height * 4;
        memcpy((char *)&stack[registers[REGSP]], (char *)source1_matrix, num_bytes+sizeof(MetisMatrixHeader));
        registers[REGSP] += num_bytes/8 + 1;
        registers[REGIP] += INS_PUSH_MATRIX_SIZE;
        break;

      case INS_MATRIX_MULTIPLY:
        source1_matrix      = (MetisMatrixHeader *)((uint64_t)start + get_val(ADDR_MODES));
        //source2_matrix      = (MetisMatrixHeader *)((uint64_t)start + get_dest_val(ADDR_MODES));
        //destination_matrix  = (MetisMatrixHeader *)((uint64_t)start + instruction->commands.extended.ext.matrix_multiply.destination);
        a = (float *)((uint64_t)start + get_val(ADDR_MODES)      + sizeof(MetisMatrixHeader));
        b = (float *)((uint64_t)stack + get_dest_val(ADDR_MODES) + sizeof(MetisMatrixHeader));
        d = (float *)((uint64_t)stack + instruction->commands.extended.ext.matrix_multiply.destination + sizeof(MetisMatrixHeader));
        printf("%u\n",source1_matrix->height);
        printf("%u\n",source1_matrix->width);
        /*
        for(i = 0; i < source1_matrix->height; i++) {
          for (j = 0; j < source1_matrix->width; j++) {
            for (k = 0; j < source1_matrix->width; j++) {
              d[source1_matrix->height*i + j] += a[source1_matrix->height*i + k] * b[source1_matrix->height*k+j];
            }
          }
        }*/
        registers[REGIP] += INS_MATRIX_MULTIPLY_SIZE;
        break;    
      case INS_GLDRAWELEMENTS:
        glDrawElements(instruction->commands.gldrawelements.mode, 
                       instruction->commands.gldrawelements.count, 
                       instruction->commands.gldrawelements.type, 
                       instruction->commands.gldrawelements.indices);
        registers[REGIP] += INS_GLDRAWELEMENTS_SIZE;
        break;
      case INS_GLDRAWARRAYS:
        glDrawArrays(instruction->commands.gldrawarrays.mode, 
                     instruction->commands.gldrawarrays.first, 
                     instruction->commands.gldrawarrays.count);
        registers[REGIP] += INS_GLDRAWARRAYS_SIZE;
        break;
      case INS_GLGENBUFFERS:
        glGenBuffers(instruction->commands.glgenbuffers.num_buffers, 
                     &(buffers[instruction->commands.glgenbuffers.start_index]));
        registers[REGIP] += INS_GLGENBUFFERS_SIZE;
        break;
      case INS_GLBINDBUFFER:
        glBindBuffer(instruction->commands.glbindbuffer.target, 
                     buffers[instruction->commands.glbindbuffer.buffer_index]);
        registers[REGIP] += INS_GLBINDBUFFER_SIZE;
        break;
        
      case INS_GLBUFFERDATA:
        glBufferData(instruction->commands.glbufferdata.target, 
                     instruction->commands.glbufferdata.size,
                     instruction->commands.glbufferdata.data,
                     instruction->commands.glbufferdata.usage);
        registers[REGIP] += INS_GLBUFFERDATA_SIZE;
        break;
      case INS_GLENABLEVERTEXATTRIBARRAY:
        glEnableVertexAttribArray(instruction->commands.glenablevertexattribarray.index);
        registers[REGIP] += INS_GLENABLEVERTEXATTRIBARRAY_SIZE;
        break;
      case INS_GLVERTEXATTRIBPOINTER:
        glVertexAttribPointer(instruction->commands.glvertexattribpointer.index, 
                              instruction->commands.glvertexattribpointer.size,
                              instruction->commands.glvertexattribpointer.type,
                              instruction->commands.glvertexattribpointer.normalized,
                              instruction->commands.glvertexattribpointer.stride,
                              instruction->commands.glvertexattribpointer.pointer);
        registers[REGIP] += INS_GLVERTEXATTRIBPOINTER_SIZE;
        break;
      case INS_GLDISABLEVERTEXATTRIBARRAY:
        glDisableVertexAttribArray(instruction->commands.gldisablevertexattribarray.index);
        registers[REGIP] += INS_GLDISABLEVERTEXATTRIBARRAY_SIZE;
        break;
        break; 
      case INS_DATA:
        advance = instruction->commands.data.length;
        registers[REGIP] += INS_DATA_SIZE;
        registers[REGIP] += advance;
        break;
      case INS_NOOP:
        registers[REGIP] += INS_NOOP_SIZE;
        break;
      case INS_END:
        // don't advance, then we can add instructions over
        // the end instruction...
        return true;
        break;
      case INS_ERROR:
        throw MetisException("error opcode found", __LINE__, __FILE__);
        break;
      default:
        throw MetisException("unknown opcode found", __LINE__, __FILE__);
        return false;
        break;
    };
  };
  return false;
};
