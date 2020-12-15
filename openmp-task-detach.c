#include <mpi.h>
#include <omp.h>
#include <stdio.h>
#include <stdlib.h>
#include "mpi-detach.h"
#include <unistd.h>

int main(int argc, char** argv) {
  int provided;
  MPI_Init_thread(&argc, &argv, MPI_THREAD_MULTIPLE, &provided);
  if (provided != MPI_THREAD_MULTIPLE) {
    printf("This code needs MPI_THREAD_MULTIPLE(%i), threadlevel %i was provided\n", MPI_THREAD_MULTIPLE, provided);
    MPI_Finalize();
    return -1;
  }
  int rank, size;
  MPI_Comm_size(MPI_COMM_WORLD, &size);
  MPI_Comm_rank(MPI_COMM_WORLD, &rank);
  int A[] = {1, 2, 3, 4, 5};
  int B[5];
  omp_event_handle_t event_handle;
#pragma omp parallel num_threads(2)
#pragma omp single
  {
  #pragma omp task depend(out : B) detach(event_handle)
    {
      MPI_Request req;
      printf("MPI_Irecv\n");
      MPI_Irecv(B, 5, MPI_INT, size - rank - 1, 23, MPI_COMM_WORLD, &req);
      printf("MPIX_Detach\n");
      MPIX_Detach(&req, (MPIX_Detach_callback *)omp_fulfill_event, (void*)event_handle);
    }
  #pragma omp task depend(in : B)
    {
      for (int i = 0; i < 5; i++)
        if (A[i] != B[i])
          printf("Error: A[%i] (%i) != B[%i] (%i)\n", i, A[i], i, B[i]);
      printf("Done verify\n");
    }
  sleep(1);
  #pragma omp task
    {
      printf("MPI_Send\n");
      MPI_Send(A, 5, MPI_INT, size - rank - 1, 23, MPI_COMM_WORLD);
    }
  #pragma omp taskwait
  }
  MPI_Finalize();
}
