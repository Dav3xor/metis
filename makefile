CC     = g++
CFLAGS = -Wall -std=c++11 -ggdb
#CLIBS  = -lglfw3 -lGLEW -lX11 -lXrandr -lXinerama -lXi -lXxf86vm -lXcursor -ldl -lGL -lm -lpthread /usr/lib/x86_64-linux-gnu/libboost_program_options.so.1.54.0
CLIBS  = -lglfw3 -lboost_program_options -framework Cocoa -framework OpenGL -framework IOKit -framework CoreVideo -lm
DEPS   = metis.hpp
TESTSRCS   = add_instructions.cpp file.cpp util.cpp eval.cpp test.cpp masm.cpp
METISSRCS   = add_instructions.cpp file.cpp util.cpp eval.cpp metismain.cpp masm.cpp
TESTOBJS   = $(TESTSRCS:.cpp=.o)
METISOBJS   = $(METISSRCS:.cpp=.o)
TEST   = test
METIS  = metis

all: $(TEST) $(METIS)

$(TEST): $(TESTOBJS)
	$(CC) $(CFLAGS) -o $(TEST) $(TESTOBJS) $(CLIBS)

$(METIS): $(METISOBJS)
	$(CC) $(CFLAGS) -o $(METIS) $(METISOBJS) $(CLIBS)

.cpp.o:
	$(CC) $(CFLAGS) -c $< -o $@

depend: $(SRCS)
	makedepend $^

# ----
