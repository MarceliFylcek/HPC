#include "utility.h"
#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include <mpi.h>
#include <math.h>

int main(int argc,char **argv) {

  Args ins__args;
  parseArgs(&ins__args, &argc, argv);

  //program input argument
  long inputArgument = ins__args.arg;

  struct timeval ins__tstart, ins__tstop;

  int myrank,nproc;
  long step;
  int flag  = 0;
  int result = 0;
  long mine;
  
  MPI_Init(&argc,&argv);

  // obtain my rank
  MPI_Comm_rank(MPI_COMM_WORLD,&myrank);
  // and the number of processes
  MPI_Comm_size(MPI_COMM_WORLD,&nproc);

  int end=sqrt(inputArgument);
  if(!myrank)
      gettimeofday(&ins__tstart, NULL);


  // run your computations here (including MPI communication)
  
  if (inputArgument <= 1)
  {
      flag = 1;
  }
  
  else
  {
	  //each process
	  mine = myrank +2;
	  step = nproc;
	  
	  for(long i = mine; i < end+1; i += step)
	  {
	  	if(inputArgument%i == 0 )
	  	{
	  	    flag = 1;
	  	    break;
	  	}
	  }
  }
  

  // synchronize/finalize your computations
  
  MPI_Reduce (&flag, &result, 1, MPI_INT, MPI_SUM, 0, MPI_COMM_WORLD);

  if (!myrank) {
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
  
  MPI_Finalize();

}
