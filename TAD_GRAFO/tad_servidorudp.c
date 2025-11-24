#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "tad_servidorudp.h"

#define MAX_PEERS 100

struct ServPeer {
    int seq; // Seqnumber
    char ip[16];
    int tcpPort;
    int active; //1 = slot ocupado ; 0 = slot vazio; -1 =slot removido
};

struct PeerServer {
    ServPeer peers[MAX_PEERS];
    int numPeers; //quantos peers estão registados agora
    int nextSeq; // próximo seqnumber a atribuir
};


void initServer(PeerServer* server) {
    server->numPeers = 0;
    server->nextSeq = 1;
    for (int i = 0; i < MAX_PEERS; i++) {
        server->peers[i].active = 0;
    }
}

ServPeer* registerPeer(PeerServer* server, char* ip, int tcpPort) {
    // verificar se já existe peer com mesmo IP e porta ativo
    for (int i = 0; i < server->numPeers; i++) {
        if (server->peers[i].active == 1 &&
            strcmp(server->peers[i].ip, ip) == 0 &&
            server->peers[i].tcpPort == tcpPort) {
            return NULL; // já existe, não regista de novo
        }
    }

    if (server->numPeers >= MAX_PEERS) return NULL;

    ServPeer* p = &server->peers[server->numPeers++];
    p->seq = server->nextSeq++;
    strcpy(p->ip, ip);
    p->tcpPort = tcpPort;
    p->active = 1;
    return p;
}


int removePeer(PeerServer* server, int seq) {
    for (int i = 0; i < server->numPeers; i++) {
        if (server->peers[i].seq == seq && server->peers[i].active) {
            server->peers[i].active = -1;
            return 1; // OK
        }
    }
    return 0; // NOK
}

void listPeers(PeerServer* server) {
    printf("LST\n");
    for (int i = 0; i < server->numPeers; i++) {
        if (server->peers[i].active) {
            printf("%s:%d#%d\n", server->peers[i].ip,
                                 server->peers[i].tcpPort,
                                 server->peers[i].seq);
        }
    }
}

