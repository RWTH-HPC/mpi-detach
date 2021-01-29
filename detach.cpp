#include "mpi-detach.h"
#include <atomic>
#include <chrono>
#include <condition_variable>
#include <list>
#include <set>
#include <mutex>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <thread>

using namespace std::chrono_literals;

static std::atomic<int> running{1};
static std::thread detachThread;

static std::mutex listMtx;
static std::condition_variable listCv;

static std::mutex progressMtx;

static std::once_flag onceFlag;
static int initialized{0};
static int use_progress_thread{0};

struct singleRequest {
  MPI_Request req;
  MPIX_Detach_function *callback;
  MPIX_Detach_status_function *callback_status;
  MPI_Status status;
  void *data;
  MPI_Status *statusP; // pointer to status or MPI_STATUS_IGNORE
  DLL_LOCAL singleRequest(MPI_Request *request, MPIX_Detach_function *callback,
                void *data, bool persistent=false)
      : req(*request), callback(callback), callback_status(nullptr), status(),
        data(data), statusP(MPI_STATUS_IGNORE) {
    if (!persistent)
      *request = MPI_REQUEST_NULL;
  }
  DLL_LOCAL singleRequest(MPI_Request *request, MPIX_Detach_status_function *callback,
                void *data, bool persistent=false)
      : req(*request), callback(nullptr), callback_status(callback), status(),
        data(data), statusP(&this->status) {
    if (!persistent)
      *request = MPI_REQUEST_NULL;
  }
};

static inline void copyRequests(MPI_Request *inReq, MPI_Request *outReq, int count, bool persistent){
  if(persistent)
    for (int i = 0; i < count; i++) {
      outReq[i] = inReq[i];
    }
  else
    for (int i = 0; i < count; i++) {
      outReq[i] = inReq[i];
      inReq[i] = MPI_REQUEST_NULL;
    }
}

struct allRequest {
  int count;
  MPI_Request *req;
  MPIX_Detach_function *callback;
  MPIX_Detach_all_statuses_function *callback_statuses;
  MPI_Status *statuses;
  void *data;
  DLL_LOCAL allRequest(int count, MPI_Request *array_of_requests,
             MPIX_Detach_function *callback, void *data, bool persistent=false)
      : count(count), req(new MPI_Request[count]), callback(callback),
        callback_statuses(nullptr), statuses(MPI_STATUSES_IGNORE), data(data) {
    copyRequests(req, array_of_requests, count, persistent);
  }
  DLL_LOCAL allRequest(int count, MPI_Request *array_of_requests,
             MPIX_Detach_all_statuses_function *callback, void *data, bool persistent=false)
      : count(count), req(new MPI_Request[count]), callback(nullptr),
        callback_statuses(callback), statuses(new MPI_Status[count]),
        data(data) {
    copyRequests(req, array_of_requests, count, persistent);
  }
  DLL_LOCAL ~allRequest() {
    delete[] req;
    if (statuses != MPI_STATUSES_IGNORE)
      delete[] statuses;
  }
};

static std::list<singleRequest *> singleRequestsQueue{};
static std::list<allRequest *> allRequestsQueue{};

static std::list<singleRequest *> singleRequests{};
static std::list<allRequest *> allRequests{};


int MPIX_Progress(void *arg) {
  // only one thread will execute at a time
  if (!progressMtx.try_lock())
    return MPI_SUCCESS;
  {
    std::unique_lock<std::mutex> lck(listMtx);
    if (!singleRequestsQueue.empty())
      singleRequests.splice(singleRequests.begin(), singleRequestsQueue);
    if (!allRequestsQueue.empty())
      allRequests.splice(allRequests.begin(), allRequestsQueue);
  }
  if (!singleRequests.empty()) {
    auto iter = singleRequests.begin();
    auto end = singleRequests.end();
    while (iter != end) {
      int flag;
      PMPI_Test(&(*iter)->req, &flag, (*iter)->statusP);
      if (flag) { //
        if ((*iter)->callback)
          (*iter)->callback((*iter)->data);
        else
          (*iter)->callback_status((*iter)->data, (*iter)->statusP);
        delete (*iter);
        iter = singleRequests.erase(iter);
      } else {
        iter++;
      }
    }
  }
  if (!allRequests.empty()) {
    auto iter = allRequests.begin();
    auto end = allRequests.end();
    while (iter != end) {
      int flag;
      PMPI_Testall((*iter)->count, (*iter)->req, &flag, (*iter)->statuses);
      if (flag) { //
        if ((*iter)->callback)
          (*iter)->callback((*iter)->data);
        else
          (*iter)->callback_statuses((*iter)->data, (*iter)->count,
                                     (*iter)->statuses);
        delete (*iter);
        iter = allRequests.erase(iter);
      } else {
        iter++;
      }
    }
  }
  progressMtx.unlock();
  return MPI_SUCCESS;
}

