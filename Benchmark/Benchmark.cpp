#include <iostream>
#include <ctime>
#include <cstdlib>
#include <fstream>
#include <numeric>
#include <vector>
#include <limits>
#include <iomanip>

using namespace std;

// Número de elementos para el modo Demo
const int N = 10000;
double x[N], y[N];

// Control word para la FPU (0x027F = doble precisión, redondeo a par)
unsigned short cw = 0x027F;

float calcularRegresionCPP(const double* x, const double* y, int n, double& pendiente, double& interseccion) {
    clock_t inicio = clock();
    double sumaX = 0.0, sumaY = 0.0, sumaXY = 0.0, sumaX2 = 0.0;
    for (int i = 0; i < n; ++i) {
        sumaX += x[i];
        sumaY += y[i];
        sumaXY += x[i] * y[i];
        sumaX2 += x[i] * x[i];
    }
    pendiente = (n * sumaXY - sumaX * sumaY) / (n * sumaX2 - sumaX * sumaX);
    interseccion = (sumaY - pendiente * sumaX) / n;
    clock_t fin = clock();
    return 1000.0f * (float)(fin - inicio) / CLOCKS_PER_SEC;
}

// Calcula la regresión lineal y = m*x + b usando ensamblador x86 (FPU)
float calcularRegresionASM(const double* x, const double* y, int n, double& pendiente, double& interseccion) {
    // Acumuladores para las sumatorias necesarias
    double sumaX = 0.0, sumaX2 = 0.0, sumaY = 0.0, sumaXY = 0.0;
    double resultadoM = 0.0, resultadoB = 0.0;

    clock_t inicio = clock();

    __asm {
        finit                         // Inicializa la FPU
        fldcw word ptr[cw]            // Configura el control word (doble precisión, redondeo a par)

        // === SUMA DE X ===
        xor esi, esi                  // i = 0
        mov ecx, n                   // ecx = tamaño de la muestra
        fldz                          // ST0 = 0.0 (acumulador)
        loop_sumaX :
        cmp esi, ecx
            jge end_sumaX
            mov eax, x
            fld qword ptr[eax + esi * 8] // Carga x[i]
            faddp st(1), st(0)           // Acumula en ST0
            inc esi
            jmp loop_sumaX
            end_sumaX :
        fstp qword ptr[sumaX]        // Guarda sumaX

            // === SUMA DE X^2 ===
            xor esi, esi
            mov ecx, n
            fldz
            loop_sumaX2 :
        cmp esi, ecx
            jge end_sumaX2
            mov eax, x
            fld qword ptr[eax + esi * 8] // x[i]
            fmul st(0), st(0)            // x[i]^2
            faddp st(1), st(0)
            inc esi
            jmp loop_sumaX2
            end_sumaX2 :
        fstp qword ptr[sumaX2]

            // === SUMA DE Y ===
            xor esi, esi
            mov ecx, n
            fldz
            loop_sumaY :
        cmp esi, ecx
            jge end_sumaY
            mov eax, y
            fld qword ptr[eax + esi * 8] // y[i]
            faddp st(1), st(0)
            inc esi
            jmp loop_sumaY
            end_sumaY :
        fstp qword ptr[sumaY]

            // === SUMA DE X*Y ===
            xor esi, esi
            mov ecx, n
            fldz
            loop_sumaXY :
        cmp esi, ecx
            jge end_sumaXY
            mov eax, x
            fld qword ptr[eax + esi * 8] // x[i]
            mov ebx, y
            fld qword ptr[ebx + esi * 8] // y[i]
            fmulp st(1), st(0)           // x[i] * y[i]
            faddp st(1), st(0)
            inc esi
            jmp loop_sumaXY
            end_sumaXY :
        fstp qword ptr[sumaXY]

            // === CÁLCULO DE PENDIENTE (m) ===
            fild n                        // ST0 = n
            fld qword ptr[sumaXY]        // ST0 = sumaXY, ST1 = n
            fmulp st(1), st(0)           // ST0 = n * sumaXY

            fld qword ptr[sumaX]         // ST0 = sumaX, ST1 = n*sumaXY
            fld qword ptr[sumaY]         // ST0 = sumaY, ST1 = sumaX, ST2 = n*sumaXY
            fmulp st(1), st(0)           // ST0 = sumaX * sumaY
            fsubp st(1), st(0)           // ST0 = num = n*sumaXY - sumaX*sumaY

            fld qword ptr[sumaX2]        // ST0 = sumaX2, ST1 = num
            fild n
            fmulp st(1), st(0)           // ST0 = n * sumaX2

            fld qword ptr[sumaX]
            fmul st(0), st(0)            // ST0 = sumaX^2
            fsubp st(1), st(0)           // ST0 = den = n*sumaX2 - sumaX^2

            fdivp st(1), st(0)           // ST0 = pendiente
            fstp qword ptr[resultadoM]

            // === CÁLCULO DE INTERSECCIÓN (b) ===
            fld qword ptr[sumaY]         // ST0 = sumaY
            fld qword ptr[resultadoM]    // ST0 = pendiente, ST1 = sumaY
            fld qword ptr[sumaX]         // ST0 = sumaX, ST1 = pendiente, ST2 = sumaY
            fmulp st(1), st(0)           // ST0 = pendiente * sumaX
            fsubp st(1), st(0)           // ST0 = sumaY - pendiente * sumaX
            fild n
            fdivp st(1), st(0)           // ST0 = intersección
            fstp qword ptr[resultadoB]
    }

    clock_t fin = clock();
    pendiente = resultadoM;
    interseccion = resultadoB;

    return 1000.0f * (float)(fin - inicio) / CLOCKS_PER_SEC; // Devuelve tiempo en ms
}

