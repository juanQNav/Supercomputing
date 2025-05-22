/*
-Materia: Supercomputo
-Semestre: 8 (2024-2025/II)
-Nombre del alumno: Quistian Navarro Juan Luis
-Clave del alumno: 341807
-Carrera: Ing. en Sistemas Inteligentes
-Nombre de tarea o programa: Suma de dos arreglos de enteros
-Periodo de evaluación (parcial 1, 2, 3, 4 o 5): 2
-Descripción: Suma de dos arreglos de enteros usando MPI
-Avance logrado (0 a 100%): 100%
-Comentarios adicionales: NA
*/

#include <stdio.h>
#include <stdlib.h>
#include <mpi.h>
#include <time.h>

// reserve memmory using malloc
void createArray(int **a, int N)
{
    *a = (int *)malloc(N * sizeof(int));

    if (*a == NULL)
    {
        fprintf(stderr, "Error allocating memory\n");
        exit(EXIT_FAILURE);
    }
}

// fill array with random numbers
void fillArray(int *a, int N)
{
    srand(time(NULL));
    for (int i = 0; i < N; i++)
    {
        a[i] = rand() % 100;
    }
}

// print array
void printArray(int *a, int N)
{
    for (int i = 0; i < N; i++)
    {
        printf("%d ", a[i]);
    }
    printf("\n");
}

int main(int argc, char *argv[])
{

    int idProc, numProc;

    MPI_Init(&argc, &argv);                  // Initialize MPI, send argc and argv from the main function
    MPI_Comm_rank(MPI_COMM_WORLD, &idProc);  // Get the rank of the process
    MPI_Comm_size(MPI_COMM_WORLD, &numProc); // Get the id of the process (numproc)

    int N = 100;
    int suma = 0;
    int nDatos = N / (numProc - 1);
    int nDatosU = nDatos + N % (numProc - 1);

    int *a = NULL;
    int *b = NULL;
    int *c = NULL;

    if (idProc == 0)
    {
        createArray(&a, N);
        createArray(&b, N);
        createArray(&c, N);

        fillArray(a, N);
        fillArray(b, N);

        for (int i = 1; i <= numProc - 2; i++)
        {
            // send initial address of the array, number of elements to send,type ,id of the process to send to, tag, communicator
            MPI_Send(a + (i - 1) * nDatos, nDatos, MPI_INT, i, 0, MPI_COMM_WORLD);
            MPI_Send(b + (i - 1) * nDatos, nDatos, MPI_INT, i, 0, MPI_COMM_WORLD);
        }
        MPI_Send(a + (numProc - 2) * nDatos, nDatosU, MPI_INT, numProc - 1, 0, MPI_COMM_WORLD);
        MPI_Send(b + (numProc - 2) * nDatos, nDatosU, MPI_INT, numProc - 1, 0, MPI_COMM_WORLD);

        for (int i = 1; i <= numProc - 1; i++)
        {
            if (i == numProc - 1)
            {
                // receive the intial address of the array, max number of elements to receive, data type, id of the process of source, tag, communicator
                MPI_Recv(c + (numProc - 2) * nDatos, nDatosU, MPI_INT, i, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
            }
            else
            {
                // receive the intial address of the array, max number of elements to receive, data type, id of the process of source, tag, communicator
                MPI_Recv(c + (i - 1) * nDatos, nDatos, MPI_INT, i, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
            }
        }

        printArray(a, N);
        printf("\n");
        printArray(b, N);
        printf("result:\n");
        printArray(c, N);
    }
    else
    {
        if (idProc == numProc - 1)
        {
            nDatos = nDatosU;
        }

        createArray(&a, nDatos);
        createArray(&b, nDatos);
        createArray(&c, nDatos);

        // receive the intial address of the array, max number of elements to receive, data type, id of the process of source, tag, communicator
        MPI_Recv(a, nDatos, MPI_INT, 0, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        MPI_Recv(b, nDatos, MPI_INT, 0, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);

        for (int i = 0; i <= nDatos - 1; i++)
        {
            c[i] = a[i] + b[i];
        }

        // send initial address of the array, number of elements to send,type ,id of the process to send to, tag, communicator
        MPI_Send(c, nDatos, MPI_INT, 0, 0, MPI_COMM_WORLD);
    }

    free(a);
    free(b);
    free(c);
    MPI_Finalize();
    return 0;
}