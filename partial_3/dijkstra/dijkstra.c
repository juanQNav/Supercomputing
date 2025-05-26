/*
-Materia: Supercomputo
-Semestre: 8 (2024-2025/II)
-Nombre del alumno 1: Luis Angel Sanchez Muñiz
-Clave del alumno 1: 337911
-Nombre del alumno 2: Quistian Navarro Juan Luis
-Clave del alumno 2: 341807
-Carrera: Ing. en Sistemas Inteligentes
-Nombre de tarea o programa: Algoritmo Dijkstra Secuencial
-Periodo de evaluación (parcial 1, 2, 3, 4 o 5): 3
-Descripción: Implementación del algoritmo de Dijkstra para encontrar el camino más corto
-Avance logrado (0 a 100%): 100%
-Comentarios adicionales: Encuentra caminos más cortos desde cada vértice a todos los vértices
*/
#include <stdio.h>
#include <stdlib.h>
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
}Vertice;

typedef struct{
  Vertice vertices[N_VERTICES];
  int matriz_adyacencia[N_VERTICES][N_VERTICES];
}Grafo;


void label_vertices(Grafo *g, int num_vertices) {
    for(int i = 0; i < num_vertices; i++) {
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

void set_adj_matrix(Grafo *g, int num_vertices, int matriz[N_VERTICES][N_VERTICES]){
  for(int i = 0; i < num_vertices; i++){
    for(int j = 0; j < num_vertices; j++){
      g->matriz_adyacencia[i][j] = matriz[i][j];
    }
  }
}

void print_grafo(Grafo *g, int num_vertices){
  printf("Grafo: \n");
  for(int i = 0; i < num_vertices; i++){
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

void init_dij(int *distancia, int *visitado, int *anterior, int origen, int num_vertices){
   for (int i = 0; i < num_vertices; i++)
  {
    distancia[i] = INF;
    visitado[i] = 0;
    anterior[i] = -1;
  }
  distancia[origen] = 0;
}

void reconstruct_path(int destino,int *anterior, int *path, int *path_length) {
    int temp_path[N_VERTICES];
    int length = 0;
    int current = destino;

    while (current != -1) {
        temp_path[length++] = current;
        current = anterior[current];
    }

    *path_length = length;
    for (int i = 0; i < length; i++) {
        path[i] = temp_path[length - 1 - i];
    }
}

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

void dijkstra(Grafo* g, int origen, int num_vertices, int *distancia, int *anterior) {
    int visitado[N_VERTICES];
    init_dij(distancia, visitado, anterior, origen, num_vertices);

  for(int i = 0; i < num_vertices -1; i++){
    int u = -1;
    int min_dist = INF;

    for(int j = 0; j < num_vertices; j++){
      if(!visitado[j] && distancia[j] < min_dist){
        min_dist = distancia[j];
        u = j;
      }
    }

    if(u == -1) break;

    visitado[u] = 1;

    for(int v = 0; v < num_vertices; v++){
      if(g->matriz_adyacencia[u][v] > 0 && !visitado[v]){
        int nueva_dist = distancia[u] + g->matriz_adyacencia[u][v];
        if(nueva_dist < distancia[v]){
          distancia[v] = nueva_dist;
          anterior[v] = u;
        }
      }
    }
  }
 }

void exportar_grafo_dot(Grafo* g, int num_vertices, const char* filename) {
  FILE* file = fopen(filename, "w");
  if (!file) {
    perror("Error al abrir el archivo .dot");
    return;
  }

  fprintf(file, "graph G {\n");

  for (int i = 0; i < num_vertices; i++) {
    fprintf(file, "  %d [label=\"%s\"];\n", i, g->vertices[i].nombre);
  }

  for (int i = 0; i < num_vertices; i++) {
    for (int j = i + 1; j < num_vertices; j++) {
      if (g->matriz_adyacencia[i][j] > 0) {
        fprintf(file, "  %d -- %d [label=\"%d\"];\n", i, j, g->matriz_adyacencia[i][j]);
      }
    }
  }

  fprintf(file, "}\n");
  fclose(file);
}

int main(int argc, char **argv) {
    int num_vertices = N_VERTICES;
    if (argc > 1) {
        num_vertices = atoi(argv[1]);
        if (num_vertices <= 0 || num_vertices > N_VERTICES) {
            fprintf(stderr, "Número de vértices debe ser entre 1 y %d\n", N_VERTICES);
            return 1;
        }
    }

    Grafo g;
    clock_t start, end;
    double cpu_time_used;

    // int m[N_VERTICES][N_VERTICES] = {
    //     {0, 2, 6, 0, 0, 0, 0},
    //     {2, 0, 0, 5, 0, 0, 0},
    //     {6, 0, 0, 8, 0, 0, 0},
    //     {0, 5, 8, 0, 10, 15, 0},
    //     {0, 0, 0, 10, 0, 6, 2},
    //     {0, 0, 0, 15, 6, 0, 6},
    //     {0, 0, 0, 0, 2, 6, 0}
    // };

    start = clock();

    label_vertices(&g, num_vertices);
    fill_matrix(&g, num_vertices);
    // set_adj_matrix(&g, num_vertices, m);
    // print_grafo(&g, num_vertices);

    printf("\nCalculando caminos más cortos desde cada vértice...\n");

    int **all_distances = (int **)malloc(num_vertices * sizeof(int *));
    int **all_predecessors = (int **)malloc(num_vertices * sizeof(int *));

    if (!all_distances || !all_predecessors) {
        perror("Error asignando memoria");
        return 1;
    }

    for (int i = 0; i < num_vertices; i++) {
        all_distances[i] = (int *)malloc(num_vertices * sizeof(int));
        all_predecessors[i] = (int *)malloc(num_vertices * sizeof(int));
        if (!all_distances[i] || !all_predecessors[i]) {
            perror("Error asignando memoria para filas");
            return 1;
        }
    }


    for (int origen = 0; origen < num_vertices; origen++) {
        int distancia[N_VERTICES];
        int anterior[N_VERTICES];
        dijkstra(&g, origen, num_vertices, distancia, anterior);

        for (int i = 0; i < num_vertices; i++) {
            all_distances[origen][i] = distancia[i];
            all_predecessors[origen][i] = anterior[i];
        }
    }

    end = clock();
    cpu_time_used = ((double)(end - start)) / CLOCKS_PER_SEC;

    FILE *paths_file = fopen("shortest_paths_seq.txt", "w");
    if (paths_file == NULL) {
        perror("Error opening paths file");
        return 1;
    }

    fprintf(paths_file, "CAMINOS MAS CORTOS ENTRE TODOS LOS VERTICES (Secuencial)\n");
    fprintf(paths_file, "=======================================================\n");
    fprintf(paths_file, "Número de vértices: %d\n\n", num_vertices);

    for (int src = 0; src < num_vertices; src++) {
        fprintf(paths_file, "Desde el vértice %d:\n", src);
        fprintf(paths_file, "-------------------\n");

        for (int dest = 0; dest < num_vertices; dest++) {
            if (src == dest) {
                fprintf(paths_file, "  Hacia vértice %d: Distancia = 0, Camino = %d\n", dest, dest);
            } else if (all_distances[src][dest] == INF) {
                fprintf(paths_file, "  Hacia vértice %d: No hay camino\n", dest);
            } else {
                int path[N_VERTICES];
                int path_length;
                reconstruct_path(dest, all_predecessors[src], path, &path_length);

                char path_str[1000];
                path_to_string(path, path_length, path_str);

                fprintf(paths_file, "  Hacia vértice %d: Distancia = %d, Camino = %s\n",
                        dest, all_distances[src][dest], path_str);
            }
        }
        fprintf(paths_file, "\n");
    }

    fclose(paths_file);

    FILE *distances_csv = fopen("all_distances_seq.csv", "w");
    if (distances_csv != NULL) {
        fprintf(distances_csv, "Source\\Destination");
        for (int j = 0; j < num_vertices; j++) {
            fprintf(distances_csv, ",V%d", j);
        }
        fprintf(distances_csv, "\n");

        for (int i = 0; i < num_vertices; i++) {
            fprintf(distances_csv, "V%d", i);
            for (int j = 0; j < num_vertices; j++) {
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

    int file_exists = access("sequential_results.csv", F_OK) == 0;
    FILE *csv_file = fopen("sequential_results.csv", file_exists ? "a" : "w");
    if (csv_file == NULL) {
        perror("Error opening CSV file");
        return 1;
    }

    if (!file_exists) {
        fprintf(csv_file, "Nodes,Time(s),Algorithm\n");
    }

    fprintf(csv_file, "%d,%.6f,Dijkstra_All_Pairs_Sequential\n", num_vertices, cpu_time_used);
    fclose(csv_file);

    printf("\nProcesamiento completado!\n");
    printf("Tiempo total: %.6f segundos\n", cpu_time_used);
    printf("Archivos generados:\n");
    printf("  - shortest_paths_seq.txt: Todos los caminos más cortos\n");
    printf("  - all_distances_seq.csv: Matriz de distancias\n");
    printf("  - sequential_results.csv: Métricas de rendimiento\n");

    if (num_vertices <= 20) {
        exportar_grafo_dot(&g, num_vertices, "grafo_seq.dot");
    }

    return 0;
}