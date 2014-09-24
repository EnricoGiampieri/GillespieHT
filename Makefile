C = ./cJSON/cJSON.c ioutils.c msutils.c gillespie.c ms.c  
CCOMP = mpicc     # compilatore
CFLAGS= -std=c99 -std=c99  -openmp  -parallel -g -mkl -DMKL_ILP64 -DVECTOR_SIZE=32
LIB = -I./cJSON  -lpthread -lm 

COUT= GillespieHT.x  

$(COUT): $(C)
	$(CCOMP) -O2 $(CFLAGS) $(C) $(LIB) -o $(COUT)  


.PHONY: debug clean  


debug: $(C)
	 $(CCOMP) -g -O0 $(CFLAGS) $(C) $(LIB) -o $(COUT) 

clean:
	rm *.out* *.e* *.txt core*.* $(COUT)

