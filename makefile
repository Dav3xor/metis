CC     = g++
CFLAGS = -Wall -std=c++11 
CLIBS  = -lglfw3 -lGLEW -lX11 -lXrandr -lXinerama -lXi -lXxf86vm -lXcursor -ldl -lGL -lm -lpthread
DEPS   = metis.hpp
OBJ    = add_commands.cpp file.cpp util.cpp test.cpp

%.o: %.cpp $(DEPS)
	$(CC)  $(CFLAGS) -c -o $@ @<

test: $(OBJ)
	g++ -o $@ $^  $(CFLAGS) $(CLIBS)



