#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <wchar.h>

#define TAG 42
#define N 10   //Matrix Dimension
#define F 10    //Ignite probability F(Fire)
#define P 70       //New Tree probability 
// 1(TREE) 2(EMPTY) 3(BURNING TREE)

void generation(char matrix[N][N]){
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

void print_forest_array(char * temp,int rank){
    char *fileName = (char*) malloc(sizeof(char)*25); //Dim of the fileName string

    FILE *fp;
    sprintf(fileName,"output_check_%d.txt",rank);

    fp = fopen(fileName,"w");

    for(int i = 0; i < N; i++){
        if(i == 0){
            fprintf(fp,"--------------------RANK %d------------------------------\n",rank);
        }else{
            fprintf(fp,"--------------------------------------------------\n");
        }

        for(int j = 0; j < N; j++){
            fprintf(fp,"| %c |",temp[((i*N)+j)]);
        }
        fprintf(fp,"\n");
    }
    fprintf(fp,"--------------------------------------------------\n");
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
         //Stampa per verifica della creazione dela matrice temporanea
        /*int count = 0;
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
    char *retMatrix = malloc(sizeof *retMatrix * N * N);

    srand(rank + 1); //Annullo il comportamento di srand(1);

    //Starting point
    if (prec == -10) {
        start = 0;
    } else {
        start = sendCount[prec];
    }

    for (int i = 0; i < N && total != 0; i++) {
        for(int j = 0; j < N; j++){

            if ((*temp)[(i*N)+j] == '3') { //1)A burning cell turns into an empty cell
                retMatrix[(i*N)+j] = '2';
            }

            if ((*temp)[(i*N)+j] == '2') { //4)An empty space fills with a tree with probability P
                if ((rand() % 101) <= P) {
                    retMatrix[(i*N)+j] = '1';
                }
                //printf("4) rank = %d retMatrix[%d] = %c\n", rank,(i*N)+j,retMatrix[(i*N)+j]);
            }

            if (prec == -10) {//First Row
                if(j == 0 && ((i*N)+j+1) != '3' && (((i+1)*N)+j) != '3' ){ //3)A tree ignites with probability F even if no neighbor is burning
                    if((rand() % 101) <= F){
                        retMatrix[(i*N)+j] = '3';
                    }
                    //printf("3)rank = %d retMatrix[%d] = %c\n", rank,(i*N)+j,retMatrix[(i*N)+j]);
                }else if(j == (N-1) && ((i*N)+(j-1)) != '3' && (((i+1)*N)+j) != '3'){
                    if((rand() % 101) <= F){
                        retMatrix[(i*N)+j] = '3';
                    }
                    //printf("3)rank = %d retMatrix[%d] = %c\n", rank,(i*N)+j,retMatrix[(i*N)+j]);
                }else if( ((i*N)+j-1) != '3' && ((i*N)+j+1) != '3' && (((i+1)*N)+j) != '3'){
                    if((rand() % 101) <= F){
                        retMatrix[(i*N)+j] = '3';
                    }
                    //printf("3)rank = %d retMatrix[%d] = %c\n", rank,(i*N)+j,retMatrix[(i*N)+j]);
                } // 3) END -------------------------------------------------------

                if((j == 0) && ( ((i*N)+j+1) == '3' || (((i+1)*N)+j) == '3') ){ //2)A tree will burn if at least one neighbor is burning
                    retMatrix[(i*N)+j] = '3';
                    //printf("2)rank = %d retMatrix[%d] = %c\n", rank,(i*N)+j,retMatrix[(i*N)+j]);
                }else if( (j == (N-1)) && ( ((i*N)+(j-1)) == '3' || (((i+1)*N)+j) ) == '3') {
                    retMatrix[(i*N)+j] = '3';
                    //printf("2)rank = %d retMatrix[%d] = %c\n", rank,(i*N)+j,retMatrix[(i*N)+j]);
                }else if(((i*N)+j-1) == '3' || ((i*N)+j+1) == '3' || (((i+1)*N)+j) == '3'){
                    retMatrix[(i*N)+j] = '3';
                    //printf("2)rank = %d retMatrix[%d] = %c\n", rank,(i*N)+j,retMatrix[(i*N)+j]);
                }// 2) END ---------------------------------------------------------

            } else if (dest == -10) {//Last Row
                if(j == 0 && ((i*N)+j+1) != '3' && (((i-1)*N)+j) != '3' ){ //3)A tree ignites with probability F even if no neighbor is burning
                    if((rand() % 101) <= F){
                        retMatrix[(i*N)+j] = '3';
                    }
                    //printf("3)rank = %d retMatrix[%d] = %c\n", rank,(i*N)+j,retMatrix[(i*N)+j]);
                }else if(j == (N-1) && ((i*N)+(j-1)) != '3' && (((i-1)*N)+j) != '3'){
                    if((rand() % 101) <= F){
                        retMatrix[(i*N)+j] = '3';
                    }
                    //printf("3)rank = %d retMatrix[%d] = %c\n", rank,(i*N)+j,retMatrix[(i*N)+j]);
                }else if( ((i*N)+j-1) != '3' && ((i*N)+j+1) != '3' && (((i-1)*N)+j) != '3'){
                    if((rand() % 101) <= F){
                        retMatrix[(i*N)+j] = '3';
                    }
                    //printf("3)rank = %d retMatrix[%d] = %c\n", rank,(i*N)+j,retMatrix[(i*N)+j]);
                } // 3) END -------------------------------------------------------

                if((j == 0) && ( ((i*N)+j+1) == '3' || (((i-1)*N)+j) == '3') ){ //2)A tree will burn if at least one neighbor is burning
                    retMatrix[(i*N)+j] = '3';
                    //printf("2)rank = %d retMatrix[%d] = %c\n", rank,(i*N)+j,retMatrix[(i*N)+j]);
                }else if( (j == (N-1)) && ( ((i*N)+(j-1)) == '3' || (((i-1)*N)+j) == '3' ) ) {
                    retMatrix[(i*N)+j] = '3';
                    //printf("2)rank = %d retMatrix[%d] = %c\n", rank,(i*N)+j,retMatrix[(i*N)+j]);
                }else if(((i*N)+j-1) == '3' || ((i*N)+j+1) == '3' || (((i-1)*N)+j) == '3'){
                    retMatrix[(i*N)+j] = '3';
                    //printf("2)rank = %d retMatrix[%d] = %c\n", rank,(i*N)+j,retMatrix[(i*N)+j]);
                }// 2) END ---------------------------------------------------------

            } else {
                if(j == 0 && ((i*N)+j+1) != '3' && (((i+1)*N)+j) != '3' && (((i-1)*N)+j) != '3' ){ //3)A tree ignites with probability F even if no neighbor is burning
                    if((rand() % 101) <= F){
                        retMatrix[(i*N)+j] = '3';
                    }
                    //printf("3)rank = %d retMatrix[%d] = %c\n", rank,(i*N)+j,retMatrix[(i*N)+j]);
                }else if(j == (N-1) && ((i*N)+(j-1)) != '3' && (((i+1)*N)+j) != '3' && (((i-1)*N)+j) != '3'){
                    if((rand() % 101) <= F){
                        retMatrix[(i*N)+j] = '3';
                    }
                    //printf("3)rank = %d retMatrix[%d] = %c\n", rank,(i*N)+j,retMatrix[(i*N)+j]);
                }else if( ((i*N)+j-1) != '3' && ((i*N)+j+1) != '3' && (((i+1)*N)+j) != '3' && (((i-1)*N)+j) != '3'){
                    if((rand() % 101) <= F){
                        retMatrix[(i*N)+j] = '3';
                    }
                    //printf("3)rank = %d retMatrix[%d] = %c\n", rank,(i*N)+j,retMatrix[(i*N)+j]);
                } // 3) END -------------------------------------------------------

                if((j == 0) && ( ((i*N)+j+1) == '3' || (((i-1)*N)+j) == '3' || (((i+1)*N)+j) == '3') ){ //2)A tree will burn if at least one neighbor is burning
                    retMatrix[(i*N)+j] = '3';
                    //printf("2)rank = %d retMatrix[%d] = %c\n", rank,(i*N)+j,retMatrix[(i*N)+j]);
                }else if( (j == (N-1)) && ( ((i*N)+(j-1)) == '3' || (((i-1)*N)+j) == '3' || (((i+1)*N)+j) == '3' ) ) {
                    retMatrix[(i*N)+j] = '3';
                    //printf("2)rank = %d retMatrix[%d] = %c\n", rank,(i*N)+j,retMatrix[(i*N)+j]);
                }else if(((i*N)+j-1) == '3' || ((i*N)+j+1) == '3' || (((i-1)*N)+j) == '3' || (((i+1)*N)+j) == '3'){
                    retMatrix[(i*N)+j] = '3';
                    //printf("2)rank = %d retMatrix[%d] = %c\n", rank,(i*N)+j,retMatrix[(i*N)+j]);
                }// 2) END ---------------------------------------------------------

            }
            total--;
        }
    }
    print_forest_array(retMatrix,rank);
    return retMatrix;
}