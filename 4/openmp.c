#include "utility.h"
#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include <omp.h>


int main(int argc,char **argv) 
{
  Args ins__args;
  parseArgs(&ins__args, &argc, argv);

  //set number of threads
  omp_set_num_threads(ins__args.n_thr);
  
  //program input argument
  long inputArgument = ins__args.arg; 

  struct timeval ins__tstart, ins__tstop;
  gettimeofday(&ins__tstart, NULL);

  double precision = inputArgument;
  double pi =0;
  int mine, sign;
  // run your computations here (including OpenMP stuff)

  omp_lock_t writelock;
  omp_init_lock(&writelock);
  int threadnum = ins__args.n_thr;

  if (precision<threadnum) 
  {
    printf("Precision smaller than the number of threads - try again.");
    return -1;
  }
  else
  {
    int myrank = 0;
    //omp_get_thread_num();

    // each process performs computations on its part
    # pragma omp parallel private(mine, myrank, sign) reduction(+:pi)
    {
      myrank = omp_get_thread_num(); 
      mine=myrank*2+1;
      sign=(((mine-1)/2)%2)?-1:1;
      for (;mine<precision;) 
      {
        // printf("\nProcess %d %d %d", myrank,sign,mine);
        // fflush(stdout);
        pi+=sign/(double)mine;
        mine+=2*threadnum;
        sign=(((mine-1)/2)%2)?-1:1;
      }
    }

  pi *= 4;
  printf("%f\n", pi);
  
  // synchronize/finalize your computations
  gettimeofday(&ins__tstop, NULL);
  ins__printtime(&ins__tstart, &ins__tstop, ins__args.marker);

  }
}
