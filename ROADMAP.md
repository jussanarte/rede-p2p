# 🗺️ ROADMAP.md — Guia de Implementação

> Este ficheiro é um guia passo-a-passo para completar as funcionalidades em falta no projeto.
> Cada passo inclui: o que fazer, onde alterar, e conceitos-chave para aprender.

---

## Fase 0 — Corrigir o Makefile e estrutura de compilação

**Objetivo:** Conseguir compilar o projeto com `make`.

> **Estado:** Esta fase já está concluída. O Makefile existe e os `#include` já foram corrigidos para usar `.h` em vez de `.c`.

### Passo 0.1 — Makefile

O Makefile compila dois alvos independentes (ver ficheiro `Makefile` na raiz).

### Passo 0.2 — Includes corrigidos

Os `#include "arquivo.c"` foram substituídos por `#include "arquivo.h"` em todos os módulos.
#include "TAD_GRAFO_PEERS.c"

// DEPOIS:
#include "menu.h"
#include "TAD_GRAFO_PEERS.h"
```

### Passo 0.3 — Completar headers

Adicionar funções em falta nos headers:

**`graph.h`** — adicionar:
```c
void limparPeersDesconhecidos(graph* g, const char* lst);
int  count_list(vizinho* v);
```

**`tcp_client.h`** — adicionar:
```c
int send_FRC(const char* ip, int port, int mySeq);
```

### Passo 0.4 — Remover include guards trocados

- `graph.h`: adicionar `#ifndef GRAPH_H` / `#define` / `#endif`
- `graph.c`: remover `#ifndef TAD_GRAFO_PEERS_H` (está no sítio errado)
- `peer_store.h`: renomear `GRAFO_H` para `PEER_STORE_H`

### Passo 0.5 — Testar compilação

```bash
make clean && make
```

Se compilar sem erros, avança para a Fase 1.

---

## Fase 1 — Comando `release seqnumber`

**Objetivo:** Permitir ao utilizador remover manualmente uma ligação com um vizinho interno.

**Enunciado (Sec. 2.2):** "O utilizador solicita a remoção da ligação com o vizinho interno de número de seqnumber. Este comando serve para teste da aplicação."

### Passo 1.1 — Adicionar ao menu

Em `client/menu.c`, na função `processarOpcao()`:

```c
else if (strncmp(opcao, "release ", 8) == 0) {
    int seq;
    if (sscanf(opcao + 8, "%d", &seq) == 1) {
        release(seq);
    } else {
        printf("Uso: release <seqnumber>\n");
    }
}
```

### Passo 1.2 — Criar `release.c` e `release.h`

```c
// release.h
#ifndef RELEASE_H
#define RELEASE_H
void release(int seq);
#endif
```

```c
// release.c
#include <stdio.h>
#include "release.h"
#include "graph.h"

extern peer* eu;
extern graph* rede;

void release(int seq) {
    if (eu == NULL) {
        printf("Ainda não estás registado na rede.\n");
        return;
    }

    // Verificar se o peer com esse seq é um vizinho interno
    vizinho* v = eu->internos;
    while (v) {
        if (v->seq == seq) {
            // Encontrou — remover ligação
            peer* outro = findPeer(rede, seq);
            if (outro) {
                removeLigacao(outro, eu);
                removeVizinho(&eu->internos, seq);
                printf("Ligação com peer seq=%d removida.\n", seq);
            } else {
                printf("Peer seq=%d não encontrado no grafo.\n", seq);
            }
            return;
        }
        v = v->next;
    }

    printf("Peer seq=%d não é vizinho interno.\n", seq);
}
```

**Conceito-chave:** Este comando é voluntário — o utilizador remove uma ligação que ele próprio escolhe. Diferente do `leave` (remove tudo) e do FRC (forçado pelo peer remoto).

---

## Fase 2 — Gestão de identificadores (post, unpost, list)

**Objetivo:** Permitir ao peer criar, remover e listar identificadores de conteúdos que conhece.

### Passo 2.1 — Criar estrutura de dados para identificadores

Em `client/graph.c/h`, adicionar ao struct `Peer`:

```c
struct Peer {
    // ... campos existentes ...
    char** identifiers;     // Array de strings (identificadores)
    int numIdentifiers;     // Número de identificadores conhecidos
};
```

Em `createGraph()`, inicializar:
```c
g->peers[i].identifiers = NULL;
g->peers[i].numIdentifiers = 0;
```

### Passo 2.2 — Criar funções de gestão

