#ifndef UDP_CLIENT_H
#define UDP_CLIENT_H

// Funções que o p2pnet usa para falar com o servidor UDP
int do_REG(int tcpPort);
int do_PEERS(char* buffer, int size);
int do_UNR(int seq);

#endif
