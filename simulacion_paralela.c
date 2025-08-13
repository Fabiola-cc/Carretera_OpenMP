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
    int inId; // posición directa
    int sId;
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
        --semaforos[1].timer;
        switch (semaforos[0].state) {
            case 0:
                semaforos[0].state = 2; // rojo → verde
                semaforos[0].timer = 2;
                semaforos[1].state = 0; // amarillo → rojo
                semaforos[1].timer = 3;
                break;
            case 1:
                semaforos[0].state = 0; // amarillo → rojo
                semaforos[0].timer = 3;
                semaforos[1].state = 2; // rojo → verde
                semaforos[1].timer = 2;
                break;
            case 2: 
                semaforos[0].state = 1; // verde → amarillo
                semaforos[0].timer = 1;
                semaforos[1].state = 0; // permanece en rojo
                break;
        }
    }
}

Vehicle movement(Vehicle vehicle, Intersection intersecciones[], int total_intersecciones, unsigned int *seed){
    int inId = vehicle.inId;
    int sId = vehicle.sId;

    const char* tipo;
    if (vehicle.tipo == 0)
        tipo = "carro";
    else if (vehicle.tipo == 1)
        tipo = "moto";
    else
        tipo = "camion";

    // Usa rand_r para uso paralelo seguro
    int r = rand_r(seed) % 100; 

    int semaforo_state = intersecciones[inId].semaforos[sId].state;

    switch (semaforo_state){
        case 2: //verde
            if (r >= 70)
                return vehicle;

            inId = (inId + sId + 1) % total_intersecciones;
            sId = (sId + 1) % 2;
            break;

        case 1: //amarillo
            if (r >= 30)
                return vehicle;

            inId = (inId + sId + 1) % total_intersecciones;
            sId = (sId + 1) % 2;
            break;

        default: //rojo
            return vehicle;
    }

    vehicle.inId = inId;
    vehicle.sId = sId;

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

    // Buffers para estados de semáforos (antes y después)
    typedef struct {
        int before0, before1;
        int after0, after1;
    } SemStateLog;

    SemStateLog sem_logs[CUANTAS_INTERSECCIONES];

    // Guardar estados antes
    for (int i = 0; i < CUANTAS_INTERSECCIONES; i++) {
        sem_logs[i].before0 = intersecciones[i].semaforos[0].state;
        sem_logs[i].before1 = intersecciones[i].semaforos[1].state;
    }

    // Actualizar semáforos (paralelo)
    #pragma omp parallel for schedule(static)
    for (int i = 0; i < CUANTAS_INTERSECCIONES; i++) {
        update_semaphore(intersecciones[i].semaforos);
    }

    // Guardar estados después
    for (int i = 0; i < CUANTAS_INTERSECCIONES; i++) {
        sem_logs[i].after0 = intersecciones[i].semaforos[0].state;
        sem_logs[i].after1 = intersecciones[i].semaforos[1].state;
    }

    // Buffer de logs por vehículo
    char log_buffer[CUANTOS_VEHICULOS][100] = {{0}};

    // Mover vehículos y guardar logs en buffer
    #pragma omp parallel for schedule(dynamic)
    for (int j = 0; j < CUANTOS_VEHICULOS; j++) {
        if (vehiculos[j].state == 1) {
            Vehicle v_old = vehiculos[j];
            
            unsigned int seed = time(NULL) ^ omp_get_thread_num() ^ j;
            vehiculos[j] = movement(vehiculos[j], intersecciones, CUANTAS_INTERSECCIONES, &seed);

            // Ejemplo de guardar log (puedes personalizar)
            const char* tipo_str = (vehiculos[j].tipo == 0) ? "carro" :
                                  (vehiculos[j].tipo == 1) ? "moto" : "camion";

            snprintf(log_buffer[j], 100, 
                "Vehículo %d (%s) pasó de %d_%d a %d_%d\n", 
                vehiculos[j].id_v, tipo_str,
                v_old.inId, v_old.sId, 
                vehiculos[j].inId, vehiculos[j].sId);
        }
    }

    // Imprimir logs secuencialmente
    // Estados de semáforos
    const char *color_str[] = {"rojo", "amarillo", "verde"};
    for (int i = 0; i < CUANTAS_INTERSECCIONES; i++) {
        if (sem_logs[i].before0 != sem_logs[i].after0) {
            printf("Intersección %d, Semáforo 0 cambió de %s a %s\n",
                i, color_str[sem_logs[i].before0], color_str[sem_logs[i].after0]);
        }
        if (sem_logs[i].before1 != sem_logs[i].after1) {
            printf("Intersección %d, Semáforo 1 cambió de %s a %s\n",
                i, color_str[sem_logs[i].before1], color_str[sem_logs[i].after1]);
        }
    }

    // Movimiento de vehículos
    for (int j = 0; j < CUANTOS_VEHICULOS; j++) {
        if (log_buffer[j][0] != '\0') {
            printf("%s", log_buffer[j]);
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
    #pragma omp parallel for
    for (int idx = 0; idx < CUANTAS_INTERSECCIONES; idx++) {
        Semaphore s0 = {.id_s = idx * 2, .timer = 0};
        s0.state = rand() % 3;
        intersecciones[idx].id_i = idx;
        intersecciones[idx].semaforos[0] = s0;

        Semaphore s1 = {.id_s = idx * 2 + 1, .timer = 0};
        if (s0.state == 2 || s0.state == 1)
            s1.state = 0;
        else
            s1.state = (rand() % 2) + 1;
        intersecciones[idx].semaforos[1] = s1;
    }

    // Inicializar vehículos
    #pragma omp parallel for
    for (int i = 0; i < CUANTOS_VEHICULOS; i++) {
        vehiculos[i].id_v = i;
        vehiculos[i].tipo = rand() % 3; // 0=carro, 1=moto, 2=camion

        int inter_idx = i % CUANTAS_INTERSECCIONES;
        int semaf_idx = rand() % 2; // índice dentro de la intersección (0 o 1)

        // Guardar posición 
        vehiculos[i].inId = inter_idx;
        vehiculos[i].sId = semaf_idx;

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