Em `client/graph.c`, adicionar:

```c
// Adiciona identificador ao peer
int addIdentifier(peer* p, const char* id) {
    if (!p || !id) return -1;

    // Verificar se já existe
    for (int i = 0; i < p->numIdentifiers; i++) {
        if (strcmp(p->identifiers[i], id) == 0)
            return -1; // já existe
    }

    // Redimensionar array
    p->identifiers = realloc(p->identifiers,
                            (p->numIdentifiers + 1) * sizeof(char*));
    p->identifiers[p->numIdentifiers] = strdup(id);
    p->numIdentifiers++;
    return 0;
}

// Remove identificador do peer
int removeIdentifier(peer* p, const char* id) {
    if (!p || !id) return -1;

    for (int i = 0; i < p->numIdentifiers; i++) {
        if (strcmp(p->identifiers[i], id) == 0) {
            free(p->identifiers[i]);
            // Deslocar restantes
            for (int j = i; j < p->numIdentifiers - 1; j++)
                p->identifiers[j] = p->identifiers[j + 1];
            p->numIdentifiers--;
            return 0;
        }
    }
    return -1; // não encontrado
}

// Verifica se peer conhece o identificador
int hasIdentifier(peer* p, const char* id) {
    if (!p || !id) return 0;
    for (int i = 0; i < p->numIdentifiers; i++) {
        if (strcmp(p->identifiers[i], id) == 0)
            return 1;
    }
    return 0;
}
```

**Conceito-chave:** Identificadores são apenas strings partilhadas. O conteúdo propriamente dito não é transferido — apenas o conhecimento do seu identificador.

### Passo 2.3 — Adicionar comandos ao menu

Em `client/menu.c`, adicionar:

```c
else if (strcmp(opcao, "list identifiers") == 0) {
    listIdentifiers();
}
else if (strncmp(opcao, "post ", 5) == 0) {
    postIdentifier(opcao + 5);
}
else if (strncmp(opcao, "unpost ", 7) == 0) {
    unpostIdentifier(opcao + 7);
}
```

### Passo 2.4 — Criar `identifiers.c` e `identifiers.h`

```c
// identifiers.h
#ifndef IDENTIFIERS_H
#define IDENTIFIERS_H
void listIdentifiers(void);
void postIdentifier(const char* id);
void unpostIdentifier(const char* id);
#endif
```

```c
// identifiers.c
#include <stdio.h>
#include <string.h>
#include "identifiers.h"
#include "TAD_GRAFO_PEERS.h"

extern peer* eu;

void listIdentifiers(void) {
    if (eu == NULL) {
        printf("Ainda não estás registado na rede.\n");
        return;
    }
    if (eu->numIdentifiers == 0) {
        printf("Nenhum identificador conhecido.\n");
        return;
    }
    printf("Identificadores conhecidos:\n");
    for (int i = 0; i < eu->numIdentifiers; i++)
        printf("  - %s\n", eu->identifiers[i]);
}

void postIdentifier(const char* id) {
    if (eu == NULL) {
        printf("Ainda não estás registado na rede.\n");
        return;
    }
    if (addIdentifier(eu, id) == 0)
        printf("Identificador '%s' adicionado.\n", id);
    else
        printf("Identificador '%s' já existe ou erro.\n", id);
}

void unpostIdentifier(const char* id) {
    if (eu == NULL) {
        printf("Ainda não estás registado na rede.\n");
        return;
    }
    if (removeIdentifier(eu, id) == 0)
        printf("Identificador '%s' removido.\n", id);
    else
        printf("Identificador '%s' não encontrado.\n", id);
}
```

---

## Fase 3 — Pesquisa de identificadores (search + QRY/FND/NOTFND)

**Objetivo:** Implementar a pesquisa recursiva de identificadores na rede sobreposta com controlo de hop count.

**Enunciado (Sec. 2.5):** "Um peer A pergunta a um peer B se este conhece identifier. Em caso afirmativo, B responde FND; em caso negativo, B responde NOTFND. Se B não tiver o identifier e hopcount > 1, B interroga os seus vizinhos com QRY identifier hopcount-1."

### Passo 3.1 — Criar protocolo QRY/FND/NOTFND no TCP

O TCP server (`client/tcp_server.c`) precisa de processar mensagens `QRY`:

```c
// No tcp_server_loop(), adicionar:
if (sscanf(buffer, "QRY %s %d", identifier, &hc) == 2) {
    handleQRY(conn, identifier, hc);
    close(conn);
    return 0;
}
```

