#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <wchar.h>

#define TAG 42
#define F 10 //100    //Ignite probability F(Fire)
#define P 70 //100       //New Tree probability

#define TREE "🌲"
#define BURNING_TREE "🔥"
#define EMPTY "🚫"

// 1(TREE) 2(EMPTY) 3(BURNING TREE)

void generation(int N, char **matrix){
    srand(42);
    for(int i=0; i < N; i++){
        for(int j =0; j < N; j++){
            int randValue = (rand() % 101);
            if(randValue <= F){
                (*matrix)[(i*N)+j] = '3';
            }else if(randValue > F && randValue <= P){
                (*matrix)[(i*N)+j] = '1';
            }else{
                (*matrix)[(i*N)+j] = '2';
            }
        }
    }
}

void generationDeterministic(int N, char **matrix){
    for(int i = 0; i < N; i++){
        for(int j = 0; j < N; j++){
            if((i % 3) == 0){
                (*matrix)[(i*N)+j] = '1'; //TREE
            }else if((i % 3) == 1){
                (*matrix)[(i*N)+j] = '2'; //EMPTY
            }else{
                (*matrix)[(i*N)+j] = '3'; //BURNING TREE
            }
        }
    }
}
/**
 *
 * @param N Matrix dimension
 * @param matrix Forest Matrix
 * @return 0 if matrix is empty 1 otherwise
 */
int isEmpty(int N, char *matrix, int rank){
    int flag = 0;
    for(int i = 0; i < N; i++){
        for( int j = 0; j < N; j++){
            if(matrix[(i*N)+j] != '2'){
                flag = 1;
                //printf("Matrice %d ok\n",rank);
                return flag;
            }
        }
    }
    printf("Foresta %d bruciata\n",rank);
    return flag;
}

void print_forest(int N, char *matrix, int I){
    char *fileName = (char*) malloc(sizeof(char)*25); //Dim of the fileName string
    sprintf(fileName,"master_matrix_day_%d.txt",I);
    FILE *fp = fopen(fileName,"a");

    fprintf(fp,"--------------------\\DAY %d/------------------------------\n",I);
    printf("--------------------\\DAY %d/------------------------------\n",I);
    for(int i=0; i < N; i++){
        fprintf(fp,"--------------------------------------------------\n");
        printf("--------------------------------------------------\n");
        for(int j=0; j < N; j++){
            if(matrix[(i*N)+j] == '1'){
                fprintf(fp,"| %s |",TREE);
            }else if(matrix[(i*N)+j] == '2'){
                fprintf(fp,"| %s |",EMPTY);
            }else if(matrix[(i*N)+j] == '3'){
                fprintf(fp,"| %s |",BURNING_TREE);
            }
            printf("| %c |",matrix[(i*N)+j]);
        }
        fprintf(fp,"\n");
        printf("\n");
    }
    fprintf(fp,"--------------------------------------------------\n");
    printf("--------------------------------------------------\n");
}

