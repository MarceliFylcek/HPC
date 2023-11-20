#include "utility.h"
#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include <mpi.h>
#include <time.h>

#define DATA 0
#define RESULT 1
#define FINISH 2
#define TASK_FINISHED 4
#define MAX_VALUE 1001

//number of values per slave
#define RANGESIZE 200


#define DEBUG

int main(int argc,char **argv) {
  
  Args ins__args;
  parseArgs(&ins__args, &argc, argv);

  //program input argument
  long inputArgument = ins__args.arg; 
  
  int* tab = malloc(sizeof(int)*inputArgument);
  int* temp_tab = malloc(sizeof(int)*RANGESIZE);
  int* temp_results = malloc(sizeof(int)*MAX_VALUE);
  int* results = malloc(sizeof(int)*MAX_VALUE);
  

  MPI_Status status;

  struct timeval ins__tstart, ins__tstop;

  int myrank,nproc;
  
  MPI_Init(&argc,&argv);

  // obtain my rank
  MPI_Comm_rank(MPI_COMM_WORLD,&myrank);
  // and the number of processes
  MPI_Comm_size(MPI_COMM_WORLD,&nproc);

  if(!myrank)
      gettimeofday(&ins__tstart, NULL);

  // run your computations here (including MPI communication)
  
  if (nproc < 2)
    {
        printf ("Run with at least 2 processes\n");
        fflush(stdout);
    MPI_Finalize ();
    return -1;
    }
 
    //100 ranges  >=200 slaves
    if(inputArgument/RANGESIZE < 2*(nproc-1))
    {
        printf ("More subranges needed\n");
        fflush(stdout);
	MPI_Finalize ();
	return -1;
    }


   //MASTER
   if(myrank == 0)
   {
        for(int i = 0; i<MAX_VALUE; i++)
        { 
            results[i] = 0;
            temp_results[i] = 0;
        }

       srand(time(NULL));
       
       //num of packets to be sent
       int packets = inputArgument/RANGESIZE;

       int remainder = inputArgument%RANGESIZE;

       printf("[MASTER]%d packets of %d size + %d numbers\n", packets, RANGESIZE, remainder);
       fflush(stdout);

       //num of packets sent
       int sentcount = 0;
       
       //SEND IT to everybody
       for(int i=1; i<nproc; i++)
       {
           //get random numbers
           for(int j = 0; j<RANGESIZE; j++)
           {
               // 0-1000
               temp_tab[j] = rand()%MAX_VALUE;
           }

          #ifdef DEBUG
            printf("[MASTER]sending packet number: %d to slave %d\n", sentcount, i);
            fflush(stdout);
            #endif

           MPI_Send(temp_tab, RANGESIZE, MPI_INT, i, DATA, MPI_COMM_WORLD);
           sentcount++;

           
       } 

       do
       {
           for(int j = 0; j<RANGESIZE; j++)
           {
               // 0-1000
               temp_tab[j] = rand()%MAX_VALUE;
           }
           MPI_Recv (NULL, 0, MPI_INT, MPI_ANY_SOURCE, TASK_FINISHED, MPI_COMM_WORLD, &status);
           MPI_Send(temp_tab, RANGESIZE, MPI_INT, status.MPI_SOURCE, DATA, MPI_COMM_WORLD);
           #ifdef DEBUG
            printf("[MASTER]Slave %d has returned. Sending package %d\n", status.MPI_SOURCE, sentcount);
            fflush(stdout);
            #endif
  	       sentcount++;
       }while (sentcount < packets-1);
       
        //send remainders
        MPI_Recv (NULL, 0, MPI_INT, MPI_ANY_SOURCE, TASK_FINISHED, MPI_COMM_WORLD, &status);
        for(int j = 0; j<remainder; j++)
        {
            // 0-1000
            temp_tab[j] = rand()%MAX_VALUE;
        }
        MPI_Send(temp_tab, remainder, MPI_INT, status.MPI_SOURCE, DATA, MPI_COMM_WORLD);

        //get response
       for(int i = 0; i < nproc-1; i++)
       {
            MPI_Recv(NULL, 0, MPI_INT, MPI_ANY_SOURCE, TASK_FINISHED, MPI_COMM_WORLD, &status);
             #ifdef DEBUG
            printf("[MASTER]Slave %d has returned last package\n", status.MPI_SOURCE);
            fflush(stdout);
            #endif
       }

       for(int i =1; i < nproc; i++)
       {
            MPI_Send (NULL, 0, MPI_INT, i, FINISH, MPI_COMM_WORLD);
       }


   }
   
   //Slave
   else
   {
    int received =0;

    //set all zeros
    for(int i = 0; i<MAX_VALUE; i++)
    {
        temp_results[i] =  0;
    }

   	do
   	{
   	    MPI_Probe (0, MPI_ANY_TAG, MPI_COMM_WORLD, &status);
   	    if (status.MPI_TAG == DATA)
	    {
	     MPI_Recv (temp_tab, RANGESIZE, MPI_INT, 0, DATA, MPI_COMM_WORLD,
				&status);

        received++;
         #ifdef DEBUG
            printf("[SLAVE]slave: %d, received %d. package\n", myrank, received);
            fflush(stdout);
            #endif
	     for(int i=0; i<RANGESIZE; i++)
            {
                temp_results[temp_tab[i]]++;
            }
            MPI_Send (NULL, 0, MPI_INT, 0, TASK_FINISHED,
				MPI_COMM_WORLD);
	    }
   		
   	}
   	while (status.MPI_TAG != FINISH);
    #ifdef DEBUG
            printf("[SLAVE]slave: %d shutting down\n", myrank);
            fflush(stdout);
            #endif
   }


   MPI_Reduce(temp_results, results, MAX_VALUE, MPI_INT, MPI_SUM, 0, MPI_COMM_WORLD); 
  // synchronize/finalize your computations
  
  if (!myrank) {
    gettimeofday(&ins__tstop, NULL);
    ins__printtime(&ins__tstart, &ins__tstop, ins__args.marker);

    int sum=0;
    for(int i = 0; i < MAX_VALUE; i++)
    {
        printf("%d - %d times\n", i, results[i]);
        sum += results[i];
        fflush(stdout);
    }

    printf("Sum: %d\n", sum);
  }
  
  MPI_Finalize();

}
