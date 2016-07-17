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


#define CMD_ERROR                  0   //     if we get a 0, it's an error

// jumps, stack
#define CMD_JUMP                   1   // *   jump to index ...
#define CMD_JUMPI                  2   //     jump to immediate index
#define CMD_JIZZ                   3   // *   jump to index ... if zero
#define CMD_PUSH                   4   // *   push from immediate value
#define CMD_POP                    5   // *   pull from stack to ...
#define CMD_STORE                  6   // *   store ... into stack offset #...


// Math
#define CMD_INC                    7   // *   increment ... 
#define CMD_DEC                    8   // *   decrement ... 
#define CMD_ADD                    9   // *   A = A+...  (integer)
#define CMD_SUB                   10   // *   A = A-...  (integer)
#define CMD_MUL                   11   // *   A = A*...  (integer)
#define CMD_DIV                   12   // *   A = A/...  (integer)
#define CMD_MOD                   13   // *   A = A%...  (integer)

// Bitwise
#define CMD_AND                   14   // *   A = A&...  (integer) 
#define CMD_OR                    15   // *   A = A|...  (integer) 
#define CMD_XOR                   16   // *   A = A^...  (integer) 
#define CMD_NOT                   17   // *   A = A&...  (integer) 

#define CMD_GLDRAW_ES             32   //     GLDrawElements, using stack args
#define CMD_GLDRAW_EI             33   //     GLDrawElements, using immediate
#define CMD_GLDRAW_AS             34   //     GLDrawArrays, using stack args
#define CMD_GLDRAW_AI             35   //     GLDrawArrays, using immediate

#define CMD_LOG                  192   //     log string pointed at by command

#define CMD_END                  255   //     End Program

#define ADDR_MODES               instruction->commands.extended.addr_mode

#define MATH_OPERATION(op)       set_val(ADDR_MODES, \
                                         get_dest_val(ADDR_MODES) op \
                                         get_val(ADDR_MODES));


struct MetisInstruction {
  uint8_t type;
  union commands_t {
    struct extended_t {
      uint8_t addr_mode;
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
    struct jump_t {
      uint64_t value;
    } jump;
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
      
    bool Eval() {
      while(cur <= end) {
        MetisInstruction *instruction = (MetisInstruction *)cur;
        switch (instruction->type) {
          // instruction index and stack instructions
          case CMD_JUMP:
            cur = start + get_val(ADDR_MODES);
            break;
          case CMD_JUMPI:
            cur = start + instruction->commands.jump.value;
            break;
          case CMD_JIZZ:
            if (get_val(ADDR_MODES)==0) {
              cur = (uint8_t *)get_dest_val(ADDR_MODES);
            }
          case CMD_PUSH:
            push(get_val(ADDR_MODES));
            break;
          case CMD_POP:
            set_val(ADDR_MODES, pop());
            break;
          case CMD_STORE:
            set_val(ADDR_MODES,
                    get_val(ADDR_MODES));

          // math instructions
          case CMD_INC:
            set_val(ADDR_MODES,
                    get_val(ADDR_MODES)+1);
            break;
          case CMD_DEC:
            set_val(ADDR_MODES,
                    get_val(ADDR_MODES)-1);
            break;
          case CMD_ADD:
            MATH_OPERATION(+);
            break;
          case CMD_SUB:
            MATH_OPERATION(-);
            break;
          case CMD_MUL:
            MATH_OPERATION(*);
            break;
          case CMD_DIV:
            MATH_OPERATION(/);
            break;
          case CMD_MOD:
            MATH_OPERATION(%);
            break;
          
          // logical operation          
          case CMD_AND:
            MATH_OPERATION(&);
            break;
          case CMD_OR:
            MATH_OPERATION(|);
            break;
          case CMD_XOR:
            MATH_OPERATION(^);
            break;
          case CMD_NOT:
            set_val(ADDR_MODES, ~get_val(ADDR_MODES));
            break;

          case CMD_END:
            return true;
            break;
          case CMD_ERROR:
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
};


