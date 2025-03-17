#include <stdio.h>
#include <mpi.h>
#include <random>
#include <math.h>

#define matrix_t int **
#define row_t int *
#define element_t int
#define MPI_ELEMENT_T MPI_INT

#define error_t unsigned char
#define ERR_VAL 1

#define ROW_SIZE 6

matrix_t emptyMatrix(int rowCount, int rowSize) {
    matrix_t matrix = (matrix_t)malloc(rowCount * sizeof(row_t));
    for (int i = 0; i < rowCount; i++) {
        matrix[i] = (row_t)malloc(rowSize * sizeof(element_t));
        for (int j = 0; j < rowSize; j++) matrix[i][j] = 0;
    }

    return matrix;
}

row_t emptyRow(int rowSize) {
    row_t row = (row_t)malloc(rowSize * sizeof(element_t));
    for (int i = 0; i < rowSize; i++) row[i] = 0;

    return row;
}

matrix_t generateMatrix(int rowCount, int rowSize) {
    srand((unsigned int)time(NULL));
    matrix_t matrix = emptyMatrix(rowCount, rowSize);
    for (int i = 0; i < rowCount; i++) {
        for (int j = 0; j < rowSize; j++) {
            element_t el = rand() % 9 + 1;
            matrix[i][j] = rand() % 2 == 0 ? el : -el;
        }
    }

    return matrix;
}

row_t flat(matrix_t matrix, int rowCount, int rowSize) {
    row_t row = emptyRow(rowCount * rowSize);
    for (int i = 0; i < rowCount; i++) {
        for (int j = 0; j < rowSize; j++) {
            row[i * rowSize + j] = matrix[i][j];
        }
    }

    return row;
}

matrix_t deflat(row_t row, int rowCount, int rowSize) {
    matrix_t matrix = emptyMatrix(rowCount, rowSize);
    for (int i = 0; i < rowCount; i++) {
        for (int j = 0; j < rowSize; j++) {
            matrix[i][j] = row[i * rowSize + j];
        }
    }

    return matrix;
}

void printMatrix(matrix_t matrix, int rowCount, int rowSize) {
    for (int i = 0; i < rowCount; i++) {
        for (int j = 0; j < rowSize; j++) printf("%d ", matrix[i][j]);
        printf("\n");
    }
}

void printRow(row_t row, int rowSize) {
    for (int i = 0; i < rowSize; i++) printf("%d ", row[i]);
    printf("\n");
}

row_t balanceRow(row_t row, int rowSize) {
    row_t brow = emptyRow(rowSize);
    int beamScales = 0;
    for (int i = 0; i < rowSize; i++) {
        brow[i] = row[i];
        beamScales += row[i] > 0 ? 1 : row[i] == 0 ? 0 : -1;
    }

    for (int i = 0; i < rowSize; i++) {
        if (beamScales == 0) break;
        if (brow[i] == 0) continue;
        if ((brow[i] > 0) == (beamScales > 0)) {
            brow[i] = -brow[i];
            beamScales += beamScales > 0 ? -2 : 2;
        }
    }

    return brow;
}

int countToReference(row_t row, int rowSize, element_t reference) {
    int count = 0;
    for (int i = 0; i < rowSize; i++) {
        if (row[i] > reference) {
            count++;
        }
    }

    return count;
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

    matrix_t matrix = emptyMatrix(size, ROW_SIZE);
    element_t reference = 0;

    if (rank == 0) {
        matrix = generateMatrix(size, ROW_SIZE);
        reference = matrix[size - 1][ROW_SIZE - 1];

        printf("Generated matrix:\n");
        printMatrix(matrix, size, ROW_SIZE);

        printf("Reference: %d\n", reference);
    }

    row_t absoluteRow = flat(matrix, size, ROW_SIZE);
    row_t dedicatedRow = emptyRow(ROW_SIZE);
    
    
    MPI_Scatter(absoluteRow, ROW_SIZE, MPI_ELEMENT_T, dedicatedRow, ROW_SIZE, MPI_ELEMENT_T, 0, MPI_COMM_WORLD);
    MPI_Bcast(&reference, 1, MPI_ELEMENT_T, 0, MPI_COMM_WORLD);



    row_t balancedRow = balanceRow(dedicatedRow, ROW_SIZE);
    int count = countToReference(balancedRow, ROW_SIZE, reference);

    printf("#%d Count: %d Modidfied row: ", rank, count);
    printRow(balancedRow, ROW_SIZE);

    row_t modAbsoluteRow = emptyRow(size * ROW_SIZE);
    int globalCount = 0;
    MPI_Reduce(&count, &globalCount, 1,  MPI_INT, MPI_SUM, 0, MPI_COMM_WORLD);
    MPI_Gather(balancedRow, ROW_SIZE, MPI_ELEMENT_T, modAbsoluteRow, ROW_SIZE, MPI_ELEMENT_T, 0, MPI_COMM_WORLD);

    if (rank == 0) {
        printf("Global count: %d\n", globalCount);
        matrix_t globalMatrix = deflat(modAbsoluteRow, size, ROW_SIZE);

        printf("Global matrix:\n");
        printMatrix(globalMatrix, size, ROW_SIZE);
    }

    MPI_Finalize();
    return 0;
}