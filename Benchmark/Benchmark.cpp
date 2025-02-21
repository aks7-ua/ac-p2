// Benchmark.cpp : Este archivo contiene la función "main". La ejecución del programa comienza y termina ahí.

#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define N 512  // Tamaño de la matriz

// Inicializa una matriz con valores aleatorios
void initialize_matrix(int matrix[N][N]) {
    for (int i = 0; i < N; i++) {
        for (int j = 0; j < N; j++) {
            matrix[i][j] = rand() % 10; // Valores entre 0 y 9
        }
    }
}

// Multiplicación de matrices
void multiply_matrices(int A[N][N], int B[N][N], int C[N][N]) {
    for (int i = 0; i < N; i++) {
        for (int j = 0; j < N; j++) {
            C[i][j] = 0;
            for (int k = 0; k < N; k++) {
                C[i][j] += A[i][k] * B[k][j];
            }
        }
    }
}

void print_matrices(int X[N][N]) {
    for (int i = 0; i < N; i++) {
        for (int j = 0; j < N; j++) {
            printf("%d ", X[i][j]);
        }
        printf("\n");
    }
}

int main() {
    static int A[N][N], B[N][N], C[N][N];
    srand(time(NULL));

    initialize_matrix(A);
    initialize_matrix(B);

    clock_t start = clock();
    multiply_matrices(A, B, C);
    clock_t end = clock();

    //print_matrices(A);
    //printf("\n");
    //print_matrices(B);
    //printf("\n");
    //print_matrices(C);
    // print_matrices(B);
    // print_matrices(C);
    

    double time_taken = (double)(end - start) / CLOCKS_PER_SEC;
    printf("Tiempo de ejecución: %.6f segundos\n", time_taken);

    return 0;
}