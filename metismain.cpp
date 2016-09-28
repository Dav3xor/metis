#include <boost/program_options.hpp>

using namespace boost::program_options;
using namespace std;

#include "metis.hpp"

int main(int argc, char *argv[]) 
{
  MetisContext c;

  string input_file;
  options_description desc("Allowed options");
  desc.add_options()
       ("help,h", "print usage message")
       ("run,r",  value<string>(&input_file), "run <filename>");
        
  uint8_t buf[10000];
  uint8_t glbuf[10000];
  uint64_t stack[500];
  
  GLFWwindow *win = c.create_window(0,"title");
  win=c.current_window(0);
  win=c.current_window(0);

  variables_map args;
  try {
    store(parse_command_line(argc, argv, desc), args);

    if (args.count("help")) {
      cout << desc << endl;
      return 0;
    }
    if (args.count("run")) {
      cout << args["run"].as<string>() << endl;
    }
  } catch(required_option& e) {
    cerr << "ERROR: " << e.what() << endl << endl;
    return 1;
  } catch(invalid_command_line_syntax &e) {
    cerr << "ERROR: " << e.what() << endl << endl;
    return 1;
  }

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
