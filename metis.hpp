#ifndef METIS_VM
#define METIS_VM

#define TESTING_ENVIRONMENT 1

//    TODO: 
// *  1. add build system
// *  2. add minimal gl subset...
// *  3. open window, draw something
// *  4. matrix ops
//    5. organize gl instructions (remove not needed arguments, make indexes 16 bit, etc...)

#include <exception>
#include <stdexcept>
#include <sstream>
#include <fstream>
#include <unordered_map>
#include <memory>
#include <functional>

#include <cstdint>
#include <cstdio>
#include <cstring>

#define GL_GLEXT_PROTOTYPES
#ifdef __APPLE__
  #include <OpenGL/gl3.h>
  #include <OpenGL/glext.h>
  #define GLFW_INCLUDE_GLCOREARB
#else
  #include <GL/gl.h>
  #include <GL/glext.h>
#endif

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

#define INS_GLDRAWELEMENTS_SIZE              1+(sizeof(GLenum)*2)+sizeof(GLsizei)+sizeof(uint64_t )
#define INS_GLDRAWARRAYS_SIZE                1+sizeof(GLenum)+sizeof(GLint)+sizeof(GLsizei)
#define INS_GLGENBUFFERS_SIZE                1+sizeof(GLsizei)+sizeof(metisgl_identifier)
#define INS_GLGENVERTEXARRAYS_SIZE           1+sizeof(GLsizei)+sizeof(metisgl_identifier)
#define INS_GLBINDVERTEXARRAY_SIZE           1+sizeof(metisgl_identifier)
#define INS_GLBINDBUFFER_SIZE                1+sizeof(GLenum)+sizeof(metisgl_identifier)
#define INS_GLBUFFERDATA_SIZE                1+sizeof(GLenum)+sizeof(GLsizeiptr)+sizeof(uint64_t)+sizeof(GLenum)
#define INS_GLENABLEVERTEXATTRIBARRAY_SIZE   1+sizeof(GLuint)
#define INS_GLVERTEXATTRIBPOINTER_SIZE       1+sizeof(GLuint)+sizeof(GLint)+sizeof(GLenum)+\
                                               sizeof(GLboolean)+sizeof(GLsizei)+sizeof(uint64_t )
#define INS_GLDISABLEVERTEXATTRIBARRAY_SIZE  1+sizeof(GLuint)
#define INS_GLENABLE_SIZE                    1+sizeof(GLenum)
#define INS_GLDEPTHFUNC_SIZE                 1+sizeof(GLenum)

#define INS_GLCREATESHADER_SIZE              1+sizeof(GLenum)+sizeof(metisgl_identifier)
#define INS_GLSHADERSOURCE_SIZE              1+sizeof(GLuint)+sizeof(metisgl_identifier)
#define INS_GLCOMPILESHADER_SIZE             1+sizeof(metisgl_identifier)
#define INS_GLCREATEPROGRAM_SIZE             1+sizeof(metisgl_identifier)
#define INS_GLATTACHSHADER_SIZE              1+sizeof(metisgl_identifier) + sizeof(metisgl_identifier)
#define INS_GLLINKPROGRAM_SIZE               1+sizeof(metisgl_identifier)
#define INS_GLDETACHSHADER_SIZE              1+sizeof(metisgl_identifier) + sizeof(metisgl_identifier)
#define INS_GLDELETESHADER_SIZE              1+sizeof(metisgl_identifier)
#define INS_GLUSEPROGRAM_SIZE                1+sizeof(metisgl_identifier)

#define INS_GLUNIFORMFV_SIZE                 2+sizeof(metisgl_identifier)
#define INS_GLUNIFORMIV_SIZE                 2+sizeof(metisgl_identifier)
#define INS_GLUNIFORMUIV_SIZE                2+sizeof(metisgl_identifier)
#define INS_GLUNIFORMMATRIXFV_SIZE           2+sizeof(metisgl_identifier)
#define INS_GLGETUNIFORMLOCATION_SIZE        2+sizeof(metisgl_identifier)+sizeof(metisgl_identifier)

