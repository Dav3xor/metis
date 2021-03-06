#define CATCH_CONFIG_MAIN

#include <chrono>
#include <fcntl.h>

#include "metis.hpp"
#include "catch.hpp"



MetisContext c; 

TEST_CASE( "addressing modes", "[MetisVM]" ) {
  uint8_t buf[10000];
  uint64_t stack[5];
  MetisVM m(buf,10000, stack, 5, NULL, 0);

  REQUIRE( BUILD_ADDR(REGB, REGC) == 0x21);
  REQUIRE( GET_SRC(0x21) == 1);
  REQUIRE( GET_DEST(0x21) == 2);
}


TEST_CASE( "labels", "[MetisVM]" ) {
  uint8_t buf[10000];
  uint64_t stack[5];
  MetisVM m(buf,10000, stack, 5, NULL, 0);
  m.add_label_ip("start");
  m.add_storei(REGA,100);
  m.add_storei(REGB,100);
  m.add_label_ip("end");

  m.add_label_val("arbitrary", 555);
  m.add_end();

  m.eval();

  REQUIRE( m.get_label("start") == 0);
  REQUIRE( m.get_label("end") == INS_STOREI_SIZE*2);
  REQUIRE( m.get_label("arbitrary") == 555);
  REQUIRE_THROWS_AS( m.get_label("x"), MetisException);
}

TEST_CASE( "instruction doesn't fit", "[MetisVM]" ) {
  uint8_t buf[15];
  uint64_t stack[5];
  MetisVM m(buf, 15, stack, 5, NULL, 0);
  m.hard_reset();

  m.add_jumpi(0);
  REQUIRE_THROWS_AS( m.add_jumpi(0), MetisException);
}
  
TEST_CASE( "invalid location", "[MetisVM]" ) {
  uint8_t buf[50];
  uint64_t stack[5];
  MetisVM m(buf, 50, stack, 5, NULL, 0);
  m.hard_reset();

  m.add_jumpi(49);
  REQUIRE_THROWS_AS( m.add_jumpi(50), MetisException);
}

TEST_CASE( "null pointer", "[MetisVM]" ) {
  uint8_t buf[50];
  uint64_t stack[5];
  MetisVM m(buf, 50, stack, 5, NULL, 0);
  m.hard_reset();
  
  m.add_label_ip("x");
  REQUIRE_THROWS_AS( m.add_label_ip(NULL), MetisException);
}

TEST_CASE( "add_*() return values", "[MetisVM]") {
  uint8_t buf[10000];
  uint64_t stack[5];
  MetisVM m(buf,10000, stack, 5, NULL, 0);
  m.hard_reset(); 

  uint64_t start = m.get_registers()[REGIP];
  m.add_noop();
  REQUIRE( m.get_registers()[REGIP] == start+1);

  m.add_end();
  REQUIRE( m.get_registers()[REGIP] == start+2);

  m.add_jumpi(5);
  REQUIRE( m.get_registers()[REGIP] == start+11);
}

TEST_CASE( "eval starting at label", "[MetisVM]" ) {
  uint8_t buf[10000];
  uint64_t stack[5];
  MetisVM m(buf,10000, stack, 5, NULL, 0);
  m.hard_reset(); 

  // test that a simple store works
  m.add_end();
  m.add_storei(REGA,100);
  m.add_label_ip("start1");
  m.add_storei(REGB,101);
  m.add_label_ip("start2");
  m.add_end();

  REQUIRE( m.eval() == true);
  REQUIRE( m.get_registers()[REGA] == 0);
  REQUIRE( m.get_registers()[REGB] == 0);

  m.reset();
  REQUIRE( m.eval("start1") == true);
  REQUIRE( m.get_registers()[REGA] == 0);
  REQUIRE( m.get_registers()[REGB] == 101);
  
  m.reset();
  REQUIRE( m.eval("start2") == true);
  REQUIRE( m.get_registers()[REGA] == 0);
  REQUIRE( m.get_registers()[REGB] == 0);
}
  
TEST_CASE( "stack push/pop", "[MetisVM]" ) {
  uint8_t buf[10000];
  uint64_t stack[5];
  MetisVM m(buf,10000, stack, 5, NULL, 0);
  m.hard_reset(); 

  // test that a simple store works 
  m.add_storei(STACK_PUSH,100);
  m.add_end();

  m.eval();

  REQUIRE( m.cur_stack_size() == 1);
  REQUIRE( m.cur_stack_val() == 100 );

  // test that we can have more than one item on the stack...
  m.add_storei(STACK_PUSH,101);
  m.add_end();
 
  m.eval();
 
  REQUIRE( m.cur_stack_size() == 2);
  REQUIRE( m.cur_stack_val() == 101 );

  // test that you can move a value from a register
  // onto the stack.
  m.add_storei(REGA, 102);
  m.add_store(REGA,STACK_PUSH);
  m.add_end();
  
  m.eval();

  REQUIRE( m.cur_stack_size() == 3);
  REQUIRE( m.cur_stack_val() == 102 );

  m.add_storei(REGA, 103);
  m.add_store(REGA,STACK_PUSH);
  m.add_end();
  
  m.eval();
 
  REQUIRE( m.get_registers()[REGA] == 103);
  REQUIRE( m.cur_stack_size() == 4);
  REQUIRE( m.cur_stack_val() == 103 );

  m.add_store(STACK_POP,REGB);
  m.add_end();
  
  m.eval();
  
  REQUIRE( m.get_registers()[REGA] == 103);
  REQUIRE( m.get_registers()[REGB] == 103);
  REQUIRE( m.cur_stack_size() == 3);
  REQUIRE( m.cur_stack_val() == 102 );
  
  m.add_store(STACK_POP,REGA);
  m.add_end();
  
  m.eval();
  
  REQUIRE( m.get_registers()[REGA] == 102);
  REQUIRE( m.get_registers()[REGB] == 103);
  REQUIRE( m.cur_stack_size() == 2);
  REQUIRE( m.cur_stack_val() == 101 );

  // make sure we can fill the stack, but not go over...
  m.add_store(REGA,STACK_PUSH);
  m.add_store(REGA,STACK_PUSH);
  m.add_store(REGA,STACK_PUSH);
  m.add_end();
  
  m.eval();

  REQUIRE( m.cur_stack_size() == 5);
  
  m.add_store(REGA,STACK_PUSH);
 
  // should throw a stack full exception 
  REQUIRE_THROWS_AS(m.eval(),MetisException);

  REQUIRE( m.cur_stack_size() == 5);
  
  m.reset(); 
  m.add_store(STACK_POP, REGA);
  m.add_end();
  
  // should throw an empty stack exception
  REQUIRE_THROWS_AS(m.eval(), MetisException);

  // ditto
  REQUIRE_THROWS_AS(m.cur_stack_val(), MetisException);
}

TEST_CASE( "inc/dec", "[MetisVM]" ) {
  uint8_t buf[10000];
  uint64_t stack[5];
  MetisVM m(buf,10000, stack, 5, NULL, 0);
  m.hard_reset();
  m.add_storei(REGA, 0);
  m.add_inc(REGA,REGA);
  m.add_end();

  m.eval();

  REQUIRE(m.get_registers()[REGA] == 1);

  m.add_inc(REGA,REGA);
  m.add_end();

  m.eval();
  REQUIRE(m.get_registers()[REGA] == 2);

  m.add_dec(REGA,REGA);
  m.add_end();


  m.eval();
  REQUIRE(m.get_registers()[REGA] == 1);
} 


