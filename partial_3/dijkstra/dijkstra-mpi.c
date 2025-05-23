#include <stdio.h>
#include <stdlib.h>
#include <mpi.h>
#include <time.h>
#include <unistd.h>

#define N_VERTICES 7
#define INF 99999
#define NAME_LEN 1

typedef struct {
    int id;
    char nombre[20];
} Vertice;

typedef struct {
    Vertice vertices[N_VERTICES];
    int matriz_adyacencia[N_VERTICES][N_VERTICES];
} Grafo;

void nombra_vertices(Grafo *g, int num_vertices) {
    for (int i = 0; i < num_vertices; i++) {
        g->vertices[i].id = i;
        snprintf(g->vertices[i].nombre, 20, "%d", i);
    }
}

void introduce_matriz(Grafo *g, int num_vertices, int matriz[N_VERTICES][N_VERTICES]) {
    for (int i = 0; i < num_vertices; i++) {
        for (int j = 0; j < num_vertices; j++) {
            g->matriz_adyacencia[i][j] = matriz[i][j];
        }
    }
}

void print_grafo(Grafo *g, int num_vertices) {
    printf("Grafo: \n");
    for (int i = 0; i < num_vertices; i++) {
        printf("V %d: %s\n", g->vertices[i].id, g->vertices[i].nombre);
    }

    printf("\nMatriz de adyacencia:\n");
    for (int i = 0; i < num_vertices; i++) {
        for (int j = 0; j < num_vertices; j++) {
            printf("%5d ", g->matriz_adyacencia[i][j]);
        }
        printf("\n");
    }
}

void create_array(int **a, int size) {
    *a = (int *)malloc(size * sizeof(int));
    if (*a == NULL) {
        fprintf(stderr, "Error allocating memory\n");
        MPI_Abort(MPI_COMM_WORLD, EXIT_FAILURE);
    }
}

void create_matrix(int ***a, int rows, int cols) {
    *a = (int **)malloc(rows * sizeof(int *));
    if (*a == NULL) {
        fprintf(stderr, "Error allocating memory\n");
        MPI_Abort(MPI_COMM_WORLD, EXIT_FAILURE);
    }
    for (int i = 0; i < rows; i++) {
        (*a)[i] = (int *)malloc(cols * sizeof(int));
        if ((*a)[i] == NULL) {
            fprintf(stderr, "Error allocating memory\n");
            MPI_Abort(MPI_COMM_WORLD, EXIT_FAILURE);
        }
    }
}

void free_matrix(int ***a, int rows) {
    if (*a != NULL) {
        for (int i = 0; i < rows; i++) {
            free((*a)[i]);
        }
        free(*a);
        *a = NULL;
    }
}

void init_dijkstra(int *distancia, int *visitado, int *anterior, int padded_n, int origen) {
    for (int i = 0; i < padded_n; i++) {
        distancia[i] = INF;
        visitado[i] = 0;
        anterior[i] = -1;
    }
    distancia[origen] = 0;
}

void imprimir_camino(int *anterior, int destino) {
    if (anterior[destino] != -1) {
        imprimir_camino(anterior, anterior[destino]);
        printf(" -> ");
    }
    printf("%d", destino);
}

int find_local_min(int *distancia, int *visitado, int start, int end) {
    int min_dist = INF;
    int u = -1;
    for (int i = start; i < end; i++) {
        if (!visitado[i] && distancia[i] < min_dist) {
            min_dist = distancia[i];
            u = i;
        }
    }
    return u;
}

void dijkstra(int **local_matrix, int *distancia, int *visitado, int *anterior,
              int padded_n, int numProc, int idProc, int origen) {
    int local_size = padded_n / numProc;
    int start = idProc * local_size;
    int end = start + local_size;

    for (int i = 0; i < padded_n - 1; i++) {
        int local_u = find_local_min(distancia, visitado, start, end);
        int local_min_dist = (local_u == -1) ? INF : distancia[local_u];

        int *all_min_dists = NULL;
        int *all_min_indices = NULL;

        if (idProc == 0) {
            create_array(&all_min_dists, numProc);
            create_array(&all_min_indices, numProc);
        }

        MPI_Gather(&local_min_dist,1, MPI_INT, all_min_dists, 1, MPI_INT, 0, MPI_COMM_WORLD);
        MPI_Gather(&local_u, 1, MPI_INT, all_min_indices, 1, MPI_INT, 0, MPI_COMM_WORLD);

        int global_u = -1;
        int global_min_dist = INF;

        if (idProc == 0) {
            for (int p = 0; p < numProc; p++) {
                if (all_min_dists[p] < global_min_dist) {
                    global_min_dist = all_min_dists[p];
                    global_u = all_min_indices[p];
                }
            }
            free(all_min_dists);
            free(all_min_indices);
        }

        MPI_Bcast(&global_u, 1, MPI_INT, 0, MPI_COMM_WORLD);
        MPI_Bcast(&global_min_dist, 1, MPI_INT, 0, MPI_COMM_WORLD);

        if (global_min_dist == INF) break;

        visitado[global_u] = 1;

        for (int v = start; v < end; v++) {
            if (!visitado[v] && local_matrix[global_u][v - start] > 0) {
                int nueva_dist = distancia[global_u] + local_matrix[global_u][v - start];
                if (nueva_dist < distancia[v]) {
                    distancia[v] = nueva_dist;
                    anterior[v] = global_u;
                }
            }
        }

        // distribute the updated distances and predecessors
        MPI_Gather(distancia + start, local_size, MPI_INT,
                  idProc == 0 ? distancia : NULL, local_size, MPI_INT,
                  0, MPI_COMM_WORLD);

        MPI_Gather(anterior + start, local_size, MPI_INT,
                  idProc == 0 ? anterior : NULL, local_size, MPI_INT,
                  0, MPI_COMM_WORLD);

        // broadcast the updated distances and predecessors
        MPI_Bcast(distancia, padded_n, MPI_INT, 0, MPI_COMM_WORLD);
        MPI_Bcast(anterior, padded_n, MPI_INT, 0, MPI_COMM_WORLD);
    }
}