#define INS_GLGENTEXTURES_SIZE               1+sizeof(GLsizei)+sizeof(metisgl_identifier)
#define INS_GLBINDTEXTURE_SIZE               1+sizeof(GLenum)+sizeof(metisgl_identifier)
#define INS_GLTEXPARAMETERI_SIZE             1+sizeof(GLenum)+sizeof(GLenum)+sizeof(GLint)
#define INS_GLTEXPARAMETERFV_SIZE            2+sizeof(GLenum)+sizeof(GLenum)
#define INS_GLGENERATEMIPMAP_SIZE            1+sizeof(GLenum)
#define INS_GLTEXIMAGE2D_SIZE                1+sizeof(GLenum)+sizeof(GLint)+sizeof(GLint)+sizeof(GLsizei)+sizeof(GLsizei)+sizeof(GLint)+sizeof(GLenum)+sizeof(GLenum)+sizeof(uint64_t )
#define INS_GLGETATTRIBLOCATION_SIZE         1+sizeof(metisgl_identifier)
#define INS_GLACTIVETEXTURE_SIZE             1+sizeof(GLenum)

#define INS_LOG_SIZE                         1 
#define INS_DATA_SIZE                        9 
#define INS_PUSH_MATRIX_SIZE                 9
#define INS_MATRIX_MULTIPLY_SIZE             10
#define INS_MATRIX_ADD_SIZE                  10 
#define INS_VECTOR_DOT_SIZE                  10
#define INS_VECTOR_CROSS_SIZE                10
#define INS_NOOP_SIZE                        1
#define INS_END_SIZE                         1


#define ADVANCE(extended, data)   (1+extended+data)



#define MATH_OPERATION(op) \
  set_val(ADDR_MODES, \
          get_dest_val(ADDR_MODES) op \
          get_val(ADDR_MODES)); \
  registers[REGIP] += INS_MATH_SIZE;

#define MATH_METHOD(method_name,byte_code) \
      uint64_t method_name(address_mode src, address_mode dest) { \
        MetisInstruction *instruction                   = (MetisInstruction *)registers[REGIP]; \
        instruction->type                               = byte_code; \
        instruction->commands.extended.addr_mode        = BUILD_ADDR(src, dest); \
        registers[REGIP] += ADVANCE(1, 0); \
        return (uint64_t)code_start-(uint64_t)instruction; \
      };

#define METIS_NUM_BUFFERS 16

// addressing modes
enum address_mode: uint8_t {REGA                    =    0,
                            REGB                    =    1,
                            REGC                    =    2,
                            REGD                    =    3,
                            REGSP                   =    4,  // stack pointer
                            REGIP                   =    5,  // instruction pointer
                            REGBP                   =    6,  // buffer pointer
                            REGERR                  =    7,
                            STACK_PUSH              =    8,
                            STACK_POP               =    9 };

class MasmException: public runtime_error {
  public:
    MasmException(string error, 
                   const int line, 
                   const int column): runtime_error("Metis VM"), 
                                      error_str(error), 
                                      column(column), line(line) {};
  private:  
    string     error_str;
    const int  column;
    const int   line;

    virtual const char* what() const throw()
    {
      cnvt.str("");
      cnvt << runtime_error::what() << ": " << error_str << " -- line: " << line << " column: " << column;
      return cnvt.str().c_str();
    }
    static ostringstream cnvt;
};

class MetisException: public runtime_error {
  public:
    MetisException(string error, 
                   const int line, 
                   const char *file): runtime_error("Metis VM"), 
                                      error_str(error), 
                                      file(file), line(line) {};
  private:  
    string     error_str;
    const char *file;
    const int   line;

    virtual const char* what() const throw()
    {
      cnvt.str("");
      cnvt << runtime_error::what() << ": " << error_str << " -- " << file << ":" << line;
      return cnvt.str().c_str();
    }
    static ostringstream cnvt;
};

void error_callback(int error, const char* description);
void print_glerrors(unsigned int line, const char *file);
void print_matrix(float *matrix, uint8_t width, uint8_t height);