TEST_CASE( "jumpi", "[MetisVM]" ) {
  uint8_t buf[10000];
  uint64_t stack[5];
  MetisVM m(buf,10000, stack, 5, NULL, 0);
  m.hard_reset();
  m.add_storei(REGA,1);
  m.add_jumpi(INS_STOREI_SIZE*2+INS_JUMPI_SIZE);
  m.add_storei(REGA,2);
  m.add_storei(REGB,3);
  m.add_end();
  
  m.eval();

  REQUIRE( m.get_registers()[REGA] == 1 );
  REQUIRE( m.get_registers()[REGB] == 3 );
}


TEST_CASE( "jump", "[MetisVM]" ) {
  uint8_t buf[10000];
  uint64_t stack[5];
  MetisVM m(buf,10000, stack, 5, NULL, 0);

  m.hard_reset();
  m.add_label_ip("1");         // 0
  m.add_storei(REGA,INS_STOREI_SIZE+INS_JUMP_SIZE+INS_STOREI_SIZE);

  m.add_label_ip("2");         // 32
  m.add_jump(REGA);         // 64

  m.add_label_ip("3");
   m.add_storei(REGA,2);

  m.add_label_ip("4");
  m.add_storei(REGB,3);

  m.add_label_ip("5");
  m.add_end();

  m.add_label_ip("6");
  
  m.eval();

  REQUIRE( m.get_registers()[REGA] == 22 );
  REQUIRE( m.get_registers()[REGB] == 3 );
  REQUIRE( m.get_label("1") == 0 );
  REQUIRE( m.get_label("2") == INS_STOREI_SIZE );
  REQUIRE( m.get_label("3") == INS_STOREI_SIZE + INS_JUMP_SIZE );
  REQUIRE( m.get_label("4") == INS_STOREI_SIZE + INS_JUMP_SIZE + INS_STOREI_SIZE );
  REQUIRE( m.get_label("5") == INS_STOREI_SIZE + INS_JUMP_SIZE + INS_STOREI_SIZE*2 );
  REQUIRE( m.get_label("6") == INS_STOREI_SIZE + INS_JUMP_SIZE + INS_STOREI_SIZE*2 + INS_END_SIZE );
}

TEST_CASE( "jump if zero", "[MetisVM]" ) {
  uint8_t buf[10000];
  uint64_t stack[5];
  MetisVM m(buf,10000, stack, 5, NULL, 0);
  m.hard_reset();

  uint64_t start = m.get_registers()[REGIP];

  m.add_storei(REGA,5);
  m.add_storei(REGB,0);
  m.add_storei(REGC, 45);
  m.add_label_ip("loop start");        
  m.add_inc(REGB, REGB);
  m.add_dec(REGA, REGA);
  m.add_jizz(REGA,REGC);
  m.add_jumpi(m.get_label("loop start"));
  m.add_end();

  m.eval();

  REQUIRE( m.get_registers()[REGA] == 0 );
  REQUIRE( m.get_registers()[REGB] == 5 );
  REQUIRE( m.get_registers()[REGC] == 45 );
}

TEST_CASE( "jump if not zero (jnz)", "[MetisVM]" ) {
  uint8_t buf[10000];
  uint64_t stack[5];
  uint64_t start_loc;
  MetisVM m(buf,10000, stack, 5, NULL, 0);
  m.hard_reset();


  m.add_storei(REGA,5);
  m.add_storei(REGB,0);
  m.add_storei(REGC, INS_STOREI_SIZE*3);

  m.add_inc(REGB, REGB);
  m.add_dec(REGA, REGA);
  m.add_jnz(REGA,REGC);
  m.add_end();

  m.eval();

  REQUIRE( m.get_registers()[REGA] == 0 );
  REQUIRE( m.get_registers()[REGB] == 5 );
  REQUIRE( m.get_registers()[REGC] == 30 );

  
}

TEST_CASE( "jump if not equal (jne)", "[MetisVM]" ) {
  uint8_t buf[10000];
  uint64_t stack[5];
  MetisVM m(buf,10000, stack, 5, NULL, 0);
  m.hard_reset();


  m.add_storei(REGA,5);
  m.add_storei(REGB,0);
  m.add_jne(REGA, REGB, INS_STOREI_SIZE*3+INS_JNE_SIZE);
  m.add_storei(REGC, 264);
  m.add_storei(REGA, 6);
  m.add_end();

  m.eval();

  REQUIRE( m.get_registers()[REGA] == 6 );
  REQUIRE( m.get_registers()[REGB] == 0 );
  REQUIRE( m.get_registers()[REGC] == 0 );
  
  m.hard_reset();
  
  m.add_storei(REGA,5);
  m.add_storei(REGB,5);
  m.add_jne(REGA, REGB, 132);
  m.add_storei(REGC, 264);
  m.add_storei(REGA, 6);
  m.add_end();

  m.eval();
  
  REQUIRE( m.get_registers()[REGA] == 6 );
  REQUIRE( m.get_registers()[REGB] == 5 );
  REQUIRE( m.get_registers()[REGC] == 264 );
}


TEST_CASE( "jump if equal (jmpe)", "[MetisVM]" ) {
  uint8_t buf[10000];
  uint64_t stack[5];
  MetisVM m(buf,10000, stack, 5, NULL, 0);
  m.hard_reset();


  m.add_storei(REGA,0);
  m.add_storei(REGB,0);
  m.add_jmpe(REGA, REGB, INS_STOREI_SIZE*3+INS_JMPE_SIZE);
  m.add_storei(REGC, 264);
  m.add_storei(REGA, 6);
  m.add_end();

  m.eval();

  REQUIRE( m.get_registers()[REGA] == 6 );
  REQUIRE( m.get_registers()[REGB] == 0 );
  REQUIRE( m.get_registers()[REGC] == 0 );
  
  m.hard_reset();
  
  m.add_storei(REGA,5);
  m.add_storei(REGB,0);
  m.add_jmpe(REGA, REGB, 132);
  m.add_storei(REGC, 264);
  m.add_storei(REGA, 6);
  m.add_end();

  m.eval();
  
  REQUIRE( m.get_registers()[REGA] == 6 );
  REQUIRE( m.get_registers()[REGB] == 0 );
  REQUIRE( m.get_registers()[REGC] == 264 );
}


TEST_CASE( "store", "[MetisVM]" ) {
  uint8_t buf[10000];
  uint64_t stack[5];
  MetisVM m(buf,10000, stack, 5, NULL, 0);
  m.hard_reset();
  
  m.add_storei(STACK_PUSH,1);
  m.add_store(STACK_POP,REGA);
  
  m.add_storei(STACK_PUSH,2);
  m.add_store(STACK_POP,REGB);
  
  m.add_storei(STACK_PUSH,3);
  m.add_store(STACK_POP,REGC);
  
  m.add_storei(STACK_PUSH,4);
  m.add_store(STACK_POP,REGD);

  m.add_store(REGA, STACK_PUSH);
  m.add_end();

  
  m.eval();

  REQUIRE( m.get_registers()[REGA] == 1);
  REQUIRE( m.get_registers()[REGB] == 2);
  REQUIRE( m.get_registers()[REGC] == 3);
  REQUIRE( m.get_registers()[REGD] == 4);
  REQUIRE( m.cur_stack_val() == 1);
  
}

TEST_CASE( "storei", "[MetisVM]" ) {
  uint8_t buf[10000];
  uint64_t stack[5];
  MetisVM m(buf,10000, stack, 5, NULL, 0);
  m.hard_reset();
  
  m.add_storei(REGA, 1);
  m.add_storei(REGB, 2);
  m.add_storei(REGC, 3);
  m.add_storei(REGD, 4);
  m.add_storei(STACK_PUSH, 5);
  m.add_end();

  
  m.eval();

  REQUIRE( m.get_registers()[REGA] == 1);
  REQUIRE( m.get_registers()[REGB] == 2);
  REQUIRE( m.get_registers()[REGC] == 3);
  REQUIRE( m.get_registers()[REGD] == 4);
  REQUIRE( m.cur_stack_val() == 5);
}

