/*
-Materia: Supercomputo
-Semestre: 8 (2024-2025/II)
-Nombre del alumno 1: Luis Angel Sanchez Muñiz
-Clave del alumno 1: 337911
-Nombre del alumno 2: Quistian Navarro Juan Luis
-Clave del alumno 2: 341807
-Carrera: Ing. en Sistemas Inteligentes
-Nombre de tarea o programa: Algoritmo Dijkstra MPI
-Periodo de evaluación (parcial 1, 2, 3, 4 o 5): 3
-Descripción: Implementación del algoritmo de Dijkstra usando MPI para encontrar el camino más corto en un grafo distribuido
-Avance logrado (0 a 100%): 100%
-Comentarios adicionales: Encuentra caminos más cortos desde cada vértice a todos los vértices
*/
#include <stdio.h>
#include <stdlib.h>
#include <mpi.h>
#include <time.h>
#include <unistd.h>
#include <string.h>

#define N_VERTICES 1000
#define INF 99999
#define NAME_LEN 2
#define MAX_WEIGHT 11
#define SEED 42

typedef struct {
    int id;
    char nombre[20];
} Vertice;

typedef struct {
    Vertice vertices[N_VERTICES];
    int matriz_adyacencia[N_VERTICES][N_VERTICES];
} Grafo;

typedef struct {
    int dist;
    int node;
} DistNode;

void label_vertices(Grafo *g, int num_vertices){
    for(int i = 0; i < num_vertices; i++){
        g->vertices[i].id = i;
        snprintf(g->vertices[i].nombre, sizeof(g->vertices[i].nombre), "V%d", i);
    }
}

void fill_matrix(Grafo *g, int num_vertices){
    srand(SEED);
    for(int i = 0; i < num_vertices; i++){
        for(int j = 0; j < num_vertices; j++){
            if(i == j){
                g->matriz_adyacencia[i][j] = 0;
            } else {
                g->matriz_adyacencia[i][j] = rand() % MAX_WEIGHT;
            }
        }
    }
}

void set_adj_matrix(Grafo *g, int num_vertices, int matrix[N_VERTICES][N_VERTICES]) {
    for (int i = 0; i < num_vertices; i++) {
        for (int j = 0; j < num_vertices; j++) {
            g->matriz_adyacencia[i][j] = matrix[i][j];
            // Si no hay conexión directa (valor 0 excepto diagonal), usar INF
            if (i != j && matrix[i][j] == 0) {
                g->matriz_adyacencia[i][j] = INF;
            }
        }
    }
}

