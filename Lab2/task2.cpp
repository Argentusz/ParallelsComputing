#include <stdio.h>
#include <mpi.h>
#include <random>
#include <math.h>

#define vector_t int *
#define element_t int

#define SLICE_SIZE 5

int getSum(vector_t v) {
    int sum = 0;
    for (int i = 0; i < SLICE_SIZE; i++) sum += v[i];
    return sum;
}

vector_t getRandomVector(size_t length) {
    vector_t vector = (vector_t)malloc(length * sizeof(element_t));
    srand((unsigned int)time(NULL));
    for (int i = 0; i < length; i++) vector[i] = rand() % 19 - 9;
    return vector;
}

int mainProcess(int size) {
    size_t vectorLength = (size - 1) * SLICE_SIZE;
    vector_t vector = getRandomVector(vectorLength);

    printf("Generated vector:");
    for (int i = 0; i < vectorLength; i++) printf(" %d", vector[i]);
    printf("\n");

    for (int i = 1; i < size; i++) {
        vector_t vptr = vector + (i - 1) * SLICE_SIZE;
        int errc = MPI_Send(vptr, SLICE_SIZE, MPI_INT, i, i, MPI_COMM_WORLD);
        if (errc != MPI_SUCCESS) return errc;
    }

    int sum = 0; 
    for (int i = 1; i < size; i++) {
        MPI_Status s;
        int localResult;
        int errc = MPI_Recv(&localResult, 1, MPI_INT, i, size + i, MPI_COMM_WORLD, &s);
        if (errc != MPI_SUCCESS) return errc;
        sum += localResult;
    }

    float avg = (float)(sum) / vectorLength;
    printf("Total sum: %d. Calculated average: %f\n", sum, avg);
    free(vector);

    return MPI_SUCCESS;
}

int calculatorProcess(int rank, int size) {
    element_t v[SLICE_SIZE];
    MPI_Status s;
    int errc = MPI_Recv(&v, SLICE_SIZE, MPI_INT, 0, rank, MPI_COMM_WORLD, &s);
    if (errc != MPI_SUCCESS) return errc;

    int sum = getSum(v);
    printf("Process #%d Got vector: %d %d %d %d %d. Calculated sum = %d\n", rank, v[0], v[1], v[2], v[3], v[4], sum);

    return MPI_Send(&sum, 1, MPI_INT, 0, size + rank, MPI_COMM_WORLD);
}

int main(int argc, char** argv) {
    MPI_Init(&argc, &argv);
    
    int rank, size;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);
    if (size < 2) {
        if (rank == 0) printf("Expected more than one process\n");
        MPI_Finalize();
        return EINVAL;
    }

    int errc = !rank ? mainProcess(size) : calculatorProcess(rank, size);
 
    MPI_Finalize();
    return errc;
}