TEST_CASE( "store_sr", "[MetisVM]" ) {
  uint8_t buf[10000];
  uint64_t stack[5];
  MetisVM m(buf,10000, stack, 5, NULL, 0);
  m.hard_reset();
  
  m.add_storei(STACK_PUSH, 1);
  m.add_storei(STACK_PUSH, 2);
  m.add_storei(STACK_PUSH, 3);
  
  m.add_storei(REGA, 10);
  m.add_store_sr(REGA, 2);
  m.add_storei(REGA, 20);
  m.add_store_sr(REGA, 1);

  m.add_store(STACK_POP, REGA);
  m.add_store(STACK_POP, REGB);
  m.add_store(STACK_POP, REGC);
  m.add_end();

  m.eval();
  
  REQUIRE( m.get_registers()[REGA] == 3);
  REQUIRE( m.get_registers()[REGB] == 20);
  REQUIRE( m.get_registers()[REGC] == 10);
}
TEST_CASE( "load_sr", "[MetisVM]" ) {
  uint8_t buf[10000];
  uint64_t stack[5];
  MetisVM m(buf,10000, stack, 5, NULL, 0);
  m.hard_reset();
  
  m.add_storei(STACK_PUSH, 1);
  m.add_storei(STACK_PUSH, 2);
  m.add_storei(STACK_PUSH, 3);
  
  m.add_load_sr(0, REGA);
  m.add_load_sr(1, REGB);
  m.add_load_sr(2, REGC);

  m.add_end();

  m.eval();
  
  REQUIRE( m.get_registers()[REGA] == 3);
  REQUIRE( m.get_registers()[REGB] == 2);
  REQUIRE( m.get_registers()[REGC] == 1);
}

TEST_CASE( "stack_adj", "[MetisVM]" ) {
  uint8_t buf[10000];
  uint64_t stack[5];
  MetisVM m(buf,10000, stack, 5, NULL, 0);
  m.hard_reset();

  m.add_storei(STACK_PUSH,100);
  m.add_storei(STACK_PUSH,101);
  m.add_storei(STACK_PUSH,102);
  m.add_storei(STACK_PUSH,103);
  m.add_stack_adj(2);
  m.add_store(STACK_POP, REGA);
  m.add_end();

  m.eval();

  REQUIRE( m.get_registers()[REGSP] == 1);
  REQUIRE( m.get_registers()[REGA] == 101);
}

TEST_CASE( "pushr/popr", "[MetisVM]" ) {
  uint8_t buf[10000];
  uint64_t stack[50];
  MetisVM m(buf,10000, stack, 20, NULL, 0);
  m.hard_reset();

  m.add_storei(REGA,100);
  m.add_storei(REGB,200);
  m.add_storei(REGC,300);
  m.add_storei(REGD,400);
  // can't set SP/IP without running off the rails
  m.add_storei(REGBP,700);
  m.add_storei(REGERR,800);
  m.add_pushr(REGA_F|REGB_F|REGC_F|REGD_F|REGSP_F|REGIP_F|REGBP_F|REGERR_F);
  m.add_end();

  m.eval();
  REQUIRE( m.cur_stack_val(0) == 800);
  REQUIRE( m.cur_stack_val(1) == 700);
  //REQUIRE( m.cur_stack_val(2) == 600);
  REQUIRE( m.cur_stack_val(3) == 4);
  REQUIRE( m.cur_stack_val(4) == 400);
  REQUIRE( m.cur_stack_val(5) == 300);
  REQUIRE( m.cur_stack_val(6) == 200);
  REQUIRE( m.cur_stack_val(7) == 100);
  
  m.hard_reset();

  m.add_storei(REGA,100);
  m.add_storei(REGC,200);
  m.add_storei(REGD,300);
  m.add_pushr(REGA_F|REGC_F|REGD_F);
  m.add_storei(REGA,101);
  m.add_storei(REGC,202);
  m.add_storei(REGD,303);
  m.add_popr(REGA_F|REGC_F|REGD_F);
  m.add_end();

  m.eval();
  REQUIRE( m.get_registers()[REGA] == 100);
  REQUIRE( m.get_registers()[REGC] == 200);
  REQUIRE( m.get_registers()[REGD] == 300);
}



TEST_CASE( "noop", "[MetisVM]" ) {
  uint8_t buf[10000];
  uint64_t stack[5];
  MetisVM m(buf,10000, stack, 5, NULL, 0);
  m.hard_reset();
  
  m.add_storei(REGA,5);
  m.add_noop();
  m.add_noop();
  m.add_noop();
  m.add_noop();
  m.add_storei(REGA,6);
  m.add_noop();
  m.add_noop();
  m.add_noop();
  m.add_storei(REGB,7);
  m.add_end();

  m.eval();
  REQUIRE( m.get_registers()[REGA] == 6);
  REQUIRE( m.get_registers()[REGB] == 7);
}

TEST_CASE( "math", "[MetisVM]" ) {
  uint8_t buf[10000];
  uint64_t stack[5];
  MetisVM m(buf,10000, stack, 5, NULL, 0);
  m.hard_reset();
  m.add_storei(REGA,5);
  m.add_storei(REGB,2);
  m.add_inc(REGA, REGA);  // 6
  m.add_dec(REGA, REGA);  // 5
  m.add_add(REGB, REGA);  // 7
  m.add_sub(REGB, REGA);  // 5
  m.add_mul(REGB, REGA);  // 10
  m.add_div(REGB, REGA);  // 5
  m.add_mod(REGB, REGA);  // 1
  m.add_end();

  m.eval();
  
  REQUIRE( m.get_registers()[REGA] == 1);
  REQUIRE( m.get_registers()[REGB] == 2);
}

TEST_CASE( "fp math", "[MetisVM]" ) {
  uint8_t buf[10000];
  uint64_t stack[5];
  MetisVM m(buf,10000, stack, 5, NULL, 0);
  m.hard_reset();
  m.add_storei_double(REGA,5.0);
  m.add_storei_double(REGB,2.5);
  m.add_fpadd(REGB, REGA);  // 7.5
  m.add_fpsub(REGB, REGA);  // 5.0
  m.add_fpmul(REGB, REGA);  // 12.25
  m.add_fpdiv(REGB, REGA);  // 5
  m.add_end();

  m.eval();
  
  REQUIRE( m.get_registers_cell()[REGA].whole_double == 5.0);
  REQUIRE( m.get_registers_cell()[REGB].whole_double == 2.5);
}

TEST_CASE( "trig", "[MetisVM]" ) {
  uint8_t buf[10000];
  uint64_t stack[10];
  MetisVM m(buf,10000, stack, 10, NULL, 0);
  m.hard_reset();
  m.add_storei_double(REGA,0.0);
  m.add_storei_double(REGB,0.1);
  m.add_atan2(REGB, REGA, STACK_PUSH);
  m.add_sin(REGA, STACK_PUSH);  
  m.add_cos(REGA, STACK_PUSH);  
  m.add_tan(REGA, STACK_PUSH);  
  m.add_sin(REGB, STACK_PUSH);  
  m.add_cos(REGB, STACK_PUSH);  
  m.add_tan(REGB, STACK_PUSH);  
  
  m.add_end();

  m.eval();
  
  REQUIRE( m.cur_stack_val_cell(6)->whole_double == Approx(1.5707963267948966));
  REQUIRE( m.cur_stack_val_cell(5)->whole_double == 0.0);
  REQUIRE( m.cur_stack_val_cell(4)->whole_double == 1.0);
  REQUIRE( m.cur_stack_val_cell(3)->whole_double == 0.0);
  REQUIRE( m.cur_stack_val_cell(2)->whole_double == Approx(0.0998334166));
  REQUIRE( m.cur_stack_val_cell(1)->whole_double == Approx(0.9950041653));
  REQUIRE( m.cur_stack_val_cell(0)->whole_double == Approx(0.1003346721));
}

