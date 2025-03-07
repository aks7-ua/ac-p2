#include <iostream>
#include <iomanip>
#include <ctime>

using namespace std;

// Tamaño de la muestra
const int K_SAMPLE_SIZE = 10000;

// Función que calcula la regresión lineal en C
void rectaC(int x[], int y[]) {

    double avgX = 0.0, avgY = 0.0, varX = 0.0, covXY = 0.0;

    // Calculamos la media de X e Y
    for (int i = 0; i < K_SAMPLE_SIZE; i++) {
        avgX += x[i];
        avgY += y[i];
    }
    avgX /= K_SAMPLE_SIZE;
    avgY /= K_SAMPLE_SIZE;

    // Calculamos la varianza de X y la covarianza entre X e Y
    for (int i = 0; i < K_SAMPLE_SIZE; i++) {
        varX += (x[i] - avgX) * (x[i] - avgX);
        covXY += (x[i] - avgX) * (y[i] - avgY);
    }

    varX /= (K_SAMPLE_SIZE - 1);
    covXY /= (K_SAMPLE_SIZE - 1);

    // Cálculo de la pendiente y la ordenada en el origen
    double m = covXY / varX;
    double b = avgY - (m * avgX);

    //cout << "Ecuacion de la recta (C): y = " << m << "x + " << b << endl;
}


void rectaEnsamblador(int x[], int y[]) {
    int sumX = 0, sumY = 0, sumXCuadrado = 0, sumXY = 0;
    float mediaX, mediaY, varianzaX, covarianzaXY, m, b;
    int n_menos_uno = K_SAMPLE_SIZE - 1;  // Para el cálculo muestral

    __asm {
        xor esi, esi; Inicializar índice
        xor eax, eax
        xor ebx, ebx
        xor edx, edx

        ; Bucle para calcular sumatorias
        bucle :
        cmp esi, K_SAMPLE_SIZE
            jge terminar

            mov eax, [x + esi * 4]; x[i]
            mov ebx, [y + esi * 4]; y[i]

            add sumX, eax; sumX += x[i]
            add sumY, ebx; sumY += y[i]

            imul eax, eax
            add sumXCuadrado, eax; sumXCuadrado += x[i] ^ 2

            mov eax, [x + esi * 4]
            imul eax, ebx
            add sumXY, eax; sumXY += x[i] * y[i]

            inc esi
            jmp bucle

            terminar :

        ; Calcular mediaX = sumX / K_SAMPLE_SIZE
            fild sumX
            fild K_SAMPLE_SIZE
            fdiv
            fstp mediaX

            ; Calcular mediaY = sumY / K_SAMPLE_SIZE
            fild sumY
            fild K_SAMPLE_SIZE
            fdiv
            fstp mediaY

            ; Calcular varianzaX = (sumXCuadrado / K_SAMPLE_SIZE) - (mediaX * mediaX)
            fild sumXCuadrado
            fild K_SAMPLE_SIZE
            fdiv
            fld mediaX
            fmul mediaX
            fsub
            fstp varianzaX

            ; Calcular covarianzaXY = (sumXY / K_SAMPLE_SIZE) - (mediaX * mediaY)
            fild sumXY
            fild K_SAMPLE_SIZE
            fdiv
            fld mediaX
            fmul mediaY
            fsub
            fstp covarianzaXY

            ; Calcular pendiente m = covarianzaXY / varianzaX
            fld covarianzaXY
            fld varianzaX
            fdiv
            fstp m

            ; Calcular ordenada al origen b = mediaY - (m * mediaX)
            fld mediaX
            fmul m
            fld mediaY
            fsub
            fstp b
    }

    //cout << "Ecuacion de la recta (ASM optimizado): y = " << m << "x + " << b << endl;
}


// Función para inicializar datos de prueba
void generarDatos(int* x, int* y, int n) {
    for (int i = 0; i < n; i++) {
        x[i] = rand() % 10;
        y[i] = rand() % 10;
        // x[i] = i;                // Valores predecibles para x
        // y[i] = 2 * i + 3;        // Simula una relación lineal (y = 2x + 3)
    }
}

// Función genérica para medir el tiempo de ejecución de una función
double medirTiempo(void (*func)(int*, int*), int* x, int* y, int iteraciones) {
    auto inicio = clock();
    for (int i = 0; i < iteraciones; i++) {
        func(x, y);
    }
    return static_cast<double>(clock() - inicio) / CLOCKS_PER_SEC;
}

int main() {
    int x[K_SAMPLE_SIZE], y[K_SAMPLE_SIZE];
    srand(time(NULL));

    // Medición de tiempos
    double tiempo_C_5000 = medirTiempo(rectaC, x, y, 5000);
    double tiempo_Asm_5000 = medirTiempo(rectaEnsamblador, x, y, 5000);
    double tiempo_C_15000 = medirTiempo(rectaC, x, y, 15000);
    double tiempo_Asm_15000 = medirTiempo(rectaEnsamblador, x, y, 15000);

    // Mostrar resultados
    cout << fixed << setprecision(6);
    cout << "Tiempo en C (5000 iteraciones): " << tiempo_C_5000 << " segundos\n";
    cout << "Tiempo en Ensamblador (5000 iteraciones): " << tiempo_Asm_5000 << " segundos\n";
    cout << "Tiempo en C (15000 iteraciones): " << tiempo_C_15000 << " segundos\n";
    cout << "Tiempo en Ensamblador (15000 iteraciones): " << tiempo_Asm_15000 << " segundos\n";

    // Main parametrizado para que ejecute 10 veces la regresión
    // con itereaciones de 5000 en 5000
    /*int x[K_SAMPLE_SIZE], y[K_SAMPLE_SIZE];
    srand(time(NULL));

    cout << fixed << setprecision(6);

    for (int i = 1; i <= 10; ++i)
    {
        int iteraciones = i * 5000;
        double tiempo_C = medirTiempo(rectaC, x, y, iteraciones);
        // double tiempo_Asm = medirTiempo(rectaEnsamblador, x, y, iteraciones);

        cout << "Tiempo en C (" << iteraciones << " iteraciones): " << tiempo_C << " segundos\n";
        // cout << "Tiempo en Ensamblador (" << iteraciones << " iteraciones): " << tiempo_Asm << " segundos\n";
    }*/

    return 0;
}
