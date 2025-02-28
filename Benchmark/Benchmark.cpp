#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>

#define SAMPLES 3
#define MAX_SIZE 1000

int A[MAX_SIZE][MAX_SIZE], B[MAX_SIZE][MAX_SIZE], C[MAX_SIZE][MAX_SIZE];

void initialize_matrix(int matrix[MAX_SIZE][MAX_SIZE], int N) {
    for (int i = 0; i < N; i++) {
        for (int j = 0; j < N; j++) {
            matrix[i][j] = rand() % 10;
        }
    }
}

void multiply_matrices(int N) {
    for (int i = 0; i < N; i++) {
        for (int j = 0; j < N; j++) {
            C[i][j] = 0;
            for (int k = 0; k < N; k++) {
                C[i][j] += A[i][k] * B[k][j];
            }
        }
    }
}

void print_matrix(int matrix[MAX_SIZE][MAX_SIZE], int N) {
    printf("Matriz (%dx%d):\n", N, N);
    for (int i = 0; i < N; i++) {
        for (int j = 0; j < N; j++) {
            printf("%2d ", matrix[i][j]); // %2d para alineación bonita
        }
        printf("\n");
    }
    printf("\n");
}

void multiply_matrices_asm(int N) {
    __asm {
        mov ecx, N
        xor esi, esi
        loop_i :
        xor edi, edi
            loop_j :
        xor eax, eax
            xor edx, edx
            loop_k :
        mov ebx, esi
            imul ebx, N
            add ebx, edx
            mov ebx, [A + ebx * 4]

            mov edi, edx
            imul edi, N
            add edi, ecx
            mov edi, [B + edi * 4]

            imul ebx, edi
            add eax, ebx

            inc edx
            cmp edx, N
            jl loop_k

            mov ebx, esi
            imul ebx, N
            add ebx, edi
            mov[C + ebx * 4], eax

            inc edi
            cmp edi, N
            jl loop_j

            inc esi
            cmp esi, N
            jl loop_i
    }
}

int main() {
    srand(time(NULL));

    for (int i = 0; i < SAMPLES; i++) {
        int N = 1000;

        clock_t start, end;
        initialize_matrix(A, N);
        initialize_matrix(B, N);

        start = clock();
        multiply_matrices(N);
        end = clock();
        printf("C - Tiempo: %.3f s\n", (double)(end - start) / CLOCKS_PER_SEC);

        start = clock();
        multiply_matrices_asm(N);
        end = clock();
        printf("ASM - Tiempo: %.3f s\n", (double)(end - start) / CLOCKS_PER_SEC);
    }
    return 0;
}
