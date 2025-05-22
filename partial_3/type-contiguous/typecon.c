/*
-Materia: Supercomputo
-Semestre: 8 (2024-2025/II)
-Nombre del alumno: Quistian Navarro Juan Luis
-Clave del alumno: 341807
-Carrera: Ing. en Sistemas Inteligentes
-Nombre de tarea o programa: MPI_Type_Contiguous
-Periodo de evaluación (parcial 1, 2, 3, 4 o 5): 3
-Descripción: MPI_Type_Contiguous
-Avance logrado (0 a 100%): 100%
-Comentarios adicionales: NA
*/

#include <stdio.h>
#include <stdlib.h>
#include <mpi.h>
#include <time.h>

// struct
typedef struct
{
    float x;
    float y;
    float z;
} COORD;

void createArray(COORD **a, int N)
{
    *a = (COORD *)malloc(N * sizeof(COORD));

    if (a == NULL)
    {
        fprintf(stderr, "Error allocating memory\n");
        exit(EXIT_FAILURE);
    }
}

void fillArray(COORD *a, int N)
{
    srand(time(NULL));
    for (int i = 0; i < N; i++)
    {
        a[i].x = rand() % 100;
        a[i].y = rand() % 100;
        a[i].z = rand() % 100;
    }
}

void sumArray(COORD *a, COORD *b, COORD *c, int N)
{
    for (int i = 0; i < N; i++)
    {
        c[i].x = a[i].x + b[i].x;
        c[i].y = a[i].y + b[i].y;
        c[i].z = a[i].z + b[i].z;
    }
}

void printArray(COORD *a, int N)
{
    for (int i = 0; i < N; i++)
    {
        printf("(%.2f, %.2f, %.2f) ", a[i].x, a[i].y, a[i].z);
    }
    printf("\n");
}

int main(int argc, char **argv)
{
    int idProc, numProc;

    MPI_Init(&argc, &argv);                  // Initialize MPI, send argc and argv from the main function
    MPI_Comm_rank(MPI_COMM_WORLD, &idProc);  // Get the rank of the process
    MPI_Comm_size(MPI_COMM_WORLD, &numProc); // Get the id of the process (numproc)

    int *sendcounts = NULL; // number of elements to send to each process
    int *displs = NULL;     // displacement for each process

    int N = 10; // number of elements in the array

    if (idProc == 0)
    {
        sendcounts = (int *)malloc(numProc * sizeof(int));
        displs = (int *)malloc(numProc * sizeof(int));

        int base = N / numProc;  // base number of elements to send to each process
        int extra = N % numProc; // extra number of elements to send to the last process
        int offset = 0;          // offset for each process

        for (int i = 0; i < numProc; i++)
        {
            sendcounts[i] = base + (i < extra ? 1 : 0); // first extra processes get one more element
            displs[i] = offset;                         // displacement from the start of the array
            offset += sendcounts[i];                    // update offset for next process
        }
    }

    COORD *a = NULL;
    COORD *b = NULL;
    COORD *c = NULL;

    if (idProc == 0)
    {
        createArray(&a, N);
        createArray(&b, N);
        createArray(&c, N);

        fillArray(a, N);
        fillArray(b, N);
        fillArray(c, N);
    }

    int localCount = 0;

    MPI_Scatter(
        sendcounts,    // send initial address of the array
        1,             // number of elements to send to each process
        MPI_INT,       // data type send
        &localCount,   // receive initial address of the array
        1,             // number of elements to receive
        MPI_INT,       // data type receive
        0,             // id of the process to send to
        MPI_COMM_WORLD // communicator
    );

    COORD *localA = NULL;
    COORD *localB = NULL;
    COORD *localC = NULL;

    createArray(&localA, localCount);
    createArray(&localB, localCount);
    createArray(&localC, localCount);

    // type contiguous
    MPI_Datatype MPI_COORD;
    MPI_Type_contiguous(3, MPI_FLOAT, &MPI_COORD); // 3 floats in a struct
    MPI_Type_commit(&MPI_COORD);                   // commit the type

    MPI_Scatterv(
        a,             // send initial address of the array
        sendcounts,    // number of elements to send to each process
        displs,        // displacement for each process
        MPI_COORD,     // data type to send
        localA,        // receive initial address of the array
        localCount,    // number of elements to receive
        MPI_COORD,     // data type to receive
        0,             // id of the prcess to send to
        MPI_COMM_WORLD // communicator
    );

    MPI_Scatterv(
        b,             // send initial address of the array
        sendcounts,    // number of elements to send to each process
        displs,        // displacement for each process
        MPI_COORD,     // data type to send
        localB,        // receive initial address of the array
        localCount,    // number of elements to receive
        MPI_COORD,     // data type to receive
        0,             // id of the prcess to send to
        MPI_COMM_WORLD // communicator
    );

    // sum the arrays
    sumArray(localA, localB, localC, localCount);

    // recollect the results
    MPI_Gatherv(
        localC,        // send initial address of the array
        localCount,    // number of elements to send from each process
        MPI_COORD,     // data type to send
        c,             // receive initial address of the array
        sendcounts,    // number of elements to receive from each process
        displs,        // displacement for each process
        MPI_COORD,     // data type to receive
        0,             // id of the process to send to
        MPI_COMM_WORLD // communicator
    );

    if (idProc == 0)
    {
        printf("Array A:\n");
        printArray(a, N);
        printf("Array B:\n");
        printArray(b, N);
        printf("Result (A + B):\n");
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

    MPI_Type_free(&MPI_COORD); // free the type
    MPI_Finalize();            // finalize MPI
    return 0;
}