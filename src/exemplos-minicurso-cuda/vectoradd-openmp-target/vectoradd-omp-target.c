#include <stdio.h>
#include <stdlib.h>
#include <semaphore.h>
#include <inttypes.h>
#include <stdbool.h>

#ifdef _OPENMP
#include <omp.h>
#else
#define omp_get_thread_num() 0
#define omp_get_num_threads() 1
#define omp_get_num_procs()                             \
  (system("cat /proc/cpuinfo | grep 'processor' | wc -l"))
#endif

#define THRESHOLD 1024

// Entrada e saída.
float *h_a;
float *h_b;
float *h_c;

int n = 0;

void init_array(int n) {
  fprintf(stdout, "Thread[%lu]: Initializing the arrays.\n", (long int) pthread_self());
  int i;
  // Initialize vectors on host.
	for (i = 0; i < n; i++) {
	  h_a[i] = 0.5;
	  h_b[i] = 0.5;
	}
}

void print_array(int n) {
  int i;
  for (i = 0; i < n; i++) {
    fprintf(stdout, "Thread[%lu]: h_c[%07d]: %f.\n", (long int) pthread_self(), i, h_c[i]);
  }
}

void check_result(int n){
  // Soma dos elementos do array C e divide por N, o valor deve ser igual a 1.
  int i;
  float sum = 0;
  fprintf(stdout, "Thread[%lu]: Checking.\n", (long int) pthread_self());
  
  for (i = 0; i < n; i++) {
	  sum += h_c[i];
	}
	
  fprintf(stdout, "Thread[%lu]: Final Result: (%f, %f).\n", (long int) pthread_self(), sum, (float)(sum / (float)n));
}


void vecaddgpu(float *c, float *a, float *b){
  #pragma omp target data map(to: a[0:n], b[:n]) map(from: c[0:n]) if(n>THRESHOLD)
  {
    #pragma omp target if(n>THRESHOLD)
    #pragma omp parallel for if(n>THRESHOLD)
    for( int i = 0; i < n; ++i ){
      c[i] = a[i] + b[i];
    }
  }
}

int main(int argc, char *argv[]) {
  int i, num_elements, num_threads = 0;
  long id, ii, ff;

  if(argc < 2){
    fprintf(stderr, "Uso: %s <num_elements>\n", argv[0]);
    exit(0);
  }

  n = atoi(argv[1]);

  fprintf(stdout, "Thread[%lu]: Allocating the arrays.\n", (long int) pthread_self());
  h_a = (float*) malloc(n*sizeof(float));
  h_b = (float*) malloc(n*sizeof(float));
  h_c = (float*) malloc(n*sizeof(float));

  init_array(n);

  fprintf(stdout, "Thread[%lu]: Before vectoradd function.\n", (long int) pthread_self());

  vecaddgpu(h_c, h_a, h_b);

  fprintf(stdout, "Thread[%lu]: Checking the result.\n", (long int) pthread_self());
  check_result(n);

  fprintf(stdout, "Thread[%lu]: Fui, Tchau!\n", (long int) pthread_self());
  
  return 0;
}
