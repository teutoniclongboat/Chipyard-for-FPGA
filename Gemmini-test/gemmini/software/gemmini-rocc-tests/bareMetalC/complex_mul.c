// See LICENSE for license details.

#include <stdint.h>
#include <stddef.h>
#include <assert.h>
#include <stdlib.h>
#include <stdio.h>
#ifndef BAREMETAL
#include <sys/mman.h>
#endif
#include "include/gemmini_testutils.h"
#include <complex.h>




int main() {
#ifndef BAREMETAL
    if (mlockall(MCL_CURRENT | MCL_FUTURE) != 0) {
      perror("mlockall failed");
      exit(1);
    }
#endif

  printf("Flush Gemmini TLB of stale virtual addresses\n");
  gemmini_flush(0);

  printf("Initialize our input and output matrices in main memory\n");
  elem_z In1[DIM][DIM];
  elem_z In2[DIM][DIM];
  elem_z Out[DIM][DIM];

  elem_z z1; // z1 = (real)+(imag)*i
  z1.real = 0;
  z1.imag = 1;
  
  for (size_t i = 0; i < DIM; i++){
    for(size_t j = 0; j < DIM; j++){
      if (i == j){
        In1[i][j] = z1;
        In2[i][j] = z1;
      }
    }
  }
  
  printf("Input matrix: \n");
  printComplexMat(In1);

  printf("Calculate the scratchpad addresses of all our matrices\n");
  printf("  Note: The scratchpad is \"row-addressed\", where each address contains one matrix row\n");
  size_t In1_sp_addr = 0;
  size_t In2_sp_addr = DIM;
  size_t Out_sp_addr = 2*DIM;

  printf("Move \"In\" matrix from main memory into Gemmini's scratchpad\n");
  gemmini_config_ld(DIM * sizeof(elem_t));
  gemmini_config_st(DIM * sizeof(elem_t));
  gemmini_mvin(In1, In1_sp_addr);
  gemmini_mvin(In2, In2_sp_addr);

  printf("Multiply \"In1\" and \"In2\" matrix to the Out matrix\n");
  cmplx_mult(In1, In2, Out);
  printf("Output matrix (Input * Input): \n");
  printComplexMat(Out);
  
  printf("Move \"Out\" matrix from Gemmini's scratchpad into main memory\n");
  gemmini_config_st(DIM * sizeof(elem_t));
  gemmini_mvout(Out, Out_sp_addr);

  printf("Fence till Gemmini completes all memory operations\n");
  gemmini_fence();

  printf("Complex matrix multiplication is complete, as expected\n");
  exit(0);
}

