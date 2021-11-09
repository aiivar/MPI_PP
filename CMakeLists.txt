cmake_minimum_required(VERSION 3.6)
project(MPI)

include_directories(/etc/alternatives/mpi-x86_64-linux-gnu)

set(CMAKE_C_COMPILER "/usr/bin/mpicc")
set(CMAKE_CXX_COMPILER "/usr/bin/mpic++")

set(CMAKE_CXX_STANDARD 14)

add_executable(MPI main.cpp)
