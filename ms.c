/*
  
  GillespieHT software
  
  Copyright (c) 2014, Eric Pascolo

  Permission is hereby granted, free of charge, to any person obtaining a copy
  of this software and associated documentation files (the "Software"), to deal
  in the Software without restriction, including without limitation the rights
  to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
  copies of the Software, and to permit persons to whom the Software is
  furnished to do so, subject to the following conditions:
 
  The above copyright notice and this permission notice shall be included in
  all copies or substantial portions of the Software.
 
  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
  AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
  OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
  THE SOFTWARE.
 
 */


#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <stdbool.h>
#include <mpi.h>
#include <unistd.h>

#include "msutils.h"
#include "ioutils.h"
#include "gillespie.h"

void Master(int nproc, char ** listFile, int numFile)
{
  int numSlave = nproc-1;
  
  for(int i=0;i<numFile;i++){
    int whois = comm_get_ready_slave();
    MPI_Send( &i, 1, MPI_INT, whois, 0, MPI_COMM_WORLD );
  }
  
  for (int i=1;i<nproc;i++){
    int ciccio =-1;
    MPI_Send( &ciccio, 1, MPI_INT, i, 0, MPI_COMM_WORLD );
  }
}

void Slave(int myid, int nproc,char * dirIn,char * dirOut, char ** listFile, int numFile)
{
  
  char * istantSlave;
  istantSlave = (char *) malloc(100*sizeof(char ));
  bool work_to_do = true;
   
  
  while(work_to_do)
  {
    
    int idxFile = 0;
    MPI_Status status;
    MPI_Send( &myid, 1, MPI_INT, 0, myid, MPI_COMM_WORLD );
    MPI_Recv( &idxFile, 1, MPI_INT, 0, MPI_ANY_TAG, MPI_COMM_WORLD, &status );
    
    if(idxFile == -1)
    {
      work_to_do = false;
      break;
    }
    
    printf("SLAVE %5d %10s: %30s -> file %5d\n",myid,"BEGIN",getTime(istantSlave),idxFile);
                       
    BioSystem sistema;
    char * json_system;  
    
    //set input path file
    char * fileNameIn = (char *) malloc(1000*sizeof(char)); 
    char * fileNameOut = (char *) malloc(1000*sizeof(char)); 
    strcpy( fileNameIn, dirIn);
    strcat( fileNameIn, listFile[idxFile]);
    
    //extract system from file
    json_system = fget_contents(fileNameIn);
    sistema = extract_from_json(json_system);
    
    //set output path file
    strcpy( fileNameOut,dirOut);
    strcat( fileNameOut, sistema.name);
    strcat( fileNameOut, ".out");
    
    
    //algo submission
    gillespie(sistema,fileNameOut);
    
    printf("SLAVE %5d %10s: %30s -> file %5d\n",myid,"END",getTime(istantSlave),idxFile);
                       
  }
  
  printf("SLAVE %5d %10s: %30s \n",myid,"SHUTDOWN",getTime(istantSlave));
  
}

/*
 
 MAIN FUNCTION
 
 */

int main ( int argc, char *argv[] )
{
    
  //Control number of input parameter
  if(argc<3)
  {
    printf("ERROR MISSING DIR PATH IN/OUT \n");
    return 1;
  }
  
  //MPI vars
  int error = 0; // mi restituisce gli errori mpi
  int nproc = 0; // numero processori totali 
  int myid  = 0; // id singolo processore
  
  //init MPI
  error = MPI_Init(&argc, &argv);
  
  //init MPI Comm
  error = MPI_Comm_size(MPI_COMM_WORLD, &nproc);
  error = MPI_Comm_rank(MPI_COMM_WORLD, &myid);
  
  //check directory
  char * dirIn;
  char * dirOut;
  char * istant;
  dirIn = (char *) malloc(500*sizeof(char ));
  dirOut = (char *) malloc(500*sizeof(char ));
  istant = (char *) malloc(100*sizeof(char ));
  strcpy(dirIn,argv[1]);
  strcpy(dirOut,argv[2]);
  strcat(dirIn,"/");
  strcat(dirOut,"/");
  
  
  //printf("I'm %d of %d\n",myid,nproc);
  
  // read number of json file in input dir
  int numFile = 0;
  numFile = readDirectoryNum(dirIn);
  
  //create file list structure
  char ** list;
  list = (char **) malloc(numFile*sizeof(char*));
  for(int i=0;i<numFile;i++)
    list[i] = (char *) malloc(200*sizeof(char));
  
  // read list file in input directory
  readDirectory(dirIn,list,numFile);
  
  if(myid == 0)
  {
  printf("\n");
  printf("GILLESPIE HT v 1.0 \n");
  printf("Gillespie algo high throughput software\n");
  printf("https://github.com/EricPasc/GillespieHT\n");
  printf("Created by Eric Pascolo\n");
  printf("\n");
  printf("\tParallel Run with %d slave\n",nproc-1);
  printf("\tInput directory : %s \n",dirIn);
  printf("\tOutput directory : %s \n",dirOut);
  printf("\tNumber of file: %d \n",numFile);
  printf("\nBEGIN Simulation at %s\n",getTime(istant));
  printf("\n");
  printf("\n");
  printf("\tLIST FILE\n");
  printf("\t---------\n");
  for(int i=0;i<numFile;i++)
    printf("\t%5d %20s\n",i,list[i]);
  printf("\t---------\n\n");
  }
  
  MPI_Barrier(MPI_COMM_WORLD);
  
  if(myid == 0)
  {
    Master(nproc,list,numFile);
    
  }
  else
    Slave(myid,nproc,dirIn,dirOut,list,numFile);
  
   MPI_Barrier(MPI_COMM_WORLD);
  
  if(myid == 0)
  {
  printf("\nEND Simulation at %s\n",getTime(istant));
  }
  
  error = MPI_Finalize();
  return 0;
  
  
}
