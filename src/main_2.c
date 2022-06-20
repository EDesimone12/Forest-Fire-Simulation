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
    char *temp = malloc(sizeof *forest * N * N); //Temp Matrix

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
    int* sendCount = (int*) malloc ( size_p* sizeof(int));
    int* displacement = (int*) malloc(size_p * sizeof(int));

    if(my_rank != 0){
        precDest(my_rank,size_p,&prec,&dest);
    }

    divWork2(N,size_p,&sendCount,&displacement);
    char *recvBuff;
    if(my_rank == 1 || my_rank == (size_p-1)){
        recvBuff = (char*) calloc(sendCount[my_rank]+N,sizeof(char));
    }else{
        recvBuff = (char*) calloc(sendCount[my_rank]+(2*N),sizeof(char));
    }

    if(my_rank == 1){
        MPI_Scatterv(forest,sendCount,displacement,MPI_CHAR,recvBuff,sendCount[my_rank],MPI_CHAR,0,MPI_COMM_WORLD);
    }else{
        MPI_Scatterv(forest,sendCount,displacement,MPI_CHAR,(recvBuff+N),sendCount[my_rank],MPI_CHAR,0,MPI_COMM_WORLD);
    }

    for(int index = 0; index < I; index++){

        if(my_rank == 0){
            //Stampo la matrice
            print_forest(N,forest,index);
        }

        if( my_rank != 0){
            if(prec != -10){
                //Ricevo dal precedente
                MPI_Irecv(recvBuff,N,MPI_CHAR,prec,TAG,MPI_COMM_WORLD,&req1);
            }
            //Ricevo dal successivo
            if(dest != -10){
                if(my_rank == 1){
                    MPI_Irecv((recvBuff+sendCount[my_rank]),N,MPI_CHAR,dest,TAG,MPI_COMM_WORLD,&req2);
                }else{
                    MPI_Irecv((recvBuff+N+sendCount[my_rank]),N,MPI_CHAR,dest,TAG,MPI_COMM_WORLD,&req2);
                }
            }
            //Invio al precedente
            if(prec != -10){
                MPI_Isend((recvBuff+N),N,MPI_CHAR,prec,TAG,MPI_COMM_WORLD,&req);
            }
            if(dest != -10){
                //Invio al successivo
                if(my_rank == 1){
                    MPI_Isend((recvBuff+sendCount[my_rank])-N,N,MPI_CHAR,dest,TAG,MPI_COMM_WORLD,&req);
                }else{
                    MPI_Isend((recvBuff+N+sendCount[my_rank])-N,N,MPI_CHAR,dest,TAG,MPI_COMM_WORLD,&req);
                }
            }

            //Lavoro sui miei elementi
            int start = my_rank == 1 ? 0 : 1;
            int end = my_rank == 1 ? sendCount[my_rank] : sendCount[my_rank] + N;

            checkMine(recvBuff,temp,start,end,my_rank,prec,dest, N,0); //flag a 0 check senza vicini

            MPI_Wait(&req1,&Stat1);
            MPI_Wait(&req2,&Stat2);

            checkMine(recvBuff,temp,start,end,my_rank,prec,dest, N,1); //flag a 1 check con vicini

            //free(tempMatrix);
        }
        char* suppPointer;
        suppPointer = my_rank == 1 ? recvBuff : recvBuff+N;
        recvBuff = my_rank == 1 ? temp : temp+N;
        temp = suppPointer;

        //free(recvBuff);
    }
    //MPI_Gatherv(sendBuff,sendCount[my_rank],MPI_CHAR,forest,sendCount,displacement,MPI_CHAR,0,MPI_COMM_WORLD);

    /*if(my_rank == 0){
        print_forest(N,forest,index);
    }*/

    /*if(!isEmpty(N,forest,my_rank)){
        MPI_Abort(MPI_COMM_WORLD,EXIT_FAILURE);
    }*/

    free(forest);
    free(sendCount);
    free(displacement);

    MPI_Barrier(MPI_COMM_WORLD); /* tutti i processi hanno terminato */
    end = MPI_Wtime();
    MPI_Finalize();
    if (my_rank == 0) { /* Master node scrive su stdout il tempo o su file */
        printf("Time in s = %f\n", end-start);
    }
}