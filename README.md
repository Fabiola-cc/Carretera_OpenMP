# Simulación de Tráfico con Intersecciones y Semáforos 🚦

Este proyecto implementa una **simulación de tráfico** que involucra intersecciones, semáforos y vehículos en movimiento.
Incluye dos versiones del código:

1. **Versión secuencial** – Ejecución paso a paso en un solo hilo.
2. **Versión paralelizada con OpenMP** – Aprovecha múltiples núcleos para mejorar el rendimiento.

---

## 📋 Descripción del problema

La simulación modela un conjunto de intersecciones con semáforos y un flujo de vehículos que circulan entre ellas.
Cada ciclo de la simulación:

* Actualiza el estado de los semáforos.
* Mueve los vehículos entre intersecciones.
* Registra los cambios y genera logs del tráfico.

El objetivo principal es **reducir el tiempo de simulación** mediante paralelización de las tareas que pueden ejecutarse de forma independiente.

---

## ⚡ Estrategia de paralelización

En la versión con OpenMP:

* **Inicialización de intersecciones y semáforos**
  Paralelizada por intersección para evitar *race conditions* entre semáforos que comparten datos.

* **Actualización de semáforos**
  Paralelizada con `#pragma omp parallel for schedule(static)` ya que las actualizaciones son independientes por intersección.

* **Movimiento de vehículos**
  Paralelizado con `#pragma omp parallel for schedule(dynamic)` para balancear la carga de trabajo, ya que no todos los vehículos se mueven en cada ciclo.

* **Logs de tráfico**
  El registro se almacena en un buffer privado por hilo y se imprime de forma secuencial para evitar intercalado en la salida estándar.

---

## 🛠 Justificación del uso de OpenMP

**OpenMP** permite convertir bucles independientes en paralelos con mínimas modificaciones al código:

* **`schedule(dynamic)`** en el movimiento de vehículos permite un mejor balance cuando las tareas son irregulares.
* **`schedule(static)`** en la actualización de semáforos es más eficiente cuando las tareas tienen similar duración.
* Uso de **variables privadas por hilo** (`rand_r()`, buffers de logs) para evitar *race conditions*.

## 🚀 Compilación y ejecución

### Versión secuencial

```bash
gcc -o simulacion simulacion_secuencial.c
./simulacion
```

### Versión paralela

```bash
gcc -fopenmp -o simulacion simulacion_paralela.c
./simulacion
```

Puedes ajustar el número de hilos:

```bash
export OMP_NUM_THREADS=4
```

---

## 📊 Comparación de rendimiento

| Versión      | Tiempo de ejecución (ejemplo) | Núcleos usados |
| ------------ | ----------------------------- | -------------- |
| Secuencial   | 2.35 s                        | 1              |
| Paralelizada | 0.82 s                        | 4              |

> *Los resultados dependen del hardware y parámetros de simulación.*