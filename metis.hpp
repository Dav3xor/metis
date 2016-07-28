#include <exception>
#include <stdexcept>
#include <sstream>
#include <fstream>
#include <unordered_map>
#include <memory>

#include <cstdint>
#include <cstdio>
#include <cstring>

#include <GL/gl.h>

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
                                  cur += ADVANCE(1, 0);

#define ADVANCE(extended, data) (32)
//#define ADVANCE(extended, data)   sizeof(MetisInstruction)                 
//#define ADVANCE(extended, data)   (1+extended+data)

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

ostringstream MetisException::cnvt;
   
class MetisVM {
  public:
    // simple reset, do not remove existing code
    void reset(void) {
      cur                   = start;
      registers[REGA]   = 0;
      registers[REGB]   = 0;
      registers[REGC]   = 0;
      registers[REGD]   = 0;
      registers[REGS]   = 0;
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
    }

    void add_end(void) {
      MetisInstruction *instruction            = (MetisInstruction *)cur;
      instruction->type                        = INS_END;      
      cur += ADVANCE(0, 0);
    };
    
    void add_jump(address_mode src) {
      MetisInstruction *instruction            = (MetisInstruction *)cur;
      instruction->type                        = INS_JUMP;      
      instruction->commands.extended.addr_mode = BUILD_ADDR(src, 0);
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
    
    void add_label(const char *label) {
      // not really an instruction, but it basically acts like one...
      labels[label] = (uint64_t)(cur-start);
    }

    
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
    // TODO: work on cleaning this up... 
    // 1. move into separate file
    // 2. abstract sections better
    void save(const string &filename) {
      ofstream outfile(filename, ios::out|ios::binary);
      outfile.write("METIS  1  ",10);
      uint16_t header_len = 0;
      outfile.write("H",1);
      outfile.write((char *) &header_len, 2);
      for (auto kv : labels) {
        uint16_t label_len = kv.first.length();
        outfile.write("L",1);
        outfile.write((char *) &label_len,2);
        outfile.write(kv.first.c_str(), kv.first.length());
        outfile.write((char *) &kv.second, sizeof(uint64_t));
      }

      uint64_t code_len = cur-start;
      outfile.write("C",1);
      outfile.write((char *)&code_len, 8);
      outfile.write((char *)start, code_len);
      outfile.close();
    }
    void load(const string &filename) {
      char header[9];
      char label[MAX_LABEL_LEN+1];
      uint16_t label_len;
      uint16_t header_len;
      uint64_t code_len;
      uint64_t value;

      reset();
      labels.clear();

      ifstream infile(filename, ios::in|ios::binary);

      infile.read(header,10);
      header[8] = '\0';
      while(!(infile.eof())) {
        char type;
        infile.read(&type,1);
        switch(type) {
          case 'H':     // header, ignore for now...
            infile.read((char *)&header_len, 2);
            break;
          case 'L':
            infile.read((char *)&label_len,2);
            if (label_len > MAX_LABEL_LEN) {
              throw MetisException("label too big?!? (load)");
            }
            infile.read(label, label_len);
            label[label_len]='\0';

            infile.read((char *)&value,8);

            labels[label] = value;
            break;
          case 'C':
            // TODO: allow multiple code segments, load 
            //       them one after another...
            infile.read((char *)&code_len,8);
            if (code_len > end-start) {
              throw MetisException("code too big?!? (load)");
            }
            infile.read((char *) start, code_len);
            break;
        }
      }
      infile.close();
    }
            
            
            
    bool eval() {
      reset();
      while(cur <= end) {
        MetisInstruction *instruction = (MetisInstruction *)cur;
        switch (instruction->type) {
          // instruction index and stack instructions
          case INS_JUMP:
            cur = start + get_val(ADDR_MODES);
            break;
          case INS_JUMPI:
            cur = start + instruction->commands.jumpi.value;
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


    uint64_t get_label(const char *label) {
      return labels.at(label);
    }

    uint64_t *get_registers  (void)  { return registers; };
    uint64_t  cur_stack_val  (void)  {
      if ( registers[REGS    ] > 0) {
        return stack[registers[REGS    ]-1]; 
      } else {
        throw MetisException("stack empty (cur_stack_val)");
      }
    }
    uint64_t  cur_stack_size (void)  { return registers[REGS    ]; };
  
  private:
    uint64_t    registers[8];
    uint64_t    *stack;
    uint64_t    stack_size;

    unordered_map<string, uint64_t> labels;

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
      if( registers[REGS] >= stack_size) {
        throw MetisException("stack full (push)");
      }
      stack[registers[REGS]] = val;
      registers[REGS] += 1;
    }

    uint64_t pop() {
      if(registers[REGS] == 0) {
        throw MetisException("stack empty (pop)");
      }
      registers[REGS] -= 1;
      return stack[registers[REGS]];
    }

    void set_val(uint8_t location, uint64_t value) {
      // high bits are destination
      location = GET_DEST(location);
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
        case REGS:
        case REGERR:
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
        case REGS:
        case REGERR:
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


