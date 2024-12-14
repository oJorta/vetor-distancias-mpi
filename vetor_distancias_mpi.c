#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>
#include <limits.h>

#define INF INT_MAX  // infinito para as arestas inexistentes

void update_distances(int* distances, int* neighbor_distances, int num_nodes, int edge_weight);

int main(int argc, char** argv) {
    MPI_Init(&argc, &argv);

    int world_rank, world_size;
    MPI_Comm_rank(MPI_COMM_WORLD, &world_rank);
    MPI_Comm_size(MPI_COMM_WORLD, &world_size);

    int num_nodes = 7;
    int distances[num_nodes];
    int neighbors[num_nodes];

    // matriz de adjacência com pesos das arestas
    int graph[7][7] = {
        {0, 3, INF, INF, INF, INF, INF}, // A
        {3, 0, INF, 1, 2, INF, INF},      // B
        {INF, INF, 0, 5, INF, 4, INF}, // C
        {INF, 1, 5, 0, INF, INF, INF},  // D
        {INF, 2, INF, INF, 0, INF, INF}, // E
        {INF, INF, 4, INF, INF, 0, 6}, // F
        {INF, INF, INF, INF, INF, 6, 0}   // G
    };

    // inicialização do vetor de distâncias e vizinhos
    for (int i = 0; i < num_nodes; i++) {
        distances[i] = INF;
        neighbors[i] = INF;
    }
    distances[world_rank] = 0;  // distância para si mesmo é zero

    // transmite a matriz de adjacência para todos os nós
    MPI_Bcast(graph, num_nodes * num_nodes, MPI_INT, 0, MPI_COMM_WORLD);

    // cada nó armazena os pesos das arestas com seus vizinhos
    for (int i = 0; i < num_nodes; i++) {
        neighbors[i] = graph[world_rank][i];
    }

    int converged = 0;
    while (!converged) {
        converged = 1;  // assumimos inicialmente que convergimos

        for (int i = 0; i < num_nodes; i++) {
            if (neighbors[i] != INF && i != world_rank) {
                int neighbor_distances[num_nodes];
                MPI_Send(distances, num_nodes, MPI_INT, i, 0, MPI_COMM_WORLD);
                MPI_Recv(neighbor_distances, num_nodes, MPI_INT, i, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);

                int old_distances[num_nodes];
                for (int j = 0; j < num_nodes; j++) {
                    old_distances[j] = distances[j];
                }

                update_distances(distances, neighbor_distances, num_nodes, neighbors[i]);

                // verifica se algo mudou
                for (int k = 0; k < num_nodes; k++) {
                    if (distances[k] != old_distances[k]) {
                        converged = 0;
                    }
                }
            }
        }

        // verificar se todos os nós convergiram
        int global_converged;
        MPI_Allreduce(&converged, &global_converged, 1, MPI_INT, MPI_LAND, MPI_COMM_WORLD);
        converged = global_converged;
    }

    // Exibe o vetor final de distâncias
    printf("N\u00f3 %d: ", world_rank);
    for (int i = 0; i < num_nodes; i++) {
        if (distances[i] == INF) {
            printf("INF ");
        } else {
            printf("%d ", distances[i]);
        }
    }
    printf("\n");

    MPI_Finalize();
    return 0;
}

// atualizar o vetor de distâncias
void update_distances(int* distances, int* neighbor_distances, int num_nodes, int edge_weight) {
    for (int i = 0; i < num_nodes; i++) {
        if (neighbor_distances[i] != INF) {  // verifica se o vizinho tem uma distância válida
            int new_distance = edge_weight + neighbor_distances[i];
            if (new_distance < distances[i]) {
                distances[i] = new_distance;  // atualiza apenas se a nova distância for menor
            }
        }
    }
}