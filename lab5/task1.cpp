#include <stdio.h>
#include <mpi.h>
#include <random>

#define element_t float
#define row_t     float *
#define matrix_t  float **

#define error_t unsigned char
#define ERR_VAL 1

typedef struct SqareMatrix {
    matrix_t data;
    size_t size;
} SqaureMatrix;

void smPrint(SqareMatrix sm) {
    for (size_t i = 0; i < sm.size; i++) {
        for (size_t j = 0; j < sm.size; j++) {
            printf("%.2f ", sm.data[i][j]);
        }
        printf("\n");
    }
}

SqaureMatrix smEmpty(size_t size) {
    SqareMatrix sm;
    sm.size = size;
    sm.data = (matrix_t)malloc(sizeof(row_t) * sm.size);

    for (int i = 0; i < sm.size; i++) {
        sm.data[i] = (row_t)malloc(sizeof(element_t) * sm.size);
        for (int j = 0; j < sm.size; j++) {
            sm.data[i][j] = 0;
        }
    }

    return sm;
}

SqaureMatrix smGenerate(size_t size) {
    srand(time(NULL));
    SqareMatrix sm;
    sm.size = size;
    sm.data = (matrix_t)malloc(sizeof(row_t) * sm.size);

    for (int i = 0; i < sm.size; i++) {
        sm.data[i] = (row_t)malloc(sizeof(element_t) * sm.size);
        for (int j = 0; j < sm.size; j++) {
            sm.data[i][j] = (element_t)(rand() % 19 - 9);
        }
    }

    return sm;
}

int main(int argc, char** argv) {
    // Initialize MPI
    MPI_Init(&argc, &argv);

    int rank, size;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);
    if (size < 2) {
        if (rank == 0) printf("Expected more than one process\n");
        MPI_Finalize();
        return ERR_VAL;
    }

    SqaureMatrix sm = rank == 0 
        ? smGenerate(size) 
        : smEmpty(size);

    if (rank == 0) { 
        printf("Generated matrix:\n");
        smPrint(sm);
    }

    for (int i = 0; i < sm.size; i++) {
        MPI_Bcast(sm.data[i], sm.size, MPI_FLOAT, 0, MPI_COMM_WORLD);
    }

    for (int pivot = 0; pivot < sm.size; pivot++) {
        if (rank == pivot) {
            element_t pval = sm.data[pivot][pivot];
            for (int j = pivot; j < sm.size; j++) {
                sm.data[pivot][j] /= pval;
            }
        }

        MPI_Bcast(sm.data[pivot], sm.size, MPI_FLOAT, pivot, MPI_COMM_WORLD);
        for (int i = pivot + 1; i < sm.size; i++) {
            if (rank == i) {
                element_t factor = sm.data[i][pivot];
                for (int j = pivot; j < sm.size; j++) {
                    sm.data[i][j] -= factor * sm.data[pivot][j];
                }
            }
        }
    }

    if (rank == 0) {
        printf("Row echelon form of original matrix:\n");
        smPrint(sm);
    }

    MPI_Finalize();
    return 0;
}