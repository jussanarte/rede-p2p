#ifndef TAD_GRAFO_PEERS_H
#define TAD_GRAFO_PEERS_H

typedef struct Graph graph;
typedef struct Peer peer;
typedef struct Vizinho vizinho;


// ---------- prototipos das funcoes ----------
// cria vizinho
vizinho* createVizinho(int seq, const char* ip, int port);
// a adiciona b como externo, b adiciona a como interno
void addLigacao(peer* a, peer* b);
// operacoes basicas sobre vizinhos
void removeVizinho(vizinho** head, int seq);
void removeLigacao(peer* a, peer* b);
// imprime vizinhos de um peer (internos/externos)
void printVizinho(peer* p);
// operacoes sobre o grafo
graph* createGraph(void);
peer*  addPeer(graph* g, int seq, const char* ip, int tcpPort);
peer*  findPeer(graph* g, int seq);

#endif
