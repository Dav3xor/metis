#include "metis.hpp"


void error_callback(int error, const char* description) {
  fprintf(stderr, "Error: %s\n", description);
}

void print_matrix(float *matrix, uint8_t width, uint8_t height) {
  for (int i=0; i<height; i++) {
    for (int j=0; j<width; j++) {
      printf("%.2f ", matrix[i*width+j]);
    }
    printf("\n");
  }
}
