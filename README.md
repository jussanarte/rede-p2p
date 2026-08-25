# 📖 rede-p2p

Aplicação **Peer-to-Peer Overlay Network** em **C** que permite a um conjunto de peers manterem uma rede sobreposta construída à base de sessões TCP, suportando registo num servidor UDP, estabelecimento e terminação de ligações, e partilha de conteúdos.

🧪 Desenvolvido para o **Projecto #1 — Peer-to-Peer Overlay Network**, da cadeira de **Redes de Computadores I** do **ISPTEC**.

## 📑 Índice

- [Objetivo](#objetivo)
- [Contexto académico](#contexto-académico)
- [Funcionalidades](#funcionalidades)
- [Protocolos](#protocolos)
- [Estrutura do repositório](#estrutura-do-repositório])
- [Requisitos](#requisitos)
- [Como executar](#como-executar)
- [Exemplo de execução](#exemplo-de-execução)
- [Atribuição](#atribuição)
- [Integrantes](#integrantes)
- [Licença](#licença)

## 🎯 Objetivo

Implementar uma rede sobreposta peer-to-peer onde:

- Um **servidor UDP** gere o registo e listagem de peers na rede.
- Cada **peer** participa na rede sobreposta estabelecendo sessões TCP com outros peers, distinguindo vizinhos internos (que aceitaram a ligação) de vizinhos externos (que iniciaram a ligação).
- Os peers podem **partilhar e pesquisar conteúdos** (identificadores) através da rede sobreposta com propagação controlada por hop count.

A aplicação compreende duas componentes fundamentais:

1. **Manutenção da rede sobreposta** — adesão (join) e abandono (leave) de peers, com reconexão automática quando um peer fica sem vizinhos externos.
2. **Partilha e pesquisa de conteúdos** — criação, remoção e pesquisa de identificadores na rede sobreposta.

## 🏫 Contexto académico

| Campo | Valor |
|---|---|
| Instituição | Instituto Superior Politécnico de Tecnologias e Ciências (ISPTEC) |
| Departamento | Engenharia e Tecnologias |
| Curso | Licenciatura em Engenharia Informática (EINF) |
| Cadeira | Redes de Computadores I (RC-I) |
| Projecto | #1 — Peer-to-Peer Overlay Network |
| Professor | João Costa |
| Ano letivo | 2025/2026 |

## ⚙️ Funcionalidades

### Interface de utilizador (cliente)

| Comando | Descrição | Estado |
|---|---|---|
| `join` | Regista o peer no servidor UDP, obtém lista de peers e estabelece ligações TCP | ✅ Implementado |
| `leave` | Remove registo do servidor e fecha todas as ligações TCP | ✅ Implementado |
| `show neighbors` | Mostra lista de vizinhos internos e externos | ✅ Implementado |
| `exit` | Executa `leave` e fecha a aplicação | ✅ Implementado |
| `release seqnumber` | Remove ligação com vizinho interno de(seqnumber | ❌ Por implementar |
| `list identifiers` | Mostra identificadores conhecidos pelo peer | ❌ Por implementar |
| `post identifier` | Adiciona um identificador à lista de conhecidos | ❌ Por implementar |
| `search identifier` | Pesquisa um identificador na rede sobreposta | ❌ Por implementar |
| `unpost identifier` | Remove um identificador da lista de conhecidos | ❌ Por implementar |

### Invocação

```bash
p2pnet [-s addr] [-p prport] [-l lnkport] [-n neigh] [-h hc]
```

| Argumento | Descrição | Omissão |
|---|---|---|
| `-s addr` | Endereço IP do servidor de peers | `192.168.56.21` |
| `-p prport` | Porta UDP do servidor de peers | `58000` |
| `-l lnkport` | Porta TCP onde o peer escuta | *(obrigatório)* |
| `-n neigh` | Máximo de vizinhos internos/externos (N+ = N-) | *(obrigatório)* |
| `-h hc` | Máximo de saltos para pesquisa de identificadores | ❌ Por implementar |

## 📡 Protocolos

### Comunicação com o servidor de peers (UDP)

| Comando | Descrição | Resposta |
|---|---|---|
| `REG lnkport` | Regista peer no servidor | `SQN seqnumber` ou `NOK` |
| `UNR seqnumber` | Remove registo do peer | `OK` ou `NOK` |
| `PEERS` | Solicita lista de peers | `LST\nIP:port#seq\n...` ou `NOK` |

### Estabelecimento e terminação de ligações (TCP)

| Comando | Descrição | Resposta |
|---|---|---|
| `LNK seqnumber` | Pedido de ligação à rede sobreposta | `CNF` (aceite) ou fecho de sessão (recusado) |
| `FRC seqnumber` | Pedido forçado de ligação | `CNF` (aceite, com expulsão de vizinho maior) ou fecho de sessão |
| `CNF` | Confirmação de ligação | — |
| `UNL seqnumber` | Notificação de desligamento | — |

### Pesquisa de identificadores (TCP) — ❌ Por implementar

| Comando | Descrição | Resposta |
|---|---|---|
| `QRY identifier hopcount` | Pesquisa recursiva de identificador | `FND identifier` ou `NOTFND identifier` |

## 📁 Estrutura do repositório

```
rede-p2p/
├── README.md                       # Este ficheiro
├── ARCHITECTURE.md                 # Arquitetura detalhada do projeto
├── ROADMAP.md                      # Guia de implementação das funcionalidades em falta
├── Enunciado.txt                   # Enunciado do projecto
├── Makefile                        # Sistema de build
├── Vagrantfile                     # Infraestrutura de VMs
├── bootstrap_client.sh             # Provisionamento do cliente (Wireshark, nmap)
├── bootstrap_server.sh             # Provisionamento do servidor UDP
├── client/
│   ├── main.c                      # Main do cliente P2P (select loop)
│   ├── menu.c / menu.h             # Interface de utilizador
│   ├── join.c / join.h             # Comando join
│   ├── leave.c / leave.h           # Comando leave
│   ├── neighbors.c / neighbors.h   # Comando show neighbors
│   ├── tcp_client.c / tcp_client.h # Cliente TCP (LNK/FRC)
│   ├── tcp_server.c / tcp_server.h # Servidor TCP (aceitar LNK/FRC/UNL)
│   ├── udp_client.c / udp_client.h # Cliente UDP (REG/PEERS/UNR)
│   └── graph.c / graph.h           # Grafo de peers (estrutura de dados)
├── server/
│   ├── main.c                      # Main do servidor UDP
│   └── peer_store.c / peer_store.h # Gestão de registo de peers
└── .gitignore
```

## 🧰 Requisitos

- **Linux** (testado em Ubuntu via Vagrant) ou **WSL** no Windows
- **GCC** (compilador C)
- **GNU Make**
- **Vagrant + VirtualBox** (opcional; para ambiente virtualizado)

## 🚀 Como executar

### Compilar

A partir do diretório raiz do repositório:

```bash
make
```

Isto gera dois binários:

- `p2pnet` — peer (cliente P2P)
- `servidorUDP` — servidor de peers

### Executar o servidor UDP

```bash
./servidorUDP
```

O servidor fica ativo na porta UDP 58000, aguardando pedidos REG, PEERS e UNR.

### Executar um peer

Em terminal separado:

```bash
./p2pnet -l 6000 -n 3
```

O peer fica acessível na porta TCP 6000, com no máximo 3 vizinhos internos e 3 externos.

### Com argumentos opcionais

```bash
./p2pnet -s 192.168.56.21 -p 58000 -l 6000 -n 3 -h 5
```

### Com Vagrant

```bash
vagrant up udpserver client1
vagrant ssh udpserver
vagrant ssh client1
```

## 🖥️ Exemplo de execução

```
$ ./p2pnet -l 6000 -n 3

Peer iniciado. Porta TCP: 6000 | Nplus=3

===== P2PNET =====
> join
> leave
> show neighbors
> exit
> join

--- JOIN ---
SEQ atribuído: 1
Lista recebida do servidor:
LST
192.168.56.11:6000#1

JOIN Finalizado. Ligações externas estabelecidas: 0

===== P2PNET =====
> show neighbors

Peer 192.168.56.11:6000 (Seq 1)
Internos:
   (nenhum)
Externos:
   (nenhum)
```

## 🙏 Atribuição

O protocolo UDP de comunicação com o servidor de peers e a estrutura de dados do grafo de peers foram desenvolvidos com base nos materiais de aula da disciplina de Redes de Computadores I.

## 👥 Integrantes

| Nome | Número de estudante |
|---|---|
| Isabel Marques | 20231238 |
| Jussana Paim | 20230132 |
| Norberto Cassoma | 20230873 |
| Oldmar Filindo | 20231359 |

## 📄 Licença

Distribuído sob a licença **MIT**. Veja o arquivo [LICENSE](LICENSE).
