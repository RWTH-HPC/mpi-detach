#include <mpi.h>

#if defined _WIN32 || defined __CYGWIN__
  #ifdef BUILDING_DLL
    #ifdef __GNUC__
      #define DLL_PUBLIC __attribute__ ((dllexport))
    #else
      #define DLL_PUBLIC __declspec(dllexport) // Note: actually gcc seems to also supports this syntax.
    #endif
  #else
    #ifdef __GNUC__
      #define DLL_PUBLIC __attribute__ ((dllimport))
    #else
      #define DLL_PUBLIC __declspec(dllimport) // Note: actually gcc seems to also supports this syntax.
    #endif
  #endif
  #define DLL_LOCAL
#else
  #if __GNUC__ >= 4
    #define DLL_PUBLIC __attribute__ ((visibility ("default")))
    #define DLL_LOCAL  __attribute__ ((visibility ("hidden")))
  #else
    #define DLL_PUBLIC
    #define DLL_LOCAL
  #endif
#endif


typedef void MPIX_Detach_function(void *);
typedef void MPIX_Detach_status_function(void *, MPI_Status *);
typedef void MPIX_Detach_all_statuses_function(void *, int, MPI_Status[]);

// lazy for compatibility
typedef MPIX_Detach_function MPIX_Detach_callback;
typedef MPIX_Detach_status_function MPIX_Detach_callback_status;
typedef MPIX_Detach_all_statuses_function MPIX_Detach_all_callback_statuses;

#ifdef __cplusplus
extern "C" {
#endif

DLL_PUBLIC int MPIX_Detach(MPI_Request *request, MPIX_Detach_callback *callback,
                void *data);

DLL_PUBLIC int MPIX_Detach_status(MPI_Request *request,
                       MPIX_Detach_callback_status *callback, void *data);

DLL_PUBLIC int MPIX_Detach_each(int count, MPI_Request array_of_requests[],
                     MPIX_Detach_callback *callback, void *array_of_data[]);

DLL_PUBLIC int MPIX_Detach_each_status(int count, MPI_Request array_of_requests[],
                            MPIX_Detach_callback_status *callback,
                            void *array_of_data[]);

DLL_PUBLIC int MPIX_Detach_all(int count, MPI_Request array_of_requests[],
                    MPIX_Detach_callback *callback, void *data);

DLL_PUBLIC int MPIX_Detach_all_status(int count, MPI_Request array_of_requests[],
                           MPIX_Detach_all_callback_statuses *callback,
                           void *data);

DLL_PUBLIC int MPIX_Progress(void *);

#ifdef __cplusplus
}
#endif