class MetisContext {
  public:
    MetisContext() {
      if(!glfwInit()) {
        printf("glfwInit failed\n");
      }
      glfwWindowHint (GLFW_SAMPLES, 4);
      glfwWindowHint (GLFW_CONTEXT_VERSION_MAJOR, 3);
      glfwWindowHint (GLFW_CONTEXT_VERSION_MINOR, 2);
      glfwWindowHint (GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
      glfwWindowHint (GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

  
      printf("MetisVM: startup\n");
      glfwSetErrorCallback(error_callback); 
      monitor = glfwGetPrimaryMonitor();
    }


    GLFWwindow *create_window(uint32_t window_id, const char *title) {
      if (window_id>7) {
        throw MetisException("invalid window id",__LINE__,__FILE__);
      }

      //const GLFWvidmode *vidmode = glfwGetVideoMode(monitor);
      //windows[window_id] = glfwCreateWindow(vidmode->width, vidmode->height, title, monitor, NULL);
      windows[window_id] = glfwCreateWindow(500,500, title, NULL, NULL);

      if(!windows[window_id]) {
        glfwTerminate();
        throw MetisException("unable to open window",__LINE__,__FILE__);
      }
      return windows[window_id];
    }

    void close_window(uint32_t window_id) {
      glfwDestroyWindow(windows[window_id]);
      windows[window_id] = 0;
    }

    GLFWwindow *current_window(uint32_t window_id) {
      if (!(windows[window_id])) {
        glfwTerminate();
        throw MetisException("current window not valid",__LINE__,__FILE__);
      }
      glfwMakeContextCurrent(windows[window_id]);
      cur_window = window_id;
      return windows[window_id];
    }
        
    ~MetisContext() {
      glfwTerminate();
    }
  private:
    GLFWwindow  *windows[8];
    GLFWmonitor *monitor;
    uint32_t cur_window; 
};

struct MetisMatrixHeader {
  uint8_t   width;
  uint8_t   height;
  uint16_t  filler;       // remove me when this grows...
}__attribute__((packed));

union MetisMemoryCell {
  uint8_t             ubytes[8];
  uint16_t            ushorts[4];
  uint32_t            uints[2];
  uint64_t            ulong;
  
  int8_t              bytes[8];
  int16_t             shorts[4];
  int32_t             ints[2];
  int64_t             whole;
  
  float               floats[2];
  double              whole_double;

  MetisMatrixHeader   matrix;
}__attribute__((packed));

typedef uint16_t metisgl_identifier;

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
                               
                               // Matrix Ops
                               INS_MATRIX_MULTIPLY               =   20,   // *   Matrix Multiplication
                               INS_PUSH_MATRIX                   =   21,   //     Push Matrix onto stack
                               
                               // Vector Ops                    
                               INS_VECTOR_DOT                    =   25,   // *   Dot Product
                               INS_VECTOR_CROSS                  =   26,   // *   Cross Product
                               INS_MATRIX_ADD                    =   27,   // *   Cross Product

                               // GL Instructions
                               INS_GLDRAWELEMENTS                =   32,   
                               INS_GLDRAWARRAYS                  =   34,   
                               INS_GLGENBUFFERS                  =   35,   
                               INS_GLBINDBUFFER                  =   36,   
                               INS_GLBUFFERDATA                  =   37,   
                               INS_GLENABLEVERTEXATTRIBARRAY     =   38,   
                               INS_GLVERTEXATTRIBPOINTER         =   39,   
                               INS_GLDISABLEVERTEXATTRIBARRAY    =   40,   
                               INS_GLGENVERTEXARRAYS             =   41,   
                               INS_GLBINDVERTEXARRAY             =   42,   
                               INS_GLENABLE                      =   43,
                               INS_GLDEPTHFUNC                   =   44,
                               INS_GLCREATESHADER                =   45,
                               INS_GLSHADERSOURCE                =   46,
                               INS_GLCOMPILESHADER               =   47,
                               INS_GLCREATEPROGRAM               =   48,
                               INS_GLATTACHSHADER                =   49,
                               INS_GLLINKPROGRAM                 =   50,
                               INS_GLDETACHSHADER                =   51,
                               INS_GLDELETESHADER                =   52,
                               INS_GLUSEPROGRAM                  =   53,
                               INS_GLUNIFORMFV                   =   54,
                               INS_GLUNIFORMIV                   =   55,
                               INS_GLUNIFORMUIV                  =   56,
                               INS_GLUNIFORMMATRIXFV             =   57,
                               INS_GLGETUNIFORMLOCATION          =   58,

                               INS_GLGENTEXTURES                 =   59,
                               INS_GLBINDTEXTURE                 =   60,
                               INS_GLTEXPARAMETERI               =   61,
                               INS_GLTEXPARAMETERFV              =   62,
                               INS_GLGENERATEMIPMAP              =   63,
                               INS_GLTEXIMAGE2D                  =   64,
                               INS_GLGETATTRIBLOCATION           =   65,
                               INS_GLACTIVETEXTURE               =   66,

                               INS_LOG                           =  192,   //     log string pointed at by command
                               INS_DATA                          =  193,   //     global data
                               INS_NOOP                          =  254,
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
      registers[REGIP]  = (uint64_t)code_start;
      registers[REGBP]  = (uint64_t)buffer;
      registers[REGERR] = 0;
    };