DLL_LOCAL void run(void) {
  while (running || !singleRequests.empty() || !allRequests.empty()) {
    do {
      std::unique_lock<std::mutex> lck(listMtx);
      if (!singleRequestsQueue.empty())
        singleRequests.splice(singleRequests.begin(), singleRequestsQueue);
      if (!allRequestsQueue.empty())
        allRequests.splice(allRequests.begin(), allRequestsQueue);
      if (singleRequests.empty() && allRequests.empty())
        while (running && singleRequestsQueue.empty() &&
               allRequestsQueue.empty()) {
          listCv.wait(lck);
        }
    } while (running && singleRequests.empty() && allRequests.empty());
    MPIX_Progress(NULL);
    std::this_thread::sleep_for(2ms);
  }
}

// using __attribute__((constructor)) does not work,
// because c++ std library might not be initialized yet
DLL_LOCAL void initDetach() {
  initialized = 1;
  // MPIX_DETACH=progress  to use a progress thread
  char *env = getenv("MPIX_DETACH");
  if (env && std::string(env) == "progress") {
    use_progress_thread = 1;
    detachThread = std::thread(run);
  }
}

// This function is assigned to execute after
// main using __attribute__((destructor))
DLL_LOCAL void finiDetach() {
  if (use_progress_thread) {
    {
      std::unique_lock<std::mutex> lck(listMtx);
      // make sure all requests get finally processed
      // access to the *Queue lists is shared, so need to be locked
      while (!singleRequestsQueue.empty() || !allRequestsQueue.empty()) {
        listCv.notify_one();
        lck.unlock();
        std::this_thread::sleep_for(2ms);
        lck.lock();
      } // after while, lock is always set for changing running and notify
      running = 0;
      listCv.notify_one();
    } // now wait for the progress thread to finish, i.e. all requests are
      // handled
    detachThread.join();
  } else {
    while (!singleRequestsQueue.empty() || !allRequestsQueue.empty() ||
           !singleRequests.empty() || !allRequests.empty())
      MPIX_Progress(NULL);
  }
}

int MPIX_Detach(MPI_Request *request, MPIX_Detach_function *callback,
                void *data) {
  std::call_once(onceFlag, initDetach);
  int flag;
  PMPI_Test(request, &flag, MPI_STATUS_IGNORE);
  if (flag) {
    callback(data);
  } else {
    std::unique_lock<std::mutex> lck(listMtx);
    singleRequestsQueue.push_back(new singleRequest(request, callback, data));
    listCv.notify_one();
  }
  return MPI_SUCCESS;
}

int MPIX_Detach_status(MPI_Request *request,
                       MPIX_Detach_status_function *callback, void *data) {
  std::call_once(onceFlag, initDetach);
  int flag;
  MPI_Status status;
  PMPI_Test(request, &flag, &status);
  if (flag) {
    callback(data, &status);
  } else {
    std::unique_lock<std::mutex> lck(listMtx);
    singleRequestsQueue.push_back(new singleRequest(request, callback, data));
    listCv.notify_one();
  }
  return MPI_SUCCESS;
}

int MPIX_Detach_each(int count, MPI_Request array_of_requests[],
                     MPIX_Detach_function *callback, void *array_of_data[]) {
  std::call_once(onceFlag, initDetach);
  int flag;
  for (int i = 0; i < count; i++) {
    PMPI_Test(array_of_requests + i, &flag, MPI_STATUS_IGNORE);
    if (flag) {
      callback(array_of_data[i]);
    } else {
      std::unique_lock<std::mutex> lck(listMtx);
      singleRequestsQueue.push_back(
          new singleRequest(array_of_requests + i, callback, array_of_data[i]));
      listCv.notify_one();
    }
  }
  return MPI_SUCCESS;
}

int MPIX_Detach_each_status(int count, MPI_Request array_of_requests[],
                            MPIX_Detach_status_function *callback,
                            void *array_of_data[]) {
  std::call_once(onceFlag, initDetach);
  int flag;
  MPI_Status status;
  for (int i = 0; i < count; i++) {
    PMPI_Test(array_of_requests + i, &flag, &status);
    if (flag) {
      callback(array_of_data[i], &status);
    } else {
      std::unique_lock<std::mutex> lck(listMtx);
      singleRequestsQueue.push_back(
          new singleRequest(array_of_requests + i, callback, array_of_data[i]));
      listCv.notify_one();
    }
  }
  return MPI_SUCCESS;
}

int MPIX_Detach_all(int count, MPI_Request array_of_requests[],
                    MPIX_Detach_function *callback, void *data) {
  std::call_once(onceFlag, initDetach);
  int flag;
  PMPI_Testall(count, array_of_requests, &flag, MPI_STATUSES_IGNORE);
  if (flag) {
    callback(data);
  } else {
    std::unique_lock<std::mutex> lck(listMtx);
    allRequestsQueue.push_back(
        new allRequest(count, array_of_requests, callback, data));
    listCv.notify_one();
  }
  return MPI_SUCCESS;
}

