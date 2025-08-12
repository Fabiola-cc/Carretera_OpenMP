#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>
#include <stdbool.h>
#include <omp.h>

// Estructuras de datos
typedef struct {
    int id_v;
    int tipo; // 0=carro, 1=moto, 2=camion
    int state; // 0=en espera, 1=movimiento
    char position[11]; // idInterseccion_idSemaforo
} Vehicle;

typedef struct {
    int id_s;
    int state; // 0=rojo, 1=amarillo, 2=verde
    int timer;  // tiempo restante en ese estado
} Semaphore;

typedef struct {
    int id_i;
    Semaphore semaforos[2]; //semaforos en la intersección
} Intersection;

void update_semaphore(Semaphore semaforos[]) {
    if (--semaforos[0].timer <= 0) {
        switch (semaforos[0].state) {
            case 0: // rojo → verde
                semaforos[0].state = 2;
                semaforos[0].timer = 5;
                semaforos[1].state = 0;
                semaforos[1].timer = 8;
                break;
            case 1: // amarillo → rojo
                semaforos[0].state = 0;
                semaforos[0].timer = 8;
                semaforos[1].state = 2;
                semaforos[1].timer = 5;
                break;
            case 2: // verde → amarillo
                semaforos[0].state = 1;
                semaforos[0].timer = 3;
                break;
        }
    }
}

Vehicle movement(Vehicle vehicle, Intersection intersecciones[], int total_intersecciones){
    int inId, sId;
    sscanf(vehicle.position, "%d_%d", &inId, &sId);

    const char* tipo;
    if (vehicle.tipo == 0)
        tipo = "carro";
    else if (vehicle.tipo == 1)
        tipo = "moto";
    else if (vehicle.tipo == 2)
        tipo = "camion";

    printf("\nEl %s %d está en el semáforo %d", tipo, vehicle.id_v, sId);
    
    // obtener estado de semaforo
    int semaforo_state = intersecciones[inId].semaforos[sId].state;
    // genera número entre 0 y 99
    int r = rand() % 100; 

    switch (semaforo_state){
        case 2: //verde
            // simular probabilidad de avanzar a siguiente interseccion
            if (r >= 70)  // 70% de probabilidad 
                return vehicle;

            inId = (inId + sId + 1) % total_intersecciones;
            sId = (sId + 1) % 2;  // índice dentro de la intersección (0 o 1)
            printf(" (en verde) y avanza a la intersección %d", inId);
            
            break;

        case 1: //amarillo
            // simular probabilidad de avanzar a siguiente interseccion
            if (r >= 30) // 30% de probabilidad
                return vehicle;

            inId = (inId + sId + 1) % total_intersecciones;
            sId = (sId + 1) % 2;  // índice dentro de la intersección (0 o 1)
            printf(" (en amarillo) y avanza a la intersección %d", inId);
            
            break;

        default: //rojos
            printf(" en rojo");
            return vehicle;
    }

    // Guardar nueva posicion
    sprintf(
            vehicle.position,
            "%d_%d",
            intersecciones[inId].id_i,
            intersecciones[inId].semaforos[sId].id_s
        );
    return vehicle;
}

#define CUANTOS_VEHICULOS 40
#define CUANTAS_INTERSECCIONES 5

/*
    Lógica general del ciclo, cambian semáforos y los vehículos afectados se mueven
*/
int action(Intersection intersecciones[], Vehicle vehiculos[]) {
    int hilos = omp_get_max_threads();
    omp_set_num_threads(hilos);

    // Paralelizar por intersección
    #pragma omp parallel for schedule(dynamic)
    for (int i = 0; i < CUANTAS_INTERSECCIONES; i++) {
        printf("\n\nIntersección %d (Hilo %d)\n", i, omp_get_thread_num());
        update_semaphore(intersecciones[i].semaforos);
        
        // Paralelismo anidado por vehículo
        #pragma omp parallel for schedule(auto)
        for (int j = 0; j < CUANTOS_VEHICULOS; j++) {
            int inId, sId;
            sscanf(vehiculos[j].position, "%d_%d", &inId, &sId);

            if (inId == intersecciones[i].id_i) {
                vehiculos[j] = movement(vehiculos[j], intersecciones, CUANTAS_INTERSECCIONES);
            }
        }
    }

    printf("\n");
    return 0;
}

/**
 * Para efectos de la simulación asumimos que la intersección 1 semaforo 1 es seguida por la 3 y del semaforo 0 seguida por la 2
 * Si hay N cantidad de intersecciones se pasa de la N a la 1 y 2 (formando un ciclo)
 * Además si un auto está en el semáforo 0 de n intersección, avanzará al semáforo 1 de la intersección n+1 y viceversa
 */
int main() {
    srand(time(NULL)); // Semilla para valores aleatorios

    int cuantos_semaforos = CUANTAS_INTERSECCIONES*2;
    
    Vehicle vehiculos[CUANTOS_VEHICULOS];
    Intersection intersecciones[CUANTAS_INTERSECCIONES];

    // Inicializar semáforos
    int inter_creadas = 0;
    for (int i = 0; i < cuantos_semaforos; i++) {
        Semaphore tempS;
        tempS.id_s = i;
        tempS.timer = 0;

        if (i % 2 == 0) { // índice par → primer semáforo de la intersección
            tempS.state = rand() % 3; // 0=rojo,1=amarillo,2=verde
            intersecciones[inter_creadas].id_i = inter_creadas; // ID de intersección = contador
            intersecciones[inter_creadas].semaforos[0] = tempS;
        } else { // índice impar → segundo semáforo
            // Definir estado congruente según el primero
            int estado_primero = intersecciones[inter_creadas].semaforos[0].state;
            if (estado_primero == 2) {          // primero verde → segundo rojo
                tempS.state = 0;
            } else if (estado_primero == 1) {   // primero amarillo → segundo rojo
                tempS.state = 0;
            } else {                            // primero rojo → segundo verde o amarillo
                tempS.state = (rand() % 2) + 1; // 1 o 2
            }
            intersecciones[inter_creadas].semaforos[1] = tempS;
            inter_creadas++; // cerrar esta intersección
        }
    }

    // Inicializar vehículos
    for (int i = 0; i < CUANTOS_VEHICULOS; i++) {
        vehiculos[i].id_v = i;
        vehiculos[i].tipo = rand() % 3; // 0=carro, 1=moto, 2=camion

        int inter_idx = (inter_idx + 1) % CUANTAS_INTERSECCIONES;
        int semaf_idx = rand() % 2; // índice dentro de la intersección (0 o 1)

        // Guardar posición como "interseccionID_semaforoID"
        sprintf(
            vehiculos[i].position,
            "%d_%d",
            inter_idx,
            semaf_idx
        );

        // Estado del vehículo según el semáforo asignado
        int semaforo_state = intersecciones[inter_idx].semaforos[semaf_idx].state;
        vehiculos[i].state = (semaforo_state == 0) ? 0 : 1;
    }

    int time_execution = 10;
    int i = 0;
    while (i < 10){
        printf("\n----- Ciclo %d -----\n", i);
        action(intersecciones, vehiculos);
        i++;
    }
    
    return 0;
}