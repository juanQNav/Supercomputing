/*
-Materia Supercomputo:
-Semestre: 8 (2024-2025/II)
-Nombre del alumno: Quistian Navarro Juan Luis
-Clave del alumno: 341807
-Carrera: Ing. en Sistemas Inteligentes
-Nombre de tarea o programa: MPI: Suma de un arreglo
-Periodo de evaluación (parcial 1, 2, 3, 4 o 5): 2
-Descripción: Suma de un un arreglo de enteros usando MPI
-Avance logrado (0 a 100%): 100%
-Comentarios adicionales: NA
*/

#include <stdio.h>
#include <stdlib.h>
#include <mpi.h>

// reserve memmory using malloc
void createArray(int **array, int size)
{
    *array = (int *)malloc(size * sizeof(int));
    if (*array == NULL)
    {
        fprintf(stderr, "Error allocating memory\n");
        exit(EXIT_FAILURE);
    }
}

// fill array with random numbers
void fillArray(int *array, int size)
{
    for (int i = 0; i < size; i++)
    {
        array[i] = rand() % 100;
    }
}

// print array
void printArray(int *array, int size)
{
    for (int i = 0; i < size; i++)
    {
        printf("%d ", array[i]);
    }
    printf("\n");
}

int main(int argc, char *argv[])
{
    int idProc, numProc;
    MPI_Init(&argc, &argv);                  // Initialize MPI, sends argc and argv from the main function
    MPI_Comm_rank(MPI_COMM_WORLD, &idProc);  // Get the rank of the process
    MPI_Comm_size(MPI_COMM_WORLD, &numProc); // Get the id of the process (numproc)

    int N = 3000;
    int suma = 0;
    int nDatos = N / (numProc - 1);
    int nDatosU = nDatos + N % (numProc - 1);

    if (idProc == 0)
    {
        int *array;
        createArray(&array, N);
        fillArray(array, N);

        for (int i = 1; i <= numProc - 2; i++)
        {
            MPI_Send(array + (i - 1) * nDatos, nDatos, MPI_INT, i, 0, MPI_COMM_WORLD); // send initial address of the array, number of elements to send, id of the process to send to, tag, communicator
        }
        MPI_Send(array + (numProc - 2) * nDatos, nDatosU, MPI_INT, numProc - 1, 0, MPI_COMM_WORLD); // send initial address of the array, number of elements to send, id of the process to send to, tag, communicator

        for (int i = 1; i <= numProc - 1; i++)
        {
            int sumap = 0;
            MPI_Recv(&sumap, 1, MPI_INT, i, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE); // receive the intial address of the array, max number of elements to receive, data type, id of the process of source, tag, communicator
            suma += sumap;
        }

        printArray(array, N);
        printf("La suma es: %d\n", suma);
        free(array);
    }
    else
    {
        if (idProc == numProc - 1)
        {
            nDatos = nDatosU;
        }

        int *array;
        createArray(&array, nDatos);
        MPI_Recv(array, nDatos, MPI_INT, 0, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE); // receive the intial address of the array, max number of elements to receive, data type, id of the process of source, tag, communicator

        int suma_partial = 0;
        for (int i = 0; i < nDatos; i++)
        {
            suma_partial += array[i];
        }

        MPI_Send(&suma_partial, 1, MPI_INT, 0, 0, MPI_COMM_WORLD); // send initial address of the array, number of elements to send, id of the process to send to, tag, communicator
        free(array);
    }
    MPI_Finalize();
    return 0;
}