    void hard_reset(void) {
      labels.empty();
      memset(code_start,0,code_end-code_start);
      reset();
    }

    MetisVM(uint8_t *instruction_loc, uint64_t instruction_len, 
            uint64_t *stack_loc, uint64_t stack_len,
            uint8_t *glbuffer_loc, uint64_t glbuffer_len) {
      code_start            = instruction_loc;
      code_end              = instruction_loc+instruction_len;
      stack                 = (MetisMemoryCell *)stack_loc;
      stack_size            = stack_len;
      buffer                = glbuffer_loc;
      buffer_size           = glbuffer_len;
      buffer_end            = glbuffer_loc;
      numcommands           = 0;

      strcpy((char *)header,"METIS  1 ");
      reset();
    }
    uint64_t add_error       (void);
    uint64_t add_end         (void);   
    uint64_t add_noop        (void);   
    uint64_t add_jump        (address_mode src);
    uint64_t add_jumpi       (uint64_t location);
    uint64_t add_jizz        (address_mode src, address_mode dest);
    uint64_t add_jnz         (address_mode src, address_mode dest);
    uint64_t add_jne         (address_mode src, address_mode dest, uint64_t location);
    uint64_t add_jmpe        (address_mode src, address_mode dest, uint64_t location);
    uint64_t add_store       (address_mode src, address_mode dest);
    uint64_t add_storei      (address_mode dest, uint64_t value);
    uint64_t add_label_ip    (const char *label);
    uint64_t add_label_val   (const char *label, uint64_t val);

    // data gets mixed in with the instructions
    uint64_t add_data             (const uint8_t *data, 
                                   const uint64_t length, 
                                   const char *label);
    uint64_t add_matrix           (uint8_t width, uint8_t height,
                                   const uint8_t *data, 
                                   const char *label);
    uint64_t add_identity_matrix  (uint8_t width, uint8_t height,
                                   const char *label);
    uint64_t add_push_matrix      (uint64_t location);
    uint64_t add_matrix_multiply  (address_mode src1, address_mode src2, uint64_t destination);
    uint64_t add_matrix_add       (address_mode src1, address_mode src2, uint64_t destination);
    uint64_t add_vector_dot       (address_mode src1, address_mode src2, uint64_t destination);
    uint64_t add_vector_cross     (address_mode src1, address_mode src2, uint64_t destination);

    // buffer gets made into a gl buffer, stored separately.
    uint64_t add_buffer           (const uint8_t *buffer, const uint64_t length, const char *label);
    uint64_t add_not              (address_mode src, address_mode dest);

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

