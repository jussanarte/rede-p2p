# 🏗️ ARCHITECTURE.md — Arquitetura do rede-p2p

## Visão geral

O sistema compõe-se de dois programas independentes:

1. **`servidorUDP`** — Servidor de peers centralizado via UDP
2. **`p2pnet`** — Peer (cliente + servidor TCP) que participa na rede sobreposta

```
┌─────────────────────────────────────────────────────────────────┐
│                        REDE SOBREPOSTA                         │
│                                                                 │
│   ┌──────────┐         TCP          ┌──────────┐               │
│   │  Peer A  │◄────────────────────►│  Peer B  │               │
│   │ (p2pnet) │   LNK/FRC/UNL/QRY   │ (p2pnet) │               │
│   └────┬─────┘                      └────┬─────┘               │
│        │                                  │                     │
│        │              TCP                 │                     │
│        │         ┌──────────┐             │                     │
│        └────────►│  Peer C  │◄────────────┘                     │
│                  │ (p2pnet) │                                   │
│                  └────┬─────┘                                   │
│                       │                                         │
└───────────────────────┼─────────────────────────────────────────┘
                        │
                        │ UDP
                        ▼
              ┌─────────────────┐
              │  Servidor UDP   │
              │ (servidorUDP)   │
              │  192.168.56.21  │
              │  porta 58000    │
              └─────────────────┘
```

## Componentes

### 1. Servidor UDP (`serverUDP.c` + `TAD_SERVIDORUDP.c/h`)

**Responsabilidade:** Gestão centralizada do registo de peers.

**Protocolo UDP (porta 58000):**
- `REG lnkport` → Registra peer, responde `SQN seqnumber`
- `PEERS` → Lista todos os peers ativos, responde `LST\nIP:port#seq\n...`
- `UNR seqnumber` → Remove peer, responde `OK` ou `NOK`

**Estrutura de dados:**
```c
struct PeerServer {
    ServPeer peers[MAX_PEERS];  // Array de slots (max 100)
    int numPeers;               // Contador de peers registados
    int nextSeq;                // Próximo seqnumber a atribuir
};

struct ServPeer {
    int seq;        // Seqnumber (ID único)
    char ip[16];    // Endereço IP
    int tcpPort;    // Porta TCP de escuta
    int active;     // 1=ativo, 0=vazio, 0=removido
};
```

**Nota:** O servidor UDP é **stateless no que diz respeito às ligações** — apenas guarda quem está registado. As ligações TCP entre peers são geridas inteiramente pelos próprios peers.

---

### 2. Peer — Cliente P2P (`p2pnet.c`)

**Responsabilidade:** Interface de utilizador + multiplexação de eventos.

**Arquitetura interna do peer:**

```
p2pnet.c (main)
│
├── parseArgs()           → Lê argumentos (-s, -p, -l, -n)
├── createGraph()         → Inicializa estrutura de grafo
│
└── Loop principal (select)
    │
    ├── STDIN_FILENO      → Lê comando do utilizador
    │   └── processarOpcao()  → Dispatch para join/leave/show/exit
    │
    └── listen_fd         → Nova ligação TCP recebida
        └── tcp_server_loop()  → Processa LNK/FRC/UNL
```

**Fluxo do `select()`:**
1. `FD_ZERO` — Limpa o set
2. `FD_SET(STDIN_FILENO)` — Aguarda input do teclado
3. `FD_SET(listen_fd)` — Aguarda ligações TCP (se servidor ativo)
4. `select()` — Bloqueia até haver evento
5. Processa teclado ou nova ligação TCP

---

### 3. Módulo de Interface (`menu.c/h`)

**Responsabilidade:** Apresentar opções e dispatchar comandos.

**Comandos suportados:**
- `join` → Chama `join()`
- `leave` → Chama `leave()`
- `show neighbors` → Chama `printVizinho(eu)`
- `exit` → Chama `leave()` + `exit(0)`

**Nota:** `showneighbors.c` define `show_neighbors()` mas esta função **nunca é chamada** — `menu.c` usa diretamente `printVizinho()` de `TAD_GRAFO_PEERS.c`.

---

### 4. Módulo Join (`join.c/h`)

