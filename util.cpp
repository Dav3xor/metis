#include "metis.hpp"


void error_callback(int error, const char* description) {
  fprintf(stderr, "Error: %s\n", description);
}

void print_glerrors(unsigned int line, char *file) {
  bool got_one = false;
  GLenum err = GL_NO_ERROR;
  while((err = glGetError()) != GL_NO_ERROR) {
    got_one = true; 
    printf("MetisVM GL Error: %d -- line: %d, file: %s\n", err, line, file);
  } 
  if(got_one) {
    exit(0);
  }
}

void print_matrix(float *matrix, uint8_t width, uint8_t height) {
  for (int i=0; i<height; i++) {
    for (int j=0; j<width; j++) {
      printf("%.2f ", matrix[i*width+j]);
    }
    printf("\n");
  }
}
