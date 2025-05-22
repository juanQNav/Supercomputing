/*
-Materia: Supercomputo
-Semestre: 8 (2024-2025/II)
-Nombre del alumno: Quistian Navarro Juan Luis
-Clave del alumno: 341807
-Carrera: Ing. en Sistemas Inteligentes
-Nombre de tarea o programa: MPI_Type_create_struct
-Periodo de evaluación (parcial 1, 2, 3, 4 o 5): 3
-Descripción: MPI_Type_create_struct
-Avance logrado (0 a 100%): 100%
-Comentarios adicionales: NA
*/

#include <stdio.h>
#include <stdlib.h>
#include <mpi.h>
#include <time.h>

#define NAME_LEN 255
#define GRADES_LEN 4

typedef struct
{
    int id;
    int subjectId;
    char name[NAME_LEN];
    float grades[GRADES_LEN];
    float average;
} STUDENT;

void createArray(STUDENT **a, int N)
{
    *a = (STUDENT *)malloc(N * sizeof(STUDENT));

    if (a == NULL)
    {
        fprintf(stderr, "Error allocating memory\n");
        exit(EXIT_FAILURE);
    }
}

void fillArray(STUDENT *a, int N)
{
    srand(time(NULL));
    for (int i = 0; i < N; i++)
    {
        a[i].id = i;
        a[i].subjectId = rand() % 10;
        a[i].name[0] = 'A' + rand() % 26;
        a[i].name[1] = 'A' + rand() % 26;
        a[i].name[2] = 'A' + rand() % 26;
        a[i].name[3] = 'A' + rand() % 26;

        for (int j = 0; j < GRADES_LEN; j++)
        {
            a[i].grades[j] = (float)rand() / (float)RAND_MAX * 100.0;
        }
        a[i].average = 0.0;
    }
}

float calculateAverage(float *grades, int N)
{
    float sum = 0;

    for (int i = 0; i < N; i++)
    {
        sum += grades[i];
    }

    return sum / N;
}

// function to calculate average of grades for each student and count failed students
int getFailedStudents(STUDENT *students, int N)
{
    int nFailedStudents = 0;
    for (int i = 0; i < N; i++)
    {
        students[i].average = calculateAverage(students[i].grades, GRADES_LEN);
        if (students[i].average < 60.0)
        {
            nFailedStudents++;
        }
    }
    return nFailedStudents;
}

void printArray(STUDENT *a, int N)
{
    for (int i = 0; i < N; i++)
    {
        printf("ID: %d, Subject ID: %d, Name: %s, Grades: [%.2f, %.2f, %.2f, %.2f], Average: %.2f\n",
               a[i].id, a[i].subjectId, a[i].name,
               a[i].grades[0], a[i].grades[1], a[i].grades[2], a[i].grades[3], a[i].average);
    }
    printf("\n");
}

int main(int argc, char **argv)
{
    int idProc, numProc;

    MPI_Init(&argc, &argv);                  // Initialize MPI, send argc and argv from the main function
    MPI_Comm_rank(MPI_COMM_WORLD, &idProc);  // Get the rank of the process
    MPI_Comm_size(MPI_COMM_WORLD, &numProc); // Get the id of the process (numproc)

    // struct of the student
    MPI_Datatype MPI_STUDENT;                                                   // define the MPI datatype
    MPI_Datatype types[5] = {MPI_INT, MPI_INT, MPI_CHAR, MPI_FLOAT, MPI_FLOAT}; // types of the fields
    int blocklengths[5] = {1, 1, NAME_LEN, GRADES_LEN, 1};                      // number of elements in each field
    MPI_Aint blockDisp[5];                                                      // displacement of each field

    blockDisp[0] = offsetof(STUDENT, id);
    blockDisp[1] = offsetof(STUDENT, subjectId);
    blockDisp[2] = offsetof(STUDENT, name);
    blockDisp[3] = offsetof(STUDENT, grades);
    blockDisp[4] = offsetof(STUDENT, average);

    MPI_Type_create_struct(5, blocklengths, blockDisp, types, &MPI_STUDENT); // create the MPI datatype
    MPI_Type_commit(&MPI_STUDENT);                                           // commit the datatype

    int *sendcounts = NULL; // number of elements to send to each process
    int *displs = NULL;     // displacement for each process

    int N = 20; // number of elements in the array

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

    STUDENT *S = NULL;

    if (idProc == 0)
    {
        createArray(&S, N);

        fillArray(S, N);
        printf("Students:\n");
        printArray(S, N);
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

    STUDENT *localStudents = NULL;

    createArray(&localStudents, localCount);

    MPI_Scatterv(
        S,             // send initial address of the array
        sendcounts,    // number of elements to send to each process
        displs,        // displacement for each process
        MPI_STUDENT,   // data type to send
        localStudents, // receive initial address of the array
        localCount,    // number of elements to receive
        MPI_STUDENT,   // data type to receive
        0,             // id of the process to send to
        MPI_COMM_WORLD // communicator
    );

    // calculate the average of the grade for each students and count failed students
    int localfailedStudents = 0, totalFailedStudents = 0;
    localfailedStudents = getFailedStudents(localStudents, localCount);

    MPI_Gatherv(
        localStudents, // send initial address of the array
        localCount,    // number of elements to send from each process
        MPI_STUDENT,   // data type to send
        S,             // receive initial address of the array
        sendcounts,    // number of elements to receive from each process
        displs,        // displacement for each process
        MPI_STUDENT,   // data type to receive
        0,             // id of the process to send to
        MPI_COMM_WORLD // communicator
    );

    MPI_Reduce(
        &localfailedStudents, // send initial address of the array
        &totalFailedStudents, // receive initial address of the array
        1,                    // number of elements to send from each process
        MPI_INT,              // data type to send
        MPI_SUM,              // operation to perform
        0,                    // id of the process to send to
        MPI_COMM_WORLD        // communicator
    );

    if (idProc == 0)
    {
        printf("Average of all students:\n");
        printArray(S, N);
        printf("Total failed students: %d\n", totalFailedStudents);
    }

    free(localStudents); // free the local array

    if (idProc == 0)
    {
        free(S);
        free(sendcounts);
        free(displs);
    }

    MPI_Type_free(&MPI_STUDENT); // free the type
    MPI_Finalize();              // finalize MPI
    return 0;
}