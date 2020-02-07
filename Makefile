CC ?= gcc
CXX ?= g++
MPICC ?= mpicc
MPICXX ?= mpic++


test: test.c libdetach.so mpi-detach.h
	$(MPICC) test.c -L. -Wl,--rpath,. -ldetach -o test


libdetach.so: detach.cpp mpi-detach.h
	$(MPICXX)  -DOMPI_SKIP_MPICXX=1 -shared -fPIC detach.cpp -o libdetach.so -g


