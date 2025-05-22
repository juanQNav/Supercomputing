/*
-Materia: Supercomputo
-Semestre: 8 (2024-2025/II)
-Nombre del alumno: Quistian Navarro Juan Luis
-Clave del alumno: 341807
-Carrera: Ing. en Sistemas Inteligentes
-Nombre de tarea o programa: Suma de dos arreglos de enteros usando MPI scatterv y gatherv
-Periodo de evaluación (parcial 1, 2, 3, 4 o 5): 3
-Descripción: Suma de dos arreglos de enteros usando MPI scatterv y gatherv
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

    int *sendcounts = NULL; // number of elements to send to each process
    int *displs = NULL;     // displacement for each process

    int N = 10;

    if (idProc == 0)
    {
        sendcounts = (int *)malloc(numProc * sizeof(int)); // allocate memory for sendcounts
        displs = (int *)malloc(numProc * sizeof(int));     // allocate memory for displs

        int base = N / numProc;  // base number of elements to send to each process
        int extra = N % numProc; // extra number of elements to send to the last process
        int offset = 0;          // offset for each process

        // calculate sendcounts and displs for each process
        for (int i = 0; i < numProc; i++)
        {
            sendcounts[i] = base + (i < extra ? 1 : 0); // first extra processes get one more element
            displs[i] = offset;                         // displacement from the start of the array
            offset += sendcounts[i];                    // update offset for next process
        }
    }

    // int suma = 0;
    // int nDatos = N / (numProc - 1);
    // int nDatosU = nDatos + N % (numProc - 1);

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

    int *localA = NULL;
    int *localB = NULL;
    int *localC = NULL;

    createArray(&localA, localCount);
    createArray(&localB, localCount);
    createArray(&localC, localCount);

    MPI_Scatterv(
        a,             // send initial address of the array
        sendcounts,    // number of elements to send to each process
        displs,        // displacement for each process
        MPI_INT,       // data type
        localA,        // receive initial address of the array
        localCount,    // number of elements to receive
        MPI_INT,       // data type
        0,             // id of the process to send to
        MPI_COMM_WORLD // communicator
    );
    MPI_Scatterv(
        b,             // send initial address of the array
        sendcounts,    // number of elements to send to each process
        displs,        // displacement for each process
        MPI_INT,       // data type
        localB,        // receive initial address of the array
        localCount,    // number of elements to receive
        MPI_INT,       // data type
        0,             // id of the process to send to
        MPI_COMM_WORLD // communicator
    );

    // additon of the arrays
    for (int i = 0; i < localCount; i++)
    {
        localC[i] = localA[i] + localB[i];
    }

    // recollect the results
    MPI_Gatherv(
        localC,        // send initial address of the array
        localCount,    // number of elements to send
        MPI_INT,       // data type
        c,             // receive initial address of the array
        sendcounts,    // number of elements to receive
        displs,        // displacement for each process
        MPI_INT,       // data type
        0,             // id of the process to send to
        MPI_COMM_WORLD // communicator
    );

    if (idProc == 0)
    {
        printf("Array A:\n");
        printArray(a, N);
        printf("\nArray B:\n");
        printArray(b, N);
        printf("\nResult (A + B):\n");
        printArray(c, N);
    }

    free(localA);
    free(localB);
    free(localC);

    if (idProc == 0)
    {
        free(a);
        free(b);
        free(c);
        free(sendcounts);
        free(displs);
    }

    MPI_Finalize();
    return 0;
}