    uint64_t add_gldrawelements(GLenum mode, GLsizei count, 
                                GLenum type, uint64_t indices);
    uint64_t add_gldrawarrays(GLenum mode, 
                              GLint first, 
                              GLsizei count);
    uint64_t add_glgenbuffers(GLsizei num_identifiers, 
                              metisgl_identifier start_index);
    uint64_t add_glgenvertexarrays(GLsizei num_identifiers, 
                                   metisgl_identifier start_index);
    uint64_t add_glbindvertexarray(metisgl_identifier array);
    uint64_t add_glbindbuffer(GLenum target, 
                              metisgl_identifier buffer_index);
    uint64_t add_glbufferdata(GLenum target, 
                              GLsizeiptr size, 
                              uint64_t data_index, 
                              GLenum usage);
    uint64_t add_glenablevertexattribarray(GLuint index);     // not a metisgl_identifier...
    uint64_t add_glvertexattribpointer(GLuint index, GLint size, 
                                       GLenum type, GLboolean normalized, 
                                       GLsizei stride, uint64_t pointer);
    uint64_t add_gldisablevertexattribarray(GLuint index);
    uint64_t add_glenable(GLenum capability);
    uint64_t add_gldepthfunc(GLenum function);
    uint64_t add_glcreateshader(GLenum type, 
                                metisgl_identifier start_index);
    uint64_t add_glshadersource(GLuint shader, 
                                metisgl_identifier source_index);
    uint64_t add_glcompileshader(metisgl_identifier index);
    uint64_t add_glcreateprogram(metisgl_identifier program_index);
    uint64_t add_glattachshader(metisgl_identifier program_index,
                                metisgl_identifier shader_index);
    uint64_t add_gllinkprogram(metisgl_identifier program_index);
    uint64_t add_gldetachshader(metisgl_identifier program_index,
                                metisgl_identifier shader_index);
    uint64_t add_gldeleteshader(metisgl_identifier shader_index);
    uint64_t add_gluseprogram(metisgl_identifier shader_index);
    uint64_t add_gluniformfv(address_mode src, metisgl_identifier uniform_index);
    uint64_t add_gluniformiv(address_mode src, metisgl_identifier uniform_index);
    uint64_t add_gluniformuiv(address_mode src, metisgl_identifier uniform_index);
    uint64_t add_gluniformmatrixfv(address_mode src, metisgl_identifier uniform_index);
    uint64_t add_glgetuniformlocation(metisgl_identifier program_index,
                                      metisgl_identifier uniform_index, 
                                      const char *uniform_name);

    uint64_t add_glgentextures(GLsizei num_identifiers, 
                               metisgl_identifier start_index);                   
    uint64_t add_glbindtexture(GLenum target, 
                               metisgl_identifier texture_index);
    uint64_t add_gltexparameteri(GLenum target, GLenum pname, GLint param);
    uint64_t add_gltexparameterfv(address_mode src, GLenum target, GLenum pname);
    uint64_t add_glgeneratemipmap(GLenum target);
    uint64_t add_glteximage2d(GLenum target, GLint level, GLint internal_format,
                              GLsizei width, GLsizei height, GLint border,
                              GLenum format, GLenum type, uint64_t data_index);
    uint64_t add_glgetattriblocation(metisgl_identifier attrib_index, const char *attrib_name);
    uint64_t add_glactivetexture(GLenum texture);

    bool doCompileShader(uint16_t index);
    bool doLinkProgram(uint16_t index);

    void save(const string &filename);
    void load(const string &filename);           
            
    bool eval();        
    bool eval(const char *label);
    bool do_eval();


    GLuint get_glidentifier(uint16_t index) {
      return glidentifiers[index];
    }

    uint64_t get_label(const char *label) {
      try {
        return labels.at(label);
      } catch (...) {
        throw MetisException(string("label does not exist - ") + label, __LINE__,__FILE__);
      }
    }

    uint8_t  *get_ptr_from_label (const char *label) {
      return (uint8_t *)(code_start + get_label(label));
    }
    uint8_t  *get_ptr_stack (void) {
      return (uint8_t *)stack;
    }

    uint8_t  *get_bufloc_from_label (const char *label) {
      return (uint8_t *)(buffer + get_label(label));
    }
    uint64_t *get_registers  (void)  { return registers; };