void print_forest_array(int N, char * temp,int rank){
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

void divWork2(int N, int size, int** sendCount, int** displacement){
    int numberOfRow = N / (size - 1);
    int restVal = N % (size - 1);

    int pos = 0;
    (*displacement)[0] = 0;
    (*sendCount)[0] = 0; //Al processo master non dò nulla da fare.

    for(int i = 1; i < size;i++){
        (*sendCount)[i] = (numberOfRow*N);
        if(restVal > 0){
            (*sendCount)[i] += N;
            restVal--;
        }
        //Calcolo il displacement per la scatterv
        (*displacement)[i] = pos;
        pos = pos + (*sendCount)[i];
        //printf("Displ[%d]:%d \n",i,(*displacement)[i]);
        //printf("SendCount[%d]: %d \n ",i,(*sendCount)[i]);
    }
}

char* prepareForCheck(int N, char* preNeighbor,char* recvBuff,char* destNeighbor,int* sendCount,int my_rank,int prec, int dest,int* total){
        char *arr;
        if(prec == -10){
            arr = calloc((sendCount[my_rank] + N),sizeof(char) );
        }else if(dest == -10){
            arr = calloc((sendCount[my_rank] + N),sizeof(char));
        }else{
            arr = calloc((N + sendCount[my_rank] + N),sizeof(char));
        }
        if(prec != -10){
            memcpy(arr,preNeighbor,N);
            *total += N;
            memcpy(arr+N,recvBuff,sendCount[my_rank]);
            *total += sendCount[my_rank];
            if(dest != -10){
                memcpy(arr+N+sendCount[my_rank],destNeighbor, N);
                *total += N;
            }
        }else{
            memcpy(arr, recvBuff, sendCount[my_rank]);
            *total += sendCount[my_rank];
            memcpy(arr+sendCount[my_rank],destNeighbor,N);
            *total += N;
        }
        printf("rank:%d arr:%s\n",my_rank,arr);
        return arr;
}

char* check(int N, char* temp, int* sendCount, int rank,int prec, int dest, int total) {
    int startI = 0;
    int end = 0;
    char *retMatrix = (char*) calloc((N * N),sizeof(char));

    srand(rank + 1); //Annullo il comportamento di srand(1);

    //Starting point
    if (prec == -10) {
        startI = 0;
    } else {
        startI = 1;
    }
    if(prec == -10 || dest == -10){
        end =  (sendCount[rank]/N);
    }else{
        end =  (sendCount[rank]/N);
    }
    printf("rank:%d startI=%d end=%d temp:%s\n",rank,startI,end,temp);

    for (int i = startI; i < (sendCount[rank]/N); i++) {
        for(int j = 0; j < N; j++){
            if (temp[(i*N)+j] == '3') { //1)A burning cell turns into an empty cell - 3 --> 2
                retMatrix[(i*N)+j] = '2';
                //total--;
                continue;
            }

            if (temp[(i*N)+j] == '2') { //4)An empty space fills with a tree with probability P - 2 --> 1
                if ((rand() % 101) <= P) {
                    retMatrix[(i*N)+j] = '1';
                }else{
                    retMatrix[(i*N)+j] = temp[(i*N)+j];
                }
                //total--;
                continue;
                //printf("4) rank = %d retMatrix[%d] = %c\n", rank,(i*N)+j,retMatrix[(i*N)+j]);
            }

            if (prec == -10) {//First Row
                if(j == 0 && (temp[(i*N)+j+1]) != '3' && (temp[((i+1)*N)+j]) != '3' ){ //3)A tree ignites with probability F even if no neighbor is burning
                    if((rand() % 101) <= F){
                        retMatrix[(i*N)+j] = '3';
                    }else{
                        retMatrix[(i*N)+j] = temp[(i*N)+j];
                    }
                    //total--;
                    continue;
                    //printf("3)rank = %d retMatrix[%d] = %c\n", rank,(i*N)+j,retMatrix[(i*N)+j]);
                }else if(j == (N-1) && (temp[(i*N)+(j-1)]) != '3' && (temp[((i+1)*N)+j]) != '3'){
                    if((rand() % 101) <= F){
                        retMatrix[(i*N)+j] = '3';
                    }else{
                        retMatrix[(i*N)+j] = temp[(i*N)+j];
                    }
                    //total--;
                    continue;
                    //printf("3)rank = %d retMatrix[%d] = %c\n", rank,(i*N)+j,retMatrix[(i*N)+j]);
                }else if( temp[((i*N)+j-1)] != '3' && temp[((i*N)+j+1)] != '3' && temp[(((i+1)*N)+j)] != '3'){
                    if((rand() % 101) <= F){
                        retMatrix[(i*N)+j] = '3';
                    }else{
                        retMatrix[(i*N)+j] = temp[(i*N)+j];
                    }
                    //total--;
                    continue;
                    //printf("3)rank = %d retMatrix[%d] = %c\n", rank,(i*N)+j,retMatrix[(i*N)+j]);
                } // 3) END -------------------------------------------------------

                if((j == 0) && ( temp[((i*N)+j+1)] == '3' || temp[(((i+1)*N)+j)] == '3') ){ //2)A tree will burn if at least one neighbor is burning
                    retMatrix[(i*N)+j] = '3';
                    //total--;
                    continue;
                    //printf("2)rank = %d retMatrix[%d] = %c\n", rank,(i*N)+j,retMatrix[(i*N)+j]);
                }else if( (j == (N-1)) && ( temp[((i*N)+(j-1))] == '3' || temp[(((i+1)*N)+j)] ) == '3') {
                    retMatrix[(i*N)+j] = '3';
                    //total--;
                    continue;
                    //printf("2)rank = %d retMatrix[%d] = %c\n", rank,(i*N)+j,retMatrix[(i*N)+j]);
                }else if( temp[((i*N)+j-1)] == '3' || temp[((i*N)+j+1)] == '3' || temp[(((i+1)*N)+j)] == '3'){
                    retMatrix[(i*N)+j] = '3';
                    //total--;
                    continue;
                    //printf("2)rank = %d retMatrix[%d] = %c\n", rank,(i*N)+j,retMatrix[(i*N)+j]);
                }else{
                    retMatrix[(i*N)+j] = temp[(i*N)+j];
                    //total--;
                    continue;
                }// 2) END ---------------------------------------------------------

            } else if (dest == -10) {//Last Row
                if(j == 0 && temp[((i*N)+j+1)] != '3' && temp[(((i-1)*N)+j)] != '3' ){ //3)A tree ignites with probability F even if no neighbor is burning
                    if((rand() % 101) <= F){
                        retMatrix[(i*N)+j] = '3';
                    }else{
                        retMatrix[(i*N)+j] = temp[(i*N)+j];
                    }
                    //total--;
                    continue;
                    //printf("3)rank = %d retMatrix[%d] = %c\n", rank,(i*N)+j,retMatrix[(i*N)+j]);
                }else if(j == (N-1) && temp[((i*N)+(j-1))] != '3' && temp[(((i-1)*N)+j)] != '3'){
                    if((rand() % 101) <= F){
                        retMatrix[(i*N)+j] = '3';
                    }else{
                        retMatrix[(i*N)+j] = temp[(i*N)+j];
                    }
                    //total--;
                    continue;
                    //printf("3)rank = %d retMatrix[%d] = %c\n", rank,(i*N)+j,retMatrix[(i*N)+j]);
                }else if( temp[((i*N)+j-1)] != '3' && temp[((i*N)+j+1)] != '3' && temp[(((i-1)*N)+j)] != '3'){
                    if((rand() % 101) <= F){
                        retMatrix[(i*N)+j] = '3';
                    }else{
                        retMatrix[(i*N)+j] = temp[(i*N)+j];
                    }
                    //total--;
                    continue;
                    //printf("3)rank = %d retMatrix[%d] = %c\n", rank,(i*N)+j,retMatrix[(i*N)+j]);
                } // 3) END -------------------------------------------------------

                if((j == 0) && ( temp[((i*N)+j+1)] == '3' || temp[(((i-1)*N)+j)] == '3') ){ //2)A tree will burn if at least one neighbor is burning
                    retMatrix[(i*N)+j] = '3';
                    //total--;
                    continue;
                    //printf("2)rank = %d retMatrix[%d] = %c\n", rank,(i*N)+j,retMatrix[(i*N)+j]);
                }else if( (j == (N-1)) && ( temp[((i*N)+(j-1))] == '3' || temp[(((i-1)*N)+j)] == '3' ) ) {
                    retMatrix[(i*N)+j] = '3';
                    //total--;
                    continue;
                    //printf("2)rank = %d retMatrix[%d] = %c\n", rank,(i*N)+j,retMatrix[(i*N)+j]);
                }else if(temp[((i*N)+j-1)] == '3' || temp[((i*N)+j+1)] == '3' || temp[(((i-1)*N)+j)] == '3'){
                    retMatrix[(i*N)+j] = '3';
                    //total--;
                    continue;
                    //printf("2)rank = %d retMatrix[%d] = %c\n", rank,(i*N)+j,retMatrix[(i*N)+j]);
                }else{
                    retMatrix[(i*N)+j] = temp[(i*N)+j];
                    //total--;
                    continue;
                }// 2) END ---------------------------------------------------------

            } else {
                if(j == 0 && temp[((i*N)+j+1)] != '3' && temp[(((i+1)*N)+j)] != '3' && temp[(((i-1)*N)+j)] != '3' ){ //3)A tree ignites with probability F even if no neighbor is burning
                    if((rand() % 101) <= F){
                        retMatrix[(i*N)+j] = '3';
                    }else{
                        retMatrix[(i*N)+j] = temp[(i*N)+j];
                    }
                    //total--;
                    continue;
                    //printf("3)rank = %d retMatrix[%d] = %c\n", rank,(i*N)+j,retMatrix[(i*N)+j]);
                }else if(j == (N-1) && temp[((i*N)+(j-1))] != '3' && temp[(((i+1)*N)+j)] != '3' && temp[(((i-1)*N)+j)] != '3'){
                    if((rand() % 101) <= F){
                        retMatrix[(i*N)+j] = '3';
                    }else{
                        retMatrix[(i*N)+j] = temp[(i*N)+j];
                    }
                    //total--;
                    continue;
                    //printf("3)rank = %d retMatrix[%d] = %c\n", rank,(i*N)+j,retMatrix[(i*N)+j]);
                }else if( temp[((i*N)+j-1)] != '3' && temp[((i*N)+j+1)] != '3' && temp[(((i+1)*N)+j)] != '3' && temp[(((i-1)*N)+j)] != '3'){
                    if((rand() % 101) <= F){
                        retMatrix[(i*N)+j] = '3';
                    }else{
                        retMatrix[(i*N)+j] = temp[(i*N)+j];
                    }
                    //total--;
                    continue;
                    //printf("3)rank = %d retMatrix[%d] = %c\n", rank,(i*N)+j,retMatrix[(i*N)+j]);
                } // 3) END -------------------------------------------------------

                if((j == 0) && ( temp[((i*N)+j+1)] == '3' || temp[(((i-1)*N)+j)] == '3' || temp[(((i+1)*N)+j)] == '3') ){ //2)A tree will burn if at least one neighbor is burning
                    retMatrix[(i*N)+j] = '3';
                    //total--;
                    continue;
                    //printf("2)rank = %d retMatrix[%d] = %c\n", rank,(i*N)+j,retMatrix[(i*N)+j]);
                }else if( (j == (N-1)) && ( temp[((i*N)+(j-1))] == '3' || temp[(((i-1)*N)+j)] == '3' || temp[(((i+1)*N)+j)] == '3' ) ) {
                    retMatrix[(i*N)+j] = '3';
                    //total--;
                    continue;
                    //printf("2)rank = %d retMatrix[%d] = %c\n", rank,(i*N)+j,retMatrix[(i*N)+j]);
                }else if(temp[((i*N)+j-1)] == '3' || temp[((i*N)+j+1)] == '3' || temp[(((i-1)*N)+j)] == '3' || temp[(((i+1)*N)+j)] == '3'){
                    retMatrix[(i*N)+j] = '3';
                    //total--;
                    continue;
                    //printf("2)rank = %d retMatrix[%d] = %c\n", rank,(i*N)+j,retMatrix[(i*N)+j]);
                }else{
                    retMatrix[(i*N)+j] = temp[(i*N)+j];
                    //total--;
                    continue;
                }// 2) END ---------------------------------------------------------

            }
            total--;
        }
    }
    //print_forest_array(N,retMatrix,rank);
    if(startI == 1){
        printf("rank:%d ret:%s\n",rank,retMatrix+N);
    }else{
        printf("rank:%d ret:%s\n",rank,retMatrix);
    }

    return retMatrix;
    /*if(prec == -10){
        return retMatrix;
    }else if(dest == -10){
        return (retMatrix+N);
    }else{
        return (retMatrix+N);
    }*/
}