**Responsabilidade:** Implementar o protocolo de adesão à rede sobreposta.

**Fluxo:**

```
join()
│
├── 1) REG (UDP)
│   └── do_REG(LNKPORT) → Obtém mySeq
│
├── 2) Criar peer local
│   └── addPeer(rede, mySeq, "192.168.56.11", LNKPORT)
│
├── 3) PEERS (UDP)
│   ├── do_PEERS(buffer) → Obtém lista LST
│   ├── processPeersList(buffer) → Preenche grafo
│   └── limparPeersDesconhecidos(rede, buffer)
│
├── 4) TCP → Tentar ligações (max N+ peers)
│   ├── Para cada peer com seq < mySeq:
│   │   ├── Tentar LNK → send_LNK()
│   │   │   └── Sucesso: addLigacao(eu, p)
│   │   └── Se LNK falhou e FRC disponível:
│   │       └── Tentar FRC → send_FRC()
│   │           └── Sucesso: addLigacao(eu, p)
│   └── No máximo 1 FRC por join
│
└── 5) Iniciar servidor TCP
    └── start_tcp_server(LNKPORT)
```

---

### 5. Módulo Leave (`leave.c/h`)

**Responsabilidade:** Implementar o protocolo de abandono da rede sobreposta.

**Fluxo:**

```
leave()
│
├── 1) Fechar servidor TCP
│   └── close(listen_fd)
│
├── 2) Avisar servidor UDP
│   └── do_UNR(mySeq)
│
├── 3) Avisar vizinhos internos (UNL)
│   └── Para cada v em eu->internos: enviar_UNL()
│
├── 4) Avisar vizinhos externos (UNL)
│   └── Para cada v em eu->externos: enviar_UNL()
│
├── 5) Remover ligações no grafo local
│   ├── Para cada v em eu->internos:
│   │   ├── removeLigacao(outro, eu)
│   │   └── removeVizinho(&eu->internos, v->seq)
│   └── Para cada v em eu->externos:
│       ├── removeLigacao(eu, outro)
│       └── removeVizinho(&eu->externos, v->seq)
│
└── 6) Reset local
    ├── mySeq = -1
    └── eu = NULL
```

---

### 6. Módulo TCP Client (`tcp_client.c/h`)

**Responsabilidade:** Enviar pedidos LNK e FRC a outros peers.

**Funções:**
- `send_LNK(ip, port, mySeq)` → Conecta, envia `LNK seq`, espera `CNF`
- `send_FRC(ip, port, mySeq)` → Conecta, envia `FRC seq`, espera `CNF`

**Protocolo:**
```
Cliente                          Servidor
  │                                │
  │──── connect() ────────────────►│
  │──── "LNK 5\n" ───────────────►│
  │                                │ (verifica numInternos < N-)
  │◄─── "CNF\n" ──────────────────│ (se aceite)
  │──── close() ──────────────────►│
```

---

### 7. Módulo TCP Server (`tcp_server.c/h`)

**Responsabilidade:** Aceitar e processar pedidos LNK, FRC e UNL de outros peers.

**Funções:**
- `start_tcp_server(lnkport)` → Cria socket TCP, bind, listen
- `tcp_server_loop(listen_fd)` → Aceita 1 conexão, processa mensagem

**Fluxo de processamento:**

```
tcp_server_loop()
│
├── accept() → Nova conexão
├── read() → Lê mensagem
│
├── Se "LNK seq":
│   ├── Se numInternos < N-: Aceita (addLigacao), envia CNF
│   └── Senão: Recusa (fecha sessão sem resposta)
│
├── Se "FRC seq":
│   ├── Se numInternos < N-: Aceita (como LNK)
│   └── Se numInternos == N-:
│       ├── Procura vizinho interno com seq > seq do pedido
│       ├── Se encontra: Expulsa vizinho, aceita novo peer
│       └── Se não encontra: Recusa
│
├── Se "UNL seq":
│   └── removeLigacao(remoto, eu)
│
└── Senão: Fecha conexão
```

---

### 8. Módulo UDP Client (`udp_client.c/h`)

**Responsabilidade:** Comunicar com o servidor UDP.