    uint64_t  cur_stack_val  (uint64_t offset=0)  {
      if ( registers[REGSP] > 0) {
        return stack[registers[REGSP]-1-offset].whole; 
      } else {
        throw MetisException("attempted to read empty stack (cur_stack_val)",__LINE__,__FILE__);
      }
    }
    uint64_t  cur_stack_size (void)  { return registers[REGSP]; };
  
  private:
    uint64_t           registers[8];
    uint8_t            isizes[256];
    MetisMemoryCell   *stack;
    uint64_t           stack_size;

    GLuint             glidentifiers[METIS_NUM_BUFFERS];
    uint64_t           numcommands;
    
    uint8_t           *code_start;
    uint8_t           *code_end;

    uint8_t           *buffer;      
    uint64_t           buffer_size;
    uint8_t           *buffer_end;

    uint8_t            header[11];
    unordered_map<string, uint64_t> labels;


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
            
            struct ext_matrix_multiply_t {
              uint64_t destination;
            }__attribute__((packed)) matrix_multiply;
            
            struct ext_matrix_add_t {
              uint64_t destination;
            }__attribute__((packed)) matrix_add;

            struct ext_vector_dot_t {
              uint64_t destination;
            }__attribute__((packed)) vector_dot;

            struct ext_vector_cross_t {
              uint64_t destination;
            }__attribute__((packed)) vector_cross;

            struct gluniformfv_t {
              metisgl_identifier uniform_index;
            }__attribute__((packed)) gluniformfv;

            struct gluniformiv_t {
              metisgl_identifier uniform_index;
            }__attribute__((packed)) gluniformiv;

            struct gluniformuiv_t {
              metisgl_identifier uniform_index;
            }__attribute__((packed)) gluniformuiv;

            struct gluniformmatrixfv_t {
              metisgl_identifier uniform_index;
            }__attribute__((packed)) gluniformmatrixfv;

