#include "utility.h"
#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include <mpi.h>
#include <math.h>

#define DATA 1
#define FINISH 2

int main(int argc,char **argv) {

  Args ins__args;
  parseArgs(&ins__args, &argc, argv);

  //program input argument
  long inputArgument = ins__args.arg;

  struct timeval ins__tstart, ins__tstop;

  int myrank,nproc;
  long step;
  int final_result = 0;
  long mine;
  int found = 0; 
  int iterations = 0 ;
  int flag = 0;

  MPI_Init(&argc,&argv);
  MPI_Status status;

  // obtain my rank
  MPI_Comm_rank(MPI_COMM_WORLD,&myrank);
  // and the number of processes
  MPI_Comm_size(MPI_COMM_WORLD,&nproc);

  if (inputArgument < 0) {
    printf("%ld is negative\n", inputArgument);
    fflush(stdout);
    MPI_Finalize();
    return -1;
  }

  int end=sqrt(inputArgument);

  if(!myrank)
  {
     printf("Num of processes: %d\n", nproc);
     fflush(stdout);
     gettimeofday(&ins__tstart, NULL);
  }
      
  //each process
  mine = myrank +2;
  step = nproc;
  
  MPI_Request request;

  //Receive from any source, no data just 
  MPI_Irecv(NULL, 0, MPI_INT, MPI_ANY_SOURCE, FINISH, MPI_COMM_WORLD, &request);

  iterations = 0;
 
  for(long i = mine; i < end+1; i+= step)
  {
    iterations++;
    if(inputArgument%i == 0)
    {
      found = 1;
      printf("[%d]Found it. My iterations: %d\n", myrank, iterations);
      fflush(stdout);
      break;
    }
    MPI_Test(&request, &flag, &status);
    if (flag)
    {
      printf("[%d] %d has found it. My iterations %d\n", myrank, status.MPI_SOURCE, iterations);
      fflush(stdout);
      break;
    }   
  }
  
  //It was me who found it
  if (found == 1)
  {
    //I'm telling others
    for(int i = 0; i < nproc; i++)
    {
      if (i!=myrank)
        MPI_Send(NULL, 0, MPI_INT, i, FINISH, MPI_COMM_WORLD);
    }
  }
  else if(flag == 0)
  {
    printf("[%d]I have finised my search. My iterations: %d\n", myrank, iterations);
    fflush(stdout);
  }


  MPI_Reduce(&found, &final_result, 1, MPI_INT, MPI_SUM, 0, MPI_COMM_WORLD);


  if (!myrank) {
  gettimeofday(&ins__tstop, NULL);
  ins__printtime(&ins__tstart, &ins__tstop, ins__args.marker);
  if(final_result == 0)
  {
    printf("%ld is prime\n", inputArgument);
    fflush(stdout);
  }
  else
  {
      printf("%ld is not prime\n", inputArgument);
      fflush(stdout);
  }
  }
  MPI_Finalize();
}



