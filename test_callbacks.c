#include "mpi-detach.h"
#include "test_callbacks.h"
#include <stdio.h>

void Detach_callback(void *data) {
  printf("Detach_callback: %s\n", (const char *)data);
}

void Detach_callback_status(void *data, MPI_Status *status) {
  printf("Detach_callback_status: %s with tag %i\n", (const char *)data, status->MPI_TAG);
}

void Detach_callback_statuses(void *data, int count, MPI_Status *status) {
  printf("Detach_callback_statuses: %s\n", (const char *)data);
  for (int i=0; i< count; i++){
    printf("with tag: %i\n", status[i].MPI_TAG);
  }
}

