

#include <stdio.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netinet/in.h>

#include "leave.h"
#include "graph.h"
#include "tcp_server.h"
#include "udp_client.h"

extern graph* rede;
extern peer* eu;
extern int mySeq;
extern int listen_fd;


void enviar_UNL(const char* ip, int port, int mySeq) {
    int fd = socket(AF_INET, SOCK_STREAM, 0);
    if (fd < 0) return;

    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    addr.sin_addr.s_addr = inet_addr(ip);

    if (connect(fd, (struct sockaddr*)&addr, sizeof(addr)) == 0) {
        char msg[32];
        snprintf(msg, sizeof(msg), "UNL %d\n", mySeq);
        write(fd, msg, strlen(msg));
    }

    close(fd);
}

void leave() {
    if (mySeq == -1) {
        printf("Ainda não estás registado.\n");
        return;
    }
    
    
     if (listen_fd > 0) {
		    close(listen_fd);
		    listen_fd = -1;
		}


    printf("--- LEAVE INICIADO ---\n");

    // 1) Avisar servidor
    if (do_UNR(mySeq) == 0)
        printf("[OK] Removido do servidor de peers.\n");
    else
        printf("[ERRO] Falha ao remover no servidor.\n");

    // 2) Avisar vizinhos internos
    for (vizinho* v = eu->internos; v != NULL; v = v->next) {
        enviar_UNL(v->ip, v->port, mySeq);
    }

    // 3) Avisar vizinhos externos
    for (vizinho* v = eu->externos; v != NULL; v = v->next) {
        enviar_UNL(v->ip, v->port, mySeq);
    }


		  
    // 4) Agora remover as ligações no grafo local
    while (eu->internos != NULL) {
        vizinho* v = eu->internos;
        peer* outro = findPeer(rede, v->seq);
        if (outro)
            removeLigacao(outro, eu);
        removeVizinho(&eu->internos, v->seq);
    }

    while (eu->externos != NULL) {
        vizinho* v = eu->externos;
        peer* outro = findPeer(rede, v->seq);
        if (outro)
            removeLigacao(eu, outro);
        removeVizinho(&eu->externos, v->seq);
    }

    // 5) Reset local
    mySeq = -1;
    eu = NULL;

    // NÃO limpar rede inteira: rede->numPeers = 0 está ERRADO!
}