### Passo 3.2 — Criar `search.c` e `search.h`

```c
// search.h
#ifndef SEARCH_H
#define SEARCH_H
void searchIdentifier(const char* id);
#endif
```

```c
// search.c
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include "search.h"
#include "graph.h"

extern peer* eu;
extern graph* rede;
extern int Nplus;

// Envia QRY a um peer específico e retorna 1 (FND) ou 0 (NOTFND)
static int queryPeer(const char* ip, int port, const char* id, int hc) {
    int fd = socket(AF_INET, SOCK_STREAM, 0);
    if (fd < 0) return 0;

    struct sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    addr.sin_addr.s饱 = inet_addr(ip);

    if (connect(fd, (struct sockaddr*)&addr, sizeof(addr)) < 0) {
        close(fd);
        return 0;
    }

    char msg[128];
    snprintf(msg, sizeof(msg), "QRY %s %d\n", id, hc);
    write(fd, msg, strlen(msg));

    char resp[64];
    int n = read(fd, resp, sizeof(resp) - 1);
    close(fd);

    if (n <= 0) return 0;
    resp[n] = '\0';

    return (strncmp(resp, "FND", 3) == 0) ? 1 : 0;
}

void searchIdentifier(const char* id) {
    if (eu == NULL) {
        printf("Ainda não estás registado na rede.\n");
        return;
    }

    // Se já conhece o identificador
    if (hasIdentifier(eu, id)) {
        printf("Já tens conhecimento de '%s'.\n", id);
        return;
    }

    printf("A pesquisar '%s' na rede...\n", id);

    // Procurar em todos os vizinhos (internos e externos)
    int found = 0;

    // Vizinhos externos
    vizinho* v = eu->externos;
    while (v && !found) {
        if (queryPeer(v->ip, v->port, id, Nplus)) {
            found = 1;
        }
        v = v->next;
    }

    // Vizinhos internos (se ainda não encontrou)
    v = eu->internos;
    while (v && !found) {
        if (queryPeer(v->ip, v->port, id, Nplus)) {
            found = 1;
        }
        v = v->next;
    }

    if (found) {
        addIdentifier(eu, id);
        printf("Identificador '%s' encontrado na rede!\n", id);
    } else {
        printf("Identificador '%s' não encontrado na rede.\n", id);
    }
}
```

### Passo 3.3 — Implementar handleQRY no TCP server

```c
// Em tcp_server.c, nova função:
void handleQRY(int conn, const char* id, int hc) {
    // Se conhece o identificador, responde FND
    if (hasIdentifier(eu, id)) {
        char resp[64];
        snprintf(resp, sizeof(resp), "FND %s\n", id);
        write(conn, resp, strlen(resp));
        return;
    }

    // Se hopcount == 1, não pode propagar mais
    if (hc <= 1) {
        write(conn, "NOTFND\n", 7);
        return;
    }

    // Propagar para vizinhos (exceto quem nos perguntou)
    // Nota: numa implementação completa, precisas de saber quem perguntou
    // para não criar loops. Por agora, responde NOTFND.
    write(conn, "NOTFND\n", 7);
}
```

**Conceito-chave — Propagação recursiva:**

```
Peer A (originador)
│ QRY "video.mp4" 3
├──► Peer B: QRY "video.mp4" 2
│    ├──► Peer C: QRY "video.mp4" 1
│    │    └── Não propaga mais (hc=1), responde NOTFND
│    └── Peer D: QRY "video.mp4" 1
│         └── Conhece! Responde FND
└── Peer B responde FND a A
```

**Nota importante:** A propagação recursiva no enunciado exige que o peer intermédio **não guarde** conhecimento do identificador. A implementação acima é simplificada — numa completa, o peer B precisaria de reenviar o QRY para os seus vizinhos (exceto A) e aggregar respostas.

### Passo 3.4 — Adicionar ao menu

```c
else if (strncmp(opcao, "search ", 7) == 0) {
    searchIdentifier(opcao + 7);
}
```

---

## Fase 4 — Argumento `-h hc` (hop count)

**Objetivo:** Suportar o parâmetro `-h` na invocação da aplicação.

### Passo 4.1 — Adicionar variável global

Em `client/main.c`:
```c
int MAX_HC = 5; // valor por omissão
```

### Passo 4.2 — Atualizar parseArgs

```c
else if (strcmp(argv[i], "-h") == 0 && i + 1 < argc) {
    MAX_HC = atoi(argv[++i]);
}
```

### Passo 4.3 — Validar

