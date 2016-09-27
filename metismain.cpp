#include "metis.hpp"

int main(void) 
{
  MetisContext c;
  
  uint8_t buf[10000];
  uint8_t glbuf[10000];
  uint64_t stack[500];
  
  GLFWwindow *win = c.create_window(0,"title");
  win=c.current_window(0);
  win=c.current_window(0);

  MetisVM m(buf,10000, stack, 5, glbuf, 10000);
  m.load("wintest.metis");
  m.eval("init");

  while(!glfwWindowShouldClose(win)) {
    glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
    m.eval("mainloop");
    glfwSwapBuffers(win);
    glfwPollEvents();
  }
}
