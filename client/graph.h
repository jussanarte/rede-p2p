#ifndef GRAPH_H
#define GRAPH_H

typedef struct Graph graph;
typedef struct Peer peer;
typedef struct Vizinho vizinho;

// ---------- criação ----------

vizinho* createVizinho(int seq, const char* ip, int port);

// ---------- ligações ----------

void addLigacao(peer* a, peer* b);
void removeVizinho(vizinho** head, int seq);
void removeLigacao(peer* a, peer* b);

// ---------- impressão ----------

void printVizinho(peer* p);

// ---------- grafo ----------

graph* createGraph(void);
peer*  addPeer(graph* g, int seq, const char* ip, int tcpPort);
peer*  findPeer(graph* g, int seq);
void   limparPeersDesconhecidos(graph* g, const char* lst);
int    count_list(vizinho* v);

#endif
