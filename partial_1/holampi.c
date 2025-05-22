#include <mpi.h>
#include <stdio.h>
#include <unistd.h>

int main(int argc, char** argv) {
    int rank, size;
    char hostname[256];

    
    MPI_Init(&argc, &argv);
    
    MPI_Comm_size(MPI_COMM_WORLD, &size);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
        
    gethostname(hostname, sizeof(hostname));
    
    printf("Hola Mundo desde el proceso %d de %d en %s\n", rank, size, hostname);
     
    MPI_Finalize();
    
    return 0;
}