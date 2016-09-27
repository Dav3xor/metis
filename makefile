CC     = g++
CFLAGS = -Wall -std=c++11 -ggdb
CLIBS  = -lglfw3 -lGLEW -lX11 -lXrandr -lXinerama -lXi -lXxf86vm -lXcursor -ldl -lGL -lm -lpthread
#CLIBS  = -lglfw3 -framework Cocoa -framework OpenGL -framework IOKit -framework CoreVideo -lm
DEPS   = metis.hpp
SRCS   = add_instructions.cpp file.cpp util.cpp eval.cpp test.cpp 
OBJS   = $(SRCS:.cpp=.o)
TEST   = test


$(TEST): $(OBJS)
	$(CC) $(CFLAGS) -o $(TEST) $(OBJS) $(CLIBS)

.cpp.o:
	$(CC) $(CFLAGS) -c $< -o $@

depend: $(SRCS)
	makedepend $^

# ----
