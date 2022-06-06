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
Per configurare l'ambiente di sviluppo è stato utilizzato un container Docker creato a partire da 
```
git clone https://github.com/spagnuolocarmine/docker-mpi.git
cd docker-mpi
docker build --no-cache -t dockermpi .
docker run -it -t dockermpi:latest
```
L'ambiente effettivo di esecuzione ha comportato invece la creazione di un Cluster omogeneo formato da N macchine.
É stato utilizzato [GCP(Google Cloud Platform)](https://cloud.google.com) per la creazione del cluster composto da 6 macchine __e2-standard-4(4 vCPU, 16GB di Memoria)__.

La configurazione del cluster è stata realizzata mediante la seguente guida `https://github.com/spagnuolocarmine/ubuntu-openmpi-openmp`
## Soluzione Proposta
L'algoritmo è stato implementato attraverso il Linguaggio C ed [OpenMPI](https://www.open-mpi.org/doc/), un'implementazione dello standard [MPI(Message Passing Interface)](https://it.wikipedia.org/wiki/Message_Passing_Interface#Implementazioni)

L'algoritmo prende in input N ed I, rispettivamente:
* N - Dimensione della Matrice NxN
* I - Numero di Iterazioni dell'algoritmo sulla Foresta a meno di terminazioni anticipate

## Analisi del Codice
## Analisi Performance
## Conclusione
