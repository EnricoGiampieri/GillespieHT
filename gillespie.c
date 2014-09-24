
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "omp.h"
#include "mkl.h"
#include "mkl_vsl.h";
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


#include "ioutils.h"


int gillespie(BioSystem  sistema,char * fileOut)
{
  __assume_aligned(sistema.status, VECTOR_SIZE);
  __assume_aligned(sistema.stechioMatrix, VECTOR_SIZE);
  __assume_aligned(sistema.Karray, VECTOR_SIZE);
  
  //init var
  double timeElapsed = 0;
  int numR = sistema.numReactions;
  int numS = sistema.numSpecies;
  
  double * Knew;
  Knew = (double *) mkl_malloc(numR*sizeof(double),VECTOR_SIZE);
  
  //generazione random
  double *randomVector;
  randomVector = (double *) malloc(1*sizeof(double));
  double *randTime;
  randTime = (double *) malloc(1*sizeof(double));
  VSLStreamStatePtr stream; //creazione stream mkl vsl
  int statusVsl; // var che restituisce lo stato della funz vdRngGaussian mkl_vsl lib
  statusVsl = vslNewStream( &stream, VSL_BRNG_SFMT19937, time(NULL) ); //associo lo status
  
   //var for manage output
   char ** output;
   output = (char **) malloc(500*sizeof(char*));
   for(int i=0;i<500;i++)
	output[i] = (char *) malloc(10000*sizeof(char*));
   char  *istData, *statusData;
   statusData = (char *) malloc(100*sizeof(char*));
   istData = (char *) malloc(1000*sizeof(char));
   int countChar = 0;
   
   
  //MONTECARLO LOOP
  while(timeElapsed<sistema.time)
  {
    
    //begin write status
    countChar++;
    if(countChar==500)
    {
      store_data(fileOut, output);  
    }
    
    for(int u=0;u<numS;u++)
    {
      sprintf(statusData,"%lf \t", sistema.status[u]);
      strcat(output[countChar], statusData);
    }
    strcat(output[countChar], "\n");
    //end write status 
    
    //reset vettore knew
    #pragma omp parallel for simd
    for(int i =0; i< numR;i++)
      Knew[i] = 1.0;
    
    //reaction loop update K
    #pragma omp parallel for simd
    for(int i=0; i<numR;i++)
    {
      for(int j=0;j<numS;j++)
      {
	if(sistema.stechioMatrix[j+i*numS] < 0)
	{
	  for(int f=0;f>sistema.stechioMatrix[j+i*numS];f--)
	  {
	   Knew[i] = (sistema.status[j]+f)* Knew[i];
	  }
	}
      }
      Knew[i] = Knew[i] * sistema.Karray[i];
    }    
   
    //calculate comulative array of Knew
    double normCost = cblas_dasum (numR, Knew, 1);
    
    //exit criteria
    if(normCost <= 0.0)
      break;
    
    for(int i = 1;i<numR;i++){
      Knew[i] = Knew[i]+Knew[i-1];	  
    }
   
    //random time
    statusVsl = vdRngExponential( VSL_RNG_METHOD_EXPONENTIAL_ICDF_ACCURATE, stream, 1,randTime, 0, 1/normCost );
    
    timeElapsed = timeElapsed + randTime[0];
    
   
    #pragma omp parallel for simd
    for(int i = 0;i<numR;i++){
      Knew[i] = Knew[i]/normCost;
    }
    
    //selection reaction
    int idx_sel = 0;
    statusVsl = vdRngUniform(VSL_RNG_METHOD_UNIFORM_STD_ACCURATE, stream,1, randomVector, 0, 1 );
     
    for(int i=0; i<numR;i++)
      if(randomVector[0]<=Knew[i]){
	idx_sel = i;
	break;
     }
   
    
    vdAdd(numS, sistema.status, &sistema.stechioMatrix[idx_sel*numS], sistema.status );
    
    
    //begin write istant data
    sprintf(istData,"%lf\t%d\t %lf\n", timeElapsed, idx_sel,normCost);
    strcat(output[countChar], istData);
    strcat(output[countChar], "\n\n");  
    //end write istant data
        
  }
  
  store_data(fileOut, output);
  
}


