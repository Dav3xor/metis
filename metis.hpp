#ifndef METIS_VM
#define METIS_VM


#include <exception>
#include <stdexcept>
#include <sstream>
#include <fstream>
#include <unordered_map>
#include <memory>

#include <cstdint>
#include <cstdio>
#include <cstring>

#include <GLFW/glfw3.h>

using namespace std;

#define MAX_LABEL_LEN             128
#define ADDR_MODES                instruction->commands.extended.addr_mode

#define BUILD_ADDR(src, dest)     ((dest << 4) + src)
#define GET_DEST(location)        (location >> 4)
#define GET_SRC(location)         (location & 0x0F)
#define GET_LABEL(instruction)    ((char *)(&instruction->type)+1)

#define MATH_OPERATION(op)        set_val(ADDR_MODES, \
                                          get_dest_val(ADDR_MODES) op \
                                          get_val(ADDR_MODES)); \
                                  registers[REGIP] += ADVANCE(1, 0);

#define ADVANCE(extended, data) (32)
//#define ADVANCE(extended, data)   sizeof(MetisInstruction)                 
//#define ADVANCE(extended, data)   (1+extended+data)

#define MATH_METHOD(method_name,byte_code) void method_name(address_mode src, address_mode dest) { \
      MetisInstruction *instruction                   = (MetisInstruction *)registers[REGIP]; \
      instruction->type                               = byte_code; \
      instruction->commands.extended.addr_mode        = BUILD_ADDR(src, dest); \
      registers[REGIP] += ADVANCE(1, 0); \
    };


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
   
class MetisVM {
  private:
    // first, the list of instructions...
    enum instruction: uint8_t {INS_ERROR                =    0,   //     should never happen
                               INS_JUMP                 =    1,   // *   jump to index ...
                               INS_JUMPI                =    2,   //     jump to immediate index
                               INS_JIZZ                 =    3,   // *   jump to index ... if zero
                               INS_JNZ                  =    4,   // *   jump to index ... if zero
                               INS_JNE                  =    5,   // *   jump if not equal
                               INS_JMPE                 =    6,   // *   jump if equal
                               INS_STORE                =    7,   // *   store ... into stack offset #...
                               INS_STOREI               =    8,   // *   store immediate value into 

                               // Math
                               INS_INC                  =    9,   // *   increment ... 
                               INS_DEC                  =   10,   // *   decrement ... 
                               INS_ADD                  =   11,   // *   A = A+...  (integer)
                               INS_SUB                  =   12,   // *   A = A-...  (integer)
                               INS_MUL                  =   13,   // *   A = A*...  (integer)
                               INS_DIV                  =   14,   // *   A = A/...  (integer)
                               INS_MOD                  =   15,   // *   A = A%...  (integer)

                               // Bitwise
                               INS_AND                  =   16,   // *   A = A&...  (integer) 
                               INS_OR                   =   17,   // *   A = A|...  (integer) 
                               INS_XOR                  =   18,   // *   A = A^...  (integer) 
                               INS_NOT                  =   19,   // *   A = A&...  (integer) 

                               INS_GLDRAWELEMENTS       =   32,   //     GLDrawElements, using stack args
                               INS_GLDRAWELEMENTSI      =   33,   //     GLDrawElements, using immediate
                               INS_GLDRAWARRAYS         =   34,   //     GLDrawArrays, using stack args
                               INS_GLDRAWARRAYSI        =   35,   //     GLDrawArrays, using immediate
 
                               INS_LOG                  =  192,   //     log string pointed at by command
                               INS_DATA                 =  193,   //     global data

