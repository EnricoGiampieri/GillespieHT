
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "omp.h"
#include "mkl.h"
#include "mkl_vsl.h";
#include "ioutils.h"


int gillespie(BioSystem  sistema,char * fileOut)
{
  
  //init var
  double timeElapsed = 0;
  int numR = sistema.numReactions;
  int numS = sistema.numSpecies;
  
  double * Knew;
  Knew = (double *) malloc(numR*sizeof(double));
  
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
   // printf("%lf",normCost);
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
    //strcat(output[countChar], "\n");
    //end write istant data
        
  }
  
  store_data(fileOut, output);
  
}


