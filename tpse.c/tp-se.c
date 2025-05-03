#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <semaphore.h>
#include <unistd.h>
#include <time.h>

#define NB_BUS_X 5
#define NB_BUS_Y 4
#define NB_TRAJETS_ALLER_RETOUR 10
#define MAX_THREADS (NB_BUS_X + NB_BUS_Y)

typedef struct {
    int id;
    char ville_depart;
} bus_info;

sem_t sem_tunnel;
sem_t mutex_nb_X;
sem_t mutex_nb_Y;
int nb_bus_X_dans_tunnel = 0;
int nb_bus_Y_dans_tunnel = 0;


void entrer_tunnel_X_vers_Y(int id) {
    sem_wait(&mutex_nb_X);
    nb_bus_X_dans_tunnel++;
    if (nb_bus_X_dans_tunnel == 1) {
        sem_wait(&sem_tunnel);
        printf("--> Direction X->Y ouverte.\n");
    }
    sem_post(&mutex_nb_X);

    printf("Bus [X%d] entre dans le tunnel (X->Y). %d bus X dedans.\n", id, nb_bus_X_dans_tunnel);
}

void sortir_tunnel_X_vers_Y(int id) {
     sem_wait(&mutex_nb_X);
     nb_bus_X_dans_tunnel--;
     printf("Bus [X%d] sort du tunnel (arrive en Y). %d bus X restants.\n", id, nb_bus_X_dans_tunnel);
     if (nb_bus_X_dans_tunnel == 0) {
         printf("<-- Direction X->Y fermee.\n");
         sem_post(&sem_tunnel);
     }
     sem_post(&mutex_nb_X);
}

void entrer_tunnel_Y_vers_X(int id) {
    sem_wait(&mutex_nb_Y);
    nb_bus_Y_dans_tunnel++;
    if (nb_bus_Y_dans_tunnel == 1) {
        sem_wait(&sem_tunnel);
         printf("--> Direction Y->X ouverte.\n");
    }
    sem_post(&mutex_nb_Y);

    printf("Bus [Y%d] entre dans le tunnel (Y->X). %d bus Y dedans.\n", id, nb_bus_Y_dans_tunnel);
}

void sortir_tunnel_Y_vers_X(int id) {
    sem_wait(&mutex_nb_Y);
    nb_bus_Y_dans_tunnel--;
    printf("Bus [Y%d] sort du tunnel (arrive en X). %d bus Y restants.\n", id, nb_bus_Y_dans_tunnel);
    if (nb_bus_Y_dans_tunnel == 0) {
        printf("<-- Direction Y->X fermee.\n");
        sem_post(&sem_tunnel);
    }
    sem_post(&mutex_nb_Y);
}

void *fonction_bus(void *arg) {
    bus_info *info = (bus_info *)arg;
    int id = info->id;
    char ville_depart_initiale = info->ville_depart;
    char ville_actuelle = ville_depart_initiale;
    char ville_destination;

    srand(time(NULL) ^ pthread_self());

    for (int i = 1; i <= NB_TRAJETS_ALLER_RETOUR; i++) {

        if (ville_actuelle == 'X') {
            ville_destination = 'Y';
            printf("Bus [%c%d] veut aller X->Y (Trajet %d aller)\n", ville_actuelle, id, i);
            entrer_tunnel_X_vers_Y(id);
            printf("    Bus [%c%d] dans tunnel: X->Y (Trajet %d)\n", ville_actuelle, id, i);
            usleep((1 + (rand() / (RAND_MAX / 0.5))) * 1000000);
            sortir_tunnel_X_vers_Y(id);
        } else {
            ville_destination = 'X';
             printf("Bus [%c%d] veut aller Y->X (Trajet %d aller)\n", ville_actuelle, id, i);
            entrer_tunnel_Y_vers_X(id);
            printf("    Bus [%c%d] dans tunnel: Y->X (Trajet %d)\n", ville_actuelle, id, i);
            usleep((1 + (rand() / (RAND_MAX / 0.5))) * 1000000);
            sortir_tunnel_Y_vers_X(id);
        }
        ville_actuelle = ville_destination;


         if (ville_actuelle == 'X') {
            ville_destination = 'Y';
            printf("Bus [%c%d] veut aller X->Y (Trajet %d retour)\n", ville_actuelle, id, i);
            entrer_tunnel_X_vers_Y(id);
            printf("    Bus [%c%d] dans tunnel: X->Y (Trajet %d)\n", ville_actuelle, id, i);
            usleep((1 + (rand() / (RAND_MAX / 0.5))) * 1000000);
            sortir_tunnel_X_vers_Y(id);
        } else {
            ville_destination = 'X';
            printf("Bus [%c%d] veut aller Y->X (Trajet %d retour)\n", ville_actuelle, id, i);
            entrer_tunnel_Y_vers_X(id);
            printf("    Bus [%c%d] dans tunnel: Y->X (Trajet %d)\n", ville_actuelle, id, i);
            usleep((1 + (rand() / (RAND_MAX / 0.5))) * 1000000);
            sortir_tunnel_Y_vers_X(id);
        }
         ville_actuelle = ville_destination;
         printf("Bus [%c%d] a termine le trajet %d A/R.\n", ville_depart_initiale, id, i);

    }
     printf("* Bus [%c%d] a termine tous ses trajets. *\n", ville_depart_initiale, id);
    free(info);
    return NULL;
}int main() {
    pthread_t threads[MAX_THREADS];
    bus_info *info;
    int thread_count = 0;

    sem_init(&sem_tunnel, 0, 1);
    sem_init(&mutex_nb_X, 0, 1);
    sem_init(&mutex_nb_Y, 0, 1);

    printf("Simulation du trafic dans le tunnel...\n");

    for (int i = 0; i < NB_BUS_X; i++) {
        info = malloc(sizeof(bus_info));
        info->id = i + 1;
        info->ville_depart = 'X';
        pthread_create(&threads[thread_count++], NULL, fonction_bus, info);
    }

    for (int i = 0; i < NB_BUS_Y; i++) {
        info = malloc(sizeof(bus_info));
        info->id = i + 1;
        info->ville_depart = 'Y';
         pthread_create(&threads[thread_count++], NULL, fonction_bus, info);
    }

    for (int i = 0; i < thread_count; i++) {
        pthread_join(threads[i], NULL);
    }

    sem_destroy(&sem_tunnel);
    sem_destroy(&mutex_nb_X);
    sem_destroy(&mutex_nb_Y);

    printf("Simulation terminee.\n");

    return 0;
}