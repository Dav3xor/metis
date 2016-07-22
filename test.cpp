#define CATCH_CONFIG_MAIN

#include "metis.hpp"
#include "catch.hpp"

TEST_CASE( "stack push/pop", "[MetisVM]" ) {
  uint8_t buf[10000];
  MetisVM m(buf,10000);
  
  m.add_storei(REGA_LOC,100);
  m.add_push(REGA_LOC);
  m.add_end();

  m.eval();

  REQUIRE( m.get_registers()[REGA_LOC] == 100 );
  REQUIRE( m.cur_stack_size() == 1);
  REQUIRE( m.cur_stack_val() == 100 );
}


