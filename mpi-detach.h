#include <mpi.h>
typedef void MPI_Detach_callback(void *);
typedef void MPI_Detach_callback_status(void *, MPI_Status *);
typedef void MPI_Detach_callback_statuses(void *, int count, MPI_Status *);

#ifdef __cplusplus
extern "C" {
#endif

int MPI_Detach(MPI_Request *request, MPI_Detach_callback *callback, void *data);

int MPI_Detach_status(MPI_Request *request,
                      MPI_Detach_callback_status *callback, void *data);

int MPI_Detach_each(int count, MPI_Request* array_of_requests,
                    MPI_Detach_callback *callback, void *array_of_data[]);

int MPI_Detach_each_status(int count, MPI_Request* array_of_requests,
                           MPI_Detach_callback_status *callback,
                           void *array_of_data[]);

int MPI_Detach_all(int count, MPI_Request* array_of_requests,
                   MPI_Detach_callback *callback, void *data);

int MPI_Detach_all_status(int count, MPI_Request* array_of_requests,
                          MPI_Detach_callback_statuses *callback, void *data);

#ifdef __cplusplus
}
#endif
