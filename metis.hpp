#ifndef METIS_VM
#define METIS_VM

//    TODO: 
// *  1. add build system
// *  2. add minimal gl subset...
//    3. open window, draw something
//    4. matrix ops
//    5. improve load/save

#include <exception>
#include <stdexcept>
#include <sstream>
#include <fstream>
#include <unordered_map>
#include <memory>

#include <cstdint>
#include <cstdio>
#include <cstring>

#define GL_GLEXT_PROTOTYPES
#include <GL/gl.h>
#include <GL/glext.h>
#include <GLFW/glfw3.h>

using namespace std;

#define MAX_LABEL_LEN             128
#define ADDR_MODES                instruction->commands.extended.addr_mode

#define BUILD_ADDR(src, dest)     ((dest << 4) + src)
#define GET_DEST(location)        (location >> 4)
#define GET_SRC(location)         (location & 0x0F)
#define GET_LABEL(instruction)    ((char *)(&instruction->type)+1)

#define INS_ERROR_SIZE    1
#define INS_JUMP_SIZE     2 
#define INS_JUMPI_SIZE    9 
#define INS_JIZZ_SIZE     2 
#define INS_JNZ_SIZE      2
#define INS_JNE_SIZE      10 
#define INS_JMPE_SIZE     10
#define INS_STORE_SIZE    2
#define INS_STOREI_SIZE   10

#define INS_MATH_SIZE     2

#define INS_GLDRAWELEMENTS_SIZE              1+(sizeof(GLenum)*2)+sizeof(GLsizei)+sizeof(GLvoid *)
#define INS_GLDRAWARRAYS_SIZE                1+sizeof(GLenum)+sizeof(GLint)+sizeof(GLsizei)
#define INS_GLGENBUFFERS_SIZE                1+sizeof(GLsizei)+sizeof(GLuint)
#define INS_GLBINDBUFFER_SIZE                1+sizeof(GLenum)+sizeof(GLuint)
#define INS_GLBUFFERDATA_SIZE                1+sizeof(GLuint)
#define INS_GLENABLEVERTEXATTRIBARRAY_SIZE   1+sizeof(GLuint)
#define INS_GLVERTEXATTRIBPOINTER_SIZE       1+sizeof(GLuint)+sizeof(GLint)+sizeof(GLenum)+\
                                               sizeof(GLboolean)+sizeof(GLsizei)+sizeof(GLvoid *)
#define INS_GLDISABLEVERTEXATTRIBARRAY_SIZE  1+sizeof(GLuint)

#define INS_LOG_SIZE                         1 
#define INS_DATA_SIZE                        9 

#define INS_END_SIZE                         1


//#define ADVANCE(extended, data) (32)
//#define ADVANCE(extended, data)   sizeof(MetisInstruction)                 
#define ADVANCE(extended, data)   (1+extended+data)


#define CHECK_INSTRUCTION(instruction_length) \
  if ((uint8_t *)(registers[REGIP] + instruction_length) > end) { \
    throw MetisException("attempt to add instruction past address space"); \
  } 

#define CHECK_LOCATION(location) \
  if (location > (uint64_t)(end-start-1)) { \
    throw MetisException("attempt to use location outside address space"); \
  }

#define CHECK_POINTER(pointer) \
  if (pointer == NULL) { \
    throw MetisException("null pointer"); \
  }

#define MATH_OPERATION(op)        set_val(ADDR_MODES, \
                                          get_dest_val(ADDR_MODES) op \
                                          get_val(ADDR_MODES)); \
                                  registers[REGIP] += INS_MATH_SIZE;

#define MATH_METHOD(method_name,byte_code) void method_name(address_mode src, address_mode dest) { \
      MetisInstruction *instruction                   = (MetisInstruction *)registers[REGIP]; \
      instruction->type                               = byte_code; \
      instruction->commands.extended.addr_mode        = BUILD_ADDR(src, dest); \
      registers[REGIP] += ADVANCE(1, 0); \
    };

#define METIS_NUM_BUFFERS 16

// addressing modes
enum address_mode: uint8_t {REGA                    =    0,
                            REGB                    =    1,
                            REGC                    =    2,
                            REGD                    =    3,
                            REGSP                   =    4,  // stack pointer
                            REGIP                   =    5,  // instruction pointer
                            REGERR                  =    6,
                            STACK_PUSH              =    8,
                            STACK_POP               =    9 };