TEST_CASE( "logic ops", "[MetisVM]" ) {
  uint8_t buf[10000];
  uint64_t stack[5];
  MetisVM m(buf,10000, stack, 5, NULL, 0);
  m.hard_reset();
  m.add_storei(REGA,10);
  m.add_storei(REGB,2);
  m.add_and(REGB, REGA);  // 2
  m.add_storei(REGB,3);
  m.add_or(REGB, REGA);  // 3
  m.add_storei(REGB,5);
  m.add_xor(REGB, REGA);  // 6
  m.add_not(REGB, REGA);  // 5
  m.add_end();

  m.eval();
  
  REQUIRE( m.get_registers()[REGA] == 0xfffffffffffffffa);
  REQUIRE( m.get_registers()[REGB] == 5);
}

TEST_CASE( "data", "[MetisVM]" ) {
  uint8_t buf[10000];
  uint64_t stack[5];
  float data[5] = {1.1,2.2,3.3,4.4,5.5};
  MetisVM m(buf,10000, stack, 5, NULL, 0);
  m.hard_reset();

  m.add_storei(REGA,10);
  m.add_storei(REGB,10);
  m.add_data((uint8_t *)data, sizeof(data), "data");
  m.add_storei(REGA,11);
  m.add_end();
  
  m.eval();

  REQUIRE( m.get_registers()[REGA] == 11);
  REQUIRE( m.get_registers()[REGB] == 10);
  float *ins_data = (float *)m.get_ptr_from_label("data");
  REQUIRE( ins_data[0] == 1.1f);
  REQUIRE( ins_data[1] == 2.2f);
  REQUIRE( ins_data[2] == 3.3f);
  REQUIRE( ins_data[3] == 4.4f);
  REQUIRE( ins_data[4] == 5.5f);
}

TEST_CASE( "time instructions", "[MetisVM]" ) {
  uint8_t buf[10000];
  uint64_t stack[5];
  FileSpec fs = {"testfile", LOCAL_FILE, O_RDONLY};
  uint8_t buffer[256];

  MetisVM m(buf,10000, stack, 5, NULL, 0);
  m.hard_reset();
  

  m.add_storei(REGC, 10000000);
  m.add_curtime(REGA);
  m.add_wait(REGC);
  m.add_curtime(REGB);
  m.add_end();

  m.eval();
  REQUIRE(m.get_registers()[REGB] - m.get_registers()[REGA] > 1000000);
}

TEST_CASE( "file io", "[MetisVM]" ) {
  uint8_t buf[10000];
  uint64_t stack[5];
  char out_file[12] = "outtestfile";

  FileSpec fs = {"testfile", LOCAL_FILE, O_RDONLY};
  uint8_t buffer[256];

  MetisVM m(buf,10000, stack, 5, NULL, 0);
  m.hard_reset();

  m.add_data((uint8_t *)&fs, sizeof(fs), "fsread");

  fs.file_flags = O_WRONLY|O_CREAT;
  strcpy(fs.path,out_file);
  m.add_data((uint8_t *)&fs, sizeof(fs), "fswrite");

  m.add_data((uint8_t *)buffer, sizeof(buffer), "buffer");
  m.add_data((uint8_t *)out_file, strlen(out_file)+1, "outfile");

  m.add_storei(REGA,m.get_label("fsread"));
  m.add_storei(REGB,m.get_label("buffer"));
  m.add_open(REGA,REGC);
  m.add_read(REGC,REGB,1000);
  m.add_close(REGC);
 
  // make sure the output file doesn't exist
  m.add_storei(REGC, m.get_label("outfile"));
  m.add_remove(REGC, STACK_PUSH);
  m.add_exists(REGC, STACK_PUSH);

  // now try to write to it.
  m.add_storei(REGA,m.get_label("fswrite"));
  m.add_open(REGA,REGC);
  m.add_write(REGC,REGB,5);
  m.add_close(REGC);

  // then make sure the written file exists
  m.add_storei(REGC, m.get_label("outfile"));
  m.add_exists(REGC, STACK_PUSH);

  // and remove it...
  m.add_remove(REGC, STACK_PUSH);

  // and make sure it has been removed...
  m.add_exists(REGC, STACK_PUSH);
 
  // and done.
  m.add_end();
  m.eval();

  REQUIRE( m.get_registers()[REGA] == 286);
  REQUIRE( m.get_registers()[REGB] == 563);

  REQUIRE( m.cur_stack_val(0) == 0);
  REQUIRE( m.cur_stack_val(1) == 1);
  REQUIRE( m.cur_stack_val(2) == 1);
  REQUIRE( m.cur_stack_val(3) == 0);
  REQUIRE( m.cur_stack_val(4) == 0);

  char *data = (char *)m.get_ptr_from_label("buffer");
  REQUIRE(string(data)== string("this is a test\n"));
}

TEST_CASE( "file seek", "[MetisVM]" ) {
  uint8_t buf[10000];
  uint64_t stack[5];
  FileSpec fs = {"testfile", LOCAL_FILE, O_RDONLY};
  Seek seek   = {5,SEEK_SET};

  uint8_t buffer[256];

  MetisVM m(buf,10000, stack, 5, NULL, 0);
  m.hard_reset();

  m.add_data((uint8_t *)&fs, sizeof(fs), "fsread");

  m.add_data((uint8_t *)&seek, sizeof(seek), "seek");

  m.add_data((uint8_t *)buffer, sizeof(buffer), "buffer");
  m.add_storei(REGA,m.get_label("fsread"));
  m.add_storei(REGB,m.get_label("buffer"));
  m.add_storei(REGD,m.get_label("seek"));
  m.add_open(REGA,REGC);
  m.add_seek(REGC, REGD);
  m.add_read(REGC,REGB,1000);
  m.add_close(REGC);
  m.add_end();
  
  m.eval();

  char *data = (char *)m.get_ptr_from_label("buffer");
  REQUIRE(string(data)== string("is a test\n"));
}

TEST_CASE( "malloc/free", "[MetisVM]" ) {
  uint8_t buf[10000];
  uint64_t stack[5];
  MetisVM m(buf,10000, stack, 5, NULL, 0);
  m.hard_reset();

  m.add_storei(REGA, 100);
  m.add_malloc(REGA, REGB);
  m.add_free(REGB);
  m.add_end();

  m.eval();
}

