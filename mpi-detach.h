#include <mpi.h>
typedef void MPIX_Detach_callback(void *, MPI_Request *);
typedef void MPIX_Detach_callback_status(void *, MPI_Request *, MPI_Status *);
typedef void MPIX_Detach_all_callback(void *, int count, MPI_Request[]);
typedef void MPIX_Detach_all_callback_statuses(void *, int count, MPI_Request[], MPI_Status[]);

#ifdef __cplusplus
extern "C" {
#endif

int MPIX_Detach(MPI_Request *request, MPIX_Detach_callback *callback, void *data);

int MPIX_Detach_status(MPI_Request *request,
                       MPIX_Detach_callback_status *callback, void *data);

int MPIX_Detach_each(int count, MPI_Request array_of_requests[],
                     MPIX_Detach_callback *callback, void *array_of_data[]);

int MPIX_Detach_each_status(int count, MPI_Request array_of_requests[],
                            MPIX_Detach_callback_status *callback,
                            void *array_of_data[]);

int MPIX_Detach_all(int count, MPI_Request array_of_requests[],
                    MPIX_Detach_all_callback *callback, void *data);

int MPIX_Detach_all_status(int count, MPI_Request array_of_requests[],
                           MPIX_Detach_all_callback_statuses *callback, void *data);

int MPIX_Progress(void*);

#ifdef __cplusplus
}
#endif
