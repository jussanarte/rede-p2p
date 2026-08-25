CC = gcc
CFLAGS = -Wall -Wextra -g

all: p2pnet servidorUDP

# ---- Cliente P2P ----
CLIENT_SRCS = client/main.c client/menu.c client/join.c client/leave.c \
              client/neighbors.c client/tcp_client.c client/tcp_server.c \
              client/udp_client.c client/graph.c

p2pnet: $(CLIENT_SRCS)
	$(CC) $(CFLAGS) -o $@ $^

# ---- Servidor UDP ----
SERVER_SRCS = server/main.c server/peer_store.c

servidorUDP: $(SERVER_SRCS)
	$(CC) $(CFLAGS) -o $@ $^

clean:
	rm -f p2pnet servidorUDP client/*.o server/*.o

.PHONY: all clean
