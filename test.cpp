#define CATCH_CONFIG_MAIN

#include "metis.hpp"
#include "catch.hpp"

TEST_CASE( "stack push/pop", "[MetisVM]" ) {
  uint8_t buf[10000];
  MetisVM m(buf,10000);
  
  m.add_storei(STACK_PUSH_LOC,100);
  m.add_end();

  m.eval();

  REQUIRE( m.cur_stack_size() == 1);
  REQUIRE( m.cur_stack_val() == 100 );

  m.add_storei(STACK_PUSH_LOC,101);
 
  m.eval();
 
  REQUIRE( m.cur_stack_size() == 2);
  REQUIRE( m.cur_stack_val() == 101 );

  m.add_storei(REGA_LOC, 102);
  m.add_push(REGA_LOC);
  
  m.eval();

  REQUIRE( m.cur_stack_size() == 3);
  REQUIRE( m.cur_stack_val() == 102 );

  m.add_storei(REGA_LOC, 103);
  m.add_store(REGA_LOC,STACK_PUSH_LOC);
  
  m.eval();
 
  REQUIRE( m.get_registers()[REGA_LOC] == 103);
  REQUIRE( m.cur_stack_size() == 4);
  REQUIRE( m.cur_stack_val() == 103 );
}


