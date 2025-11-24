#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "menu.c"

#include "TAD_GRAFO_PEERS.c"



// Variáveis globais (acessadas pelo join.c e menu.c)
graph* rede = NULL;
peer* eu = NULL;
int mySeq = -1;
int LNKPORT ;  // porta deste peer
int Nplus ;       // limite de vizinhos externos
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
}

int main(int argc, char *argv[]) {
	  parseArgs(argc, argv); 
    rede = createGraph();   // cria grafo da overlay

    char comando[100];

    while (1) {
        mostrarMenu();  // imprime o menu

        printf("> ");
        fgets(comando, sizeof(comando), stdin);
        comando[strcspn(comando, "\n")] = 0;

        processarOpcao(comando);   // passa o comando inteiro
    }

    return 0;
}