TEST_CASE ( "matrix multiply", "[MetisVM]" ) {
  uint8_t buf[10000];
  uint64_t stack[20];
  float matrix1[16]   = {1.1,1.2,1.3,1.4,
                         2.1,2.2,2.3,2.4,
                         3.1,3.2,3.3,3.4,
                         4.1,4.2,4.3,4.4};
  float matrix2[8]   = {2.0,3.0,
                        4.0,5.0,
                        6.0,7.0,
                        8.0,9.0};
  float result[16]   = {0.0,0.0,0.0,0.0,
                        0.0,0.0,0.0,0.0,
                        0.0,0.0,0.0,0.0,
                        0.0,0.0,0.0,0.0};
  MetisVM m(buf,10000, stack, 20, NULL, 0);
  m.hard_reset();
 
  uint64_t start = m.get_registers()[REGIP];

  m.add_matrix          (4,4, (uint8_t *)matrix1, "matrix1");
  m.add_identity_matrix (4,4, "identity1");
  m.add_matrix          (4,4, (uint8_t *)result, "result1");

  // split matrix2 into 2 2x2 matrices for this test...
  m.add_matrix(2,2, (uint8_t *)matrix2, "matrix2");
  m.add_matrix(2,2, (uint8_t *)&matrix2[4], "identity2");
  m.add_matrix(2,2, (uint8_t *)result, "result2");

  // then use all of matrix2 to test differently sized
  // matrices
  m.add_matrix(2,4, (uint8_t *)matrix2, "matrix3");
  m.add_matrix(2,4, (uint8_t *)result, "result3");

  m.add_storei(REGA, m.get_label("matrix1"));
  m.add_storei(REGB, m.get_label("identity1"));
  m.add_matrix_multiply(REGA,REGB, m.get_label("result1"));

  m.add_storei(REGA, m.get_label("matrix2"));
  m.add_storei(REGB, m.get_label("identity2"));
  m.add_matrix_multiply(REGA,REGB, m.get_label("result2"));

  m.add_storei(REGA, m.get_label("matrix1"));
  m.add_storei(REGB, m.get_label("matrix3"));
  m.add_matrix_multiply(REGA,REGB, m.get_label("result3"));

  m.add_end();

  m.eval();

  MetisMatrixHeader *header = (MetisMatrixHeader *)m.get_ptr_from_label("result2");
  float *matrix3 = (float *)((uint64_t)header+sizeof(MetisMatrixHeader));
  //print_matrix(matrix3, 2, 2);
  REQUIRE(matrix3[0] == Approx(36));
  REQUIRE(matrix3[1] == Approx(41));
  REQUIRE(matrix3[2] == Approx(64));
  REQUIRE(matrix3[3] == Approx(73));

  header = (MetisMatrixHeader *)m.get_ptr_from_label("result1");
  matrix3 = (float *)((uint64_t)header+sizeof(MetisMatrixHeader));
  //print_matrix(matrix3, 4, 4);
  REQUIRE(matrix3[0] == Approx(1.1));
  REQUIRE(matrix3[15] == Approx(4.4));


  header = (MetisMatrixHeader *)m.get_ptr_from_label("result3");
  matrix3 = (float *)((uint64_t)header+sizeof(MetisMatrixHeader));
  //print_matrix(matrix3, 2, 4);
  REQUIRE(matrix3[0] == Approx(26));
  REQUIRE(matrix3[7] == Approx(103));

}
   
TEST_CASE ( "vector add", "[MetisVM]" ) {
  uint8_t buf[10000];
  uint64_t stack[20];
  float matrix1[15]   = {1.1,1.2,1.3,
                         2.1,2.2,2.3,
                         3.1,3.2,3.3,
                         4.1,4.2,4.3,
                         5.1,5.2,5.3};
  float matrix2[15]   = {0.11,0.21,0.31,
                         0.12,0.22,0.32,
                         0.13,0.23,0.33,
                         0.14,0.24,0.34,
                         0.15,0.25,0.35};
  float result[15]   = {0.0,0.0,0.0,
                        0.0,0.0,0.0,
                        0.0,0.0,0.0,
                        0.0,0.0,0.0,
                        0.0,0.0,0.0};
  MetisVM m(buf,10000, stack, 20, NULL, 0);
  m.hard_reset();
 
  uint64_t start = m.get_registers()[REGIP];

  m.add_matrix          (3,5, (uint8_t *)matrix1, "matrix1");
  m.add_matrix          (3,5, (uint8_t *)matrix2, "matrix2");
  m.add_matrix          (3,5, (uint8_t *)result, "result");

  m.add_storei(REGA, m.get_label("matrix1"));
  m.add_storei(REGB, m.get_label("matrix2"));
  m.add_matrix_add(REGA,REGB, m.get_label("result"));
  m.add_storei(REGA, 1);
  m.add_end();

  m.eval();

  MetisMatrixHeader *header = (MetisMatrixHeader *)m.get_ptr_from_label("result");
  float *matrix3 = (float *)((uint64_t)header+sizeof(MetisMatrixHeader));
  //print_matrix(matrix3, 3, 5);
  REQUIRE(matrix3[0]  == Approx(1.21));
  REQUIRE(matrix3[1]  == Approx(1.41));
  REQUIRE(matrix3[2]  == Approx(1.61));
  REQUIRE(matrix3[3]  == Approx(2.22));
  REQUIRE(matrix3[14] == Approx(5.65));
}

TEST_CASE ( "vector dot", "[MetisVM]" ) {
  uint8_t buf[10000];
  uint64_t stack[20];
  float matrix1[15]   = {1.0,2.0,3.0,
                         2.1,2.2,2.3,
                         3.1,3.2,3.3,
                         4.1,4.2,4.3,
                         5.1,5.2,5.3};
  float matrix2[15]   = {6.0,7.0,8.0,
                         0.12,0.22,0.32,
                         0.13,0.23,0.33,
                         0.14,0.24,0.34,
                         0.15,0.25,0.35};
  float result[5]     = {0.0,
                          0.0,
                          0.0,
                          0.0,
                          0.0};
  MetisVM m(buf,10000, stack, 20, NULL, 0);
  m.hard_reset();
 
  uint64_t start = m.get_registers()[REGIP];

  m.add_matrix          (3,5, (uint8_t *)matrix1, "matrix1");
  m.add_matrix          (3,5, (uint8_t *)matrix2, "matrix2");
  m.add_matrix          (1,5, (uint8_t *)result, "result");

  m.add_storei(REGA, m.get_label("matrix1"));
  m.add_storei(REGB, m.get_label("matrix2"));
  m.add_vector_dot(REGA,REGB, m.get_label("result"));

  m.add_end();

  m.eval();

  MetisMatrixHeader *header = (MetisMatrixHeader *)m.get_ptr_from_label("result");
  float *matrix3 = (float *)((uint64_t)header+sizeof(MetisMatrixHeader));
  //print_matrix(matrix3, 1, 5);
  REQUIRE(matrix3[0]  == Approx(44));
  REQUIRE(matrix3[1]  == Approx(1.472));
  REQUIRE(matrix3[2]  == Approx(2.228));
  REQUIRE(matrix3[3]  == Approx(3.044));
  REQUIRE(matrix3[4]  == Approx(3.92));
}

