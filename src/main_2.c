#include "mpi.h"
#include "myforest.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>


int main(int argc, char *argv[]){
    int my_rank; //Process Rank
    int size_p; //Max P number
    int prec; //Processo precednte
    int dest; //Processo successivo
    int N = 0; // Row and Column Dimension
    int I = 0; //Iteration number
    char* sendBuff; //Temp Updated Matrix
    float start = 0; //Starting time
    float end = 0; //Finish time

    MPI_Init(&argc,&argv);
    MPI_Comm_rank (MPI_COMM_WORLD,&my_rank);
    MPI_Comm_size(MPI_COMM_WORLD,&size_p);
    MPI_Barrier(MPI_COMM_WORLD); /* tutti i processi sono inizializzati */
    start = MPI_Wtime();

    if(my_rank == 0) {
        if (argc < 3) { //0 = FILENAME - 1 = N - 2 = ITERAZIONI
            fprintf(stderr, "Errore parametri mancanti !\n");
            MPI_Abort(MPI_COMM_WORLD, EXIT_FAILURE);
            exit(0);
        }
    }
    N = atoi(argv[1]);
    I = atoi(argv[2]);

    if(my_rank == 0){
        if(!N || N < 2 || !I ){ //Bad values for N & I
            fprintf( stderr, "Errore valore parametri !\n");
            MPI_Abort(MPI_COMM_WORLD,EXIT_FAILURE);
            exit(0);
        }

        if( N < (size_p-1)){ //Bad values for N & I
            fprintf( stderr, "Numero di righe non sufficiente !\n");
            MPI_Abort(MPI_COMM_WORLD,EXIT_FAILURE);
            exit(0);
        }
    }



    char *forest = malloc(sizeof *forest * N * N); //Starter Forest Matrix

    if(my_rank == 0){
        srand(42);//Random Seed
        generation(N,&forest);
    }

    MPI_Request req = MPI_REQUEST_NULL; //Send
    MPI_Request req1 = MPI_REQUEST_NULL; //Recv pre
    MPI_Request req2 = MPI_REQUEST_NULL; //Recv post
    MPI_Status Stat1;
    MPI_Status Stat2;

    //Calcolo il numero di elementi per ogni processo
    int* sendCount = malloc ( size_p* sizeof(int));
    int* displacement = malloc(size_p * sizeof(int));
    

    for(int index = 0; index < I && isEmpty(N,forest,my_rank); index++){

        if(my_rank == 0){
            //Stampo la matrice
            //print_forest(N,forest,index);
        }
        if(my_rank != 0){
            precDest(my_rank,size_p,&prec,&dest);
        }

        divWork2(N,size_p,&sendCount,&displacement);
        char *recvBuff = (char*) malloc(sendCount[my_rank]*sizeof(char));

        MPI_Scatterv(forest,sendCount,displacement,MPI_CHAR,recvBuff,sendCount[my_rank],MPI_CHAR,0,MPI_COMM_WORLD);

        if( my_rank != 0){
            char* preNeighbor = (char*) malloc(sendCount[prec] *sizeof(char));
            char* destNeighbor = (char*) malloc(sendCount[dest] *sizeof(char));
            if(prec != -10){
                //Ricevo dal precedente
                MPI_Irecv(preNeighbor,sendCount[prec],MPI_CHAR,prec,TAG,MPI_COMM_WORLD,&req1);
            }
            //Ricevo dal successivo
            if(dest != -10){
                MPI_Irecv(destNeighbor,sendCount[dest],MPI_CHAR,dest,TAG,MPI_COMM_WORLD,&req2);
            }
            //Invio al precedente
            //printf("Sendocunt[%d]:%d\n",my_rank,sendCount[my_rank]);
            if(prec != -10){
                MPI_Isend(recvBuff,sendCount[my_rank],MPI_CHAR,prec,TAG,MPI_COMM_WORLD,&req);
            }
            if(dest != -10){
                //Invio al successivo
                MPI_Isend(recvBuff,sendCount[my_rank],MPI_CHAR,dest,TAG,MPI_COMM_WORLD,&req);
            }

            MPI_Wait(&req1,&Stat1);
            MPI_Wait(&req2,&Stat2);

            int total = 0;
            char* tempMatrix = prepareForCheck(N,preNeighbor,recvBuff,destNeighbor, sendCount, my_rank,prec,dest,&total);
            sendBuff = check(N, tempMatrix,sendCount,my_rank,prec,dest,total);

            free(preNeighbor);
            free(destNeighbor);
            free(tempMatrix);
        }

        printf("Pre gatherv, rank:%d prec:%d dest:%d sendCount[rank]:%d\n",my_rank,prec,dest,sendCount[my_rank]);
        MPI_Gatherv(sendBuff,sendCount[my_rank],MPI_CHAR,forest,sendCount,displacement,MPI_CHAR,0,MPI_COMM_WORLD);

        /*if(my_rank == 0){
            print_forest(N,forest,index);
        }*/
        free(recvBuff);
    }
    /*if(my_rank != 0){
        free(sendBuff);
    }*/
    free(forest);
    free(sendCount);
    free(displacement);

    if(!isEmpty(N,forest,my_rank)){
        MPI_Abort(MPI_COMM_WORLD,EXIT_FAILURE);
    }



    MPI_Barrier(MPI_COMM_WORLD); /* tutti i processi hanno terminato */
    end = MPI_Wtime();
    MPI_Finalize();
    if (my_rank == 0) { /* Master node scrive su stdout il tempo o su file */
        printf("Time in s = %f\n", end-start);
    }
}