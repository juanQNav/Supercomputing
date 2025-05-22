/*
-Materia: Supercomputo
-Semestre: 8 (2024-2025/II)
-Nombre del alumno: Quistian Navarro Juan Luis
-Clave del alumno: 341807
-Carrera: Ing. en Sistemas Inteligentes
-Nombre de tarea o programa: Suma de arreglo de numeros flotantes 
-Periodo de evaluación (parcial 1, 2, 3, 4 o 5): 2
-Descripción: Suma de dos arreglos de enteros usando MPI  scatterv reduce
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
    // srand(time(NULL));
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


int main(int argc, char *argv[])
{
    int idProc, numProc;

    MPI_Init(&argc, &argv);                  // Initialize MPI, send argc and argv from the main function
    MPI_Comm_rank(MPI_COMM_WORLD, &idProc);  // Get the rank of the process
    MPI_Comm_size(MPI_COMM_WORLD, &numProc); // Get the id of the process (numproc)

    int *sendcounts = NULL, *dislsps = NULL; // number of elements to send to each process
    int *displs = NULL;                      // displacement for each process

    int N = 10;

    if (idProc == 0)
    {
        sendcounts = (int *)malloc(numProc * sizeof(int)); // allocate memory for sendcounts
        displs = (int *)malloc(numProc * sizeof(int));     // allocate memory for displs

        int base = N / numProc;  // base number of elements to send to each process
        int extra = N % numProc; // extra number of elements to send to the last process
        int offset = 0;

        // calculate sendcounts and displs for each process
        for (int i = 0; i < numProc; i++)
        {
            sendcounts[i] = base + (i < extra ? 1 : 0); // first extra processes get one more element
            displs[i] = offset;                         // displacement from the start of the array
            offset += sendcounts[i];                    // update offset for next process
        }
    }

    float *a = NULL;

    if (idProc == 0)
    {
        createArray(&a, N);
        fillArray(a, N);
    }

    int localCount;

    MPI_Scatter(
        sendcounts,    // send initial address of the array
        1,             // number of elements to send to each process
        MPI_INT,       // data type
        &localCount,   // receive initial address of the array
        1, MPI_INT,    // data type
        0,             // id of the process to send to
        MPI_COMM_WORLD // communicator
    );

    float *localA = NULL;

    createArray(&localA, localCount);

    MPI_Scatterv(
        a,             // send initial address of the array
        sendcounts,    // number of elements to send to each process
        displs,        // displacement for each process
        MPI_FLOAT,     // data type
        localA,        // receive initial address of the array
        localCount,    // number of elements to receive
        MPI_FLOAT,     // data type
        0,             // id of the process to send to
        MPI_COMM_WORLD // communicator
    );


    float localSum = 0;
    float localProduct = 1;
    for (int i = 0; i < localCount; i++)
    {
        localSum += localA[i];
        localProduct *= localA[i];
    }

    float totalSum = 0;
    float totalProduct = 1;

    MPI_Reduce(&localSum, &totalSum, 1, MPI_FLOAT, MPI_SUM, 0, MPI_COMM_WORLD);
    MPI_Reduce(&localProduct, &totalProduct, 1, MPI_FLOAT, MPI_PROD, 0, MPI_COMM_WORLD);

    if (idProc == 0)
    {
        printf("Array A:\n");
        printArray(a, N);

        printf("\nSuma total: %.2f\n", totalSum);
        printf("Producto total: %.2f\n", totalProduct);
    }

    free(a);
    free(localA);
    free(sendcounts);
    free(displs);
    MPI_Finalize();
    return 0;
}
