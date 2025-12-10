#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "TAD_SERVIDORUDP.c"

#include <strings.h>      // funções de manipulação de strings (bzero, etc.)
#include <sys/types.h>    // tipos básicos usados em sockets
#include <arpa/inet.h>    // funções para endereços IP (htonl, htons, inet_ntop, etc.)
#include <sys/socket.h>   // funções de socket (socket, bind, sendto, recvfrom)
#include <netinet/in.h>   // estruturas específicas para endereços de Internet (struct sockaddr_in)

#define SERVER_PORT 58000
#define MAXLINE 1024

int main(int argc, char *argv[]) {
    int sockfd;
    struct sockaddr_in servaddr, cliaddr;// ONDE DEFINIMOS O ENDERECO DO SERVIDOR
    char buffer[MAXLINE];

    // cria socket UDP
    sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sockfd < 0) {
        perror("socket creation failed");
        exit(1);
    }

    // configura endereço do servidor
    bzero(&servaddr, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(SERVER_PORT);
    servaddr.sin_addr.s_addr = htonl(INADDR_ANY);

    // associa socket à porta
    if (bind(sockfd, (struct sockaddr*)&servaddr, sizeof(servaddr)) < 0) {
        perror("bind failed");
        exit(1);
    }

    // inicializa estrutura do servidor de peers
    PeerServer server;
    initServer(&server);

    printf("Servidor UDP ativo na porta self %d...\n", SERVER_PORT);

    // loop principal
     while (1) {
     	
    socklen_t len = sizeof(cliaddr);
    int n = recvfrom(sockfd, buffer, MAXLINE-1, 0,
                     (struct sockaddr*)&cliaddr, &len);
    buffer[n] = '\0';
    printf("Mensagem recebida: %s\n", buffer);

		    if (strncmp(buffer, "REG", 3) == 0) {
		    int tcpPort;
		    if (sscanf(buffer, "REG %d", &tcpPort) == 1) {
		        char ip[32];
		        inet_ntop(AF_INET, &cliaddr.sin_addr, ip, sizeof(ip));
		        ServPeer* p = registerPeer(&server, ip, tcpPort);
		        if (p != NULL) {
		            char resposta[32];
		            snprintf(resposta, sizeof(resposta), "SQN %d\n", p->seq);
		            sendto(sockfd, resposta, strlen(resposta), 0,
		                   (struct sockaddr*)&cliaddr, len);
		        } else {
		            sendto(sockfd, "NOK\n", 4, 0,
		                   (struct sockaddr*)&cliaddr, len);
		        }
		    } else {
		        sendto(sockfd, "NOK\n", 4, 0,
		               (struct sockaddr*)&cliaddr, len);
		    }
				} else if (strncmp(buffer, "PEERS", 5) == 0) {
			    char resposta[2048];
			    int pos = 0;
			
			    // começa com LST\n
			    pos += snprintf(resposta + pos, sizeof(resposta) - pos, "LST\n");
			
			    int ativos = 0;
			    for (int i = 0; i < server.numPeers; i++) {
			        ServPeer* p = &server.peers[i];
			        if (p->active == 1) {
			            pos += snprintf(resposta + pos, sizeof(resposta) - pos,
			                            "%s:%d#%d\n", p->ip, p->tcpPort, p->seq);
			            ativos++;
			        }
			    }
			
			    if (ativos == 0) {
			        // nenhum peer → manda NOK
			        sendto(sockfd, "NOK\n", 4, 0,
			               (struct sockaddr*)&cliaddr, len);
			    } else {
			        // manda tudo de uma vez só
			        sendto(sockfd, resposta, pos, 0,
			               (struct sockaddr*)&cliaddr, len);
			    }
			}
 else if (strncmp(buffer, "UNR", 3) == 0) {
		    int seqNumber;
		    if (sscanf(buffer, "UNR %d", &seqNumber) == 1) {
		        int ok = removePeer(&server, seqNumber);
		        if (ok) {
		            sendto(sockfd, "OK\n", 3, 0,
		                   (struct sockaddr*)&cliaddr, len);
		        } else {
		            sendto(sockfd, "NOK\n", 4, 0,
		                   (struct sockaddr*)&cliaddr, len);
		        }
		    } else {
		        sendto(sockfd, "NOK\n", 4, 0,
		               (struct sockaddr*)&cliaddr, len);
		    }
}

}

		

	 


    return 0;
}
