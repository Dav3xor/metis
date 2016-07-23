#include <cstdint>
#include <GL/gl.h>
#include <cstdio>

using namespace std;

#define STACK_SIZE                 128

#define ADDR_MODES               instruction->commands.extended.addr_mode

#define BUILD_ADDR(src, dest)    ((dest << 4) + src)

#define MATH_OPERATION(op)       set_val(ADDR_MODES, \
                                         get_dest_val(ADDR_MODES) op \
                                         get_val(ADDR_MODES)); \
                                 cur += ADVANCE(1, 0);

#define ADVANCE(extended, data)  sizeof(MetisInstruction)                 
//#define ADVANCE(extended, data)  1+extended+data

#define MATH_METHOD(method_name,byte_code) void method_name(address_mode src, address_mode dest) { \
      MetisInstruction *instruction                   = (MetisInstruction *)cur; \
      instruction->type                               = byte_code; \
      instruction->commands.extended.addr_mode        = BUILD_ADDR(src, dest); \
      cur += ADVANCE(1, 0); \
    };


// addressing modes
enum address_mode: uint8_t {REGA                    =    0,
                            REGB                    =    1,
                            REGC                    =    2,
                            REGD                    =    3,
                            REGS                    =    4,  // stack register
                            REGERR                  =    5,
                            STACK_PUSH              =    8,
                            STACK_POP               =    9 };

class MetisVM {
  public:
    void reset(void) {
      cur                   = start;
      registers[REGA]   = 0;
      registers[REGB]   = 0;
      registers[REGC]   = 0;
      registers[REGD]   = 0;
      registers[REGS]   = 0;
      registers[REGERR] = 0;
    };
    
   MetisVM(uint8_t *buf_loc, uint64_t buf_len, uint64_t *stack_loc, uint64_t stack_len) { 
      start                 = buf_loc;
      end                   = buf_loc+buf_len;
      stack                 = stack_loc;
      stack_size            = stack_len;
      numcommands           = 0;
      reset();
    }

    void add_end(void) {
      MetisInstruction *instruction            = (MetisInstruction *)cur;
      instruction->type                        = INS_END;      
      cur += ADVANCE(0, 0);
    };

    void add_jump(address_mode src, address_mode dest) {
      MetisInstruction *instruction            = (MetisInstruction *)cur;
      instruction->type                        = INS_JUMP;      
      instruction->commands.extended.addr_mode = BUILD_ADDR(src, dest);
      cur += ADVANCE(1, 0);
    };

    void add_jumpi(uint64_t location) {
      MetisInstruction *instruction                 = (MetisInstruction *)cur;
      instruction->type                             = INS_JUMPI;      
      instruction->commands.jumpi.value = location;
      cur += ADVANCE(0, sizeof(ext_jumpi_t));
    };

    void add_jizz(address_mode src, address_mode dest) {
      MetisInstruction *instruction            = (MetisInstruction *)cur;
      instruction->type                        = INS_JIZZ;      
      instruction->commands.extended.addr_mode = BUILD_ADDR(src, dest);
      cur += ADVANCE(1, 0);
    }; 
    void add_store(address_mode src, address_mode dest) {
      MetisInstruction *instruction            = (MetisInstruction *)cur;
      instruction->type                        = INS_STORE;      
      instruction->commands.extended.addr_mode = BUILD_ADDR(src, dest);
      cur += ADVANCE(1, 0);
    }; 
    void add_storei(address_mode dest, uint64_t value) {
      MetisInstruction *instruction                 = (MetisInstruction *)cur;
      instruction->type                             = INS_STOREI;      
      instruction->commands.extended.addr_mode = BUILD_ADDR(0, dest);
      instruction->commands.extended.ext.storei.value = value;
      cur += ADVANCE(1, sizeof(ext_storei_t));
    };
    
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

    void add_not(address_mode src, address_mode dest) {
      MetisInstruction *instruction            = (MetisInstruction *)cur;
      instruction->type                        = INS_NOT;      
      instruction->commands.extended.addr_mode = BUILD_ADDR(src, dest);
      cur += ADVANCE(1, 0);
    };
      
