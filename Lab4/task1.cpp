#include <stdio.h>
#include <mpi.h>
#include <random>
#include <math.h>

#define vector_t int *
#define element_t int
#define MPI_ELEMENT_T MPI_INT

#define error_t unsigned char
#define ERR_VAL 1

#define SLICE_SIZE 5

int getSum(vector_t v) {
    int sum = 0;
    for (int i = 0; i < SLICE_SIZE; i++) sum += v[i];
    return sum;
}

vector_t getRandomVector(size_t length, unsigned int seedOffset = 0) {
    vector_t vector = (vector_t)malloc(length * sizeof(element_t));
    srand((unsigned int)time(NULL) + seedOffset);
    for (int i = 0; i < length; i++) vector[i] = rand() % 19 - 9;
    return vector;
}

int main(int argc, char ** argv) {
    MPI_Init(&argc, &argv);

    int rank, size;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);
    if (size < 2) {
        if (rank == 0) printf("Expected more than one process\n");
        MPI_Finalize();
        return ERR_VAL;
    }

    vector_t localVector = getRandomVector(SLICE_SIZE, rank);
    vector_t sumVector = (vector_t)malloc(SLICE_SIZE * sizeof(element_t));
    
    printf("#%d Generated vector:", rank);
    for (int i = 0; i < SLICE_SIZE; i++) printf(" %d", localVector[i]);
    printf("\n");

    MPI_Reduce(localVector, sumVector, SLICE_SIZE, MPI_INT, MPI_SUM, 0, MPI_COMM_WORLD);
    
    if (rank == 0) {
        int sum = 0;
        for (int i = 0; i < SLICE_SIZE; i++) sum += sumVector[i];
        float average = (float)sum / (SLICE_SIZE * size);

        printf("Sum: %d Average: %f\n", sum, average);
    }


    MPI_Finalize();
    return 0;
}