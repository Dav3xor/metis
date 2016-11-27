#include <boost/program_options.hpp>
#include <iostream>
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
       ("assemble,a",  value<string>(&input_file), "assemble <filename>")
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
  } catch(required_option& e) {
    cerr << "ERROR: " << e.what() << endl << endl;
    return 1;
  } catch(invalid_command_line_syntax &e) {
    cerr << "ERROR: " << e.what() << endl << endl;
    return 1;
  }
  if (args.count("assemble")) {
    MetisVM m(buf,10000, stack, 5, glbuf, 10000);
    MetisASM a;
    a.assemble(args["assemble"].as<string>(),m);
    m.save(args["assemble"].as<string>() + ".metis");
  }
  if (args.count("run")) {
    //cout << args["run"].as<string>() << endl;
    MetisVM m(buf,10000, stack, 5, glbuf, 10000);
    m.load(args["run"].as<string>());
    m.eval("init");

    while(!glfwWindowShouldClose(win)) {
      m.eval("mainloop");
      glfwSwapBuffers(win);
      glfwPollEvents();
    }
  }
}
