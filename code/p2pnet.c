#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <sys/select.h>
#include <sys/time.h>
#include <termios.h>


#include "menu.c"
#include "TAD_GRAFO_PEERS.c"

// Variáveis globais (acessadas pelo join.c e menu.c)
graph* rede = NULL;
peer* eu = NULL;
int mySeq = -1;
int LNKPORT = 0;
 int Nplus = 0;
int listen_fd = -1;
extern char SERVER_IP[32];
extern int SERVER_UDP_PORT;

// =======================================================
// PARSE ARGS
// =======================================================
void parseArgs(int argc, char *argv[]) {

    for (int i = 1; i < argc; i++) {

        if (strcmp(argv[i], "-l") == 0 && i + 1 < argc) {
            LNKPORT = atoi(argv[++i]);
        }

        else if (strcmp(argv[i], "-n") == 0 && i + 1 < argc) {
            Nplus = atoi(argv[++i]);
        }

        else if (strcmp(argv[i], "-s") == 0 && i + 1 < argc) {
            strcpy(SERVER_IP, argv[++i]);
        }

        else if (strcmp(argv[i], "-p") == 0 && i + 1 < argc) {
            SERVER_UDP_PORT = atoi(argv[++i]);
        }
    }

    if (LNKPORT == 0) {
        printf("ERRO: tens de indicar uma porta TCP com -l\n");
        exit(1);
    }
}

// =======================================================
// MAIN COM SELECT()
// =======================================================
int main(int argc, char *argv[]) {

    parseArgs(argc, argv);

    rede = createGraph();

    fd_set readfds;
    int maxfd;
    char comando[128];

    printf("Peer iniciado. Porta TCP: %d | Nplus=%d\n", LNKPORT, Nplus);
    mostrarMenu();

    while (1) {

        FD_ZERO(&readfds);

        // 1) Teclado
        FD_SET(STDIN_FILENO, &readfds);
        maxfd = STDIN_FILENO;

        // 2) Servidor TCP ativo?
        if (listen_fd != -1) {
            FD_SET(listen_fd, &readfds);
            if (listen_fd > maxfd) maxfd = listen_fd;
        }

        // Esperar eventos
        if (select(maxfd + 1, &readfds, NULL, NULL, NULL) < 0) {
            perror("select");
            continue;
        }

        // --- EVENTO: TECLADO ---
        if (FD_ISSET(STDIN_FILENO, &readfds)) {
            fgets(comando, sizeof(comando), stdin);
            comando[strcspn(comando, "\n")] = 0;
            processarOpcao(comando);
        }

        // --- EVENTO: NOVA LIGAÇÃO TCP ---
       if (listen_fd != -1 && FD_ISSET(listen_fd, &readfds)) {

    		tcp_server_loop(listen_fd);
	
	    // limpa qualquer input “sujo” que tenha ficado na linha
	    tcflush(STDIN_FILENO, TCIFLUSH);
	
	    // reimprime o menu porque o TCP “cortou” a linha do utilizador
	    mostrarMenu();
}

    }

    return 0;
}
