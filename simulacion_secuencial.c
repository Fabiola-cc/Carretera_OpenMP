#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
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

void semaphore_logic(Semaphore semaforos[]){
    switch (semaforos[0].state) {
        case 0:
            printf("IDK0\n");
            sleep(8); // Simula espera en rojo
            semaforos[0].state = 2; //cambia a verde
            semaforos[1].state = 0; //cambia a rojo
            break;
        case 1:
            printf("IDK1\n");
            sleep(3); // Simula espera en amarillo
            semaforos[0].state = 0; //cambia a rojo
            semaforos[1].state = 2; //cambia a verde
            break;
        case 2:
            printf("IDK2\n");
            sleep(5); // Simula espera en verde
            semaforos[0].state = 1; //cambia a amarillo
            // el otro semaforo permanece en rojo
            break;
        default:
            break;
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

    printf("\nEl %s %d está en la intersección %d", tipo, vehicle.id_v, inId);
    
    // obtener estado de semaforo
    int semaforo_state = intersecciones[inId].semaforos[sId].state;
    // genera número entre 0 y 99
    int r = rand() % 100; 

    switch (semaforo_state){
        case 2: //verde
            // simular probabilidad de avanzar a siguiente interseccion
            if (r >= 70)  // 70% de probabilidad 
                return vehicle;

            inId = (inId + 2) % total_intersecciones;
            sId = (sId + 1) % 2; 
            printf(" y avanza a la intersección %d V\n", inId);
            
            break;

        case 1: //amarillo
            // simular probabilidad de avanzar a siguiente interseccion
            if (r >= 30) // 30% de probabilidad
                return vehicle;

            inId = (inId + 2) % total_intersecciones;
            sId = (sId + 1) % 2; 
            printf(" y avanza a la intersección %d A\n", inId);
            
            break;

        default: //rojos
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

/**
 * Para efectos de la simulación asumimos que la intersección 1 es seguida por la 3, la 2 por la 4, y así sucesivamente.
 * Si hay N cantidad de intersecciones se pasa de la N a la 2 (formando un ciclo) y N-1 a 1
 * Además si un auto está en el semáforo 0 de n intersección, avanzará al semáforo 1 de la intersección n+1 y viceversa
 */
int main() {
    srand(time(NULL)); // Semilla para valores aleatorios

    int cuantos_vehiculos = 40;
    int cuantas_intersecciones = 5;
    int cuantos_semaforos = cuantas_intersecciones*2;
    int max_intersección = cuantos_vehiculos/cuantas_intersecciones;

    Vehicle vehiculos[cuantos_vehiculos];
    Intersection intersecciones[cuantas_intersecciones];

    // Inicializar semáforos
    int inter_creadas = 0;
    for (int i = 0; i < cuantos_semaforos; i++) {
        Semaphore tempS;
        tempS.id_s = i;

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

    // Cambios de estado en semaforos ## Añadir paralelizacion
    // for (int i = 0; i < cuantas_intersecciones; i++){
    //     printf("\ninterseccion %d\n", i);
    //     semaphore_logic(intersecciones[i].semaforos);
    // }

    // Movimiento de vehículos ## Añadir paralelizacion
    for (int i = 0; i < cuantos_vehiculos; i++){
        printf("\nvehiculo %d\n", i);
        vehiculos[i] = movement(vehiculos[i], intersecciones, cuantas_intersecciones);
    }
    return 0;
}