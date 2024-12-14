#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>
#include <limits.h>

#define INFINITO INT_MAX  // infinito para as arestas inexistentes

void atualizar_distancias(int* distancias, int* distancias_vizinho, int total_nos, int peso_aresta);

int main(int argc, char** argv) {
    MPI_Init(&argc, &argv);

    int processId, processCount;
    MPI_Comm_rank(MPI_COMM_WORLD, &processId);
    MPI_Comm_size(MPI_COMM_WORLD, &processCount);

    int total_nos = 7;
    int distancias[total_nos];
    int pesos_vizinhos[total_nos];

    // Matriz de adjacência com pesos das arestas
    int grafo[7][7] = {
        {0, 3, INFINITO, INFINITO, INFINITO, INFINITO, INFINITO}, // A
        {3, 0, INFINITO, 1, 2, INFINITO, INFINITO},               // B
        {INFINITO, INFINITO, 0, 5, INFINITO, 4, INFINITO},        // C
        {INFINITO, 1, 5, 0, INFINITO, INFINITO, INFINITO},        // D
        {INFINITO, 2, INFINITO, INFINITO, 0, INFINITO, INFINITO}, // E
        {INFINITO, INFINITO, 4, INFINITO, INFINITO, 0, 6},        // F
        {INFINITO, INFINITO, INFINITO, INFINITO, INFINITO, 6, 0}  // G
    };

    // Inicialização do vetor de distâncias e pesos dos vizinhos
    for (int i = 0; i < total_nos; i++) {
        distancias[i] = INFINITO;
        pesos_vizinhos[i] = INFINITO;
    }
    distancias[processId] = 0;  // Distância para si mesmo é zero

    // Transmite a matriz de adjacência para todos os processos
    MPI_Bcast(grafo, total_nos * total_nos, MPI_INT, 0, MPI_COMM_WORLD);

    // Cada processo armazena os pesos das arestas com seus vizinhos
    for (int i = 0; i < total_nos; i++) {
        pesos_vizinhos[i] = grafo[processId][i];
    }

    int convergiu = 0;
    while (!convergiu) {
        convergiu = 1;  // Assumimos inicialmente que convergimos

        for (int i = 0; i < total_nos; i++) {
            if (pesos_vizinhos[i] != INFINITO && i != processId) {
                int distancias_vizinho[total_nos];
                MPI_Send(distancias, total_nos, MPI_INT, i, 0, MPI_COMM_WORLD);
                MPI_Recv(distancias_vizinho, total_nos, MPI_INT, i, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);

                int distancias_antigas[total_nos];
                for (int j = 0; j < total_nos; j++) {
                    distancias_antigas[j] = distancias[j];
                }

                atualizar_distancias(distancias, distancias_vizinho, total_nos, pesos_vizinhos[i]);

                // Verifica se algo mudou
                for (int k = 0; k < total_nos; k++) {
                    if (distancias[k] != distancias_antigas[k]) {
                        convergiu = 0;
                    }
                }
            }
        }

        // Verificar se todos os processos convergiram
        int convergencia_global;
        MPI_Allreduce(&convergiu, &convergencia_global, 1, MPI_INT, MPI_LAND, MPI_COMM_WORLD);
        convergiu = convergencia_global;
    }

    // Exibe o vetor final de distâncias
    printf("Processo %d: ", processId);
    for (int i = 0; i < total_nos; i++) {
        if (distancias[i] == INFINITO) {
            printf("INF ");
        } else {
            printf("%d ", distancias[i]);
        }
    }
    printf("\n");

    MPI_Finalize();
    return 0;
}

// Atualizar o vetor de distâncias
void atualizar_distancias(int* distancias, int* distancias_vizinho, int total_nos, int peso_aresta) {
    for (int i = 0; i < total_nos; i++) {
        if (distancias_vizinho[i] != INFINITO) {  // Verifica se o vizinho tem uma distância válida
            int nova_distancia = peso_aresta + distancias_vizinho[i];
            if (nova_distancia < distancias[i]) {
                distancias[i] = nova_distancia;  // Atualiza apenas se a nova distância for menor
            }
        }
    }
}