```c
if (MAX_HC <= 0) {
    printf("ERRO: hop count deve ser maior que zero\n");
    exit(1);
}
```

---

## Fase 5 — Reconexão ao perder vizinhos externos

**Objetivo:** Quando um peer fica sem vizinhos externos, tentar reconectar automaticamente.

**Enunciado (Sec. 2.4):** "Se a remoção da ligação deixar o peer sem vizinhos externos, então a aplicação volta a solicitar ao servidor de peers a lista de todos os peers registados."

### Passo 5.1 — Detetar perda de vizinhos externos

No `client/tcp_server.c`, quando processa `UNL` ou deteta fecho de sessão TCP:

```c
// Após remover ligação:
if (eu->numExternos == 0 && mySeq != -1) {
    printf("[AVISO] Ficaste sem vizinhos externos! A reconectar...\n");
    reconectar();
}
```

### Passo 5.2 — Criar função reconectar

```c
void reconectar(void) {
    char buffer[1024];
    int n = do_PEERS(buffer, sizeof(buffer));
    if (n <= 0 || strncmp(buffer, "LST", 3) != 0) {
        printf("[ERRO] Não foi possível obter lista de peers.\n");
        return;
    }

    processPeersList(buffer);
    limparPeersDesconhecidos(rede, buffer);

    int conectados = 0;
    int frc_usado = 0;

    for (int i = 0; i < rede->numPeers && conectados < Nplus; i++) {
        peer* p = &rede->peers[i];
        if (p->seq == -1 || p->seq >= mySeq || p->tcpPort <= 0)
            continue;

        if (send_LNK(p->ip, p->tcpPort, mySeq) == 0) {
            addLigacao(eu, p);
            conectados++;
        } else if (!frc_usado) {
            if (send_FRC(p->ip, p->tcpPort, mySeq) == 0) {
                addLigacao(eu, p);
                conectados++;
                frc_usado = 1;
            } else {
                frc_usado = 1;
            }
        }
    }

    if (conectados == 0 && eu->numInternos > 0) {
        printf("[AVISO] Não conseguiu reconectar. "
               "A rede pode estar desconexa.\n");
    }
}
```

**Conceito-chave:** O peer que ficou sem vizinhos externos tenta ligar-se a peers com seqnumber inferior (regra do enunciado). Se não conseguir, e tiver vizinhos internos, avisa que a rede pode estar desconexa.

---

## Fase 6 — Limpeza e finalização

> **Estado:** Esta fase já está parcialmente concluída (binários removidos, .vagrant removido, bootstrap_web.sh renomeado).

### Passo 6.1 — Remover binários do repo ✅

```bash
git rm --cached code/p2pnet code/p2pne code/servidorudp code/TAD_GRAFO_PEERS.h.gch
```

### Passo 6.2 — Remover .vagrant/ do tracking ✅

```bash
git rm -r --cached .vagrant/
```

### Passo 6.3 — Atualizar .gitignore ✅

```bash
*.gch
```

### Passo 6.4 — Renomear bootstrap_web.sh ✅

```bash
git mv bootstrap_web.sh bootstrap_server.sh
```

### Passo 6.5 — Commits organizados ✅

```
1. docs: adicionar README, ARCHITECTURE e ROADMAP
2. chore: renomear bootstrap_web.sh para bootstrap_server.sh
3. chore: atualizar .gitignore com binários, .gch e .vagrant
```

---

## Referência: Mapa completo de alterações por ficheiro

| Ficheiro | Fase | Alteração |
|---|---|---|
| `Makefile` | 0 | ✅ Criado |
| `client/*.c` | 0 | ✅ Includes corrigidos (`#include "x.h"` em vez de `.c`) |
| `client/graph.h` | 0 | ✅ Guards + funções em falta adicionadas |
| `client/graph.c` | 0, 2 | ✅ Guard corrigido; identificadores por implementar |
| `server/peer_store.h` | 0 | ✅ Guard corrigido |
| `client/tcp_client.h` | 0 | ✅ `send_FRC` adicionado |
| `client/menu.c` | 1, 2, 3 | Comandos novos por adicionar |
| `client/release.c/h` | 1 | Criar do zero |
| `client/identifiers.c/h` | 2 | Criar do zero |
| `client/search.c/h` | 3 | Criar do zero |
| `client/main.c` | 0, 4 | ✅ Includes corrigos; `-h` por adicionar |
| `client/tcp_server.c` | 3, 5 | QRY handler + reconexão por adicionar |