TEST_CASE ( "vector cross", "[MetisVM]" ) {
  uint8_t buf[10000];
  uint64_t stack[20];
  float matrix1[15]   = {1.0,2.0,3.0,
                         2.1,2.2,2.3,
                         3.1,3.2,3.3,
                         4.1,4.2,4.3,
                         5.1,5.2,5.3};
  float matrix2[15]   = {6.0,7.0,8.0,
                         0.12,0.22,0.32,
                         0.13,0.23,0.33,
                         0.14,0.24,0.34,
                         0.15,0.25,0.35};
  float result[15] =    {0.0,0.0,0.0,
                         0.0,0.0,0.0,
                         0.0,0.0,0.0,
                         0.0,0.0,0.0,
                         0.0,0.0,0.0};
  MetisVM m(buf,10000, stack, 20, NULL, 0);
  m.hard_reset();
 
  uint64_t start = m.get_registers()[REGIP];

  m.add_matrix          (3,5, (uint8_t *)matrix1, "matrix1");
  m.add_matrix          (3,5, (uint8_t *)matrix2, "matrix2");
  m.add_matrix          (3,5, (uint8_t *)result, "result");

  m.add_storei(REGA, m.get_label("matrix1"));
  m.add_storei(REGB, m.get_label("matrix2"));
  m.add_vector_cross(REGA,REGB, m.get_label("result"));

  m.add_end();

  m.eval();

  MetisMatrixHeader *header = (MetisMatrixHeader *)m.get_ptr_from_label("result");
  float *matrix3 = (float *)((uint64_t)header+sizeof(MetisMatrixHeader));
  //print_matrix(matrix3, 3, 5);
  REQUIRE(matrix3[0]  == Approx(-5.0));
  REQUIRE(matrix3[1]  == Approx(10.0));
  REQUIRE(matrix3[2]  == Approx(-5.0));
  REQUIRE(matrix3[3]  == Approx(.198));
  REQUIRE(matrix3[4]  == Approx(-.396));
}
TEST_CASE( "matrix add/push", "[MetisVM]" ) {
  uint8_t buf[10000];
  uint64_t stack[20];
  float matrix[16]   = {1.1,1.2,1.3,1.4,
                        2.1,2.2,2.3,2.4,
                        3.1,3.2,3.3,3.4,
                        4.1,4.2,4.3,4.4};
  float identity[16] = {1.0,0.0,0.0,0.0,
                        0.0,1.0,0.0,0.0,
                        0.0,0.0,1.0,0.0,
                        0.0,0.0,0.0,1.0};
  MetisVM m(buf,10000, stack, 20, NULL, 0);
  m.hard_reset();
 
  uint64_t start = m.get_registers()[REGIP];

  m.add_storei(REGA, 10);
  m.add_storei(STACK_PUSH, 101);
  m.add_matrix(4,4, (uint8_t *)matrix, "hi");
  m.add_storei(STACK_PUSH, 102);
  m.add_storei(REGB, 12);
  m.add_push_matrix(m.get_label("hi"));
  m.add_end();

  REQUIRE(m.eval() == true);
  //REQUIRE( m.cur_stack_size() == 11);
  REQUIRE( m.get_registers()[REGIP] - start == INS_STOREI_SIZE*4 + 
                                               INS_DATA_SIZE +
                                               64 +
                                               sizeof(MetisMatrixHeader) +
                                               INS_PUSH_MATRIX_SIZE);
  REQUIRE( m.get_registers()[REGA] == 10);
  REQUIRE( m.get_registers()[REGB] == 12);
  REQUIRE( m.cur_stack_val(9) == 102);
  
  MetisMatrixHeader *header = (MetisMatrixHeader *)m.get_ptr_from_label("hi");
  REQUIRE( header->width == 4);
  REQUIRE( header->height == 4);

  float *matrix2 = (float *)((uint64_t)header+sizeof(MetisMatrixHeader));
  REQUIRE(matrix2[0] == Approx(1.1));
  REQUIRE(matrix2[15] == Approx(4.4));

  header = (MetisMatrixHeader *)(m.get_ptr_stack()+16);
  REQUIRE( header->width == 4);
  REQUIRE( header->height == 4);

  matrix2 = (float *)((uint64_t)header+sizeof(MetisMatrixHeader));
  REQUIRE(matrix2[0] == Approx(1.1));
  REQUIRE(matrix2[15] == Approx(4.4));

}

TEST_CASE( "basic performance test", "[MetisVM]" ) {
  uint8_t buf[30000];
  uint64_t stack[50];
  float data[5] = {1.1,2.2,3.3,4.4,5.5};
  uint64_t start_loop;
 
  char loop_label[2] = {'\0','\0'};

  MetisVM m(buf,30000, stack, 50, NULL, 0);
  m.hard_reset();
  
  m.add_storei(REGA,1000000);
  m.add_storei(REGD,INS_STOREI_SIZE*2);

  for(int i=0; i<50; i++) {
    loop_label[0] = 'a' + i;
    m.add_data((uint8_t *)data, sizeof(data), loop_label);
     
    m.add_inc(REGB,REGB);
    m.add_add(REGB,REGC);
    m.add_sub(REGB,REGC);
    m.add_mul(REGB,REGC);
    m.add_div(REGB,REGC);
    m.add_mod(REGB,REGC);
    m.add_store(REGC, STACK_PUSH);
    m.add_store(STACK_POP, REGC);
    
    m.add_dec(REGB,REGB);
  }
  m.add_dec(REGA,REGA); 
  m.add_jnz(REGA, REGD);
  m.add_end();

  auto start = std::chrono::steady_clock::now(); 
  m.eval();
  auto duration  = std::chrono::duration_cast<std::chrono::milliseconds> (std::chrono::steady_clock::now() - start);
  auto elapsed   = duration.count();
  double ipers   = 505000000.0/(elapsed/1000.0);

  // using cout here because of printf specifier differences between
  // operating systems...
  cout << "---------------------------------" << endl;
  cout << "perf test duration:      " << duration.count() << "ms" << endl;
  cout << "instructions per second: " << ipers << endl;
  cout << "---------------------------------" << endl;
}
    


TEST_CASE( "load/save", "[MetisVM]" ) {
  uint8_t buf[10000];
  uint8_t glbuf[10000];
  uint64_t stack[5];
  float buffer[3] = {1.0,1.1,1.2};
  float data[3]   = {2.0,2.1,2.2};
  
  MetisVM m(buf,10000, stack, 5, glbuf, 10000);
  m.hard_reset();
 
  m.add_buffer((uint8_t*)buffer,sizeof(float)*3,"buffer");
  m.add_data((uint8_t*)data,sizeof(float)*3,"data");
  m.add_label_ip("hi!");
  m.add_storei(REGA, 0xDEADBEEF);
  m.add_label_ip("hi again!");
  m.add_end();

  m.save("test.metis");

  m.hard_reset();

  m.load("test.metis"); 
 
  REQUIRE(m.get_label("hi!") == 21);
  REQUIRE(m.get_label("hi again!") == 31);
 
  m.eval();
  
  REQUIRE(m.get_registers()[REGA] == 0xDEADBEEF);

  float *ins_data = (float *)m.get_ptr_from_label("data");
  REQUIRE(ins_data[0] == 2.0);
  REQUIRE(ins_data[1] == Approx(2.1));
  REQUIRE(ins_data[2] == Approx(2.2));
  
  float *ins_buffer = (float *)m.get_bufloc_from_label("buffer");
  REQUIRE(ins_buffer[0] == Approx(1.0));
  REQUIRE(ins_buffer[1] == Approx(1.1));
  REQUIRE(ins_buffer[2] == Approx(1.2));

  // test bad filename or file...
  REQUIRE_THROWS_AS( m.load("zyyylxsdfsdfssadfsdf"), MetisException);
}

TEST_CASE( "assembler", "[MetisVM]" ) {
  uint8_t buf[10000];
  uint8_t glbuf[10000];
  uint64_t stack[5];
  float buffer[3] = {1.0,1.1,1.2};
  float data[3]   = {2.0,2.1,2.2};
  
  MetisVM m(buf,10000, stack, 5, glbuf, 10000);
  MetisASM a;
  printf("---\n");
  a.assemble("asmtests/asmtest.m", m);
  printf("---\n");
  m.eval();
  REQUIRE( m.get_registers()[REGA] == 155);
  REQUIRE( m.get_registers()[REGB] == 204);
  
  MetisMatrixHeader *header = (MetisMatrixHeader *)m.get_ptr_from_label("stuff");
  REQUIRE( header->width == 3);
  REQUIRE( header->height == 3);

  float *matrix2 = (float *)((uint64_t)header+sizeof(MetisMatrixHeader));
  REQUIRE(matrix2[0] == Approx(1.1));
  REQUIRE(matrix2[8] == Approx(3.3));
  
  header = (MetisMatrixHeader *)m.get_ptr_from_label("identity");
  REQUIRE( header->width == 4);
  REQUIRE( header->height == 4);

  matrix2 = (float *)((uint64_t)header+sizeof(MetisMatrixHeader));
  REQUIRE(matrix2[0] == Approx(1.0));
  REQUIRE(matrix2[1] == Approx(0.0));
  REQUIRE(matrix2[15] == Approx(1.0));
  
  header = (MetisMatrixHeader *)m.get_ptr_from_label("matrixb");
  REQUIRE( header->width == 3);
  REQUIRE( header->height == 3);

  matrix2 = (float *)((uint64_t)header+sizeof(MetisMatrixHeader));
  REQUIRE(matrix2[0] == Approx(.83));
  REQUIRE(matrix2[1] == Approx(-403.53778));
  REQUIRE(matrix2[15] == Approx(0.0));
}

