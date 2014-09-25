GillespieHT
===========

WARNING : SOFTWARE IN BETA, UNDER DEVELOPMENT

Gillespie Algo High Throughput MPI application

https://github.com/EricPascolo/GillespieHT/

This software take in input a Biological linear system of reaction described to json file.
If you use MPI you can compute more system in parallel because this is High Throughput application.

COMPILATION
-----------

Requirements: Compiler C (icc preferred), Intel MKL library, MPI Software Stack

The software is compiled through Makefile.

In make file you must change the flag -DVECTOR_SIZE=$YOUR_SIZE, where $YOUR_SIZE is the size 
with which the arrays are aligned. The another flag you must change depending to your architecture
is MKL flags: -DMKL_ILP64.

For found correct MKL flags, you can use Intel® Math Kernel Library Link Line Advisor:
https://software.intel.com/en-us/articles/intel-mkl-link-line-advisor

USE application
---------------

You can launch application using:

`
mpirun -np $NCORE ./GillespieHT $DIR_INPUT $DIR_OUTPUT
`
where:

- $NCORE : numer of pes(1 master, NCORE -1 slave)
- $DIR_INPUT : directory that contains jSON file that described input system
- $DIR_OUTPUT : directory that will contain output file

jSON Input
------
#####Description

Il JSON che descrive il sistema deve avere le seguenti caratteristiche:

* nameSystem (String): system name (output file is named as this)
* simultationTime (int) : max simulation time
* species (jSON array) : array of chemical species  
  - name (String) : name of specie;
  - idx (int) : index unique corresponding specie in simulation ; 
* reaction (jSON array) : array of reaction in the system
  - K (double) : K costant of reaction
  - reagents (JSON array) : reagents array
    - idx (int) : index of species
    - stechio(int) : stechiometrics index
  - products (JSON array) : reagents array
    - idx (int) : index of species
    - stechio(int) : stechiometrics index
* status (jSON array) : number of initial quantities per species, the order is the same of species jSON array.

ATTENTION : all stechiometrics idex must be positive.

#####Example of jSON input:

```json

{
    "nameSystem": "prova1", 
    "simultationTime":1000,
    "species" :[
	  {"name":"Na","idx":0}, {"name":"Cl","idx":1},{"name":"NaCl","idx":2},
	  {"name":"CH4","idx":3},{"name":"O2","idx":4},{"name":"CO2","idx":5},
	  {"name":"H2O","idx":6},{"name":"F","idx":7},{"name":"NaF","idx":8}
    
    ],
    
    
    "status" :[{"num":0},{"num":1},{"num":3},{"num":4},{"num":5},{"num":6},{"num":7},{"num":8},{"num":9}],
    
    "reaction": [

      {  
 	"K" :       8,
        "reagents": [
	  {"idx":0,"stechio":1},{"idx":1,"stechio":1}   
	 ], 
        "products": [
	  {"idx":2,"stechio":1}   
	 ]
       },
	
	{ 
 	"K" :       10,
        "reagents": [
	  {"idx":3,"stechio":1},{"idx":4,"stechio":1}   
	 ], 
        "products": [
	  {"idx":5,"stechio":1},{"idx":6,"stechio":1}    
	 ]
       },
       
       { 
 	"K" :      12,
        "reagents": [
	  {"idx":0,"stechio":1},{"idx":7,"stechio":1}   
	 ], 
        "products": [
	  {"idx":8,"stechio":1}    
	 ]
       }
     ]
}

```

NOTE: In the example the reactions are not balanced, is not mistake, is only example.




EXTERNAL LIBS
-------------

Thanks to cJSON (http://sourceforge.net/projects/cjson/)