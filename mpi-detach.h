#include <mpi.h>
typedef void MPIX_Detach_function(void *, MPI_Request *);
typedef void MPIX_Detach_status_function(void *, MPI_Request *, MPI_Status *);
typedef void MPIX_Detach_all_function(void *, int count, MPI_Request[]);
typedef void MPIX_Detach_all_statuses_function(void *, int count, MPI_Request[], MPI_Status[]);

// lazy for compatibility
typedef MPIX_Detach_function MPIX_Detach_callback;
typedef MPIX_Detach_status_function MPIX_Detach_callback_status;
typedef MPIX_Detach_all_function MPIX_Detach_all_callback;
typedef MPIX_Detach_all_statuses_function MPIX_Detach_all_callback_statuses;

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
