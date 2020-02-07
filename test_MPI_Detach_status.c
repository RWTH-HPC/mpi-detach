#include "mpi-detach.h"
#include "test_callbacks.h"
#include <stdio.h>

int main() {
  MPI_Init(NULL, NULL);
  int a = 0, b = 1;
  int A[10], B[10];
  for (int i=0; i<10; i++)
    A[i] = B[i] = i;  
  MPI_Request req, reqs[10];

  MPI_Isend(&a, 1, MPI_INT, 0, 23, MPI_COMM_SELF, &req);
  MPI_Detach_status(&req, Detach_callback_status, "sent data with MPI_Isend");
  MPI_Recv(&b, 1, MPI_INT, 0, 23, MPI_COMM_SELF, MPI_STATUS_IGNORE);

  MPI_Finalize();
}
