#include <stdio.h>
#include <stdlib.h>
#include <time.h>

// Estructuras de datos
typedef struct {
    int id_v;
    int tipo; // 0=carro, 1=moto, 2=camion
    int state; // 0=en espera, 1=movimiento
    char position[]; // idInterseccion_idSemaforo
} Vehicle;

typedef struct {
    int id_s;
    int state; // 0=rojo, 1=amarillo, 2=verde
} Semaphore;

typedef struct {
    int id_i;
    Semaphore semaforos[2]; //semaforos en la intersección
} Intersection;

int semaphore_logic(){
    return 0;
}

int main() {
    srand(time(NULL)); // Semilla para valores aleatorios

    int cuantos_vehiculos = 40;
    int cuantos_semaforos = 10;
    int cuantas_intersecciones = cuantos_semaforos/2;

    Vehicle vehiculos[cuantos_vehiculos];
    Intersection intersecciones[cuantas_intersecciones];

    // Inicializar semáforos
    int inter_creadas = 0;
    for (int i = 0; i < cuantos_semaforos; i++) {
        Semaphore tempS;
        tempS.id_s = i;

        if (i % 2 == 0) { // índice par → primer semáforo de la intersección
            tempS.state = 0; // rojo
            intersecciones[inter_creadas].id_i = inter_creadas; // ID de intersección = contador
            intersecciones[inter_creadas].semaforos[0] = tempS;
        } else { // índice impar → segundo semáforo
            tempS.state = rand() % 2 + 1; // amarillo=1, verde=2
            intersecciones[inter_creadas].semaforos[1] = tempS;
            inter_creadas++; // cerrar esta intersección
        }
    }

    // Inicializar vehículos
    for (int i = 0; i < cuantos_vehiculos; i++) {
        vehiculos[i].id_v = i;
        vehiculos[i].tipo = rand() % 3; // 0=carro, 1=moto, 2=camion

        int inter_idx = (i * 2) % cuantas_intersecciones;
        int semaf_idx = (i * 2) % 2; // índice dentro de la intersección (0 o 1)

        // Guardar posición como "interseccionID_semaforoID"
        sprintf(
            vehiculos[i].position,
            "%d_%d",
            intersecciones[inter_idx].id_i,
            intersecciones[inter_idx].semaforos[semaf_idx].id_s
        );

        // Estado del vehículo según el semáforo asignado
        int semaforo_state = intersecciones[inter_idx].semaforos[semaf_idx].state;
        vehiculos[i].state = (semaforo_state == 0) ? 0 : 1;
    }

    // Mostrar datos
    printf("Vehículos:\n");
    for (int i = 0; i < cuantos_vehiculos; i++) {
        printf("ID:%d Tipo:%d Estado:%d\n", vehiculos[i].id_v, vehiculos[i].tipo, vehiculos[i].state);
        printf("\tPosicion:%s\n", vehiculos[i].position);
    }

    printf("\nIntersecciones:\n");
    for (int i = 0; i < cuantas_intersecciones; i++) {
        printf("Interseccion %d: S1(ID:%d, Estado:%d) S2(ID:%d, Estado:%d)\n",
               intersecciones[i].id_i,
               intersecciones[i].semaforos[0].id_s, intersecciones[i].semaforos[0].state,
               intersecciones[i].semaforos[1].id_s, intersecciones[i].semaforos[1].state);
    }

    return 0;
}