int MPIX_Detach_all_status(int count, MPI_Request array_of_requests[],
                           MPIX_Detach_all_statuses_function *callback,
                           void *data) {
  std::call_once(onceFlag, initDetach);
  int flag;
  MPI_Status* statuses = new MPI_Status[count];
  PMPI_Testall(count, array_of_requests, &flag, statuses);
  if (flag) {
    callback(data, count, statuses);
  } else {
    std::unique_lock<std::mutex> lck(listMtx);
    allRequestsQueue.push_back(
        new allRequest(count, array_of_requests, callback, data));
    listCv.notify_one();
  }
  delete statuses;
  return MPI_SUCCESS;
}

int MPIX_Start_detached(MPI_Request *request, MPIX_Detach_function *callback,
                void *data) {
  std::call_once(onceFlag, initDetach);
  PMPI_Start(request);
  int flag;
  PMPI_Test(request, &flag, MPI_STATUS_IGNORE);
  if (flag) {
    callback(data);
  } else {
    std::unique_lock<std::mutex> lck(listMtx);
    singleRequestsQueue.push_back(new singleRequest(request, callback, data, true));
    listCv.notify_one();
  }
  return MPI_SUCCESS;
}

int MPIX_Start_detached_status(MPI_Request *request,
                       MPIX_Detach_status_function *callback, void *data) {
  std::call_once(onceFlag, initDetach);
  PMPI_Start(request);
  int flag;
  MPI_Status status;
  PMPI_Test(request, &flag, &status);
  if (flag) {
    callback(data, &status);
  } else {
    std::unique_lock<std::mutex> lck(listMtx);
    singleRequestsQueue.push_back(new singleRequest(request, callback, data, true));
    listCv.notify_one();
  }
  return MPI_SUCCESS;
}

int MPIX_Start_detached_each(int count, MPI_Request array_of_requests[],
                     MPIX_Detach_function *callback, void *array_of_data[]) {
  std::call_once(onceFlag, initDetach);
  PMPI_Startall(count, array_of_requests);
  int flag;
  for (int i = 0; i < count; i++) {
    PMPI_Test(array_of_requests + i, &flag, MPI_STATUS_IGNORE);
    if (flag) {
      callback(array_of_data[i]);
    } else {
      std::unique_lock<std::mutex> lck(listMtx);
      singleRequestsQueue.push_back(
          new singleRequest(array_of_requests + i, callback, array_of_data[i], true));
      listCv.notify_one();
    }
  }
  return MPI_SUCCESS;
}

int MPIX_Start_detached_each_status(int count, MPI_Request array_of_requests[],
                            MPIX_Detach_status_function *callback,
                            void *array_of_data[]) {
  std::call_once(onceFlag, initDetach);
  PMPI_Startall(count, array_of_requests);
  int flag;
  MPI_Status status;
  for (int i = 0; i < count; i++) {
    PMPI_Test(array_of_requests + i, &flag, &status);
    if (flag) {
      callback(array_of_data[i], &status);
    } else {
      std::unique_lock<std::mutex> lck(listMtx);
      singleRequestsQueue.push_back(
          new singleRequest(array_of_requests + i, callback, array_of_data[i], true));
      listCv.notify_one();
    }
  }
  return MPI_SUCCESS;
}

int MPIX_Start_detached_all(int count, MPI_Request array_of_requests[],
                    MPIX_Detach_function *callback, void *data) {
  std::call_once(onceFlag, initDetach);
  PMPI_Startall(count, array_of_requests);
  int flag;
  PMPI_Testall(count, array_of_requests, &flag, MPI_STATUSES_IGNORE);
  if (flag) {
    callback(data);
  } else {
    std::unique_lock<std::mutex> lck(listMtx);
    allRequestsQueue.push_back(
        new allRequest(count, array_of_requests, callback, data, true));
    listCv.notify_one();
  }
  return MPI_SUCCESS;
}

int MPIX_Start_detached_all_status(int count, MPI_Request array_of_requests[],
                           MPIX_Detach_all_statuses_function *callback,
                           void *data) {
  std::call_once(onceFlag, initDetach);
  PMPI_Startall(count, array_of_requests);
  int flag;
  MPI_Status* statuses = new MPI_Status[count];
  PMPI_Testall(count, array_of_requests, &flag, statuses);
  if (flag) {
    callback(data, count, statuses);
  } else {
    std::unique_lock<std::mutex> lck(listMtx);
    allRequestsQueue.push_back(
        new allRequest(count, array_of_requests, callback, data, true));
    listCv.notify_one();
  }
  delete statuses;
  return MPI_SUCCESS;
}

DLL_PUBLIC int MPI_Finalize() {
  // we need to make sure, all communication is finished
  // before calling MPI_Finalize
  if (initialized)
    finiDetach();

  return PMPI_Finalize();
}
