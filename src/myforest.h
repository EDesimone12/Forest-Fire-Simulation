#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define TAG 42
#define N 10   //Matrix Dimension
#define F 10    //Ignite probability F(Fire)
#define P 70       //New Tree probability 

void generation(char matrix[N][N]){
    // 1(TREE) 2(EMPTY) 3(BURNING TREE)

    /*matrix = (char **)malloc(N*sizeof(char *)); //Array of pointer
    matrix[0] = (char *)malloc(N*N*sizeof(char)); //Matrix

    for(int i=1; i < N; i++)
        matrix[i] = matrix[0] + i*N; */

    srand(42);
    for(int i=0; i < N; i++){
        for(int j =0; j < N; j++){
            if((rand() %101) <= P){
                matrix[i][j] = '1';
            }else{
                matrix[i][j] = '2';
            }
        }
    }
}

void print_forest(char f[N][N]){
    for(int i=0; i < N; i++){
        printf("--------------------------------------------------\n");
        for(int j=0; j < N; j++){
            printf("| %c |",f[i][j]);
        }
        printf("\n");
    }
    printf("--------------------------------------------------\n");
}

void print_forest_array(char * temp){
    for(int i = 0; i < N; i++){
        printf("--------------------------------------------------\n");
        for(int j = 0; j < N; j++){
            printf("| %c |",temp[((i*N)+j)]);
        }
        printf("\n");
    }
    printf("--------------------------------------------------\n");
}

void precDest (int my_rank, int size_p, int *prec , int* dest){
    if(my_rank == 1){
        *prec = -10 ; //Il primo processo non ha precedenti
    }else{
        *prec = (my_rank + size_p -1) % size_p;
    }
    if(my_rank == (size_p-1) ){
        *dest = -10; //L'ultimo processo non ha successivi
    }else{
        *dest = ((my_rank + 1) % size_p);
    }
    //printf("Sono %d , pre: %d  dest:%d \n",my_rank,*prec,*dest);
}

void divWork(int size,int** sendCount, int** displacement){
    //printf("Inizio divisione...\n");
    int divVal = (N*N) / (size-1);
    int restVal = (N*N) % (size-1);

    int pos = 0;
    displacement[0][0] = 0;
    sendCount[0][0] = 0; //Al processo master non dò nulla da fare.

    for(int i = 1; i < size;i++){
        sendCount[0][i] = divVal;
        if(restVal > 0){
            sendCount[0][i]++;
            restVal--;
        }
        //Calcolo il displacement per la scatterv
        displacement[0][i] = pos;
        pos = pos + sendCount[0][i];
        //printf("Displ[%d]:%d \n",i,displacement[0][i]);
        //printf("SendCount[%d]: %d \n ",i,sendCount[0][i]);
    }
    //printf("Fine divisione...\n");
}

char* prepareForCheck(char* preNeighbor,char* recvBuff,char* destNeighbor,int* sendCount,int my_rank,int prec, int dest,int* total){
        char *arr = malloc(sizeof *arr * N * N);

        if(prec != -10){
            memcpy(arr,preNeighbor,sendCount[prec]);
            *total += sendCount[prec];
            memcpy(arr+sendCount[prec],recvBuff,sendCount[my_rank]);
            *total += sendCount[my_rank];
            //printf("rank= %d - sendCount[my_rank] %d - sendCount[dest] = %d  dest= %d\n",my_rank,sendCount[my_rank],sendCount[dest],dest);
            if(dest != -10){
                memcpy(arr+sendCount[prec]+sendCount[my_rank],destNeighbor, sendCount[dest]);
                *total += sendCount[dest];
            }
        }else{
            memcpy(arr, recvBuff, sendCount[my_rank]);
            *total += sendCount[my_rank];
            //printf("sotto - rank= %d - sendCount[my_rank] %d - sendCount[dest] = %d  dest= %d  prec = %d\n",my_rank,sendCount[my_rank],sendCount[dest],dest,prec);
            memcpy(arr+sendCount[my_rank],destNeighbor,sendCount[dest]);
            *total += sendCount[dest];
        }
        /* //Stampa per verifica della creazione dela matrice temporanea
        int count = 0;
        for(int i = 0; i < N; i++){
            for(int j = 0; j < N; j++){
                printf("(rank = %d i=%d j=%d count=%d) -elem [%d] = %c\n",my_rank,i,j,count,((i*N)+j),arr[((i*N)+j)]);
                count++;
            }
            printf("------\n");
        }*/
        return arr;
}

char* check(char** temp, int* sendCount, int rank,int prec, int dest, int total) {
    int start = 0;
    srand(rank + 1); //Annullo il comportamento si srand(1);

    //Starting point
    if (prec == -10) {
        start = 0;
    } else {
        start = sendCount[prec];
    }

    for (int i = 0; i < N; i++) {
        for (int j = 0; j < N; j++) {

            if (*temp[((i * N) + j)] == '3') { //1)A burning cell turns into an empty cell
                *temp[((i * N) + j)] = 2;
            }

            if (*temp[((i * N) + j)] == '2') { //4)An empty space fills with a tree with probability P
                if ((rand() % 101) <= P) {
                    *temp[((i * N) + j)] = '1';
                }
            }

            if (prec == -10) {
                //if(temp[(((i+1)*N)+j) == ]
            } else if (dest != -10) {

            } else {

            }
        }
    }
    //print_forest_array(*temp);
}