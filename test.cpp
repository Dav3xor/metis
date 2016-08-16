#define CATCH_CONFIG_MAIN

#include <chrono>

#include "metis.hpp"
#include "catch.hpp"




TEST_CASE( "addressing modes", "[MetisVM]" ) {
  uint8_t buf[10000];
  uint64_t stack[5];
  MetisVM m(buf,10000, stack, 5, NULL, 0);
  
  REQUIRE( BUILD_ADDR(REGB, REGC) == 0x21);
  REQUIRE( GET_SRC(0x21) == 1);
  REQUIRE( GET_DEST(0x21) == 2);
};


TEST_CASE( "labels", "[MetisVM]" ) {
  uint8_t buf[10000];
  uint64_t stack[5];
  MetisVM m(buf,10000, stack, 5, NULL, 0);
  m.add_label_ip("start");
  m.add_storei(REGA,100);
  m.add_storei(REGB,100);
  m.add_label_ip("end");

  m.add_label_val("arbitrary", 555);

  m.eval();

  REQUIRE( m.get_label("start") == 0);
  REQUIRE( m.get_label("end") == INS_STOREI_SIZE*2);
  REQUIRE( m.get_label("arbitrary") == 555);
  REQUIRE_THROWS_AS( m.get_label("x"), out_of_range);
}

TEST_CASE( "stack push/pop", "[MetisVM]" ) {
  uint8_t buf[10000];
  uint64_t stack[5];
  MetisVM m(buf,10000, stack, 5, NULL, 0);
  m.hard_reset(); 

  // test that a simple store works 
  m.add_storei(STACK_PUSH,100);

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
  MetisVM m(buf,10000, stack, 5, NULL, 0);
  m.hard_reset();
  m.add_storei(REGA, 0);
  m.add_inc(REGA,REGA);

  m.eval();

  REQUIRE(m.get_registers()[REGA] == 1);

  m.add_inc(REGA,REGA);

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


  m.add_storei(REGA,5);
  m.add_storei(REGB,0);
  m.add_storei(REGC, 256);
  m.add_label_ip("loop start");        
  m.add_inc(REGB, REGB);
  m.add_dec(REGA, REGA);
  m.add_jizz(REGA,REGC);
  m.add_jumpi(m.get_label("loop start"));
  m.add_end();

  m.eval();

  REQUIRE( m.get_registers()[REGA] == 0 );
  REQUIRE( m.get_registers()[REGB] == 5 );
  REQUIRE( m.get_registers()[REGC] == 256 );
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
  printf("---------------------------------\n");
  printf("perf test duration:      %ldms\n", duration.count());
  printf("instructions per second: %f\n", ipers);
  printf("---------------------------------\n");
}
    


TEST_CASE( "load/save", "[MetisVM]" ) {
  uint8_t buf[10000];
  uint64_t stack[5];
  MetisVM m(buf,10000, stack, 5, NULL, 0);
  m.hard_reset();
  
  m.add_label_ip("hi!");
  m.add_storei(REGA, 0xDEADBEEF);
  m.add_label_ip("hi again!");
  m.add_end();

  m.save("test.metis");

  m.hard_reset();

  m.load("test.metis"); 
 
  REQUIRE(m.get_label("hi!") == 0);
  REQUIRE(m.get_label("hi again!") == 10);
 
  m.eval();
  
  REQUIRE(m.get_registers()[REGA] == 0xDEADBEEF);
};

TEST_CASE( "window stuff", "[MetisContext]") {
  MetisContext c; 
  // make sure error conditions for creating windows work.
  //REQUIRE_THROWS_AS(c.create_window(-1,500,500,"title",NULL,NULL), MetisException);
  //REQUIRE_THROWS_AS(c.create_window(8,500,500,"title",NULL,NULL), MetisException);

  GLFWwindow *win = c.create_window(0,500,500,"title",NULL,NULL);
  //REQUIRE(win != NULL);
  //REQUIRE(c.current_window(0) != NULL);

  //REQUIRE_THROWS_AS(c.current_window(1), MetisException);
  while(!glfwWindowShouldClose(win)) {
    glClear(GL_COLOR_BUFFER_BIT);
    glfwSwapBuffers(win);
    glfwPollEvents();
  }
}