TEST_CASE( "assembler math", "[MetisVM]" ) {
  uint8_t buf[10000];
  uint8_t glbuf[10000];
  uint64_t stack[5];
  float buffer[3] = {1.0,1.1,1.2};
  float data[3]   = {2.0,2.1,2.2};
  
  MetisVM m(buf,10000, stack, 5, glbuf, 10000);
  MetisASM a;
  a.assemble("asmtests/mathtest.m", m);
  m.eval();
  REQUIRE( m.get_registers()[REGA] == 1);
  REQUIRE( m.get_registers()[REGB] == 0);
}

TEST_CASE( "assembler syntax errors", "[MetisVM]" ) {
  uint8_t buf[10000];
  uint8_t glbuf[10000];
  uint64_t stack[5];
  float buffer[3] = {1.0,1.1,1.2};
  float data[3]   = {2.0,2.1,2.2};
  
  MetisVM m(buf,10000, stack, 5, glbuf, 10000);
  MetisASM a;
  REQUIRE_THROWS_AS(a.assemble("asmtests/badinstruction.m", m), MasmException);
  REQUIRE_THROWS_AS(a.assemble("asmtests/badaddressmode.m", m), MasmException);
  REQUIRE_THROWS_AS(a.assemble("asmtests/baduint64.m", m), MasmException);
  REQUIRE_THROWS_AS(a.assemble("asmtests/uint64outofrange.m", m), MasmException);
  REQUIRE_THROWS_AS(a.assemble("asmtests/baduint8.m", m), MasmException);
  REQUIRE_THROWS_AS(a.assemble("asmtests/uint8outofrange.m", m), MasmException);
  REQUIRE_THROWS_AS(a.assemble("asmtests/uint8outofrange2.m", m), MasmException);
  REQUIRE_THROWS_AS(a.assemble("asmtests/uint8outofrange3.m", m), MasmException);
  REQUIRE_THROWS_AS(a.assemble("asmtests/badfloat.m", m), MasmException);
  REQUIRE_THROWS_AS(a.assemble("asmtests/badfloatlabel.m", m), MasmException);
  REQUIRE_THROWS_AS(a.assemble("asmtests/floatoutofrange.m", m), MasmException);
  REQUIRE_THROWS_AS(a.assemble("asmtests/badintegeraddress.m", m), MasmException);
  REQUIRE_THROWS_AS(a.assemble("asmtests/integeraddressoutofrange.m", m), MasmException);
  REQUIRE_THROWS_AS(a.assemble("asmtests/labeladdressnotdefined.m", m), MasmException);
  REQUIRE_THROWS_AS(a.assemble("asmtests/badmetisid.m", m), MasmException);
  REQUIRE_THROWS_AS(a.assemble("asmtests/metisidbadlabel.m", m), MasmException);
  
  REQUIRE_THROWS_AS(a.assemble("asmtests/glsizeiover.m", m), MasmException);
  REQUIRE_THROWS_AS(a.assemble("asmtests/glsizeiunder.m", m), MasmException);
  REQUIRE_THROWS_AS(a.assemble("asmtests/gluintover.m", m), MasmException);
  REQUIRE_THROWS_AS(a.assemble("asmtests/gluintunder.m", m), MasmException);
  REQUIRE_THROWS_AS(a.assemble("asmtests/glsizeiptrover.m", m), MasmException);
  REQUIRE_THROWS_AS(a.assemble("asmtests/glsizeiptrunder.m", m), MasmException);
  REQUIRE_THROWS_AS(a.assemble("asmtests/badglint.m", m), MasmException);
  REQUIRE_THROWS_AS(a.assemble("asmtests/glintoutofrange.m", m), MasmException);
  REQUIRE_THROWS_AS(a.assemble("asmtests/badglclampf.m", m), MasmException);
  REQUIRE_THROWS_AS(a.assemble("asmtests/badglbitfield1.m", m), MasmException);
  REQUIRE_THROWS_AS(a.assemble("asmtests/badglbitfield2.m", m), MasmException);

  a.assemble("asmtests/goodfloatlabel.m", m);
  m.eval();
  REQUIRE(m.get_label_float("is_a_float") == 5.5);
}

TEST_CASE( "window stuff", "[MetisContext]") {
  
  uint8_t buf[10000];
  uint8_t glbuf[10000];
  uint64_t stack[5];
  uint64_t triangle_location;
  uint64_t color_location;
  float buffer[9] =  {-0.8,-0.8,0.0,
                       0.8,-0.8,0.0,
                       0.0, 0.8,0.0};
  float colors[9] =  {1.0,0.5,0.0,
                      0.0,1.0,0.5,
                      0.5,0.0,1.0};
  float angle     = 0.0; 
  float delta     = 0.01;

  const char *vertex_shader =
  "#version 400\n"
  "in vec3 vp;\n"
  "in vec3 color;\n"
  "out vec3 vcolor;\n"
  "uniform float angle;\n"
  "void main () {\n"
  "  gl_Position = vec4(vp.x*cos(angle) - vp.y*sin(angle),\n"
  "                     vp.x*sin(angle) + vp.y*cos(angle),\n"
  "                     0.0,\n"
  "                     1.0);\n"
  "  vcolor = color;\n"
  "}\n";

  const char *fragment_shader =
  "#version 330 core\n"
  "in vec3 vcolor;\n"
  "out vec3 color;\n"
  "void main(){\n"
  "  color = vcolor;\n"
  "}\n";

  GLFWwindow *win = c.create_window(0,"title");
  win=c.current_window(0);

  MetisVM m(buf,10000, stack, 5, glbuf, 10000);
  m.hard_reset();
  triangle_location = m.add_buffer((uint8_t*)buffer,sizeof(buffer),"triangle");
  color_location    = m.add_buffer((uint8_t*)colors,sizeof(colors),"color");

  m.add_label_ip("init");

  m.add_glgenvertexarrays(1,0);
  m.add_glbindvertexarray(0);

  m.add_glgenbuffers(2,1);

  // set up the vertex position data
  m.add_glbindbuffer(GL_ARRAY_BUFFER, 1);
  m.add_glbufferdata(GL_ARRAY_BUFFER, sizeof(buffer), triangle_location, GL_STATIC_DRAW);
  
  m.add_glbindbuffer(GL_ARRAY_BUFFER, 2);
  m.add_glbufferdata(GL_ARRAY_BUFFER, sizeof(colors), color_location, GL_STATIC_DRAW);

  m.add_data((const uint8_t *)vertex_shader, strlen(vertex_shader)+1, "vertex_shader");
  m.add_data((const uint8_t *)fragment_shader, strlen(fragment_shader)+1, "fragment_shader");
  m.add_matrix          (1,1, (uint8_t *)&angle, "angle");
  m.add_matrix          (1,1, (uint8_t *)&delta, "delta");

  m.add_glcreateshader(GL_VERTEX_SHADER, 3);
  m.add_glshadersource(m.get_label("vertex_shader"), 3);
  m.add_glcompileshader(3);

  m.add_glcreateshader(GL_FRAGMENT_SHADER, 4);
  m.add_glshadersource(m.get_label("fragment_shader"), 4);
  m.add_glcompileshader(4);
 
  m.add_glcreateprogram(5);
  m.add_glattachshader(5,3);
  m.add_glattachshader(5,4);
  m.add_gllinkprogram(5);
  m.add_gluseprogram(5);
  m.add_glgetuniformlocation(5, 6, "angle");
  m.add_storei(REGA, m.get_label("angle"));
  m.add_storei(REGB, m.get_label("delta"));
  m.add_gluniformfv(REGA, 6);
  m.add_glclearcolor(0, 0, 0.4 , 0);
  m.add_end();

  m.add_label_ip("mainloop");
  m.add_storei(REGA, m.get_label("angle"));
  m.add_storei(REGB, m.get_label("delta"));
  m.add_matrix_add(REGA, REGB, m.get_label("angle"));
  m.add_glclear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
  m.add_gluniformfv(REGA, 6);
  m.add_glenablevertexattribarray(0);
  m.add_glbindbuffer(GL_ARRAY_BUFFER, 1);
  m.add_glvertexattribpointer(0,3,GL_FLOAT,GL_FALSE,12,0);
  m.add_glenablevertexattribarray(1);
  m.add_glbindbuffer(GL_ARRAY_BUFFER, 2);
  m.add_glvertexattribpointer(1,3,GL_FLOAT,GL_FALSE,12,0);
  m.add_gldrawarrays(GL_TRIANGLES, 0, 3);
  //m.add_gldisablevertexattribarray(0);

  m.add_end();
  // make sure error conditions for creating windows work.
  //REQUIRE_THROWS_AS(c.create_window(-1,500,500,"title",NULL,NULL), MetisException);
  //REQUIRE_THROWS_AS(c.create_window(8,500,500,"title",NULL,NULL), MetisException);

  //REQUIRE(win != NULL);
  //REQUIRE(c.current_window(0) != NULL);

  //REQUIRE_THROWS_AS(c.current_window(1), MetisException);

  m.save("wintest.metis");
  m.eval("init");
  while(!glfwWindowShouldClose(win)) {
    m.eval("mainloop");
    glfwSwapBuffers(win);
    glfwPollEvents();
  }
  c.close_window(0); 
}