            struct gltexparameterfv_t {
              GLenum target; 
              GLenum pname;
            }__attribute__((packed)) gltexparameterfv;
            
          }__attribute__((packed))ext; 
        }__attribute__((packed)) extended;

        struct gldrawelements_t {
          GLenum mode;
          GLsizei count;
          GLenum type;
          uint64_t  indices;
        }__attribute__((packed)) gldrawelements;

        struct gldrawarrays_t {
          GLenum mode;
          GLint first;
          GLsizei count;
        }__attribute__((packed))gldrawarrays;

        struct glgenbuffers_t {
          GLsizei num_identifiers;
          metisgl_identifier  start_index;
        }__attribute__((packed))glgenbuffers;

        struct glbindbuffer_t {
          GLenum target;
          metisgl_identifier buffer_index;
        }__attribute__((packed))glbindbuffer;

        struct glbufferdata_t {
          GLenum target;
          GLsizeiptr size;
          uint64_t data_index; 
          GLenum usage;
        }__attribute__((packed))glbufferdata;

        struct glgenvertexarrays_t {
          GLsizei num_identifiers;
          metisgl_identifier start_index;
        }__attribute__((packed))glgenvertexarrays;
        
        struct glbindvertexarray_t {
          metisgl_identifier array_index;
        }__attribute__((packed))glbindvertexarray;

        struct glenablevertexattribarray_t {
          GLuint index;
        }__attribute__((packed))glenablevertexattribarray;

        struct glvertexattribpointer_t {
          GLuint index;
          GLint size;
          GLenum type;
          GLboolean normalized;
          GLsizei stride;
          uint64_t pointer;
        }__attribute__((packed))glvertexattribpointer;

        struct gldisablevertexattribarray_t {
          GLuint index;
        }__attribute__((packed))gldisablevertexattribarray;

        struct glenable_t {
          GLenum capability;
        }__attribute__((packed))glenable;

        struct gldepthfunc_t {
          GLenum function;
        }__attribute__((packed))gldepthfunc;
        
        struct glcreateshader_t {
          GLenum type; 
          metisgl_identifier start_index;
        }__attribute__((packed)) glcreateshader;


        struct glshadersource_t {
          GLuint shader; 
          metisgl_identifier source_index;
        }__attribute__((packed)) glshadersource;

        struct glcompileshader_t {
          metisgl_identifier shader_index;
        }__attribute__((packed)) glcompileshader;

        struct glcreateprogram_t {
          metisgl_identifier program_index;
        }__attribute__((packed)) glcreateprogram;
          
        struct glattachshader_t {
          metisgl_identifier program_index;
          metisgl_identifier shader_index;
        }__attribute__((packed)) glattachshader;

        struct gllinkprogram_t {
          metisgl_identifier program_index;
        }__attribute__((packed)) gllinkprogram;

        struct gldetachshader_t {
          metisgl_identifier program_index;
          metisgl_identifier shader_index;
        }__attribute__((packed)) gldetachshader;

        struct gldeleteshader_t {
          metisgl_identifier shader_index;
        }__attribute__((packed)) gldeleteshader;
        
        struct gluseprogram_t {
          metisgl_identifier program_index;
        }__attribute__((packed)) gluseprogram;

        struct glgetuniformlocation_t {
          metisgl_identifier program_index;
          metisgl_identifier uniform_index;
          uint8_t id_length;
        }__attribute__((packed)) glgetuniformlocation;



        struct glgentextures_t {
          GLsizei num_identifiers; 
          metisgl_identifier start_index;
        }__attribute__((packed)) glgentextures;

        struct glbindtexture_t {
          GLenum target; 
          metisgl_identifier texture_index;
        }__attribute__((packed)) glbindtexture;

        struct gltexparameteri_t {
          GLenum target; 
          GLenum pname; 
          GLint param;
        }__attribute__((packed)) gltexparameteri;


        struct glgeneratemipmap_t {
          GLenum target;
        }__attribute__((packed)) glgeneratemipmap;

        struct glteximage2d_t {
          GLenum target; 
          GLint level; 
          GLint internal_format;
          GLsizei width; 
          GLsizei height; 
          GLint border;
          GLenum format; 
          GLenum type; 
          uint64_t data_index;
        }__attribute__((packed)) glteximage2d;

        struct glgetattriblocation_t {
          metisgl_identifier attrib_index;
          uint8_t id_length;
        }__attribute__((packed)) glgetattriblocation;

        struct glactivetexture_t {
          GLenum texture;
        }__attribute__((packed)) glactivetexture;


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
          union contents_t {
            MetisMatrixHeader matrix;
            int x;
          }__attribute__((packed)) contents;
        }__attribute__((packed)) data;
        struct pushmatrix_t {
          uint64_t location;
        }__attribute__((packed)) pushmatrix;
      }__attribute__((packed)) commands;
    }__attribute__((packed));
    void push(uint64_t val) {
      if( registers[REGSP] >= stack_size) {
        throw MetisException("stack full (push), val = " + to_string(val),__LINE__,__FILE__);
      }
      stack[registers[REGSP]].whole = val;
      registers[REGSP] += 1;
    }

    uint64_t pop() {
      if(registers[REGSP] == 0) {
        throw MetisException("stack empty (pop)",__LINE__,__FILE__);
      }
      registers[REGSP] -= 1;
      return stack[registers[REGSP]].whole;
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
        case REGBP:
          registers[location] = value;
          break;
        case STACK_PUSH:
          push(value);
          break;
        default:
          throw MetisException("unknown addressing mode (set_val) - " + to_string(location),__LINE__,__FILE__);
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
        case REGBP:
          return registers[location];
          break;
        case STACK_POP:
          return pop();
          break;
        default:
          throw MetisException("unknown addressing mode (get_val)",__LINE__,__FILE__);
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
        case REGBP:
          return registers[location];
          break;
        case STACK_POP:
          return pop();
          break;
        default:
          throw MetisException("unknown addressing mode (get_dest_val) mode = " + to_string(location),__LINE__,__FILE__);
      }
    }     
      
};

typedef function<void (MetisVM &, istream &s)> instruction_handler;
class MetisASM {
  public:
    MetisASM();
    void assemble(const string &filename, MetisVM &vm);
  private:
    unordered_map<string, instruction_handler> handlers;
    unordered_map<string, address_mode> addr_modes;
    unordered_map<string, GLenum> gl_enums;
    istream *infile;

