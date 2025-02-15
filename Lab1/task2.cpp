#include <stdio.h>
#include <mpi.h>
#include <unistd.h>

int main(int argc, char** argv) {
    MPI_Init(&argc, &argv);

    int rank, size;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);
    pid_t pid = getpid();

    printf("Hello World! I am process #%d/%d PID = %d\n", rank + 1, size, pid);
    MPI_Finalize();
    return 0;
}