    bool eval() {
      reset();
      while(cur <= end) {
        MetisInstruction *instruction = (MetisInstruction *)cur;
        switch (instruction->type) {
          // instruction index and stack instructions
          case INS_JUMP:
            cur = start + get_val(ADDR_MODES);
            cur += ADVANCE(1, 0);
            break;
          case INS_JUMPI:
            cur = start + instruction->commands.jumpi.value;
            cur += ADVANCE(0, sizeof(ext_jumpi_t));
            break;
          case INS_JIZZ:
            if (get_val(ADDR_MODES)==0) {
              cur = start + get_dest_val(ADDR_MODES);
            }
            cur += ADVANCE(1, 0);
            break;
          case INS_STORE:
            set_val(ADDR_MODES,
                    get_val(ADDR_MODES));
            cur += ADVANCE(1, 0);
            break;
          case INS_STOREI:
            set_val(ADDR_MODES,
                    instruction->commands.extended.ext.storei.value);
            cur += ADVANCE(1, sizeof(ext_storei_t));
            break;

          // math instructions
          case INS_INC:
            set_val(ADDR_MODES,
                    get_val(ADDR_MODES)+1);
            cur += ADVANCE(1, 0);
            break;
          case INS_DEC:
            set_val(ADDR_MODES,
                    get_val(ADDR_MODES)-1);
            cur += ADVANCE(1, 0);
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
            cur += ADVANCE(1, 0);
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

    uint64_t *get_registers  (void)  { return registers; };
    uint64_t  cur_stack_val  (void)  {
      if ( registers[REGS    ] > 0) {
        return stack[registers[REGS    ]-1]; 
      } else {
        throw "Metis: stack empty";
      }
    }
    uint64_t  cur_stack_size (void)  { return registers[REGS    ]; };
  
  private:
    uint64_t    registers[8];
    uint64_t    *stack;
    uint64_t    stack_size;
    uint64_t    numcommands;
    
    uint8_t    *start;
    uint8_t    *cur;
    uint8_t    *end;


    enum instruction: uint8_t {INS_ERROR                =    0,   //     should never happen
                               INS_JUMP                 =    1,   // *   jump to index ...
                               INS_JUMPI                =    2,   //     jump to immediate index
                               INS_JIZZ                 =    3,   // *   jump to index ... if zero
                               INS_STORE                =    6,   // *   store ... into stack offset #...
                               INS_STOREI               =    7,   // *   store immediate value into 

                               // Math
                               INS_INC                  =    8,   // *   increment ... 
                               INS_DEC                  =    9,   // *   decrement ... 
                               INS_ADD                  =   10,   // *   A = A+...  (integer)
                               INS_SUB                  =   11,   // *   A = A-...  (integer)
                               INS_MUL                  =   12,   // *   A = A*...  (integer)
                               INS_DIV                  =   13,   // *   A = A/...  (integer)
                               INS_MOD                  =   14,   // *   A = A%...  (integer)

                               // Bitwise
                               INS_AND                  =   15,   // *   A = A&...  (integer) 
                               INS_OR                   =   16,   // *   A = A|...  (integer) 
                               INS_XOR                  =   17,   // *   A = A^...  (integer) 
                               INS_NOT                  =   18,   // *   A = A&...  (integer) 

                               INS_GLDRAW_ES            =   32,   //     GLDrawElements, using stack args
                               INS_GLDRAW_EI            =   33,   //     GLDrawElements, using immediate
                               INS_GLDRAW_AS            =   34,   //     GLDrawArrays, using stack args
                               INS_GLDRAW_AI            =   35,   //     GLDrawArrays, using immediate
 
                               INS_LOG                  =  192,   //     log string pointed at by command

                               INS_END                  =  255,   //     End Program 
                               };

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
      } commands;
    };
    void push(uint64_t val) {
      if( registers[REGS] >= STACK_SIZE-1) {
        throw "Metis: pushing to full stack";
      }
      stack[registers[REGS]] = val;
      registers[REGS] += 1;
    }

    uint64_t pop() {
      if(registers[REGS] == 0) {
        throw "Metis: popping empty stack";
      }
      registers[REGS] -= 1;
      return stack[registers[REGS]];
    }

    void set_val(uint8_t location, uint64_t value) {
      // high bits are destination
      location = location >> 4;
      switch (location) {
        case REGA:
        case REGB:
        case REGC:
        case REGD:
        case REGS:
        case REGERR:
          registers[location] = value;
          break;
        case STACK_PUSH:
          push(value);
          break;
        default:
          throw "Metis: unknown addressing mode (write)";
      } 
    }
    uint64_t get_val(uint8_t location) {
      // low bits are source
      location = location & 0x0F;
      switch (location) {
        case REGA:
        case REGB:
        case REGC:
        case REGD:
        case REGS:
        case REGERR:
          return registers[location];
          break;
        case STACK_POP:
          return pop();
          break;
        default:
          throw "Metis: unknown addressing mode (read)";
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
        case REGS:
        case REGERR:
          return registers[location];
          break;
        case STACK_POP:
          return pop();
          break;
        default:
          throw "Metis: unknown addressing mode (dest read)";
      }
    }     
      
};


