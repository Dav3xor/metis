#include <time.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include "metis.hpp"

#define LOAD_MATRIX(type)\
        matrix_a      = (MetisMatrixHeader *)((uint64_t)code_start + get_val(ADDR_MODES));\
        matrix_b      = (MetisMatrixHeader *)((uint64_t)code_start + get_dest_val(ADDR_MODES));\
        destination_matrix  = (MetisMatrixHeader *)((uint64_t)code_start + instruction->commands.extended.ext.type.destination);\
        a = (float *)((uint64_t)code_start + get_val(ADDR_MODES)      + sizeof(MetisMatrixHeader));\
        b = (float *)((uint64_t)code_start + get_dest_val(ADDR_MODES) + sizeof(MetisMatrixHeader));\
        d = (float *)((uint64_t)code_start + instruction->commands.extended.ext.type.destination + sizeof(MetisMatrixHeader));

bool MetisVM::eval(const char *label) {
  reset();
  registers[REGIP] = (uint64_t)get_ptr_from_label(label);
  return do_eval();
}


bool MetisVM::eval() {
  reset();
  return do_eval();
}

bool MetisVM::do_eval() {
  uint64_t advance;
  MetisMatrixHeader *matrix_a;
  MetisMatrixHeader *matrix_b;
  MetisMatrixHeader *destination_matrix;
  float             *a;
  float             *b;
  float             *d;
  uint8_t            i,j,k;
  uint32_t           num_bytes;
  void              *malloc_result;
  GLchar            *string_ptr;
  GLvoid            *glvoid = 0;
  GLint              location;
  FileSpec           *filespec;
  Seek               *seek;
  SelectGroup        *selectgroup;
  int                file;
  int                signed_result;
  timespec           ts1 = {0,0};
  timespec           ts2 = {0,0};
  struct stat        filestat;
  while(registers[REGIP] <= (uint64_t)code_end) {
    MetisInstruction *instruction = (MetisInstruction *)registers[REGIP];
    //printf("--> %u\n", instruction->type);
    switch (instruction->type) {
      // instruction index and stack instructions
      case INS_JUMP:
        registers[REGIP] = (uint64_t)code_start + get_val(ADDR_MODES);
        break;

      case INS_JUMPI:
        registers[REGIP] = (uint64_t)code_start + instruction->commands.jumpi.value;
        break;

      case INS_JNE:
        if(get_val(ADDR_MODES) != get_dest_val(ADDR_MODES)) {
          registers[REGIP] = (uint64_t)code_start + instruction->commands.extended.ext.jne.value;
        } else {
          registers[REGIP] += INS_JNE_SIZE;
        }
        break; 

      case INS_JMPE:
        if(get_val(ADDR_MODES) == get_dest_val(ADDR_MODES)) {
          registers[REGIP] = (uint64_t)code_start + instruction->commands.extended.ext.jmpe.value;
        } else {
          registers[REGIP] += INS_JMPE_SIZE;
        }
        break; 

      case INS_JIZZ:
        if (get_val(ADDR_MODES)==0) {
          registers[REGIP] = (uint64_t)code_start + get_dest_val(ADDR_MODES);
        } else {
          registers[REGIP] += INS_JIZZ_SIZE;
        }
        break;

      case INS_JNZ:
        if (get_val(ADDR_MODES)!=0) {
          registers[REGIP] = (uint64_t)code_start + get_dest_val(ADDR_MODES);
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
                instruction->commands.extended.ext.storei.value.ulong);
        registers[REGIP] += INS_STOREI_SIZE;
        break;

      case INS_STORE_SR:
        stack[registers[REGSP]-instruction->commands.extended.ext.store_sr.offset].whole = get_val(ADDR_MODES);
        registers[REGIP] += INS_STORE_SR_SIZE;
        break;

      case INS_LOAD_SR:
        set_val(ADDR_MODES,
                stack[registers[REGSP]-instruction->commands.extended.ext.store_sr.offset].whole);
        registers[REGIP] += INS_LOAD_SR_SIZE;
        break;

      case INS_STACK_ADJ:
        registers[REGSP] -= instruction->commands.stack_adj.amount;
        registers[REGIP] += INS_STACK_ADJ_SIZE;
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
        matrix_a = (MetisMatrixHeader *)((uint64_t)code_start + instruction->commands.pushmatrix.location);
        num_bytes     = matrix_a->width * matrix_a->height * 4;
        memcpy((char *)&stack[registers[REGSP]], (char *)matrix_a, num_bytes+sizeof(MetisMatrixHeader));
        registers[REGSP] += num_bytes/8 + 1;
        registers[REGIP] += INS_PUSH_MATRIX_SIZE;
        break;

      case INS_MATRIX_MULTIPLY:
        LOAD_MATRIX(matrix_multiply);
        for(i = 0; i < matrix_a->height; i++) {
          for (j = 0; j < matrix_b->width; j++) {
            for (k = 0; k < matrix_a->width; k++) {
              d[destination_matrix->width*i + j] += a[matrix_a->width*i + k] * b[matrix_b->width*k+j];
            }
          }
        }
        registers[REGIP] += INS_MATRIX_MULTIPLY_SIZE;
        break;    

      case INS_MATRIX_ADD:
        // lists of vectors are stored as matrices.
        LOAD_MATRIX(matrix_add);
        for(i = 0; i < matrix_a->height; i++) {
          for (j = 0; j < matrix_a->width; j++) {
            d[i*matrix_a->width+j] = a[i*matrix_a->width+j] + b[i*matrix_a->width+j];
          }
        }
        registers[REGIP] += INS_MATRIX_ADD_SIZE;
        break;    

      case INS_VECTOR_DOT:
        // lists of vectors are stored as matrices.
        LOAD_MATRIX(vector_dot);
        for(i = 0; i < matrix_a->height; i++) {
          for (j = 0; j < matrix_a->width; j++) {
            d[i*destination_matrix->width] += a[i*matrix_a->width+j] * b[i*matrix_a->width+j];
          }
        }
        registers[REGIP] += INS_VECTOR_DOT_SIZE;
        break;    

      case INS_VECTOR_CROSS:
        // lists of vectors are stored as matrices.
        LOAD_MATRIX(vector_cross);
        for(i = 0; i < matrix_a->height; i++) {
          d[i*matrix_a->width+0] = (a[i*matrix_a->width+1] * b[i*matrix_a->width+2]) - (a[i*matrix_a->width+2] * b[i*matrix_a->width+1]); // i
          d[i*matrix_a->width+1] = (a[i*matrix_a->width+2] * b[i*matrix_a->width+0]) - (a[i*matrix_a->width+0] * b[i*matrix_a->width+2]); // j
          d[i*matrix_a->width+2] = (a[i*matrix_a->width+0] * b[i*matrix_a->width+1]) - (a[i*matrix_a->width+1] * b[i*matrix_a->width+0]); // k
        }
        registers[REGIP] += INS_VECTOR_CROSS_SIZE;
        break;  

      case INS_WAIT:
        ts1.tv_nsec = get_val(ADDR_MODES);
        cout << ts1.tv_nsec << endl;
        cout << ts1.tv_sec << endl;
        nanosleep(&ts1, NULL);
        registers[REGIP] += INS_WAIT_SIZE;
        break;
      case INS_CURTIME:
        clock_gettime(CLOCK_REALTIME, &ts2);
        set_val(ADDR_MODES,
                ts2.tv_nsec);
        registers[REGIP] += INS_CURTIME_SIZE;
        break;

      case INS_OPEN:
        filespec = (FileSpec *)((uint64_t)code_start + get_val(ADDR_MODES));

        // currently only works with files...  add sockets later
        file     = open(filespec->path, filespec->file_flags);
        set_val(ADDR_MODES, file);
        registers[REGIP] += INS_OPEN_SIZE;
        break;

      case INS_CLOSE:
        close(get_val(ADDR_MODES));
        registers[REGIP] += INS_CLOSE_SIZE;
        break;

      case INS_READ:
        buffer = (uint8_t *)((uint64_t)code_start + get_dest_val(ADDR_MODES));
        read(get_val(ADDR_MODES), 
             buffer, 
             instruction->commands.extended.ext.read.max_bytes);
        registers[REGIP] += INS_READ_SIZE;
        break;
      
      case INS_WRITE:
        buffer = (uint8_t *)((uint64_t)code_start + get_dest_val(ADDR_MODES));
        write(get_val(ADDR_MODES), 
              buffer, 
              instruction->commands.extended.ext.write.num_bytes);
        registers[REGIP] += INS_WRITE_SIZE;
        break;

      case INS_SEEK:
        seek = (Seek *)((uint64_t)code_start + get_dest_val(ADDR_MODES));

        lseek(get_val(ADDR_MODES), 
              seek->offset, 
              seek->whence);
        registers[REGIP] += INS_SEEK_SIZE;
        break;

      case INS_SELECT:
        selectgroup = (SelectGroup *)((uint64_t)code_start + get_val(ADDR_MODES));

        select(selectgroup->numlines, 
               &selectgroup->descriptors, 
               NULL, NULL, 
               &selectgroup->timeout);
        registers[REGIP] += INS_SELECT_SIZE;
        break;

      case INS_REMOVE:
        string_ptr = (char *)((uint64_t)code_start + get_val(ADDR_MODES));
        if( remove(string_ptr) != 0 ) {
          // error
          set_val(ADDR_MODES, 0);
        } else {
          set_val(ADDR_MODES, 1);
        }
        registers[REGIP] += INS_REMOVE_SIZE;
        break;

      case INS_EXISTS:
        string_ptr = (char *)((uint64_t)code_start + get_val(ADDR_MODES));
        signed_result = stat(string_ptr, &filestat);

        if(signed_result == -1) {
          set_val(ADDR_MODES, 0);
        } else {
          set_val(ADDR_MODES, 1);
        }

        registers[REGIP] += INS_EXISTS_SIZE;
        break;
        
      case INS_MALLOC:
        malloc_result = malloc(get_val(ADDR_MODES));

        // TODO: handle errors

        if (malloc_result == NULL) {
          throw MetisException(string("Malloc returned NULL "),
                               __LINE__, __FILE__);
        } 

        set_val(ADDR_MODES, (uint64_t)malloc_result);

        registers[REGIP] += INS_MALLOC_SIZE;
        break;

      case INS_FREE:
        cout << "addr: " << get_val(ADDR_MODES);
        free((void *)get_val(ADDR_MODES));
        registers[REGIP] += INS_FREE_SIZE;
        break;

      case INS_GLDRAWELEMENTS:
        glvoid = 0;
        //if(instruction->commands.gldrawelements.indices==0) {
          glvoid = (GLvoid *)(buffer + instruction->commands.gldrawelements.indices);
        //}
        glDrawElements(instruction->commands.gldrawelements.mode, 
                       instruction->commands.gldrawelements.count, 
                       instruction->commands.gldrawelements.type, 
                       glvoid);
        #ifdef TESTING_ENVIRONMENT
        print_glerrors(__LINE__,__FILE__);
        #endif
        registers[REGIP] += INS_GLDRAWELEMENTS_SIZE;
        break;

      case INS_GLDRAWARRAYS:
        glDrawArrays(instruction->commands.gldrawarrays.mode, 
                     instruction->commands.gldrawarrays.first, 
                     instruction->commands.gldrawarrays.count);
        #ifdef TESTING_ENVIRONMENT
        print_glerrors(__LINE__,__FILE__);
        #endif
        registers[REGIP] += INS_GLDRAWARRAYS_SIZE;
        break;

      case INS_GLGENBUFFERS:
        glGenBuffers(instruction->commands.glgenbuffers.num_identifiers, 
                     &(glidentifiers[instruction->commands.glgenbuffers.start_index]));
        #ifdef TESTING_ENVIRONMENT
        print_glerrors(__LINE__,__FILE__);
        #endif
        registers[REGIP] += INS_GLGENBUFFERS_SIZE;
        break;

      case INS_GLGENVERTEXARRAYS:
        glGenVertexArrays(instruction->commands.glgenvertexarrays.num_identifiers,
                          &(glidentifiers[instruction->commands.glgenvertexarrays.start_index]));
        #ifdef TESTING_ENVIRONMENT
        print_glerrors(__LINE__,__FILE__);
        #endif
        registers[REGIP] += INS_GLGENVERTEXARRAYS_SIZE;
        break;

      case INS_GLBINDBUFFER:
        glBindBuffer(instruction->commands.glbindbuffer.target, 
                     glidentifiers[instruction->commands.glbindbuffer.buffer_index]);
        #ifdef TESTING_ENVIRONMENT
        print_glerrors(__LINE__,__FILE__);
        #endif
        registers[REGIP] += INS_GLBINDBUFFER_SIZE;
        break;


      case INS_GLBINDVERTEXARRAY:
        glBindVertexArray(glidentifiers[instruction->commands.glbindvertexarray.array_index]);
        #ifdef TESTING_ENVIRONMENT
        print_glerrors(__LINE__,__FILE__);
        #endif
        registers[REGIP] += INS_GLBINDVERTEXARRAY_SIZE;
        break;


      case INS_GLBUFFERDATA:

        glBufferData(instruction->commands.glbufferdata.target, 
                     instruction->commands.glbufferdata.size,
                     buffer+instruction->commands.glbufferdata.data_index,
                     instruction->commands.glbufferdata.usage);
        #ifdef TESTING_ENVIRONMENT
        print_glerrors(__LINE__,__FILE__);
        #endif
        registers[REGIP] += INS_GLBUFFERDATA_SIZE;
        break;

      case INS_GLENABLEVERTEXATTRIBARRAY:
        glEnableVertexAttribArray(instruction->commands.glenablevertexattribarray.index);
        #ifdef TESTING_ENVIRONMENT
        print_glerrors(__LINE__,__FILE__);
        #endif
        registers[REGIP] += INS_GLENABLEVERTEXATTRIBARRAY_SIZE;
        break;

      case INS_GLVERTEXATTRIBPOINTER:
        glvoid = 0;
        if(instruction->commands.glvertexattribpointer.pointer) {
          glvoid = (GLvoid *)(instruction->commands.glvertexattribpointer.pointer);
        }
        glVertexAttribPointer(instruction->commands.glvertexattribpointer.index, 
                              instruction->commands.glvertexattribpointer.size,
                              instruction->commands.glvertexattribpointer.type,
                              instruction->commands.glvertexattribpointer.normalized,
                              instruction->commands.glvertexattribpointer.stride,
                              glvoid);
        #ifdef TESTING_ENVIRONMENT
        print_glerrors(__LINE__,__FILE__);
        #endif
        registers[REGIP] += INS_GLVERTEXATTRIBPOINTER_SIZE;
        break;

      case INS_GLDISABLEVERTEXATTRIBARRAY:
        glDisableVertexAttribArray(instruction->commands.gldisablevertexattribarray.index);
        #ifdef TESTING_ENVIRONMENT
        print_glerrors(__LINE__,__FILE__);
        #endif
        registers[REGIP] += INS_GLDISABLEVERTEXATTRIBARRAY_SIZE;
        break;

      case INS_GLENABLE:
        glEnable(instruction->commands.glenable.capability);
        #ifdef TESTING_ENVIRONMENT
        print_glerrors(__LINE__,__FILE__);
        #endif
        registers[REGIP] += INS_GLENABLE_SIZE;
        break;

      case INS_GLDEPTHFUNC:
        glDepthFunc(instruction->commands.gldepthfunc.function);
        #ifdef TESTING_ENVIRONMENT
        print_glerrors(__LINE__,__FILE__);
        #endif
        registers[REGIP] += INS_GLDEPTHFUNC_SIZE;
        break;

      case INS_GLCREATESHADER:
        glidentifiers[instruction->commands.glcreateshader.start_index] = glCreateShader(instruction->commands.glcreateshader.type);
        #ifdef TESTING_ENVIRONMENT
        print_glerrors(__LINE__,__FILE__);
        #endif
        registers[REGIP] += INS_GLCREATESHADER_SIZE;
        break;

      case INS_GLSHADERSOURCE:
        string_ptr = (GLchar *)(code_start + instruction->commands.glshadersource.shader);
        glShaderSource(glidentifiers[instruction->commands.glshadersource.source_index],
                       1, &string_ptr, NULL);
        #ifdef TESTING_ENVIRONMENT
        print_glerrors(__LINE__,__FILE__);
        #endif
        registers[REGIP] += INS_GLSHADERSOURCE_SIZE;
        break;

      case INS_GLCOMPILESHADER:
        if(!(doCompileShader(instruction->commands.glcompileshader.shader_index))) {
          return false;
        }
        #ifdef TESTING_ENVIRONMENT
        print_glerrors(__LINE__,__FILE__);
        #endif
        registers[REGIP] += INS_GLCOMPILESHADER_SIZE;
        break;

      case INS_GLCREATEPROGRAM:
        glidentifiers[instruction->commands.glcreateprogram.program_index] = glCreateProgram();
        #ifdef TESTING_ENVIRONMENT
        print_glerrors(__LINE__,__FILE__);
        #endif
        registers[REGIP] += INS_GLCREATEPROGRAM_SIZE;
        break;

      case INS_GLATTACHSHADER:
        glAttachShader(glidentifiers[instruction->commands.glattachshader.program_index], 
                       glidentifiers[instruction->commands.glattachshader.shader_index]);

        #ifdef TESTING_ENVIRONMENT
        print_glerrors(__LINE__,__FILE__);
        #endif
        registers[REGIP] += INS_GLATTACHSHADER_SIZE;
        break;

      case INS_GLLINKPROGRAM:
        if(!(doLinkProgram(instruction->commands.gllinkprogram.program_index))) {
          return false;
        }
        #ifdef TESTING_ENVIRONMENT
        print_glerrors(__LINE__,__FILE__);
        #endif
        registers[REGIP] += INS_GLLINKPROGRAM_SIZE;
        break;

      case INS_GLDETACHSHADER:
        glDetachShader(glidentifiers[instruction->commands.gldetachshader.program_index], 
                       glidentifiers[instruction->commands.gldetachshader.shader_index]);
        #ifdef TESTING_ENVIRONMENT
        print_glerrors(__LINE__,__FILE__);
        #endif
        registers[REGIP] += INS_GLDETACHSHADER_SIZE;
        break;

      case INS_GLDELETESHADER:
        glDeleteShader(glidentifiers[instruction->commands.gldeleteshader.shader_index]);
        #ifdef TESTING_ENVIRONMENT
        print_glerrors(__LINE__,__FILE__);
        #endif
        registers[REGIP] += INS_GLDELETESHADER_SIZE;
        break;

      case INS_GLUSEPROGRAM:
        glUseProgram(glidentifiers[instruction->commands.gluseprogram.program_index]);
        #ifdef TESTING_ENVIRONMENT
        print_glerrors(__LINE__,__FILE__);
        #endif
        registers[REGIP] += INS_GLUSEPROGRAM_SIZE;
        break;

      case INS_GLUNIFORMFV:
        // lists of vectors are stored as matrices.
        matrix_a  = (MetisMatrixHeader *)((uint64_t)code_start + get_val(ADDR_MODES));\
        a         = (float *)((uint64_t)matrix_a + sizeof(MetisMatrixHeader));
        location  = glidentifiers[instruction->commands.extended.ext.gluniformfv.uniform_index];
        
        switch(matrix_a->width) {
          case 1:
            glUniform1fv(location,matrix_a->height, a);
            break;
          case 2:
            glUniform2fv(location,matrix_a->height, a);
            break;
          case 3:
            glUniform3fv(location,matrix_a->height, a);
            break;
          case 4:
            glUniform4fv(location,matrix_a->height, a);
            break;
          default:
            throw MetisException(string("illegal glUniform vector size -- width = ") + 
                                 to_string(matrix_a->width) + string(" height = ") + to_string(matrix_a->height) , 
                                 __LINE__, __FILE__);
            break; 
        }
        #ifdef TESTING_ENVIRONMENT
        print_glerrors(__LINE__,__FILE__);
        #endif
        registers[REGIP] += INS_GLUNIFORMFV_SIZE;
        break;

      case INS_GLGETUNIFORMLOCATION:
        string_ptr = (GLchar *)(registers[REGIP] + INS_GLGETUNIFORMLOCATION_SIZE);
        glidentifiers[instruction->commands.glgetuniformlocation.uniform_index] = glGetUniformLocation(glidentifiers[instruction->commands.glgetuniformlocation.program_index],
                                                                                                       (const GLchar *)string_ptr);

        registers[REGIP] += INS_GLGETUNIFORMLOCATION_SIZE;
        registers[REGIP] += instruction->commands.glgetuniformlocation.id_length;
        break;
        
      case INS_GLGENTEXTURES:
        glGenTextures(instruction->commands.glgentextures.num_identifiers,
                      &glidentifiers[instruction->commands.glgentextures.start_index]);
        #ifdef TESTING_ENVIRONMENT
        print_glerrors(__LINE__,__FILE__);
        #endif
        registers[REGIP] += INS_GLGENTEXTURES_SIZE;
        break;

      case INS_GLBINDTEXTURE:
        glBindTexture(instruction->commands.glbindtexture.target,
                      glidentifiers[instruction->commands.glbindtexture.texture_index]);
        #ifdef TESTING_ENVIRONMENT
        print_glerrors(__LINE__,__FILE__);
        #endif
        registers[REGIP] += INS_GLBINDTEXTURE_SIZE;
        break;

      case INS_GLTEXPARAMETERI:
        glTexParameteri(instruction->commands.gltexparameteri.target,
                        instruction->commands.gltexparameteri.pname,
                        instruction->commands.gltexparameteri.param);
        #ifdef TESTING_ENVIRONMENT
        print_glerrors(__LINE__,__FILE__);
        #endif
        registers[REGIP] += INS_GLTEXPARAMETERI_SIZE;
        break;

      case INS_GLTEXPARAMETERFV:
        glTexParameterfv(instruction->commands.gltexparameteri.target,
                        instruction->commands.gltexparameteri.pname,
                        (GLfloat *)get_val(ADDR_MODES));
        #ifdef TESTING_ENVIRONMENT
        print_glerrors(__LINE__,__FILE__);
        #endif
        registers[REGIP] += INS_GLTEXPARAMETERFV_SIZE;
        break;

      case INS_GLGENERATEMIPMAP:
        glGenerateMipmap(instruction->commands.glgeneratemipmap.target);
        #ifdef TESTING_ENVIRONMENT
        print_glerrors(__LINE__,__FILE__);
        #endif
        registers[REGIP] += INS_GLGENERATEMIPMAP_SIZE;
        break;

      case INS_GLTEXIMAGE2D:
        glTexImage2D(instruction->commands.glteximage2d.target,
                     instruction->commands.glteximage2d.level,
                     instruction->commands.glteximage2d.internal_format,
                     instruction->commands.glteximage2d.width,
                     instruction->commands.glteximage2d.height,
                     instruction->commands.glteximage2d.border,
                     instruction->commands.glteximage2d.format,
                     instruction->commands.glteximage2d.type,
                     buffer+instruction->commands.glteximage2d.data_index);
        #ifdef TESTING_ENVIRONMENT
        print_glerrors(__LINE__,__FILE__);
        #endif
        registers[REGIP] += INS_GLTEXIMAGE2D_SIZE;
        break;

      case INS_GLGETATTRIBLOCATION:

        string_ptr = (GLchar *)(registers[REGIP] + INS_GLGETATTRIBLOCATION_SIZE);
        glidentifiers[instruction->commands.glgetattriblocation.attrib_index] = glGetAttribLocation(glidentifiers[instruction->commands.glgetattriblocation.attrib_index],
                                                                                                    (const GLchar *)string_ptr);
        #ifdef TESTING_ENVIRONMENT
        print_glerrors(__LINE__,__FILE__);
        #endif
        registers[REGIP] += INS_GLGETATTRIBLOCATION_SIZE;
        registers[REGIP] += instruction->commands.glgetattriblocation.id_length;
        break;
        
      case INS_GLACTIVETEXTURE:
        glActiveTexture(instruction->commands.glactivetexture.texture);
        #ifdef TESTING_ENVIRONMENT
        print_glerrors(__LINE__,__FILE__);
        #endif
        registers[REGIP] += INS_GLACTIVETEXTURE_SIZE;
        break;

      case INS_GLCLEAR:
        glClear(instruction->commands.glclear.flags);
        #ifdef TESTING_ENVIRONMENT
        print_glerrors(__LINE__,__FILE__);
        #endif
        registers[REGIP] += INS_GLCLEAR_SIZE;
        break;

      case INS_GLCLEARCOLOR:
        glClearColor(instruction->commands.glclearcolor.r,
                     instruction->commands.glclearcolor.g,
                     instruction->commands.glclearcolor.b,
                     instruction->commands.glclearcolor.a);
        #ifdef TESTING_ENVIRONMENT
        print_glerrors(__LINE__,__FILE__);
        #endif
        registers[REGIP] += INS_GLCLEARCOLOR_SIZE;
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
        throw MetisException(string("error opcode found -- instruction = ") + 
                             to_string(instruction->type) + string(" IP = ") + 
                             to_string(registers[REGIP]) , 
                             __LINE__, __FILE__);
        break;

      default:
        throw MetisException(string("unknown opcode found -- instruction = ") + 
                             to_string(instruction->type) + string(" IP = ") + 
                             to_string(registers[REGIP]) , 
                             __LINE__, __FILE__);
        return false;
        break;
    };
  };
  return false;
}



