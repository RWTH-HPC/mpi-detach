CC ?= gcc
CXX ?= g++
MPICC ?= mpicc
MPICXX ?= mpic++

MPICC_FLAGS ?= -cc=$(CC)
MPICXX_FLAGS ?= -cxx=$(CXX)

test: test.c libdetach.so mpi-detach.h
	$(MPICC) $(MPICC_FLAGS) test.c -L. -Wl,--rpath,. -ldetach -o test

openmp-task-detach: openmp-task-detach.c libdetach.so mpi-detach.h
	$(MPICC) $(MPICC_FLAGS) -fopenmp -fopenmp-version=50 $< -L. -Wl,--rpath,. -ldetach -o $@ -g

alltests: test_MPI_Detach_all test_MPI_Detach_all_status test_MPI_Detach 	test_MPI_Detach_each test_MPI_Detach_each_status test_MPI_Detach_status

test_%: test_%.c test_callbacks.c test_callbacks.h libdetach.so mpi-detach.h
	$(MPICC) $(MPICC_FLAGS) $< test_callbacks.c -L. -Wl,--rpath,. -ldetach -o $@ -g


libdetach.so: detach.cpp mpi-detach.h
	$(MPICXX) $(MPICXX_FLAGS) -DOMPI_SKIP_MPICXX=1 -shared -fPIC detach.cpp -o libdetach.so -g

detach.o: detach.cpp mpi-detach.h
	$(MPICXX) $(MPICXX_FLAGS) -DOMPI_SKIP_MPICXX=1 -c detach.cpp -g


openmp-task-detach.o: openmp-task-detach.c mpi-detach.h
	$(MPICC) $(MPICC_FLAGS) -fopenmp -fopenmp-version=50 -c $< -o $@ -g

openmp-task-detach-nolib: detach.o openmp-task-detach.o
	$(MPICXX) $(MPICXX_FLAGS) -fopenmp detach.o openmp-task-detach.o -o $@