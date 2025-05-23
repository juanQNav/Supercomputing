#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>

#define N_VERTICES 300
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

void nombra_vertices(Grafo *g, int num_vertices){
  for(int i = 0; i < num_vertices; i++){
    g->vertices[i].id = i;

    for (int j = 0; j < NAME_LEN; j++){
      g->vertices[i].nombre[j] = '0' + i;
    }
    g->vertices[i].nombre[NAME_LEN] = '\0';
  }
}

void llena_matriz_a(Grafo *g, int num_vertices){
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

void introduce_matriz(Grafo *g, int num_vertices, int matriz[N_VERTICES][N_VERTICES]){
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

int distancia[N_VERTICES];
int visitado[N_VERTICES];
int anterior[N_VERTICES];

void init_dij(int origen, int *d, int *v, int num_vertices){
   for (int i = 0; i < num_vertices; i++)
  {
    distancia[i] = INF;
    visitado[i] = 0;
    anterior[i] = -1;
  }
  distancia[origen] = 0;
}

void imprimir_camino(int destino) {
  if (anterior[destino] == -1) {
    printf("%d", destino);
    return;
  }

  imprimir_camino(anterior[destino]);
  printf(" -> %d", destino);
}


void dijkstra(Grafo* g, int origen, int num_vertices){
  init_dij(origen, distancia, visitado, num_vertices);

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


int main(){
  Grafo g;

  // int m[N_VERTICES][N_VERTICES] = {
  //   {0, 2, 6, 0, 0, 0, 0}, // 0
  //   {2, 0, 0, 5, 0, 0, 0}, // 1
  //   {6, 0, 0, 8, 0, 0, 0}, // 2
  //   {0, 5, 8, 0, 10, 15, 0}, // 3
  //   {0, 0, 0, 10, 0, 6, 2}, // 4
  //   {0, 0, 0, 15, 6, 0, 6}, // 5
  //   {0, 0, 0, 0, 2, 6, 0}, // 6
  // };

  clock_t start, end;
  double cpu_time_used;

  start = clock();

  nombra_vertices(&g, N_VERTICES);
  llena_matriz_a(&g, N_VERTICES);
  // introduce_matriz(&g, N_VERTICES, m);
  print_grafo(&g, N_VERTICES);

  int origen = 0;
  int destino = 2;
  dijkstra(&g, origen, N_VERTICES);

  printf("Camino más corto desde el origen %d al nodo %d:\n", origen, destino);
  imprimir_camino(destino);
  printf("\nDistancia total: %d\n", distancia[destino]);

  end = clock();
  cpu_time_used = ((double) (end - start)) / CLOCKS_PER_SEC;

  int file_exists = access("resultados.csv", F_OK) == 0;

  FILE *csv_file = fopen("resultados.csv", file_exists ? "a" : "w");
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

  printf("\nResultados exportados a 'resultados.csv'\n");

  if (N_VERTICES < 10) {
    exportar_grafo_dot(&g, N_VERTICES, "grafo.dot");
    printf("\nGrafo exportado a 'grafo.dot'. Usa Graphviz para visualizarlo.\n");
  } else {
    printf("El grafo es grande, no se recomienda usar Graphviz para visualizarlo.\n");
  }

  return 0;
}
