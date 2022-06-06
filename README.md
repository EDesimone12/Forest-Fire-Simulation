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


## Soluzione Proposta
## Analisi del Codice
## Analisi Performance
## Conclusione
