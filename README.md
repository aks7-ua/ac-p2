# Benchmark de Regresión Lineal (C++, x86 FPU y SSE)

Este proyecto implementa un benchmark de rendimiento para calcular una regresión lineal (`y = mx + b`) utilizando tres versiones diferentes:

- **Versión C++ pura**
- **Versión en ensamblador x86 FPU**
- **Versión usando instrucciones SSE (SIMD)**

Su propósito es comparar el tiempo de ejecución de cada versión en distintas arquitecturas de PC y generar resultados exportables para análisis.

---

## 🧠 Objetivo

Evaluar el rendimiento computacional de distintas implementaciones de regresión lineal y estudiar cómo afecta el uso de instrucciones de bajo nivel (ensamblador) y vectorización (SSE) en la eficiencia del cálculo.

---

## 📄 Estructura del programa

El programa ofrece dos modos:

### 1. **Modo Demo**
Calcula una regresión sobre un conjunto de datos generados linealmente (`y = 2x + 5`) y muestra los resultados y tiempos para cada versión.

```bash
Seleccione el modo:
1. Demo
2. Benchmark
0. Salir
```

### 2. **Modo Benchmark**
- Solicita el nombre del equipo (para registrar los resultados).
- Ofrece la opción de exportar a CSV.
- Ejecuta múltiples iteraciones con tamaño de muestra fijo (10000 datos).
- Registra el tiempo total que toma cada versión (C++, FPU, SSE) en completarlas.

Los datos se pueden imprimir por pantalla y también guardar en un archivo `.csv` para su posterior análisis con herramientas como Excel o Google Sheets.

---

## 📈 Ejemplo de salida (CSV)

```
Equipo,Iteraciones,Tamaño muestra,Metodo,TiempoTotal_sec
equipo-luis,10000,10000,C++,0.742
equipo-luis,10000,10000,x86,0.366
equipo-luis,10000,10000,SSE,0.045
```

---

## 🛠️ Requisitos y Compilación

Este proyecto requiere compilación en un entorno que soporte ensamblador inline x86 y SSE. Se recomienda:

- **Visual Studio (x86)** en modo 32 bits.
- Flags de compilación sugeridas: `/arch:SSE2` y `/O2`

---

## 📎 Uso y Aplicaciones

Este benchmark sirve como herramienta didáctica para comparar distintas técnicas de implementación de algoritmos numéricos:

- Evaluar optimizaciones por compilador.
- Estudiar arquitectura de procesadores.
- Analizar impacto de instrucciones SIMD vs. escalar.

---

## 🧪 Notas

- El código está diseñado para experimentación académica.
- No busca ser una implementación final de regresión lineal para uso profesional.

---

## ✍️ Autores

Grupo G5  
Asignatura: Arquitectura de Computadores