void print_grafo(Grafo *g, int num_vertices){
    printf("Graph:\n");
    for(int i = 0; i < num_vertices; i++){
        printf("V %d: %s\n", g->vertices[i].id, g->vertices[i].nombre);
    }

    printf("\nAdjacency matrix:\n");
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

int calculate_padded_vertices(int numProc, int n_vertices) {
    int paded_n, remainder, adjustement;
    if (numProc > n_vertices){
        paded_n = numProc;
    }else{
        remainder = n_vertices % numProc;
        if (remainder != 0){
            adjustement = numProc - remainder;
        }else{
            adjustement = 0;
        }
        paded_n = n_vertices + adjustement;
    }
    return paded_n;
}

void packing_column_block(int mat[][N_VERTICES],int *buffer, int n_vertices, int start_col, int loc_n){
    for(int i = 0; i < n_vertices; i++){
        for(int j = 0; j < loc_n; j++){
            buffer[i * loc_n + j] = mat[i][start_col + j];
        }
    }
}

void unpacking_column_block(int *buffer, int local_mat[][N_VERTICES], int n_vertices, int loc_n){
    for(int i = 0; i < n_vertices; i++){
        for(int j = 0; j < loc_n; j++){
            local_mat[i][j] = buffer[i * loc_n + j];
        }
    }
}

void init_dijkstra(int *loc_dist, int *loc_visit, int *loc_ant, int loc_n, int origen, int numProc, int idProc) {
    for (int i = 0; i < loc_n; i++) {
        loc_dist[i] = INF;
        loc_visit[i] = 0;
        loc_ant[i] = -1;
    }

    int owner_origen = origen / loc_n;
    if (idProc == owner_origen) {
        int local_origen = origen % loc_n;
        loc_dist[local_origen] = 0;
    }
}

// Función para reconstruir el camino desde el origen hasta un destino
void reconstruct_path(int *predecessors, int origin, int dest, int *path, int *path_length) {
    int temp_path[N_VERTICES];
    int length = 0;
    int current = dest;

    // Construir el camino en orden inverso
    while (current != -1) {
        temp_path[length++] = current;
        current = predecessors[current];
    }

    // Invertir el camino para obtenerlo en orden correcto
    *path_length = length;
    for (int i = 0; i < length; i++) {
        path[i] = temp_path[length - 1 - i];
    }
}

// Función para escribir el camino en formato string
void path_to_string(int *path, int length, char *str) {
    str[0] = '\0';
    for (int i = 0; i < length; i++) {
        char vertex[10];
        sprintf(vertex, "%d", path[i]);
        strcat(str, vertex);
        if (i < length - 1) {
            strcat(str, " -> ");
        }
    }
}

void dijkstra(int local_adj[][N_VERTICES], int *loc_dist, int *loc_visit, int *loc_pred, int paded_n, int numProc, int idProc, int origen) {
    int loc_n = paded_n / numProc;

    init_dijkstra(loc_dist, loc_visit, loc_pred, loc_n, origen, numProc, idProc);

    int *global_dist = NULL;
    int *global_visit = NULL;
    create_array(&global_dist, paded_n);
    create_array(&global_visit, paded_n);

    for (int step = 0; step < paded_n; step++) {
        int min_dist = INF;
        int local_u = -1;

        for (int i = 0; i < loc_n; i++) {
            if (loc_dist[i] < min_dist && !loc_visit[i]) {
                min_dist = loc_dist[i];
                local_u = i;
            }
        }

        DistNode local_min = {min_dist, (local_u == -1) ? -1 : idProc * loc_n + local_u};
        DistNode global_min;

        MPI_Allreduce(&local_min, &global_min, 1, MPI_2INT, MPI_MINLOC, MPI_COMM_WORLD);

        if (global_min.dist == INF) break;

        int global_u = global_min.node;
        int owner_u = global_u / loc_n;
        int local_u_idx = global_u % loc_n;

        if (idProc == owner_u) {
            loc_visit[local_u_idx] = 1;
        }

        MPI_Allgather(loc_dist, loc_n, MPI_INT, global_dist, loc_n, MPI_INT, MPI_COMM_WORLD);
        MPI_Allgather(loc_visit, loc_n, MPI_INT, global_visit, loc_n, MPI_INT, MPI_COMM_WORLD);

        for (int v = 0; v < loc_n; v++) {
            int global_v = idProc * loc_n + v;
            if (!loc_visit[v] && local_adj[global_u][v] != INF &&
                global_dist[global_u] != INF) {

                int new_dist = global_dist[global_u] + local_adj[global_u][v];
                if (new_dist < loc_dist[v]) {
                    loc_dist[v] = new_dist;
                    loc_pred[v] = global_u;
                }
            }
        }
    }

    free(global_dist);
    free(global_visit);
}

void distribute_data(int adj[][N_VERTICES], int loc_adj[][N_VERTICES], int paded_n, int numProc, int idProc){
    int loc_n = paded_n / numProc;

    if(idProc == 0){
        for(int proc=0; proc < numProc; proc++){
            if(proc == 0)
            {
                for(int i = 0; i < paded_n; i++){
                    for(int j = 0; j < loc_n; j++){
                        loc_adj[i][j] = adj[i][j];
                    }
                }
            }else{
                int *buffer = malloc(paded_n * loc_n * sizeof(int));
                packing_column_block(adj, buffer, paded_n, proc * loc_n, loc_n);
                MPI_Send(buffer, paded_n * loc_n, MPI_INT, proc, 0, MPI_COMM_WORLD);
                free(buffer);
            }
        }
    }else{
        int loc_n = paded_n / numProc;
        int *buffer = malloc(paded_n * loc_n * sizeof(int));
        MPI_Recv(buffer, paded_n * loc_n, MPI_INT, 0, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        unpacking_column_block(buffer, loc_adj, paded_n, loc_n);
        free(buffer);
    }
}

int main(int argc, char **argv) {
    MPI_Init(&argc, &argv);

    int idProc, numProc;
    MPI_Comm_rank(MPI_COMM_WORLD, &idProc);
    MPI_Comm_size(MPI_COMM_WORLD, &numProc);

    int n_vertices = N_VERTICES;
    if (argc > 1) {
        n_vertices = atoi(argv[1]);
        if (n_vertices <= 0 || n_vertices > N_VERTICES) {
            fprintf(stderr, "Número de vértices debe ser entre 1 y %d\n", N_VERTICES);
            return 1;
        }
    }
    int paded_n = calculate_padded_vertices(numProc, n_vertices);
    int loc_n = paded_n / numProc;
    clock_t start, end;
    double cpu_time_used;

    Grafo g;

    if(idProc == 0) {
        // intialize graph structure wit infinite weights
        for(int i = 0; i < N_VERTICES; i++) {
            for(int j = 0; j < N_VERTICES; j++) {
                g.matriz_adyacencia[i][j] = (i == j) ? 0 : INF;
            }
        }

        // int m[N_VERTICES][N_VERTICES] = {
        //     {0, 2, 6, 0, 0, 0, 0},
        //     {2, 0, 0, 5, 0, 0, 0},
        //     {6, 0, 0, 8, 0, 0, 0},
        //     {0, 5, 8, 0, 10, 15, 0},
        //     {0, 0, 0, 10, 0, 6, 2},
        //     {0, 0, 0, 15, 6, 0, 6},
        //     {0, 0, 0, 0, 2, 6, 0}
        // };

        label_vertices(&g, paded_n);
        fill_matrix(&g, n_vertices);

        // set_adj_matrix(&g, n_vertices, m);

        for(int i = n_vertices; i < paded_n; i++) {
            for(int j = n_vertices; j < paded_n; j++) {
                g.matriz_adyacencia[i][j] = (i == j) ? 0 : INF;
            }
        }

        start = clock();
        printf("Calculando caminos más cortos desde cada vértice...\n");
        printf("Vértices reales: %d, Vértices con padding: %d\n", n_vertices, paded_n);
    }

    MPI_Bcast(&n_vertices, 1, MPI_INT, 0, MPI_COMM_WORLD);
    MPI_Bcast(&paded_n, 1, MPI_INT, 0, MPI_COMM_WORLD);

    int loc_adj[N_VERTICES][N_VERTICES];
    int * local_dist = NULL;
    int * local_visit = NULL;
    int * local_pred = NULL;

    create_array(&local_dist, loc_n);
    create_array(&local_visit, loc_n);
    create_array(&local_pred, loc_n);

    distribute_data(g.matriz_adyacencia, loc_adj, paded_n, numProc, idProc);

    // Debug: verificar la distribución (opcional, comentar en producción)
    if (idProc == 0 && n_vertices <= 10) {
        printf("\nMatriz original:\n");
        for (int i = 0; i < n_vertices; i++) {
            for (int j = 0; j < n_vertices; j++) {
                if (g.matriz_adyacencia[i][j] == INF) {
                    printf("INF ");
                } else {
                    printf("%3d ", g.matriz_adyacencia[i][j]);
                }
            }
            printf("\n");
        }
        printf("\n");
    }

    int (*all_distances)[N_VERTICES] = NULL;
    int (*all_predecessors)[N_VERTICES] = NULL;

    if (idProc == 0) {
        all_distances = malloc(N_VERTICES * N_VERTICES * sizeof(int));
        all_predecessors = malloc(N_VERTICES * N_VERTICES * sizeof(int));
        if (!all_distances || !all_predecessors) {
            fprintf(stderr, "Error allocating memory for result matrices\n");
            MPI_Abort(MPI_COMM_WORLD, EXIT_FAILURE);
        }
    }

    for (int source = 0; source < n_vertices; source++) {
        dijkstra(loc_adj, local_dist, local_visit, local_pred, paded_n, numProc, idProc, source);

        int *final_dist = NULL;
        int *final_pred = NULL;
        if (idProc == 0) {
            final_dist = malloc(paded_n * sizeof(int));
            final_pred = malloc(paded_n * sizeof(int));
        }

        MPI_Gather(local_dist, loc_n, MPI_INT, final_dist, loc_n, MPI_INT, 0, MPI_COMM_WORLD);
        MPI_Gather(local_pred, loc_n, MPI_INT, final_pred, loc_n, MPI_INT, 0, MPI_COMM_WORLD);

        if (idProc == 0) {
            for (int i = 0; i < n_vertices; i++) {
                all_distances[source][i] = final_dist[i];
                all_predecessors[source][i] = final_pred[i];
            }
            free(final_dist);
            free(final_pred);
        }
    }

    if (idProc == 0) {
        end = clock();
        cpu_time_used = ((double)(end - start)) / CLOCKS_PER_SEC;

        FILE *paths_file = fopen("shortest_paths.txt", "w");
        if (paths_file == NULL) {
            perror("Error opening paths file");
            return 1;
        }

        fprintf(paths_file, "CAMINOS MAS CORTOS ENTRE TODOS LOS VERTICES\n");
        fprintf(paths_file, "==========================================\n");
        fprintf(paths_file, "Número de vértices: %d\n", n_vertices);
        fprintf(paths_file, "Número de procesos: %d\n\n", numProc);

        for (int src = 0; src < n_vertices; src++) {
            fprintf(paths_file, "Desde el vértice %d:\n", src);
            fprintf(paths_file, "-------------------\n");

            for (int dest = 0; dest < n_vertices; dest++) {
                if (src == dest) {
                    fprintf(paths_file, "  Hacia vértice %d: Distancia = 0, Camino = %d\n", dest, dest);
                } else if (all_distances[src][dest] == INF) {
                    fprintf(paths_file, "  Hacia vértice %d: No hay camino\n", dest);
                } else {
                    int path[N_VERTICES];
                    int path_length;
                    reconstruct_path(all_predecessors[src], src, dest, path, &path_length);

                    char path_str[1000];
                    path_to_string(path, path_length, path_str);

                    fprintf(paths_file, "  Hacia vértice %d: Distancia = %d, Camino = %s\n",
                            dest, all_distances[src][dest], path_str);
                }
            }
            fprintf(paths_file, "\n");
        }

        fclose(paths_file);

        int file_exists = access("mpi_results.csv", F_OK) == 0;
        FILE *csv_file = fopen("mpi_results.csv", file_exists ? "a" : "w");
        if (csv_file == NULL) {
            perror("Error opening CSV file");
            return 1;
        }

        if (!file_exists) {
            fprintf(csv_file, "Nodes,Processes,Time(s),Algorithm\n");
        }

        fprintf(csv_file, "%d,%d,%.6f,Dijkstra_All_Pairs\n", n_vertices, numProc, cpu_time_used);
        fclose(csv_file);

        FILE *distances_csv = fopen("all_distances.csv", "w");
        if (distances_csv != NULL) {

            fprintf(distances_csv, "Source\\Destination");
            for (int j = 0; j < n_vertices; j++) {
                fprintf(distances_csv, ",V%d", j);
            }
            fprintf(distances_csv, "\n");

            for (int i = 0; i < n_vertices; i++) {
                fprintf(distances_csv, "V%d", i);
                for (int j = 0; j < n_vertices; j++) {
                    if (all_distances[i][j] == INF) {
                        fprintf(distances_csv, ",INF");
                    } else {
                        fprintf(distances_csv, ",%d", all_distances[i][j]);
                    }
                }
                fprintf(distances_csv, "\n");
            }
            fclose(distances_csv);
        }

        printf("\nProcesamiento completado!\n");
        printf("Tiempo total: %.6f segundos\n", cpu_time_used);
        printf("Archivos generados:\n");
        printf("  - shortest_paths.txt: Todos los caminos más cortos\n");
        printf("  - all_distances.csv: Matriz de distancias\n");
        printf("  - mpi_results.csv: Métricas de rendimiento\n");

        // Liberar memoria
        if (all_distances) free(all_distances);
        if (all_predecessors) free(all_predecessors);
    }

    free(local_dist);
    free(local_visit);
    free(local_pred);
    MPI_Finalize();
    return 0;
}