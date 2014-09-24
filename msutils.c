#include <mpi.h>
#include "msutils.h"

int comm_get_ready_slave() {
	
  int whois = 0;
  int val = 0;
  MPI_Status status;
  MPI_Recv( (void *)&val, 1, MPI_INT, MPI_ANY_SOURCE, MPI_ANY_TAG,MPI_COMM_WORLD, &status );
  whois = status.MPI_SOURCE;
  return whois;
	
}