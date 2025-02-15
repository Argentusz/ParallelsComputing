#include <stdio.h>
#include <mpi.h>

int main(int argc, char ** argv) {
    MPI_Init(&argc, &argv);
    
    int rank, size;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    for(int i = 0; i < (int)(10 / size) + 1; i++) {
        int number = i * size + rank + 1;
        if (number >= 10) break;
        for (int j = 1; j < 10; j++) printf("%d * %d = %d\n", number, j, number * j);
        printf("\n");
    }

    MPI_Finalize();
    return 0;
}
