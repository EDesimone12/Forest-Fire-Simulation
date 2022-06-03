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

    MPI_Init(&argc,&argv);
    MPI_Comm_rank (MPI_COMM_WORLD,&my_rank);
    MPI_Comm_size(MPI_COMM_WORLD,&size_p);

    if(argc < 3){ //0 = FILENAME - 1 = N - 2 = ITERAZIONI
        fprintf( stderr, "Errore parametri mancanti !\n");
        MPI_Finalize();
        exit(0);
    }
    N = atoi(argv[1]);
    I = atoi(argv[2]);

    if(!N || !I){ //Bad values for N & I
        fprintf( stderr, "Errore valore parametri !\n");
        MPI_Finalize();
        exit(0);
    }


    char *forest = malloc(sizeof *forest * N * N); //Starter Forest Matrix

    if(my_rank == 0){
        srand(42);//Random Seed
        //generation(N,forest);
        generationDeterministic(N,&forest);
    }


    MPI_Request req = MPI_REQUEST_NULL; //Send
    MPI_Request req1 = MPI_REQUEST_NULL; //Recv pre
    MPI_Request req2 = MPI_REQUEST_NULL; //Recv post
    MPI_Status Stat1;
    MPI_Status Stat2;

    //Calcolo il numero di elementi per ogni processo
    int* sendCount = malloc ( size_p* sizeof(int));
    int* displacement = malloc(size_p * sizeof(int));
    

    for(int index = 0; index < I && isEmpty(N,forest); index++){
        printf("i = %d\n",index);

        if(my_rank == 0){
            //Stampo la matrice
            print_forest(N,forest,index);
        }
        if(my_rank != 0){
            precDest(my_rank,size_p,&prec,&dest);
        }

        divWork(N,size_p,&sendCount,&displacement);
        char *recvBuff = (char*) malloc(sendCount[my_rank]*sizeof(char));

        MPI_Scatterv(forest,sendCount,displacement,MPI_CHAR,recvBuff,sendCount[my_rank],MPI_CHAR,0,MPI_COMM_WORLD);

        /*if(my_rank == 1){
            for(int i = 0; i < size_p; i++){
                printf(" i:%d  Displ %d send %d \n", i,displacement[i], sendCount[i]);
                printf("| %c |",recvBuff[i]);
            }
            printf("\n");
        }*/


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

            //printf("Total = %d  Myrank = %d\n",total,my_rank);

            //Stampa precedenti e successivi per ogni processo
            /*if(my_rank == 2){


           if(prec != -10){
               printf("Sono %d - PRE\n",my_rank);
               for(int i = 0; i < sendCount[prec]; i++){
                   printf("%c ",preNeighbor[i]);
               }
               printf("\n");
           }
           if(dest != -10){
               printf("Sono %d - DEST\n",my_rank);
               for(int i = 0; i < sendCount[dest]; i++){
                   printf("%c ",destNeighbor[i]);
               }
               printf("\n");
           }
            }*/
        }

        MPI_Gatherv(sendBuff,sendCount[my_rank],MPI_CHAR,forest,sendCount,displacement,MPI_CHAR,0,MPI_COMM_WORLD);

        if(my_rank == 0){
            //print_forest(N,forest,index);
        }
        //free(recvBuff);
    MPI_Barrier(MPI_COMM_WORLD);
    }

    //free(forest);
    //free(sendBuff);
    //free(sendCount);
    //free(displacement);
    MPI_Finalize();
}