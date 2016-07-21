#include <cstdint>
#include <GL/gl.h>

#define STACK_SIZE                 128

// addressing modes
#define REGA_LOC                   0
#define REGB_LOC                   1
#define REGC_LOC                   2
#define REGD_LOC                   3
#define REGS_LOC                   4  // stack register
#define REGERR_LOC                 5
#define STACK_PUSH_LOC             8
#define STACK_POP_LOC              9


#define INS_ERROR                  0   //     if we get a 0, it's an error

// jumps, stack
#define INS_JUMP                   1   // *   jump to index ...
#define INS_JUMPI                  2   //     jump to immediate index
#define INS_JIZZ                   3   // *   jump to index ... if zero
#define INS_PUSH                   4   // *   push from immediate value
#define INS_POP                    5   // *   pull from stack to ...
#define INS_STORE                  6   // *   store ... into stack offset #...
#define INS_STOREI                 7   // *   store immediate value into 

// Math
#define INS_INC                    8   // *   increment ... 
#define INS_DEC                    9   // *   decrement ... 
#define INS_ADD                   10   // *   A = A+...  (integer)
#define INS_SUB                   11   // *   A = A-...  (integer)
#define INS_MUL                   12   // *   A = A*...  (integer)
#define INS_DIV                   13   // *   A = A/...  (integer)
#define INS_MOD                   14   // *   A = A%...  (integer)

// Bitwise
#define INS_AND                   15   // *   A = A&...  (integer) 
#define INS_OR                    16   // *   A = A|...  (integer) 
#define INS_XOR                   17   // *   A = A^...  (integer) 
#define INS_NOT                   18   // *   A = A&...  (integer) 

#define INS_GLDRAW_ES             32   //     GLDrawElements, using stack args
#define INS_GLDRAW_EI             33   //     GLDrawElements, using immediate
#define INS_GLDRAW_AS             34   //     GLDrawArrays, using stack args
#define INS_GLDRAW_AI             35   //     GLDrawArrays, using immediate

#define INS_LOG                  192   //     log string pointed at by command

#define INS_END                  255   //     End Program

#define ADDR_MODES               instruction->commands.extended.addr_mode

#define BUILD_ADDR(src, dest)    ((dest << 4) + src)

#define MATH_OPERATION(op)       set_val(ADDR_MODES, \
                                         get_dest_val(ADDR_MODES) op \
                                         get_val(ADDR_MODES)); \
                                 cur += ADVANCE(1, 0);

#define ADVANCE(extended, data)  sizeof(MetisInstruction)                 
//#define ADVANCE(extended, data)  1+extended+data

#define MATH_METHOD(method_name,byte_code) void method_name(uint8_t src, uint8_t dest) { \
      MetisInstruction *instruction                   = (MetisInstruction *)cur; \
      instruction->type                               = byte_code; \
      instruction->commands.extended.addr_mode        = BUILD_ADDR(src, dest); \
      cur += ADVANCE(1, 0); \
    };
struct MetisInstruction {
  uint8_t type;
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


class MetisVM {
  public:
    MetisVM(uint8_t *buf, uint64_t buflen) {
      start                 = buf;
      cur                   = buf;
      end                   = buf+buflen;
      registers[REGA_LOC]   = 0;
      registers[REGB_LOC]   = 0;
      registers[REGC_LOC]   = 0;
      registers[REGD_LOC]   = 0;
      registers[REGS_LOC]   = 0;
      registers[REGERR_LOC] = 0;
      numcommands           = 0;
    };

    void add_jump(uint8_t src, uint8_t dest) {
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

    void add_jizz(uint8_t src, uint8_t dest) {
      MetisInstruction *instruction            = (MetisInstruction *)cur;
      instruction->type                        = INS_JIZZ;      
      instruction->commands.extended.addr_mode = BUILD_ADDR(src, dest);
      cur += ADVANCE(1, 0);
    }; 
    void add_push(uint8_t src) {
      MetisInstruction *instruction            = (MetisInstruction *)cur;
      instruction->type                        = INS_PUSH;      
      instruction->commands.extended.addr_mode = BUILD_ADDR(src, 0);
      cur += ADVANCE(1, 0);
    }
    void add_pop(uint8_t dest) {
      MetisInstruction *instruction            = (MetisInstruction *)cur;
      instruction->type                        = INS_POP;      
      instruction->commands.extended.addr_mode = BUILD_ADDR(0, dest);
      cur += ADVANCE(1, 0);
    }
    void add_store(uint8_t src, uint8_t dest) {
      MetisInstruction *instruction            = (MetisInstruction *)cur;
      instruction->type                        = INS_STORE;      
      instruction->commands.extended.addr_mode = BUILD_ADDR(src, dest);
      cur += ADVANCE(1, 0);
    }; 
    void add_storei(uint8_t dest, uint64_t value) {
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

    void add_not(uint8_t src, uint8_t dest) {
      MetisInstruction *instruction            = (MetisInstruction *)cur;
      instruction->type                        = INS_NOT;      
      instruction->commands.extended.addr_mode = BUILD_ADDR(src, dest);
      cur += ADVANCE(1, 0);
    };
      
    bool eval() {
      cur = start;
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
          case INS_PUSH:
            push(get_val(ADDR_MODES));
            cur += ADVANCE(1, 0);
            break;
          case INS_POP:
            set_val(ADDR_MODES, pop());
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
            return true;
            break;
          case INS_ERROR:
          default:
            return false;
            break;
        };
      };
    };

  private:
    uint64_t    registers[8];
    uint64_t    stack[STACK_SIZE];
    uint64_t    numcommands;
    
    uint8_t    *start;
    uint8_t    *cur;
    uint8_t    *end;

    void push(uint64_t val) {
      if( registers[REGS_LOC] >= STACK_SIZE-1) {
        throw "Metis: pushing to full stack";
      }
      stack[registers[REGS_LOC]] = val;
      registers[REGS_LOC] += 1;
    }

    uint64_t pop() {
      if(registers[REGS_LOC] == 0) {
        throw "Metis: popping empty stack";
      }
      registers[REGS_LOC] -= 1;
      return stack[registers[REGS_LOC]+1];
    }

    void set_val(uint8_t location, uint64_t value) {
      // high bits are destination
      location = location >> 4;
      switch (location) {
        case REGA_LOC:
        case REGB_LOC:
        case REGC_LOC:
        case REGD_LOC:
        case REGS_LOC:
        case REGERR_LOC:
          registers[location] = value;
          break;
        case STACK_PUSH_LOC:
          push(value);
          break;
        default:
          throw "Metis: unknown addressing mode (write)";
      } 
    }
    uint64_t get_val(uint8_t location) {
      // low bits are source
      location = location && 0x0F;
      switch (location) {
        case REGA_LOC:
        case REGB_LOC:
        case REGC_LOC:
        case REGD_LOC:
        case REGS_LOC:
        case REGERR_LOC:
          return registers[location];
          break;
        case STACK_POP_LOC:
          return pop();
          break;
        default:
          throw "Metis: unknown addressing mode (read)";
      }
    }     
    uint64_t get_dest_val(uint8_t location) {
      // low bits are source
      location = location >> 4;
      switch (location) {
        case REGA_LOC:
        case REGB_LOC:
        case REGC_LOC:
        case REGD_LOC:
        case REGS_LOC:
        case REGERR_LOC:
          return registers[location];
          break;
        case STACK_POP_LOC:
          return pop();
          break;
        default:
          throw "Metis: unknown addressing mode (read)";
      }
    }     
      
};


