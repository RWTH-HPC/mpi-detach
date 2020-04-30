#include <mpi.h>
#include <omp.h>
#include <stdio.h>
#include <stdlib.h>
#include "mpi-detach.h"

int main() {
  MPI_Init(NULL,NULL);
  int rank, size;
  MPI_Comm_size(MPI_COMM_WORLD, &size);
  MPI_Comm_rank(MPI_COMM_WORLD, &rank);
  int A[] = {1, 2, 3, 4, 5};
  int B[5];
  omp_event_handle_t event_handle;
#pragma omp parallel
#pragma omp single
  {
  #pragma omp task detach(event_handle) depend(out : B)
    {
      MPI_Request req;
      MPI_Irecv(B, 5, MPI_INT, size - rank - 1, 23, MPI_COMM_WORLD, &req);
      MPI_Detach(&req, (MPI_Detach_callback *)omp_fulfill_event, (void*)event_handle);
    }
  #pragma omp task depend(in : B)
    {
      for (int i = 0; i < 5; i++)
        if (A[i] != B[i])
          printf("Error: A[%i] (%i) != B[%i] (%i)\n", i, A[i], i, B[i]);
    }
  #pragma omp task
    {
      MPI_Send(A, 5, MPI_INT, size - rank - 1, 23, MPI_COMM_WORLD);
    }
  #pragma omp taskwait
  }
  MPI_Finalize();
}