void distribute_matrix(Grafo *g, int **local_matrix, int padded_n, int numProc, int idProc) {
    int local_size = padded_n / numProc;
    int *padded_matrix = NULL;

    if (idProc == 0) {
        padded_matrix = (int *)malloc(padded_n * padded_n * sizeof(int));
        if (padded_matrix == NULL) {
            fprintf(stderr, "Error allocating memory\n");
            MPI_Abort(MPI_COMM_WORLD, EXIT_FAILURE);
        }

        for (int i = 0; i < padded_n; i++) {
            for (int j = 0; j < padded_n; j++) {
                if (i < N_VERTICES && j < N_VERTICES) {
                    padded_matrix[i * padded_n + j] = g->matriz_adyacencia[i][j];
                } else {
                    padded_matrix[i * padded_n + j] = INF;
                }
            }
        }
    }

    // Scatter rows to processes
    for (int i = 0; i < padded_n; i++) {
        MPI_Scatter(idProc == 0 ? padded_matrix + i * padded_n : NULL,
                   local_size, MPI_INT,
                   local_matrix[i], local_size, MPI_INT,
                   0, MPI_COMM_WORLD);
    }

    if (idProc == 0) {
        free(padded_matrix);
    }
}

int main(int argc, char **argv) {
    MPI_Init(&argc, &argv);

    int idProc, numProc;
    MPI_Comm_rank(MPI_COMM_WORLD, &idProc);
    MPI_Comm_size(MPI_COMM_WORLD, &numProc);

    Grafo g;
    int origen = 0;
    int destino = 6;

    int remainder = N_VERTICES % numProc;
    int padding = (remainder == 0) ? 0 : numProc - remainder;
    int padded_n = N_VERTICES + padding;

    int *distancia = NULL;
    int *visitado = NULL;
    int *anterior = NULL;
    create_array(&distancia, padded_n);
    create_array(&visitado, padded_n);
    create_array(&anterior, padded_n);

    int **local_matrix = NULL;
    create_matrix(&local_matrix, padded_n, padded_n / numProc);

    clock_t start, end;
    double cpu_time_used;
    if (idProc == 0) {
        int m[N_VERTICES][N_VERTICES] = {
            {0, 2, 6, 0, 0, 0, 0},
            {2, 0, 0, 5, 0, 0, 0},
            {6, 0, 0, 8, 0, 0, 0},
            {0, 5, 8, 0, 10, 15, 0},
            {0, 0, 0, 10, 0, 6, 2},
            {0, 0, 0, 15, 6, 0, 6},
            {0, 0, 0, 0, 2, 6, 0}
        };

        nombra_vertices(&g, N_VERTICES);
        introduce_matriz(&g, N_VERTICES, m);
        print_grafo(&g, N_VERTICES);

        start = clock();
    }

    distribute_matrix(&g, local_matrix, padded_n, numProc, idProc);
    init_dijkstra(distancia, visitado, anterior, padded_n, origen);
    dijkstra(local_matrix, distancia, visitado, anterior, padded_n, numProc, idProc, origen);

    if (idProc == 0) {
        printf("Camino más corto desde el origen %d al nodo %d:\n", origen, destino);
        imprimir_camino(anterior, destino);
        printf("\n Distancia total: %d\n", distancia[destino]);
    }

    if (idProc == 0) {
        end = clock();
        cpu_time_used = ((double)(end - start)) / CLOCKS_PER_SEC;

        int file_exists = access("resultados_mpi.csv", F_OK) == 0;

        FILE *csv_file = fopen("resultados_mpi.csv", file_exists ? "a" : "w");
        if (csv_file == NULL) {
            perror("Error al abrir el archivo CSV");
            return 1;
        }
        if (!file_exists) {
            fprintf(csv_file, "Nodos,Origen,Destino,Distancia,Tiempo(s),Camino\n");
        }

        fprintf(csv_file, "%d,%d,%d,%d,%.6f,\"", N_VERTICES, origen, destino, distancia[destino], cpu_time_used);

        int camino[N_VERTICES];
        int count = 0;
        for (int i = destino; i != -1; i = anterior[i]) {
            camino[count++] = i;
        }
        for (int i = count - 1; i >= 0; i--) {
            fprintf(csv_file, "%d", camino[i]);
            if (i > 0) fprintf(csv_file, " -> ");
        }

        fprintf(csv_file, "\"\n");
        fclose(csv_file);

        printf("\nResultados exportados a 'resultados_mpi.csv'\n");
    }

    free(distancia);
    free(visitado);
    free(anterior);
    free_matrix(&local_matrix, padded_n);

    MPI_Finalize();

    return 0;
}
