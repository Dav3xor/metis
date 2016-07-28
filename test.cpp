#define CATCH_CONFIG_MAIN

#include "metis.hpp"
#include "catch.hpp"

TEST_CASE( "addressing modes", "[MetisVM]" ) {
  uint8_t buf[10000];
  uint64_t stack[5];
  MetisVM m(buf,10000, stack, 5);
  
  REQUIRE( BUILD_ADDR(REGB, REGC) == 0x21);
  REQUIRE( GET_SRC(0x21) == 1);
  REQUIRE( GET_DEST(0x21) == 2);
};


TEST_CASE( "labels", "[MetisVM]" ) {
  uint8_t buf[10000];
  uint64_t stack[5];
  MetisVM m(buf,10000, stack, 5);
  m.add_label("start");
  m.add_storei(REGA,100);
  m.add_storei(REGB,100);
  m.add_label("end");

  m.eval();
  REQUIRE( m.get_label("start") == 0);
  REQUIRE( m.get_label("end") == 64);
  REQUIRE_THROWS_AS( m.get_label("x"), out_of_range);
}

TEST_CASE( "stack push/pop", "[MetisVM]" ) {
  uint8_t buf[10000];
  uint64_t stack[5];
  MetisVM m(buf,10000, stack, 5);
 

  // test that a simple store works 
  m.add_storei(STACK_PUSH,100);
  m.add_end();

  m.eval();

  REQUIRE( m.cur_stack_size() == 1);
  REQUIRE( m.cur_stack_val() == 100 );

  // test that we can have more than one item on the stack...
  m.add_storei(STACK_PUSH,101);
 
  m.eval();
 
  REQUIRE( m.cur_stack_size() == 2);
  REQUIRE( m.cur_stack_val() == 101 );

  // test that you can move a value from a register
  // onto the stack.
  m.add_storei(REGA, 102);
  m.add_store(REGA,STACK_PUSH);
  
  m.eval();

  REQUIRE( m.cur_stack_size() == 3);
  REQUIRE( m.cur_stack_val() == 102 );

  m.add_storei(REGA, 103);
  m.add_store(REGA,STACK_PUSH);
  
  m.eval();
 
  REQUIRE( m.get_registers()[REGA] == 103);
  REQUIRE( m.cur_stack_size() == 4);
  REQUIRE( m.cur_stack_val() == 103 );

  m.add_store(STACK_POP,REGB);
  
  m.eval();
  
  REQUIRE( m.get_registers()[REGA] == 103);
  REQUIRE( m.get_registers()[REGB] == 103);
  REQUIRE( m.cur_stack_size() == 3);
  REQUIRE( m.cur_stack_val() == 102 );
  
  m.add_store(STACK_POP,REGA);
  
  m.eval();
  
  REQUIRE( m.get_registers()[REGA] == 102);
  REQUIRE( m.get_registers()[REGB] == 103);
  REQUIRE( m.cur_stack_size() == 2);
  REQUIRE( m.cur_stack_val() == 101 );

  // make sure we can fill the stack, but not go over...
  m.add_store(REGA,STACK_PUSH);
  m.add_store(REGA,STACK_PUSH);
  m.add_store(REGA,STACK_PUSH);
  
  m.eval();

  REQUIRE( m.cur_stack_size() == 5);
  
  m.add_store(REGA,STACK_PUSH);
 
  // should throw a stack full exception 
  REQUIRE_THROWS_AS(m.eval(),MetisException);

  REQUIRE( m.cur_stack_size() == 5);
  
  m.reset(); 
  m.add_store(STACK_POP, REGA);
  
  // should throw an empty stack exception
  REQUIRE_THROWS_AS(m.eval(), MetisException);

  // ditto
  REQUIRE_THROWS_AS(m.cur_stack_val(), MetisException);
}

TEST_CASE( "inc/dec", "[MetisVM]" ) {
  uint8_t buf[10000];
  uint64_t stack[5];
  MetisVM m(buf,10000, stack, 5);
  m.hard_reset();
  m.add_storei(REGA, 0);
  m.add_inc(REGA,REGA);

  m.eval();

  REQUIRE(m.get_registers()[REGA] == 1);

  m.add_inc(REGA,REGA);

  m.eval();
  REQUIRE(m.get_registers()[REGA] == 2);

  m.add_dec(REGA,REGA);

  m.eval();
  REQUIRE(m.get_registers()[REGA] == 1);
} 


TEST_CASE( "jumpi", "[MetisVM]" ) {
  uint8_t buf[10000];
  uint64_t stack[5];
  MetisVM m(buf,10000, stack, 5);
  m.hard_reset();
  m.add_storei(REGA,1);
  // TODO: I don't think 64 is right?
  m.add_jumpi(64);
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
  MetisVM m(buf,10000, stack, 5);
  m.hard_reset();
  m.add_storei(REGA,64);
  // TODO: I don't think 64 is right?
  m.add_jump(REGA);
  m.add_storei(REGA,2);
  m.add_storei(REGB,3);
  m.add_end();
  
  m.eval();

  REQUIRE( m.get_registers()[REGA] == 1 );
  REQUIRE( m.get_registers()[REGB] == 3 );
}

TEST_CASE( "load/save", "[MetisVM]" ) {
  uint8_t buf[10000];
  uint64_t stack[5];
  MetisVM m(buf,10000, stack, 5);
  
  m.add_label("hi!");
  m.add_storei(REGA, 0xDEADBEEF);
  m.add_label("hi again!");
  m.add_end();

  m.save("test.metis");

  m.hard_reset();

  m.load("test.metis"); 
 
  REQUIRE(m.get_label("hi!") == 0);
  REQUIRE(m.get_label("hi again!") == 32);
  
  m.eval();
  
  REQUIRE(m.get_registers()[REGA] == 0xDEADBEEF);
};