**Funções:**
- `do_REG(tcpPort)` → Envia `REG lnkport`, retorna `seqnumber`
- `do_PEERS(buffer, size)` → Envia `PEERS`, retorna lista
- `do_UNR(seq)` → Envia `UNR seqnumber`, retorna 0 (OK) ou -1 (NOK)

**Detalhe:** Cada chamada cria um novo socket UDP, envia, recebe e fecha. Não mantém estado entre chamadas.

---

### 9. Estrutura de Dados — Grafo de Peers (`TAD_GRAFO_PEERS.c/h`)

**Responsabilidade:** Manter a representação da rede sobreposta local.

**Estruturas:**

```
Graph
├── peers[MAX_VERTICES]     → Array de Peer (max 100)
└── numPeers                → Número de peers registados

Peer
├── seq                     → Seqnumber (ID)
├── ip[16]                  → Endereço IP
├── tcpPort                 → Porta TCP
├── internos (vizinho*)     → Lista ligada de vizinhos internos
├── externos (vizinho*)     → Lista ligada de vizinhos externos
├── numInternos             → Contador de internos
└── numExternos             → Contador de externos

Vizinho (lista ligada)
├── seq                     → Seqnumber do vizinho
├── ip[16]                  → IP do vizinho
├── port                    → Porta TCP do vizinho
└── next                    → Próximo vizinho
```

**Funções principais:**
- `createGraph()` → Aloca e inicializa grafo
- `addPeer(g, seq, ip, port)` → Adiciona peer ao grafo
- `findPeer(g, seq)` → Busca peer por seqnumber
- `addLigacao(a, b)` → a ganha externo (b), b ganha interno (a)
- `removeLigacao(a, b)` → Remove ligação bidirecional
- `printVizinho(p)` → Imprime vizinhos internos/externos
- `limparPeersDesconhecidos(g, lst)` → Remove peers ausentes na lista LST

---

### 10. Estrutura de Dados — Servidor de Peers (`TAD_SERVIDORUDP.c/h`)

**Responsabilidade:** Gerir registo de peers no servidor.

**Funções:**
- `initServer(server)` → Inicializa servidor
- `registerPeer(server, ip, tcpPort)` → Registra novo peer, retorna `ServPeer*`
- `removePeer(server, seq)` → Marca peer como inativo
- `listPeers(server)` → Imprime lista de peers ativos

---

## Mapa de dependências (includes)

```
p2pnet.c
├── menu.c
│   ├── join.c
│   │   ├── udp_client.c
│   │   ├── tcp_client.c
│   │   ├── tcp_server.c
│   │   └── TAD_GRAFO_PEERS.c
│   ├── leave.c
│   │   ├── TAD_GRAFO_PEERS.c
│   │   └── tcp_server.h
│   └── showneighbors.c
│       └── TAD_GRAFO_PEERS.c
└── TAD_GRAFO_PEERS.c

serverUDP.c
└── TAD_SERVIDORUDP.c
```

**Problema:** Todos os `.c` são incluídos via `#include`, resultando em duas unidades de tradução monolíticas (uma para o cliente, outra para o servidor). Isto funciona mas impede compilação modular.

---

## Fluxo de dados — Exemplo: Join completo

```
Peer A (novo)                    Servidor UDP                    Peer B (existente)
     │                                │                                │
     │──── REG 6000 ─────────────────►│                                │
     │◄─── SQN 3 ────────────────────│                                │
     │                                │                                │
     │──── PEERS ────────────────────►│                                │
     │◄─── LST\n192.168.56.11:6000#1 │                                │
     │                                │                                │
     │ (processa lista, adiciona      │                                │
     │  Peer B ao grafo local)        │                                │
     │                                │                                │
     │──── connect() a Peer B ───────┼───────────────────────────────►│
     │──── "LNK 3\n" ───────────────┼───────────────────────────────►│
     │                                │          (verifica numInternos)│
     │◄─── "CNF\n" ─────────────────┼────────────────────────────────│
     │──── close() ──────────────────┼───────────────────────────────►│
     │                                │                                │
     │ (addLigacao(A, B):             │                                │
     │  A ganha B como externo)       │                                │
     │  (B ganha A como interno)      │                                │
```
