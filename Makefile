CC ?= gcc
CXX ?= g++
MPICC ?= mpicc
MPICXX ?= mpic++


test: test.c libdetach.so mpi-detach.h
	$(MPICC) test.c -L. -Wl,--rpath,. -ldetach -o test

openmp-task-detach: openmp-task-detach.c libdetach.so mpi-detach.h
	$(MPICC) -fopenmp -fopenmp-version=50 $< -L. -Wl,--rpath,. -ldetach -o $@ -g

alltests: test_MPI_Detach_all test_MPI_Detach_all_status test_MPI_Detach 	test_MPI_Detach_each test_MPI_Detach_each_status test_MPI_Detach_status

test_%: test_%.c test_callbacks.c test_callbacks.h libdetach.so mpi-detach.h
	$(MPICC) $< test_callbacks.c -L. -Wl,--rpath,. -ldetach -o $@ -g


libdetach.so: detach.cpp mpi-detach.h
	$(MPICXX)  -DOMPI_SKIP_MPICXX=1 -shared -fPIC detach.cpp -o libdetach.so -g