class MetisException: public runtime_error {
  public:
    MetisException(const char *error): runtime_error("Metis VM"), error_str(error) {};
  private:  
    const char *error_str;
    virtual const char* what() const throw()
    {
      cnvt.str("");
      cnvt << runtime_error::what() << ": " << error_str;
      return cnvt.str().c_str();
    }
    static ostringstream cnvt;
};

void error_callback(int error, const char* description);
   
class MetisContext {
  public:
    MetisContext() {
      if(!glfwInit()) {
        printf("glfwInit failed\n");
      }
      printf("MetisVM: startup\n");
      glfwSetErrorCallback(error_callback); 
    }
    GLFWwindow *create_window(uint32_t window_id, 
                  uint32_t width, uint32_t height, 
                  const char *title, GLFWmonitor *monitor, GLFWwindow *share) {
      if ((window_id<0)||(window_id>7)) {
        throw MetisException("invalid window id");
      }

      windows[window_id] = glfwCreateWindow(width, height, title, monitor, share);

      if(!windows[window_id]) {
        glfwTerminate();
        throw MetisException("unable to open window");
      }
      return windows[window_id];
    }

    GLFWwindow *current_window(uint32_t window_id) {
      if (!(windows[window_id])) {
        glfwTerminate();
        throw MetisException("current window not valid");
      }
      glfwMakeContextCurrent(windows[window_id]);
      cur_window = window_id;
      return windows[window_id];
    }
        
    ~MetisContext() {
      glfwTerminate();
    }
  private:
    GLFWwindow *windows[8];
    uint32_t cur_window; 
};

class MetisVM {
  private:
    // first, the list of instructions...
    enum instruction: uint8_t {INS_ERROR                         =    0,   //     should never happen
                               INS_JUMP                          =    1,   // *   jump to index ...
                               INS_JUMPI                         =    2,   //     jump to immediate index
                               INS_JIZZ                          =    3,   // *   jump to index ... if zero
                               INS_JNZ                           =    4,   // *   jump to index ... if zero
                               INS_JNE                           =    5,   // *   jump if not equal
                               INS_JMPE                          =    6,   // *   jump if equal
                               INS_STORE                         =    7,   // *   store ... into stack offset #...
                               INS_STOREI                        =    8,   // *   store immediate value into 

                               // Math
                               INS_INC                           =    9,   // *   increment ... 
                               INS_DEC                           =   10,   // *   decrement ... 
                               INS_ADD                           =   11,   // *   A = A+...  (integer)
                               INS_SUB                           =   12,   // *   A = A-...  (integer)
                               INS_MUL                           =   13,   // *   A = A*...  (integer)
                               INS_DIV                           =   14,   // *   A = A/...  (integer)
                               INS_MOD                           =   15,   // *   A = A%...  (integer)

                               // Bitwise
                               INS_AND                           =   16,   // *   A = A&...  (integer) 
                               INS_OR                            =   17,   // *   A = A|...  (integer) 
                               INS_XOR                           =   18,   // *   A = A^...  (integer) 
                               INS_NOT                           =   19,   // *   A = A&...  (integer) 

                               // GL Instructions
                               INS_GLDRAWELEMENTS                =   32,   
                               INS_GLDRAWARRAYS                  =   34,   
                               INS_GLGENBUFFERS                  =   35,   
                               INS_GLBINDBUFFER                  =   36,   
                               INS_GLBUFFERDATA                  =   37,   
                               INS_GLENABLEVERTEXATTRIBARRAY     =   38,   
                               INS_GLVERTEXATTRIBPOINTER         =   39,   
                               INS_GLDISABLEVERTEXATTRIBARRAY    =   40,   
 
                               INS_LOG                           =  192,   //     log string pointed at by command
                               INS_DATA                          =  193,   //     global data

                               INS_END                           =  255,   //     End Program 
                               };
  public:
    // simple reset, do not remove existing code
    void reset(void) {
      registers[REGA]   = 0;
      registers[REGB]   = 0;
      registers[REGC]   = 0;
      registers[REGD]   = 0;
      registers[REGSP]  = 0;
      registers[REGIP]  = (uint64_t)start;
      registers[REGERR] = 0;
    };

