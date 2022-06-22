#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <wchar.h>

#define TAG 42
#define F 100//10 //100    //Ignite probability F(Fire)
#define P 100//70 //100       //New Tree probability

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

void burningTree(char* temp, char* recvBuff,int start ,int end,int i, int j, int N){

    if(i != start && recvBuff[((i*N)+j)-N] == '3'){
        temp[(i*N)+j] = '3';
    }
    if(i != (end -1) && recvBuff[((i*N)+j)+N] == '3'){
        temp[(i*N)+j] = '3';
    }
    if(j != 0 && recvBuff[(i*N)+j-1] == '3' ){
        temp[(i*N)+j] = '3';
    }
    if(j != N-1 && recvBuff[(i*N)+j+1] == '3' ){
        temp[(i*N)+j] = '3';
    }

    if((rand() % 101) <= F){
        temp[(i*N)+j] = '3';
    }else{
        temp[(i*N)+j] = '1';
    }
}

void checkMine(char* recvBuff, char* temp, int start, int end, int rank, int prec, int dest,int N){
    for(int i = start; i < end/N; i++){
        for(int j = 0; j < N; j++){
                if(recvBuff[(i*N)+j] == '2'){ // 4) An empty space fills with a tree with probability p
                    if((rand() % 101) <= P ){
                        temp[(i*N)+j] = '1';
                    }else{
                        temp[(i*N)+j] = '2';
                    }
                }else if(recvBuff[(i*N)+j] == '3') { //1) A burning cell turns into an empty cell
                    temp[(i*N)+j] = '2';
                }
        }
    }
}


