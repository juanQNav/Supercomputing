/*
-Materia: Supercomputo
-Semestre: 8 (2024-2025/II)
-Nombre del alumno: Quistian Navarro Juan Luis
-Clave del alumno: 341807
-Carrera: Ing. en Sistemas Inteligentes
-Nombre de tarea o programa: Suma de dos arreglos de enteros
-Periodo de evaluación (parcial 1, 2, 3, 4 o 5): 2
-Descripción: Suma de dos arreglos de enteros usando MPI usando mpi bcast
-Avance logrado (0 a 100%): 100%
-Comentarios adicionales: NA
*/

#include <stdio.h>
#include <stdlib.h>
#include <mpi.h>
#include <time.h>

// reserve memmory using malloc
void createArray(float **a, int N)
{
    *a = (float *)malloc(N * sizeof(float));

    if (*a == NULL)
    {
        fprintf(stderr, "Error allocating memory\n");
        exit(EXIT_FAILURE);
    }
}

// fill array with random numbers
void fillArray(float *a, int N)
{
    srand(time(NULL));
    for (int i = 0; i < N; i++)
    {
        a[i] = (float)(rand() % 100);
    }
}

// print array
void printArray(float *a, int N)
{
    for (int i = 0; i < N; i++)
    {
        printf("%2.f ", a[i]);
    }
    printf("\n");
}

float sumArray(float *a, int N)
{
    float sum = 0;
    for (int i = 0; i < N; i++)
    {
        sum += a[i];
    }

    return sum;
}

int main(int argc, char *argv[])
{
    int idProc, numProc;

    MPI_Init(&argc, &argv);                  // Initialize MPI, send argc and argv from the main function
    MPI_Comm_rank(MPI_COMM_WORLD, &idProc);  // Get the rank of the process
    MPI_Comm_size(MPI_COMM_WORLD, &numProc); // Get the id of the process (numproc)

    int N = 10;

    float *a = NULL;

    createArray(&a, N);
    if (idProc == 0)
    {
        fillArray(a, N);
    }

    MPI_Bcast(a, N, MPI_FLOAT, 0, MPI_COMM_WORLD);

    if (idProc != 0)
    {
        printArray(a, N);
        printf("Suma es %2.f \n", sumArray(a, N));
    }

    free(a);
    MPI_Finalize();
    return 0;
}