bool MetisVM::doCompileShader(uint16_t index) {
  GLuint shader = glidentifiers[index];

  glCompileShader(shader);

  GLint isCompiled = 0;
  glGetShaderiv(shader, GL_COMPILE_STATUS, &isCompiled);
  if(isCompiled == GL_FALSE) {
    char log[1000];
    GLint len = 0;
    glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &len);

    // The maxLength includes the NULL character
    glGetShaderInfoLog(shader, 1000, &len, log);
    
    printf("MetisVM: Shader Compilation Failed:\n");
    printf("index = %u\n",index);
    printf("-------------------------------------------\n");
    printf("%s\n",log);

    throw MetisException("Shader Compilation Failed.",__LINE__,__FILE__);
    // Exit with failure.
    glDeleteShader(shader); // Don't leak the shader.
    return false;
  }
  return true;
}

bool MetisVM::doLinkProgram(uint16_t index) {
  GLuint program = glidentifiers[index];
  glLinkProgram(3);

  GLint len = 10;
  GLint isLinked = 0;
  GLint maxLength = 999;

  glGetProgramiv(program, GL_LINK_STATUS, &isLinked);
  if(isLinked == GL_FALSE) {
    char log[1000];

    glGetProgramiv(program, GL_INFO_LOG_LENGTH, &len);
    // The maxLength includes the NULL character
    glGetProgramInfoLog(program, maxLength, &maxLength, log);

    printf("Shader Program Link Failed:\n");
    printf("-------------------------------\n");
    printf("%s\n",log);
    printf("-------------------------------\n");

    throw MetisException("Shader Link Failed.",__LINE__,__FILE__);
    // Exit with failure.
    glDeleteProgram(program); // Don't leak the shader.
    return false;
  }
  return true;
} 
