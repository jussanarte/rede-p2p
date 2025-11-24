#ifndef TAD_SERVIDORUDP_H
#define TAD_SERVIDORUDP_H

typedef struct ServPeer ServPeer;
typedef struct PeerServer PeerServer;

void initServer(PeerServer* server);
ServPeer* registerPeer(PeerServer* server, char* ip, int tcpPort);
int removePeer(PeerServer* server, int seq);
void listPeers(PeerServer* server);

#endif
