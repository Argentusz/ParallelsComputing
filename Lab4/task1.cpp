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

vector_t emptyVector(size_t size) {
    vector_t vector = (vector_t)malloc(size * sizeof(element_t));
    for (int i = 0; i < size; i++) vector[i] = 0;

    return vector;
}

vector_t getRandomVector(size_t length, unsigned int seedOffset = 0) {
    vector_t vector = emptyVector(length);
    srand((unsigned int)time(NULL) + seedOffset);
    for (int i = 0; i < length; i++) vector[i] = rand() % 19 - 9;
    return vector;
}

void printVector(vector_t vector, int size) {
    for (int i = 0; i < size; i++) printf("%d ", vector[i]);
    printf("\n");
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

    size_t vectorSize = SLICE_SIZE * size;
    vector_t vector = emptyVector(vectorSize);
    if (rank == 0) {
        vector = getRandomVector(vectorSize);
        printf("Generated vector: ");
        printVector(vector, vectorSize);
    }

    vector_t slice = emptyVector(SLICE_SIZE);
    MPI_Scatter(vector, SLICE_SIZE, MPI_ELEMENT_T, slice, SLICE_SIZE, MPI_ELEMENT_T, 0, MPI_COMM_WORLD);

    vector_t sliceSum = emptyVector(SLICE_SIZE);
    MPI_Reduce(slice, sliceSum, SLICE_SIZE, MPI_INT, MPI_SUM, 0, MPI_COMM_WORLD);
    
    if (rank == 0) {
        int sum = 0;
        for (int i = 0; i < SLICE_SIZE; i++) sum += sliceSum[i];
        float average = (float)sum / (SLICE_SIZE * size);
        printf("Sum: %d Average: %f\n", sum, average);
    }


    MPI_Finalize();
    return 0;
}