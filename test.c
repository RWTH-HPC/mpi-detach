#include "mpi-detach.h"
#include <stdio.h>

void Detach_callback (void* data)
{
  printf("Detach_callback: %s\n", (const char*) data);
}

void Detach_callback_status (void* data, MPI_Status* status)
{
  printf("Detach_callback_status: %s\n", (const char*) data);
}

void Detach_callback_statuses (void* data, int count, MPI_Status* status)
{
  printf("Detach_callback_statuses: %s\n", (const char*) data);
}

int main(){
  MPI_Init(NULL,NULL);
  int a=0,b=1,c=2;
  MPI_Request req, reqs[10];
  
  MPI_Isend(&a, 1, MPI_INT, 0, 23, MPI_COMM_SELF, &req);
  MPI_Detach(&req, Detach_callback, "sent data with MPI_Isend");
  MPI_Recv(&b, 1, MPI_INT, 0, 23, MPI_COMM_SELF, MPI_STATUS_IGNORE);
  MPI_Finalize();
}