// Calcula la regresión lineal y = m*x + b usando SSE para acumulaciones y FPU para el cálculo final
float calcularRegresionSSE(const double* x, const double* y, int n, double& pendiente, double& interseccion) {
    // Variables auxiliares para almacenar sumatorias
    double sumaX = 0.0, sumaX2 = 0.0, sumaY = 0.0, sumaXY = 0.0;
    double resultadoM = 0.0, resultadoB = 0.0;

    clock_t inicio = clock();

    __asm {
        finit
        fldcw word ptr[cw]  // Set FPU control word (redondeo + precisión)

        // --- Inicializar acumuladores SSE ---
        pxor xmm2, xmm2    // xmm2 = sumaX
        pxor xmm3, xmm3    // xmm3 = sumaY
        pxor xmm4, xmm4    // xmm4 = sumaX^2
        pxor xmm5, xmm5    // xmm5 = sumaXY

        mov esi, x         // puntero al vector x
        mov edi, y         // puntero al vector y
        mov ecx, n
        shr ecx, 1         // número de iteraciones (procesamos 2 elementos por vuelta)

        sse_loop:
        cmp ecx, 0
            je sse_loop_end

            // Cargar 2 valores de x e y en xmm0 y xmm1
            movupd xmm0, [esi]   // xmm0 = { x[i], x[i+1] }
            movupd xmm1, [edi]   // xmm1 = { y[i], y[i+1] }

            // Acumulación SSE de sumX y sumY
            addpd xmm2, xmm0     // sumaX += x[i] + x[i+1]
            addpd xmm3, xmm1     // sumaY += y[i] + y[i+1]

            // sumX^2 += x[i]^2 + x[i+1]^2
            movapd xmm7, xmm0
            mulpd xmm7, xmm0     // xmm7 = x[i]^2, x[i+1]^2
            addpd xmm4, xmm7

            // sumXY += x[i]*y[i] + x[i+1]*y[i+1]
            movapd xmm6, xmm0
            mulpd xmm6, xmm1     // xmm6 = x[i]*y[i], x[i+1]*y[i+1]
            addpd xmm5, xmm6

            // avanzar punteros
            add esi, 16  // 2 * sizeof(double)
            add edi, 16
            dec ecx
            jmp sse_loop

            sse_loop_end :

        // Reducir resultados SSE a escalar con suma horizontal
        haddpd xmm2, xmm2   // xmm2[0] = suma total de X
            haddpd xmm3, xmm3   // xmm3[0] = suma total de Y
            haddpd xmm4, xmm4   // xmm4[0] = suma total de X^2
            haddpd xmm5, xmm5   // xmm5[0] = suma total de X*Y

            // Almacenar los resultados parciales en memoria
            movsd sumaX, xmm2
            movsd sumaY, xmm3
            movsd sumaX2, xmm4
            movsd sumaXY, xmm5

            // --- Cálculo de la pendiente m ---
            fild n                      // ST0 = n
            fld qword ptr[sumaXY]       // ST0 = sumaXY, ST1 = n
            fmulp st(1), st(0)          // ST0 = n * sumaXY

            fld qword ptr[sumaX]        // ST0 = sumaX, ST1 = n*sumaXY
            fld qword ptr[sumaY]        // ST0 = sumaY, ST1 = sumaX, ST2 = n*sumaXY
            fmulp st(1), st(0)          // ST0 = sumaX * sumaY
            fsubp st(1), st(0)          // ST0 = (n * sumaXY - sumaX * sumaY)

            fld qword ptr[sumaX2]       // ST0 = sumaX^2, ST1 = numerador
            fild n                      // ST0 = n, ST1 = sumaX2, ST2 = numerador
            fmulp st(1), st(0)          // ST0 = n * sumaX2, ST1 = numerador

            fld qword ptr[sumaX]        // ST0 = sumaX, ST1 = n*sumaX2, ST2 = numerador
            fmul st(0), st(0)           // ST0 = sumaX^2
            fsubp st(1), st(0)          // ST0 = (n * sumaX2 - sumaX^2)

            fdivp st(1), st(0)          // ST0 = pendiente
            fstp qword ptr[resultadoM]

            // --- Cálculo de la intersección b ---
            fld qword ptr[sumaY]        // ST0 = sumaY
            fld qword ptr[resultadoM]   // ST0 = m, ST1 = sumaY
            fld qword ptr[sumaX]        // ST0 = sumaX, ST1 = m, ST2 = sumaY
            fmulp st(1), st(0)          // ST0 = m * sumaX
            fsubp st(1), st(0)          // ST0 = (sumaY - m * sumaX)
            fild n                      // ST0 = n, ST1 = numerador
            fdivp st(1), st(0)          // ST0 = intersección
            fstp qword ptr[resultadoB]
    }

    clock_t fin = clock();
    pendiente = resultadoM;
    interseccion = resultadoB;

    return 1000.0f * (float)(fin - inicio) / CLOCKS_PER_SEC; // tiempo en milisegundos
}