    void hard_reset(void) {
      labels.empty();
      memset(start,0,end-start);
      reset();
    }

    MetisVM(uint8_t *instruction_loc, uint64_t instruction_len, 
            uint64_t *stack_loc, uint64_t stack_len,
            uint8_t *glbuffer_loc, uint64_t glbuffer_len) { 
      start                 = instruction_loc;
      end                   = instruction_loc+instruction_len;
      stack                 = stack_loc;
      stack_size            = stack_len;
      buffer                = glbuffer_loc;
      buffer_size           = glbuffer_len;
      buffer_end            = glbuffer_loc;
      numcommands           = 0;
      reset();
    }

    void     add_end       (void);   
    void     add_jump      (address_mode src);
    void     add_jumpi     (uint64_t location);
    void     add_jizz      (address_mode src, address_mode dest);
    void     add_jnz       (address_mode src, address_mode dest);
    void     add_jne       (address_mode src, address_mode dest, uint64_t location);
    void     add_jmpe      (address_mode src, address_mode dest, uint64_t location);
    void     add_store     (address_mode src, address_mode dest);
    void     add_storei    (address_mode dest, uint64_t value);
    uint64_t add_label_ip  (const char *label);
    uint64_t add_label_val (const char *label, uint64_t val);

    // data gets mixed in with the instructions
    void     add_data      (const uint8_t *data, const uint64_t length, const char *label);

    // buffer gets made into a gl buffer, stored separately.
    void     add_buffer    (const uint8_t *buffer, const uint64_t length, const char *label);

    MATH_METHOD(add_inc, INS_INC); 
    MATH_METHOD(add_dec, INS_DEC);
    MATH_METHOD(add_add, INS_ADD); 
    MATH_METHOD(add_sub, INS_SUB); 
    MATH_METHOD(add_mul, INS_MUL); 
    MATH_METHOD(add_div, INS_DIV); 
    MATH_METHOD(add_mod, INS_MOD); 
    MATH_METHOD(add_and, INS_AND); 
    MATH_METHOD(add_or,  INS_OR); 
    MATH_METHOD(add_xor, INS_XOR); 

    void add_not(address_mode src, address_mode dest);
    void add_gldrawelements(GLenum mode, GLsizei count, 
                            GLenum type, GLvoid *indices);
    void add_gldrawarrays(GLenum mode, GLint first, GLsizei count);
    void add_glgenbuffers(GLsizei num_buffers, GLuint start_index);
    void add_glbindbuffer(GLenum target, GLuint buffer_index);
    void add_glbufferdata(GLenum target, GLsizeiptr size, GLvoid *data, GLenum usage);
    void add_glenablevertexattribarray(GLuint index);
    void add_glvertexattribpointer(GLuint index, GLint size, 
                                 GLenum type, GLboolean normalized, 
                                 GLsizei stride, GLvoid *pointer);
    void add_gldisablevertexattribarray(GLuint index);