TEST_CASE( "texture stuff", "[MetisContext]") {
  
  uint8_t buf[1000];
  uint8_t glbuf[1000];
  uint64_t stack[5];
  uint64_t triangle_location;
  uint64_t texture_location;
  uint64_t color_location;
  float buffer[15] =  {-0.8,-0.8,0.0,  0.0, 10.0,
                       0.8,-0.8,0.0,  10.0, 0.0,
                       0.0, 0.8,0.0,  5.0, 10.0};
  float angle     = 0.0; 
  float delta     = 0.01;

  float texture[] = {
    0.5f, 0.0f, 0.0f,   1.0f, 1.0f, 1.0f,
    1.0f, 1.0f, 1.0f,   0.0f, 0.0f, 0.0f
  };


  const char *vertex_shader =
  "#version 400\n"
  "in vec3 vp;\n"
  "in vec2 intex;\n"
  "out vec2 texcoord;\n"
  "void main () {\n"
  "  texcoord = intex;\n"
  "  gl_Position = vec4(vp.x,\n"
  "                     vp.y,\n"
  "                     0.0,\n"
  "                     1.0);\n"
  "}\n";

  const char *fragment_shader =
  "#version 330 core\n"
  "out vec4 color;\n"
  "in vec2 texcoord;\n"
  "uniform sampler2D tex;\n"
  "void main(){\n"
  "  //color = vec4(1.0,1.0,1.0,1.0);\n"
  "  color = texture(tex, texcoord);\n"
  "}\n";
  GLFWwindow *win = c.create_window(0,"title");
  win=c.current_window(0);
  
  MetisVM m(buf,1000, stack, 5, glbuf, 1000);
  m.hard_reset();
  
  triangle_location = m.add_buffer((uint8_t*)buffer,sizeof(buffer),"triangle");
  texture_location  = m.add_buffer((const uint8_t *)texture, sizeof(texture), "texture");
  m.add_label_ip("init");


  m.add_glgenvertexarrays(1,0);
  m.add_glbindvertexarray(0);

  m.add_glgenbuffers(1,1);

  // set up the vertex position data
  m.add_glbindbuffer(GL_ARRAY_BUFFER, 1);
  m.add_glbufferdata(GL_ARRAY_BUFFER, sizeof(buffer), triangle_location, GL_STATIC_DRAW);
  

  m.add_data((const uint8_t *)vertex_shader, strlen(vertex_shader)+1, "vertex_shader");
  m.add_data((const uint8_t *)fragment_shader, strlen(fragment_shader)+1, "fragment_shader");

  m.add_glcreateshader(GL_VERTEX_SHADER, 3);
  m.add_glshadersource(m.get_label("vertex_shader"), 3);
  m.add_glcompileshader(3);

  m.add_glcreateshader(GL_FRAGMENT_SHADER, 4);
  m.add_glshadersource(m.get_label("fragment_shader"), 4);
  m.add_glcompileshader(4);
 
  m.add_glcreateprogram(5);
  m.add_glattachshader(5,3);
  m.add_glattachshader(5,4);
  m.add_gllinkprogram(5);
  m.add_gluseprogram(5);

  m.add_glgentextures(1,6);
  m.add_glactivetexture(GL_TEXTURE0);
  m.add_glbindtexture(GL_TEXTURE_2D, 6);
  m.add_gltexparameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
  m.add_gltexparameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
  m.add_gltexparameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
  m.add_gltexparameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
  m.add_glgeneratemipmap(GL_TEXTURE_2D);
  m.add_glteximage2d(GL_TEXTURE_2D, 0, GL_RGB, 2, 2, 0, GL_RGB, GL_FLOAT, texture_location); 
  m.add_end();
  
  
  m.add_label_ip("mainloop");
  m.add_glenablevertexattribarray(0);
  m.add_glbindbuffer(GL_ARRAY_BUFFER, 1);
  m.add_glvertexattribpointer(0,3,GL_FLOAT,GL_FALSE,20,0);
  m.add_glenablevertexattribarray(1);
  //m.add_glbindbuffer(GL_TEXTURE_BUFFER, 1);
  m.add_glvertexattribpointer(1,2,GL_FLOAT,GL_FALSE,20,12);
  m.add_gldrawarrays(GL_TRIANGLES, 0, 3);
  //m.add_gldisablevertexattribarray(0);
  m.add_end(); 

  /*
  m.add_label_ip("mainloop");
  m.add_glenablevertexattribarray(0);
  m.add_glbindbuffer(GL_ARRAY_BUFFER, 1);
  m.add_glvertexattribpointer(0,3,GL_FLOAT,GL_FALSE,12,0);
  m.add_glenablevertexattribarray(1);
  m.add_glbindbuffer(GL_ARRAY_BUFFER, 2);
  m.add_glvertexattribpointer(1,3,GL_FLOAT,GL_FALSE,12,0);
  m.add_gldrawarrays(GL_TRIANGLES, 0, 3);
  m.add_gldisablevertexattribarray(0);
  */ 
  m.eval("init");

  glClearColor(0.0f,0.0f,0.4f,0.0f);
  while(!glfwWindowShouldClose(win)) {
    glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
    m.eval("mainloop");
    glfwSwapBuffers(win);
    glfwPollEvents();
  }
  c.close_window(0); 
}

