#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <dirent.h> 
#include <time.h>
#include "mkl.h"
#include "cJSON.h"
#include "ioutils.h"



char * fget_contents( char * fileName)
{
  //char *fileName;
  char   *buffer;        /* holds the file contents. */
  size_t  i;             /* indexing into buffer. */
  size_t  buffer_size;   /* size of the buffer. */
  char   *temp;          /* for realloc(). */
  char    c;             /* for reading from the input. */
  FILE   *input;         /* our input stream. */
  
//   char * fileName = (char *) malloc(500*sizeof(char));// = (char *) malloc((strlen(dir)+(strlen(file)))*sizeof(char)+2);
//   char * fileName2 = (char *) malloc(500*sizeof(char)); 
//   strcpy( fileName, dir);
//   
//   
//   strcpy( fileName2, file);
//   strcat( fileName, fileName2);
  
  if ((input = fopen(fileName, "r")) == NULL) {
    fprintf(stderr, "Error opening input file %s\n", fileName);
    exit(EXIT_FAILURE);
  }
  
  /* Initial allocation of buffer */
  i = 0;
  buffer_size = BUFSIZ;
  if ((buffer = malloc(buffer_size)) == NULL) {
    fprintf(stderr, "Error allocating memory (before reading file).\n");
    fclose(input);
  }
  
  while ((c = fgetc(input)) != EOF) {
    /* Enlarge buffer if necessary. */
    if (i == buffer_size) {
      buffer_size += BUFSIZ;
      if ((temp = realloc(buffer, buffer_size)) == NULL) {
	fprintf(stderr, "Ran out of core while reading file.\n");
	fclose(input);
	free(buffer);
	exit(EXIT_FAILURE);
      }
      buffer = temp;
    }
    
    /* Add input char to the buffer. */
    buffer[i++] = c;
  }
  
  /* Test if loop terminated from error. */
  if (ferror(input)) {
    fprintf(stderr, "There was a file input error.\n");
    free(buffer);
    fclose(input);
    exit(EXIT_FAILURE);
  }
  
  /* Make the buffer a bona-fide string. */
  if (i == buffer_size) {
    buffer_size += 1;
    if ((temp = realloc(buffer, buffer_size)) == NULL) {
      fprintf(stderr, "Ran out of core (and only needed one more byte too ;_;).\n");
      fclose(input);
      free(buffer);
      exit(EXIT_FAILURE);
    }
    buffer = temp;
  }
  buffer[i] = '\0';
  
  //puts(buffer);
  
  /* Clean up. */
  //free(buffer);
  fclose(input);
  
  
  return buffer;
  
}

BioSystem extract_from_json(char * json_system){
  
  
  double * stechioMatrix;
  double * Karray;
  double * status;
  
  int matrixDimension=1;
  
  
  cJSON *root = cJSON_Parse(json_system);
  char *nameSystem = cJSON_GetObjectItem(root,"nameSystem")->valuestring;
  
  double time = cJSON_GetObjectItem(root,"simultationTime")->valueint;
  
  cJSON * reaction_array = cJSON_GetObjectItem(root,"reaction");
  int numReaction = cJSON_GetArraySize(reaction_array);
  
  cJSON * species_array = cJSON_GetObjectItem(root,"species");
  int numSpecies = cJSON_GetArraySize(species_array);
  
  cJSON * status_array = cJSON_GetObjectItem(root,"status");
  int numStato = cJSON_GetArraySize(status_array);
  status = (double*) calloc(numStato,sizeof( double ) );
  
  for(int n=0;n<numStato;n++){
    
    cJSON *stato = cJSON_GetArrayItem(status_array,n);
    status[n] = cJSON_GetObjectItem(stato,"num")->valuedouble;
    //printf("s %d = %lf \n",n,status[n]);
    
  }
  
  
  matrixDimension = numReaction*numSpecies;
  
  stechioMatrix = (double*) calloc(matrixDimension,sizeof( double ) );
  Karray = (double*) calloc(numReaction,sizeof( double ) );
  //stechioMatrixRow = (int*)malloc(matrixDimension*sizeof( int ) );
  //stechioMatrixCol = (int*)malloc(matrixDimension*sizeof( int ) );
  
  
  for (int i=0;i<numReaction;i++)
  {
    cJSON *reaction=cJSON_GetArrayItem(reaction_array,i);
    int Kcostant = cJSON_GetObjectItem(reaction,"K")->valueint;
    Karray[i] = Kcostant;
    
    cJSON *reagents_array = cJSON_GetObjectItem(reaction,"reagents");
    int numR = cJSON_GetArraySize(reagents_array);
    
    for (int j=0;j<numR;j++)
    {
      cJSON *reagent=cJSON_GetArrayItem(reagents_array,j);
      int idx = cJSON_GetObjectItem(reagent,"idx")->valueint;
      int stechio = cJSON_GetObjectItem(reagent,"stechio")->valueint;
      stechioMatrix[idx+i*numSpecies] = -stechio;
      matrixDimension = matrixDimension +1;
      
      
    }
    
    
    cJSON *products_array = cJSON_GetObjectItem(reaction,"products");
    int numP = cJSON_GetArraySize(products_array);
    
    for (int k=0;k<numP;k++)
    {
      cJSON *product=cJSON_GetArrayItem(products_array,k);
      int idx = cJSON_GetObjectItem(product,"idx")->valueint;
      int stechio = cJSON_GetObjectItem(product,"stechio")->valueint;
      stechioMatrix[idx+i*numSpecies] = stechio;
      matrixDimension = matrixDimension +1;
    }
  }
  
  
  
  BioSystem sys;
  sys.name = nameSystem;
  sys.stechioMatrix = stechioMatrix;
  sys.Karray = Karray;
  sys.status = status;
  sys.numReactions = numReaction;
  sys.numSpecies = numSpecies;
  sys.time = time;
  return sys;
  
}


int readDirectoryNum(char * namedirectory)
{
  DIR           *d;
  struct dirent *dir;
  
  char type[]=".json";
  int count = 0;
  
  d = opendir(namedirectory);
  
  if (!d)
    printf("ERROR PATH IS NOT DIR");
  
  while ((dir = readdir(d)) != NULL)
  {
    if(strstr(dir->d_name, type) != NULL)
    {
      count = count +1;
    }
  }
  closedir(d); 
  
  return count;
}

void readDirectory(char * namedirectory, char ** list, int count)
{
  DIR           *d;
  struct dirent *dir;
  
  char type[]=".json";
  
  int i = 0;
  
  d = opendir(namedirectory);
  while ((dir = readdir(d)) != NULL)
  {
    if(strstr(dir->d_name, type) != NULL)
    {
      strcpy(list[i],dir->d_name);
      i++;
      
    }
  }
  
  closedir(d);
  
}

void store_data( char *filepath,  char **data)
{
  FILE *fp = fopen(filepath, "a");
  if (fp != NULL)
  {
    for(int i =0 ; i<500; i++)
      fputs(data[i], fp);
    
    fclose(fp);
  }
}


char * getTime(char * buffer)
{
    time_t timer;
   
    struct tm* tm_info;

    time(&timer);
    tm_info = localtime(&timer);

    strftime(buffer, 100, "%H:%M:%S %Y:%m:%d ", tm_info);
    //puts(buffer);

    return buffer;
}



