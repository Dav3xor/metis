CC     = g++
CFLAGS = -Wall -std=c++11 -ggdb
CLIBS  = -lglfw3 -lGLEW -lX11 -lXrandr -lXinerama -lXi -lXxf86vm -lXcursor -ldl -lGL -lm -lpthread
DEPS   = metis.hpp
SRCS   = add_instructions.cpp file.cpp util.cpp test.cpp
OBJS   = $(SRCS:.cpp=.o)
MAIN   = test


$(MAIN): $(OBJS)
	$(CC) $(CFLAGS) -o $(MAIN) $(OBJS) $(CLIBS)

.cpp.o:
	$(CC) $(CFLAGS) -c $< -o $@

depend: $(SRCS)
	makedepend $^

# ----
