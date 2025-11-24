#ifndef GRAFO_H
#define GRAFO_H

typedef struct ServPeer ServPeer;
typedef struct PeerServer PeerServer;


// Protótipos das funções
void initServer(PeerServer* server);
ServPeer* registerPeer(PeerServer* server, char* ip, int tcpPort);
int removePeer(PeerServer* server, int seq);
void listPeers(PeerServer* server);

#endif