# MPI Detach

The MPI detach libary supports asynchronous workflows based on MPI non-blocking communication and callbacks, which are called when such non-blocking communication locally completed.

# Interface

```C++
typedef void MPIX_Detach_function(void *);
typedef void MPIX_Detach_status_function(void *, MPI_Status *);
typedef void MPIX_Detach_all_statuses_function(void *, int, MPI_Status[]);

#ifdef __cplusplus
extern "C" {
#endif

int MPIX_Detach(MPI_Request *request, MPIX_Detach_callback *callback,
                void *data);

int MPIX_Detach_status(MPI_Request *request,
                       MPIX_Detach_callback_status *callback, void *data);

int MPIX_Detach_each(int count, MPI_Request array_of_requests[],
                     MPIX_Detach_callback *callback, void *array_of_data[]);

int MPIX_Detach_each_status(int count, MPI_Request array_of_requests[],
                            MPIX_Detach_callback_status *callback,
                            void *array_of_data[]);

int MPIX_Detach_all(int count, MPI_Request array_of_requests[],
                    MPIX_Detach_callback *callback, void *data);

int MPIX_Detach_all_status(int count, MPI_Request array_of_requests[],
                           MPIX_Detach_all_callback_statuses *callback,
                           void *data);

int MPIX_Progress(void *);

#ifdef __cplusplus
}
#endif
```

# Modes of operation

## Progress thread

The library can start a progress thread to guarantee progress.  This thread is started by exporting
```bash
export MPIX_DETACH=progress
```

## Progress service

Alternatively, progress can be provided by registering `MPIX_Progress` with an external polling service.

# Example

The detached task example can be used with any OpenMP implementation
supporting detached tasks (e.g., LLVM 11 or newer):

```bash
$ make MPICC=mpicc.mpich MPICXX=mpicxx.mpich CC=clang CXX=clang++ openmp-task-detach
$ MPIX_DETACH=progress ./openmp-task-detach
MPI_Irecv
MPIX_Detach
MPI_Send
Done verify
```


# License

MIT license, see [LICENSE](./LICENSE) file
