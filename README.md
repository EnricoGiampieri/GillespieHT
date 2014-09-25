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

EXTERNAL LIBS
-------------

Thanks to cJSON (http://sourceforge.net/projects/cjson/)