    void save(const string &filename);
    void load(const string &filename);           
            
            
    bool eval() {
      reset();
      uint64_t advance;
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
          case INS_END:
            // don't advance, then we can add instructions over
            // the end instruction...
            return true;
            break;
          case INS_ERROR:
          default:
            return false;
            break;
        };
      };
      return false;
    };


    uint64_t get_label(const char *label) {
      return labels.at(label);
    }

    uint8_t  *get_ptr_from_label (const char *label) {
      return (uint8_t *)(start + get_label(label));
    }

    uint8_t  *get_bufloc_from_label (const char *label) {
      return (uint8_t *)(buffer + get_label(label));
    }
    uint64_t *get_registers  (void)  { return registers; };

    uint64_t  cur_stack_val  (void)  {
      if ( registers[REGSP] > 0) {
        return stack[registers[REGSP]-1]; 
      } else {
        throw MetisException("stack empty (cur_stack_val)");
      }
    }
    uint64_t  cur_stack_size (void)  { return registers[REGSP]; };
  
  private:
    uint64_t    registers[8];
    uint8_t     isizes[256];
    uint64_t    *stack;
    uint64_t    stack_size;

    GLuint      buffers[METIS_NUM_BUFFERS];
    unordered_map<string, uint64_t> labels;

    uint64_t    numcommands;
    
    uint8_t    *start;
    uint8_t    *end;

    uint8_t    *buffer;      
    uint64_t    buffer_size;
    uint8_t    *buffer_end;

    struct MetisInstruction {
      instruction type;
      union commands_t {
        struct extended_t {
          uint8_t addr_mode;
          union ext_t {
            struct ext_jumpi_t {
              uint64_t value;
            }__attribute__((packed))jumpi;

            struct ext_storei_t {
              uint64_t value;
            }__attribute__((packed))storei;

            struct ext_jne_t {
              uint64_t value;
            }__attribute__((packed)) jne;

            struct ext_jmpe_t {
              uint64_t value;
            }__attribute__((packed)) jmpe;

          }__attribute__((packed))ext; 
        }__attribute__((packed)) extended;

        struct gldrawelements_t {
          GLenum mode;
          GLsizei count;
          GLenum type;
          GLvoid * indices;
        }__attribute__((packed)) gldrawelements;

        struct gldrawarrays_t {
          GLenum mode;
          GLint first;
          GLsizei count;
        }__attribute__((packed))gldrawarrays;

        struct glgenbuffers_t {
          GLsizei num_buffers;
          GLuint  start_index;
        }__attribute__((packed))glgenbuffers;

        struct glbindbuffer_t {
          GLenum target;
          GLuint buffer_index;
        }__attribute__((packed))glbindbuffer;

        struct glbufferdata_t {
          GLenum target;
          GLsizeiptr size;
          GLvoid *data; 
          GLenum usage;
        }__attribute__((packed))glbufferdata;


        struct glenablevertexattribarray_t {
          GLuint index;
        }__attribute__((packed))glenablevertexattribarray;

        struct glvertexattribpointer_t {
          GLuint index;
          GLint size;
          GLenum type;
          GLboolean normalized;
          GLsizei stride;
          GLvoid *pointer;
        }__attribute__((packed))glvertexattribpointer;

        struct gldisablevertexattribarray_t {
          GLuint index;
        }__attribute__((packed))gldisablevertexattribarray;

        struct jumpi_t {
          uint64_t value;
        }__attribute__((packed)) jumpi;

        struct push_t {
          uint64_t value;
        }__attribute__((packed)) push;

        struct log_t {
          uint8_t length;
        }__attribute__((packed)) log;

        struct data_t {
          uint64_t length;
        }__attribute__((packed)) data;

      }__attribute__((packed)) commands;
    }__attribute__((packed));
    void push(uint64_t val) {
      if( registers[REGSP] >= stack_size) {
        throw MetisException("stack full (push)");
      }
      stack[registers[REGSP]] = val;
      registers[REGSP] += 1;
    }

    uint64_t pop() {
      if(registers[REGSP] == 0) {
        throw MetisException("stack empty (pop)");
      }
      registers[REGSP] -= 1;
      return stack[registers[REGSP]];
    }

    void set_val(uint8_t location, uint64_t value) {
      // high bits are destination
      location = GET_DEST(location);
      switch (location) {
        case REGA:
        case REGB:
        case REGC:
        case REGD:
        case REGSP:
        case REGERR:
        case REGIP:
          registers[location] = value;
          break;
        case STACK_PUSH:
          push(value);
          break;
        default:
          throw MetisException("unknown addressing mode (set_val)");
      } 
    }
    uint64_t get_val(uint8_t location) {
      // low bits are source
      location = GET_SRC(location);
      switch (location) {
        case REGA:
        case REGB:
        case REGC:
        case REGD:
        case REGSP:
        case REGERR:
        case REGIP:
          return registers[location];
          break;
        case STACK_POP:
          return pop();
          break;
        default:
          throw MetisException("unknown addressing mode (get_val)");
          break;
      }
    }     
    uint64_t get_dest_val(uint8_t location) {
      // low bits are source
      location = location >> 4;
      switch (location) {
        case REGA:
        case REGB:
        case REGC:
        case REGD:
        case REGSP:
        case REGERR:
        case REGIP:
          return registers[location];
          break;
        case STACK_POP:
          return pop();
          break;
        default:
          throw MetisException("unknown addressing mode (get_dest_val)");
      }
    }     
      
};


#endif