void modoDemo();
void modoBenchmark();

int main() {
    srand((unsigned int)time(NULL));
    int opcion;
    bool continuar = true;

    while (continuar) {
        cout << "\nSeleccione el modo:\n1. Demo\n2. Benchmark\n0. Salir\nOpción: ";
        cin >> opcion;

        switch (opcion) {
        case 1:
            modoDemo();
            break;
        case 2:
            modoBenchmark();
            break;
        case 0:
            continuar = false;
            break;
        default:
            cout << "Opción no válida. Intente de nuevo.\n";
        }
    }

    cout << "\nPrograma finalizado.\n";
    cout << "\nG5.\n";
    return 0;
}

void modoDemo() {
    vector<double> x(N), y(N);
    for (int i = 0; i < N; ++i) {
        x[i] = i;
        y[i] = 2.0 * i + 5.0;
    }

    double m_cpp, b_cpp, m_asm, b_asm, m_sse, b_sse;

    float t_cpp = calcularRegresionCPP(x.data(), y.data(), N, m_cpp, b_cpp);
    float t_asm = calcularRegresionASM(x.data(), y.data(), N, m_asm, b_asm);
    float t_sse = calcularRegresionSSE(x.data(), y.data(), N, m_sse, b_sse);

    cout << "\n--- MODO DEMO ---\n";
    cout << "C++:     y = " << m_cpp << "x + " << b_cpp << " (" << t_cpp << " ms)\n";
    cout << "x86 (FPU): y = " << m_asm << "x + " << b_asm << " (" << t_asm << " ms)\n";
    cout << "SSE:     y = " << m_sse << "x + " << b_sse << " (" << t_sse << " ms)\n";
}

void modoBenchmark() {
    string nombreEquipo;
    cout << "\nIngrese el nombre del equipo (por ejemplo: equipo-luis): ";
    cin >> nombreEquipo;

    int tam_muestra = 10000;
    vector<long> iteraciones = { 10000, 20000, 50000, 100000, 150000 };
    vector<double> x(tam_muestra), y(tam_muestra);
    for (int i = 0; i < tam_muestra; ++i) {
        x[i] = rand() % 1000;
        y[i] = rand() % 1000;
    }

    double m, b;
    float tiempo_total;

    bool exportar;
    cout << "\nDesea exportar los resultados a CSV? (1 = Si, 0 = No): ";
    cin >> exportar;

    ofstream resultados;
    if (exportar) {
        resultados.open("benchmark_resultados.csv");
        resultados << fixed << setprecision(6);
        resultados << "Equipo,Iteraciones,Tamaño muestra,Metodo,TiempoTotal_sec\n";
    }

    cout << fixed << setprecision(6);
    cout << "\n--- RESULTADOS DEL BENCHMARK ---\n";

    for (auto iter : iteraciones) {
        // CPP
        tiempo_total = 0.0f;
        for (long i = 0; i < iter; ++i)
            tiempo_total += calcularRegresionCPP(x.data(), y.data(), tam_muestra, m, b);
        float tiempo_cpp_s = tiempo_total / 1000.0f;
        cout << "Iteraciones: " << iter << " | C++: " << tiempo_cpp_s << " s total\n";
        if (exportar) {
            resultados << nombreEquipo << "," << iter << "," << tam_muestra << ",C++," << tiempo_cpp_s << "\n";
        }
        // x86
        tiempo_total = 0.0f;
        for (long i = 0; i < iter; ++i)
            tiempo_total += calcularRegresionASM(x.data(), y.data(), tam_muestra, m, b);
        float tiempo_asm_s = tiempo_total / 1000.0f;
        cout << "Iteraciones: " << iter << " | x86: " << tiempo_asm_s << " s total\n";
        if (exportar) {
            resultados << nombreEquipo << "," << iter << "," << tam_muestra << ",x86," << tiempo_asm_s << "\n";
        }
        // SSE
        tiempo_total = 0.0f;
        for (long i = 0; i < iter; ++i)
            tiempo_total += calcularRegresionSSE(x.data(), y.data(), tam_muestra, m, b);
        float tiempo_sse_s = tiempo_total / 1000.0f;
        cout << "Iteraciones: " << iter << " | SSE: " << tiempo_sse_s << " s total\n";
        if (exportar) {
            resultados << nombreEquipo << "," << iter << "," << tam_muestra << ",SSE," << tiempo_sse_s << "\n";
        }
    }

    if (exportar) {
        resultados.close();
        cout << "\nDatos guardados en benchmark_resultados.csv.\n";
    }
}

