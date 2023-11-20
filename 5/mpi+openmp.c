#include "utility.h"
#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include <mpi.h>
#include <omp.h>
#include <math.h>

#define RANGESIZE 1
#define DATA 0
#define RESULT 1
#define FINISH 2
//#define PRECISION 0.000001

double f (double x)
{
    return sin(x) * sin(x) / x;
}

double SimpleIntegration (double a, double b, double precision)
{
    double sum = 0;

    int i_range = (int)((b-a)/precision);
    double num;

    #pragma omp parallel for shared(a, b, i_range, precision) private(num) reduction(+:sum)
    for (int i = 0; i < i_range; i++)
    {
      num = a + (i*precision); 
      sum += f(num) * precision;
    }
    return sum;

    printf("%lf", precision);


    //Original
    //  double i;
    //  double sum = 0;
    //  for (i = a; i < b; i += precision)
	  //    sum += f (i) * precision;
    //  return sum;
}

int main(int argc,char **argv) {

  double a = 1, b = 100;
  double range[2];
  double result = 0, resulttemp;
  int sentcount = 0;
  int i;
  MPI_Status status;


  Args ins__args;
  parseArgs(&ins__args, &argc, argv);

  //set number of threads
  omp_set_num_threads(ins__args.n_thr);
  
  //program input argument
  long inputArgument = ins__args.arg;
  double precision = (double)(1/(float)inputArgument);

  struct timeval ins__tstart, ins__tstop;

  int threadsupport;
  int myrank,nproc;

  // Initialize MPI with desired support for multithreading -- state your desired support level

  MPI_Init_thread(&argc, &argv,MPI_THREAD_FUNNELED,&threadsupport); 

  if (threadsupport<MPI_THREAD_FUNNELED) {
    printf("\nThe implementation does not support MPI_THREAD_FUNNELED, it supports level %d\n",threadsupport);
    MPI_Finalize();
    return -1;
  }
  
  // obtain my rank
  MPI_Comm_rank(MPI_COMM_WORLD,&myrank);
  // and the number of processes
  MPI_Comm_size(MPI_COMM_WORLD,&nproc);

  if (!myrank) 
    gettimeofday(&ins__tstart, NULL);
  // run your computations here (including MPI communication and OpenMP stuff)
  

  if (nproc < 2)
  {
    printf ("Run with at least 2 processes");
    MPI_Finalize ();
    return -1;
  }

  if (((b - a) / RANGESIZE) < 2 * (nproc - 1))
  {
    printf ("More subranges needed");
    MPI_Finalize ();
    return -1;
  }

  //MASTER
  if (myrank == 0)
  {
    // now the master will distribute the data and slave processes will perform computations
    range[0] = a;
    // first distribute some ranges to all slaves
    for (i = 1; i < nproc; i++)
    {
        range[1] = range[0] + RANGESIZE;
        #ifdef DEBUG
          printf ("\nMaster sending range %f,%f to process %d",
              range[0], range[1], i);
          fflush (stdout);
        #endif
        // send it to process i
        MPI_Send (range, 2, MPI_DOUBLE, i, DATA, MPI_COMM_WORLD);
        sentcount++;
        range[0] = range[1];
    }

    do
    {
        // distribute remaining subranges to the processes which have completed their parts
        MPI_Recv (&resulttemp, 1, MPI_DOUBLE, MPI_ANY_SOURCE, RESULT,
        MPI_COMM_WORLD, &status);
        result += resulttemp;
        #ifdef DEBUG
            printf ("\nMaster received result %f from process %d",
            resulttemp, status.MPI_SOURCE);
            fflush (stdout);
        #endif
        // check the sender and send some more data
        range[1] = range[0] + RANGESIZE;
        if (range[1] > b)
          range[1] = b;
        #ifdef DEBUG
        printf ("\nMaster sending range %f,%f to process %d",
            range[0], range[1], status.MPI_SOURCE);
        fflush (stdout);
        #endif
        MPI_Send (range, 2, MPI_DOUBLE, status.MPI_SOURCE, DATA,
          MPI_COMM_WORLD);
        range[0] = range[1];
    }
    while (range[1] < b);


    // now receive results from the processes
    for (i = 0; i < (nproc - 1); i++)
    {
        MPI_Recv (&resulttemp, 1, MPI_DOUBLE, MPI_ANY_SOURCE, RESULT,
        MPI_COMM_WORLD, &status);
        #ifdef DEBUG
        printf ("\nMaster received result %f from process %d",
        resulttemp, status.MPI_SOURCE);
        fflush (stdout);
        #endif
        result += resulttemp;
    }
    // shut down the slaves
    for (i = 1; i < nproc; i++)
    {
      MPI_Send (NULL, 0, MPI_DOUBLE, i, FINISH, MPI_COMM_WORLD);
    }
    // now display the result
    printf ("\nHi, I am process 0, the result is %f\n", result);
  }

  //SLAVE
  else
  {
    do
	  {
      MPI_Probe (0, MPI_ANY_TAG, MPI_COMM_WORLD, &status);

	    if (status.MPI_TAG == DATA)
	    {
        MPI_Recv (range, 2, MPI_DOUBLE, 0, DATA, MPI_COMM_WORLD,
				                                                  &status);
      // compute my part
      //printf("%lf", precision);
      resulttemp = SimpleIntegration (range[0], range[1], precision);
      // send the result back
      MPI_Send (&resulttemp, 1, MPI_DOUBLE, 0, RESULT,
          MPI_COMM_WORLD);
      }
	  }
	  while (status.MPI_TAG != FINISH);
  }

  // synchronize/finalize your computations

  if (!myrank) {
    gettimeofday(&ins__tstop, NULL);
    ins__printtime(&ins__tstart, &ins__tstop, ins__args.marker);
  }
    
  MPI_Finalize();
  
}
