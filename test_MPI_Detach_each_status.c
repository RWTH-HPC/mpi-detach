#include "mpi-detach.h"
#include "test_callbacks.h"
#include <stdio.h>
#include <unistd.h>



int main() {
  MPI_Init(NULL, NULL);
  int a = 0, b = 1;
  int A[10], B[10];
  for (int i=0; i<10; i++)
    A[i] = B[i] = i;  
  MPI_Request req, reqs[10];
  const char * datas[] = {"sent data1 with MPI_Isend","sent data2 with MPI_Isend","sent data3 with MPI_Isend","sent data4 with MPI_Isend","sent data5 with MPI_Isend","sent data6 with MPI_Isend","sent data7 with MPI_Isend","sent data8 with MPI_Isend","sent data9 with MPI_Isend","sent data10 with MPI_Isend"};

  for (int i=0; i<10; i++)
    MPI_Isend(A+i, 1, MPI_INT, 0, 23, MPI_COMM_SELF, reqs+i);
  MPI_Detach_each_status(10, reqs, Detach_callback_status, (void**)datas);
  sleep(1);
  for (int i=0; i<10; i++)
    MPI_Recv(B+i, 1, MPI_INT, 0, 23, MPI_COMM_SELF, MPI_STATUS_IGNORE);





  MPI_Finalize();
}
