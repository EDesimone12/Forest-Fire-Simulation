#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <wchar.h>

#define TAG 42
#define F 100 //10    //Ignite probability F(Fire)
#define P 100 //70       //New Tree probability

#define TREE "🌲"
#define BURNING_TREE "🔥"
#define EMPTY "🚫"

// 1(TREE) 2(EMPTY) 3(BURNING TREE)

void generation(int N, char **matrix){
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

void generationDeterministic(int N, char **matrix){
    for(int i = 0; i < N; i++){
        for(int j = 0; j < N; j++){
            if((i % 2) == 0){
                matrix[i][j] = '1';
            }else{
                matrix[i][j] = '2';
            }
        }
    }
}

void print_forest(int N, char **matrix){
    FILE *fp = fopen("master_matrix.txt","w+a");

    fprintf(fp,"--------------------\\/\\/------------------------------\n");
    printf("--------------------\\/\\/------------------------------\n");
    for(int i=0; i < N; i++){
        fprintf(fp,"--------------------------------------------------\n");
        printf("--------------------------------------------------\n");
        for(int j=0; j < N; j++){
            if(matrix[i][j] == '1'){
                fprintf(fp,"| %s |",TREE);
            }else if(matrix[i][j] == '2'){
                fprintf(fp,"| %s |",EMPTY);
            }else if(matrix[i][j] == '3'){
                fprintf(fp,"| %s |",BURNING_TREE);
            }
            printf("| %c |",matrix[i][j]);
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

void divWork(int N, int size,int** sendCount, int** displacement){
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

char* prepareForCheck(int N, char* preNeighbor,char* recvBuff,char* destNeighbor,int* sendCount,int my_rank,int prec, int dest,int* total){
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

char* check(int N, char** temp, int* sendCount, int rank,int prec, int dest, int total) {
    int startI = 0;
    int startJ = 0;
    int flag = 1;
    char *retMatrix = malloc(sizeof *retMatrix * N * N);

    srand(rank + 1); //Annullo il comportamento di srand(1);

    //Starting point
    if (prec == -10) {
        startI = 0;
        startJ = 0;
    } else {
        startI = (sendCount[prec] / N);
        startJ = (sendCount[prec] % N);
    }

    for (int i = startI; i < N && total != 0; i++) {
        for(int j = 0; j < N; j++){
            if(flag){
                j = startJ;
                flag = 0;
            }
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
    print_forest_array(N,retMatrix,rank);
    return retMatrix;
}