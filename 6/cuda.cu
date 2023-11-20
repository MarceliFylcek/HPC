#include "utility.h"
#include <stdio.h>
#include <stdlib.h>
#include <cuda_runtime.h>
#include <sys/time.h>
#include <math.h>


__host__
void errorexit(const char *s) {
    printf("\n%s",s);	
    exit(EXIT_FAILURE);	 	
}


__global__
void calculate(int* result, int end, int size, long number) {
  int myindex = blockIdx.x*blockDim.x+threadIdx.x; 
   //each process
  int mine = myindex +2;
  int step = size;
  
  result[myindex] = 0;

  if (number > mine)
  {
    if(number%(long)mine == 0 )
    result[myindex] = 1;
  }
    
}

int main(int argc,char **argv) {

  Args ins__args;
  parseArgs(&ins__args, &argc, argv);
  
  //program input argument
  long inputArgument = ins__args.arg; 

  struct timeval ins__tstart, ins__tstop;
  gettimeofday(&ins__tstart, NULL);

  int threadsinblock = 1024;
  int blocksingrid = 10000;

  int size = threadsinblock * blocksingrid;
  int end=sqrt(inputArgument);
  int *dresults=NULL;
  int result = 0;


  if(size < inputArgument)
  {
    errorexit("Number too big");
  }

  int *hresults=(int*)malloc(size*sizeof(int));
  if (!hresults) errorexit("Error allocating memory on the host");


  // run your CUDA kernel(s) here

  //memory allocation on device (GPU)
  if (cudaSuccess!=cudaMalloc((void **)&dresults,size*sizeof(int)))
    errorexit("Error allocating memory on the GPU");

  //call kernel on GPU – calculation are executed on GPU
  calculate<<<blocksingrid,threadsinblock>>>(dresults, end, size, inputArgument);
  if (cudaSuccess!=cudaGetLastError())
    errorexit("Error during kernel launch");

  //copy all elements from device to host
  if (cudaSuccess!=cudaMemcpy(hresults,dresults,size*sizeof(int),cudaMemcpyDeviceToHost))
      errorexit("Error copying results");


  // synchronize/finalize your CUDA computations
    for(int i=0;i<size;i++) {
      if (hresults[i]==1)
      {
        result = 1;
        break;
      }
    }

  gettimeofday(&ins__tstop, NULL);
  ins__printtime(&ins__tstart, &ins__tstop, ins__args.marker);

  if(result == 0)
  {
    printf("%ld is prime\n", inputArgument);
  }
  else
  {
      printf("%ld is not prime\n", inputArgument);
  }

}
