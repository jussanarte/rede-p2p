typedef struct Graph graph;
typedef struct Peer peer;
typedef struct Vizinho vizinho;

// ---------- protótipos das funções ----------

// cria vizinho
vizinho* createVizinho(int seq, const char* ip, int port);

// a adiciona b como externo, b adiciona a como interno
void addLigacao(peer* a, peer* b);

// operações básicas sobre vizinhos
void removeVizinho(vizinho** head, int seq);
void removeLigacao(peer* a, peer* b);

// imprime vizinhos de um peer (internos/externos)
void printVizinho(peer* p);

// operações sobre o grafo
graph* createGraph(void);
peer*  addPeer(graph* g, int seq, const char* ip, int tcpPort);
peer*  findPeer(graph* g, int seq);