                               INS_END                  =  255,   //     End Program 
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
    MetisVM(uint8_t *buf_loc, uint64_t buf_len, uint64_t *stack_loc, uint64_t stack_len) { 
      start                 = buf_loc;
      end                   = buf_loc+buf_len;
      stack                 = stack_loc;
      stack_size            = stack_len;
      numcommands           = 0;
      reset();
      if (!glfwInit())
      {
        printf("glfwInit failed\n");
      }
      glfwSetErrorCallback(error_callback); 
    }
    ~MetisVM() {
      glfwTerminate();
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
    uint64_t add_label     (const char *label);
    void     add_data      (const uint8_t *data, const uint64_t length, const char *label);

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
    void add_gldrawelements(void);


    void save(const string &filename);
    void load(const string &filename);           
            
            
    bool eval() {
      reset();
      GLenum mode;
      GLsizei count;
      GLenum type;
      GLvoid *indices;
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
              registers[REGIP] += ADVANCE(1, sizeof(ext_jne_t));
            }
            break; 
          case INS_JMPE:
            if(get_val(ADDR_MODES) == get_dest_val(ADDR_MODES)) {
              registers[REGIP] = (uint64_t)start + instruction->commands.extended.ext.jmpe.value;
            } else {
              registers[REGIP] += ADVANCE(1, sizeof(ext_jmpe_t));
            }
            break; 
          case INS_JIZZ:
            if (get_val(ADDR_MODES)==0) {
              registers[REGIP] = (uint64_t)start + get_dest_val(ADDR_MODES);
            } else {
              registers[REGIP] += ADVANCE(1, 0);
            }
            break;
          case INS_JNZ:
            if (get_val(ADDR_MODES)!=0) {
              registers[REGIP] = (uint64_t)start + get_dest_val(ADDR_MODES);
            } else {
              registers[REGIP] += ADVANCE(1, 0);
            }
            break;
          case INS_STORE:
            set_val(ADDR_MODES,
                    get_val(ADDR_MODES));
            registers[REGIP] += ADVANCE(1, 0);
            break;
          case INS_STOREI:
            set_val(ADDR_MODES,
                    instruction->commands.extended.ext.storei.value);
            registers[REGIP] += ADVANCE(1, sizeof(ext_storei_t));
            break;
          
          // math instructions
          case INS_INC:
            set_val(ADDR_MODES,
                    get_val(ADDR_MODES)+1);
            registers[REGIP] += ADVANCE(1, 0);
            break;
          case INS_DEC:
            set_val(ADDR_MODES,
                    get_val(ADDR_MODES)-1);
            registers[REGIP] += ADVANCE(1, 0);
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
            registers[REGIP] += ADVANCE(1, 0);
            break;

          case INS_GLDRAWELEMENTS:
            printf("\n\nx\n\n\n");
            mode    = pop();
            count   = pop();
            type    = pop();     
            indices = (GLvoid *)pop();
            glDrawElements(mode, count, type, indices);
            registers[REGIP] += ADVANCE(0,0);
            break;
          case INS_DATA:
            advance = instruction->commands.data.length;
            registers[REGIP] += ADVANCE(0,sizeof(data_t));
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
    uint64_t    *stack;
    uint64_t    stack_size;

    unordered_map<string, uint64_t> labels;

    uint64_t    numcommands;
    
    uint8_t    *start;
    uint8_t    *end;



    struct MetisInstruction {
      instruction type;
      union commands_t {
        struct extended_t {
          uint8_t addr_mode;
          union ext_t {
            struct ext_jumpi_t {
              uint64_t value;
            }jumpi;
            struct ext_storei_t {
              uint64_t value;
            }storei;

            struct ext_jne_t {
              uint64_t value;
            } jne;
            struct ext_jmpe_t {
              uint64_t value;
            } jmpe;

          }ext; 
        } extended;
        struct gldrawelements_t {
          GLenum mode;
          GLsizei count;
          GLenum type;
          GLvoid * indices;
        } gldrawelements;
        struct gldrawarrays_t {
          GLenum mode;
          GLint first;
          GLsizei count;
        }gldrawarrays;
        struct jumpi_t {
          uint64_t value;
        } jumpi;
        struct push_t {
          uint64_t value;
        } push;
        struct log_t {
          uint8_t length;
        } log;
        struct data_t {
          uint64_t length;
        } data;
      } commands;
    };
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