    address_mode       get_addr_mode(void);
    uint64_t           get_uint64(void);
    uint8_t            get_uint8(void);
    float              get_float(void);
    string             get_string(void);
    string             get_line(void);
    uint64_t           get_addr(MetisVM &m);
    GLenum             get_GLenum(void);
    GLsizei            get_GLsizei(void);
    GLint              get_GLint(void);
    GLuint             get_GLuint(void);
    GLsizeiptr         get_GLsizeiptr(void);
    GLboolean          get_GLboolean(void);
    metisgl_identifier get_metisid(void);

};
#include <streambuf>

class CountingStreamBuffer : public std::streambuf
{
public:
    // constructor
    CountingStreamBuffer(std::streambuf* sbuf) : 
        streamBuf_(sbuf), 
        lineNumber_(1),
        lastLineNumber_(1),
        column_(0),
        prevColumn_(static_cast<unsigned int>(-1)),
        filePos_(0) 
    {
    }

    // Get current line number
    unsigned int        lineNumber() const  { return lineNumber_; }

    // Get line number of previously read character
    unsigned int        prevLineNumber() const { return lastLineNumber_; }

    // Get current column
    unsigned int        column() const   { return column_; }

    // Get file position
    std::streamsize     filepos() const { return filePos_; }

protected:
    CountingStreamBuffer(const CountingStreamBuffer&);
    CountingStreamBuffer& operator=(const CountingStreamBuffer&);

    // extract next character from stream w/o advancing read pos
    std::streambuf::int_type underflow() 
    { 
        return streamBuf_->sgetc(); 
    }

    // extract next character from stream
    std::streambuf::int_type uflow()
    {
        int_type rc = streamBuf_->sbumpc();

        lastLineNumber_ = lineNumber_;
        if (traits_type::eq_int_type(rc, traits_type::to_int_type('\n'))) 
        {
            ++lineNumber_;
            prevColumn_ = column_ + 1;
            column_ = static_cast<unsigned int>(-1);
        }

        ++column_;
        ++filePos_;
        return rc;
    }

    // put back last character
    std::streambuf::int_type pbackfail(std::streambuf::int_type c)
    {
        if (traits_type::eq_int_type(c, traits_type::to_int_type('\n'))) 
        {
            --lineNumber_;
            lastLineNumber_ = lineNumber_;
            column_ = prevColumn_;
            prevColumn_ = 0;
        }

        --column_;
        --filePos_;

        if (c != traits_type::eof())
            return streamBuf_->sputbackc(traits_type::to_char_type(c));  
        else 
            return streamBuf_->sungetc();
    }

    // change position by offset, according to way and mode  
    virtual std::ios::pos_type seekoff(std::ios::off_type pos, 
                                  std::ios_base::seekdir dir, 
                                  std::ios_base::openmode mode)
    {
        if (dir == std::ios_base::beg 
         && pos == static_cast<std::ios::off_type>(0))
        {
            lastLineNumber_ = 1;
            lineNumber_ = 1;
            column_ = 0;
            prevColumn_ = static_cast<unsigned int>(-1);
            filePos_ = 0;

            return streamBuf_->pubseekoff(pos, dir, mode);
        }
        else
            return std::streambuf::seekoff(pos, dir, mode);
    }

    // change to specified position, according to mode
    virtual std::ios::pos_type seekpos(std::ios::pos_type pos,
                                  std::ios_base::openmode mode)
    {   
        if (pos == static_cast<std::ios::pos_type>(0))
        {
            lastLineNumber_ = 1;
            lineNumber_ = 1;
            column_ = 0;
            prevColumn_ = static_cast<unsigned int>(-1);
            filePos_ = 0;

            return streamBuf_->pubseekpos(pos, mode);
        }
        else
            return std::streambuf::seekpos(pos, mode);
    }


private:
    std::streambuf*     streamBuf_;     // hosted streambuffer
    unsigned int        lineNumber_;    // current line number
    unsigned int        lastLineNumber_;// line number of last read character
    unsigned int        column_;        // current column
    unsigned int        prevColumn_;    // previous column
    std::streamsize     filePos_;       // file position
};

#endif
