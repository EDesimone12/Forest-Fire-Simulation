# Forest-Fire-Simulation
## Introduzione
Il [Forest-Fire-Model](https://en.wikipedia.org/wiki/Forest-fire_model) è un modello di [cellular automata](https://en.wikipedia.org/wiki/Cellular_automaton) definito attraverso una matrice NxN.
Ogni cella della matrice utilizzata in questo modello può assumere i seguenti stati:
* __TREE__
* __EMPTY__
* __BURNING_TREE__

All'interno del progetto rappresentati attraverso:

|ASCII Charachter | Stato | Emoji |
|------------------|------|-------|
|1|TREE|🌲|
|2|EMPTY|🚫|
|3|BURNING_TREE|🔥|

Il modello è definito da 4 regole fondamentali:
1. Una cella in fiamme(__BURNING_TREE__) si trasforma in una cella vuota(__EMPTY__)
2. Un albero(__TREE__) andrà in fiamme se almeno un vicino è in fiamme(__BURNING_TREE__).
3. Un albero(__TREE__) andrà in fiamme con probabilità __*f*__ anche se non ci sono vicini in fiamme(__BURNING_TREE__).
4. In una cella vuota(__EMPTY__) crescerà un albero(__TREE__) con probabilità __*p*__.

### Il concetto di vicinanza utilizzato

>__Quartiere di Von Neumann__
>
>Negli automi cellulari, il quartiere di von Neumann (o 4 quartieri) è classicamente definito su un bidimensionale reticolo quadrato ed è composto da una cella centrale e dalle sue quattro celle adiacenti. Il quartiere prende il nome John von Neumann, che lo ha utilizzato per definire il von Neumann automa cellulare.
>
> <img src="https://github.com/EDesimone12/Forest-Fire-Simulation/blob/main/img/Von_Neumann_neighborhood.png?raw=true" alt="Von_Neumann_neighborhood">

## Configurazione Ambiente ed Esecuzione
### Prerequisiti
* Ubuntu 18.04 LTS
* Docker
Per configurare l'ambiente di sviluppo è stato utilizzato un container Docker creato a partire da:
```
docker run -it --mount src="$(pwd)",target=/home,type=bind spagnuolocarmine/docker-mpi:latest
```
L'ambiente effettivo di esecuzione ha comportato invece la creazione di un Cluster omogeneo formato da N macchine.    
É stato utilizzato [GCP(Google Cloud Platform)](https://cloud.google.com) per la creazione del cluster composto da 6 macchine __e2-standard-4(4 vCPU, 16GB di Memoria)__.

La configurazione del cluster è stata realizzata mediante la seguente guida `https://github.com/spagnuolocarmine/ubuntu-openmpi-openmp`
## Soluzione Proposta
L'algoritmo è stato implementato attraverso il Linguaggio C ed [OpenMPI](https://www.open-mpi.org/doc/), un'implementazione dello standard [MPI(Message Passing Interface)](https://it.wikipedia.org/wiki/Message_Passing_Interface#Implementazioni)

L'algoritmo prende in input N ed I, rispettivamente:
* N - Dimensione della Matrice NxN
* I - Numero di Iterazioni dell'algoritmo sulla Foresta a meno di terminazioni anticipate

Il processo master si occupa della generazione di una matrice NxN che rappresenta la nostra foresta, viene poì calcolato il lavoro che spetta ad ogni processo slave e gli viene inviata la porzione di matrice da analizzare.

Successivamente ogni processo invia in maniera asincrona la propria porzione da analizzare ad i vicini e riceverà quindi dagli altri processi la loro parte.      
Ogni processo(slave) effettua i dovuti controlli sulla porzione di matrice assegnatagli ed invia al master la porzione aggiornata.

## Analisi del Codice
Analizziamo il codice associato alla generazione della foresta.
```c
    //main_2.c
    
    char *forest = malloc(sizeof *forest * N * N); //Starter Forest Matrix

    if(my_rank == 0){
        generation(N,&forest);
    }
```

```c
//myforest.h

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

```
In seguito occorre dividere il lavoro tra i processi slave ed inviargli le porzioni della foresta su cui lavorare.

```c
    //main_2.c
    
    //Calcolo il numero di elementi per ogni processo
    int* sendCount = malloc ( size_p* sizeof(int));
    int* displacement = malloc(size_p * sizeof(int));
```

```c
    //main_2.c
    divWork2(N,size_p,&sendCount,&displacement);
    char *recvBuff = (char*) malloc(sendCount[my_rank]*sizeof(char));

    MPI_Scatterv(forest,sendCount,displacement,MPI_CHAR,recvBuff,sendCount[my_rank],MPI_CHAR,0,MPI_COMM_WORLD);
```

```c
//myforest.h

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
        //printf("Displ[%d]:%d \n",i,displacement[0][i]);
        //printf("SendCount[%d]: %d \n ",i,sendCount[0][i]);
    }
}
```
Ogni processo andrà ad inviare la propria porzione di matrice ad i vicini ed a riceverla dagli altri processi.
```c
        //main_2.c
            
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
```

```c
    //main_2.c
    
    int total = 0;
    char* tempMatrix = prepareForCheck(N,preNeighbor,recvBuff,destNeighbor, sendCount, my_rank,prec,dest,&total);
    sendBuff = check(N, tempMatrix,sendCount,my_rank,prec,dest,total);
```

Qui avviene la costruzione ti una "matrice" temporanea che verrà costruita inserendo la porzione associata al processo corrente ed ai vicini, per semplificare i controlli.

```c
//myforest.h
char* prepareForCheck(int N, char* preNeighbor,char* recvBuff,char* destNeighbor,int* sendCount,int my_rank,int prec, int dest,int* total){
        char *arr;
        if(prec == -10){
            arr = malloc(sizeof *arr * (sendCount[my_rank] + sendCount[dest]));
        }else if(dest == -10){
            arr = malloc(sizeof *arr * (sendCount[prec] + sendCount[my_rank]));
        }else{
            arr = malloc(sizeof *arr * (sendCount[prec] + sendCount[my_rank] + sendCount[dest]));
        }

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
        return arr;
}
```
Successivamente occorre inviare le rispettive porzioni analizzate da ogni processo slave al processo master, che dividerà nuovamente il lavoro tra i processi per la successiva iterazione se possibile(Matrice non vuota).

```c
MPI_Gatherv(sendBuff,sendCount[my_rank],MPI_CHAR,forest,sendCount,displacement,MPI_CHAR,0,MPI_COMM_WORLD);
```
## Correttezza della soluzione
## Benchmark
Di seguito sono mostrate le misurazioni effettuate utilizzando il Cluster omogeneo [già descritto](#configurazione-ambiente-ed-esecuzione) .        

Sono state effettuate le misurazioni per ottenere:
* __Scalabilità Forte__
* __Scalabilità Debole__

### Scalabilità Forte
La scalabilità forte è stata ottenuta analizzando più esecuzioni dell'algoritmo, utilizzando il Cluster configurato, aumentando il numero di processi ad ogni esecuzione, utilizzando quindi da 2 vCPU a 24 vCPU. 

L'input utilizzato per effettuare l'analisi è stato:
* N = 5000
* I = 50

|Numero di Processi|Tempo(s)    |Speedup|
|:----------------:|:----------:|:-----:|
|1                 |27.242525   | 1.00  |
|2                 |20.424715   | 1.33  |
|3                 |13.919035   | 1.95  |
|4                 |11.126861   | 2.44  |
|5                 |9.303328    | 2.92  |
|6                 |7.762574    | 3.51  |
|7                 |6.550036    | 4.15  |
|8                 |5.783624    | 4.71  |
|9                 |5.238248    | 5.20  |
|10                |4.806311    | 5.67  |
|11                |4.596453    | 5.93  |
|12                |4.669662    | 5.84  |
|13                |4.017718    | 6.79  |
|14                |3.678917    | 7.42  |
|15                |3.514501    | 7.76  |
|16                |3.336226    | 8.18  |
|17                |3.172971    | 8.59  |
|18                |3.041431    | 8.96  |
|19                |2.980639    | 9.14  |
|20                |3.128735    | 8.73  |
|21                |2.704358    | 10.08 |
|22                |2.671802    | 10.20 |
|23                |2.584825    | 10.55 |

### Scalabilità Debole
Anche la scalabilità debole è stata ottenuta analizzando più esecuzioni dell'algoritmo sul Cluster. 

L'input utilizzato per effettuare l'analisi è stato:
* N = 400*np
* I = 50

|Numero di Processi|Tempo(s)    |Dimensione Input(N)|Dimensione Input(I)|
|:----------------:|:----------:|:-----------------:|:-----------------:|
|1                 |0.698966    | 400x400           |50                 |   
|2                 |1.189706    | 800x800           |50                 |
|3                 |1.436100    | 1200x1200         |50                 |
|4                 |1.804403    | 1600x1600         |50                 |
|5                 |2.086154    | 2000x2000         |50                 |
|6                 |2.394085    | 2400x2400         |50                 |
|7                 |2.911704    | 2800x2800         |50                 |
|8                 |3.003367    | 3200x3200         |50                 |
|9                 |3.353309    | 3600x3600         |50                 |
|10                |3.781323    | 4000x4000         |50                 |
|11                |4.120756    | 4400x4400         |50                 |
|12                |4.480034    | 4800x4800         |50                 |
|13                |4.845016    | 5200x5200         |50                 |
|14                |5.592357    | 5600x5600         |50                 |
|15                |6.413034    | 6000x6000         |50                 |
|16                |6.103976    | 6400x6400         |50                 |
|17                |7.058765    | 6800x6800         |50                 |
|18                |7.163511    | 7200x7200         |50                 |
|19                |7.486568    | 7600x7600         |50                 |
|20                |7.639772    | 8000x8000         |50                 |
|21                |8.427413    | 8400x8400         |50                 |
|22                |9.056979    | 8800x8800         |50                 |
|23                |9.590614    | 9200x9200         |50                 |
## Conclusione
