# Handshake IRC — porque o irssi não conectava

Explicação do que aconteceu quando o cliente de referência passou do `nc` para o
irssi, o que estava errado, o que mudou no código, e onde está cada coisa
referenciada no protocolo.

---

## 1. O sintoma

Depois de o registo funcionar com `nc`, o `irssi` (1.4.5) falhava:

```
16:17 -!- Irssi: Connecting to localhost [127.0.0.1] port 2000
16:17 Waiting for CAP LS response...
16:17 -!- Irssi: Connection to localhost established
16:18 -!- Irssi: Not connected to server
```

Duas coisas estranhas em simultâneo:

- `Waiting for CAP LS response...` — o cliente ficou à espera de uma resposta que
  nunca chegou.
- `Connection ... established` seguido de `Not connected to server` — o socket
  abriu, o registo foi aceite, mas o cliente não fechou a sessão.

---

## 2. Porque o `nc` enganava

O `nc` é um cliente rudimentar. O que fizemos com ele era:

```
C → S: PASS secret
C → S: NICK alice
C → S: USER alice 0 * :Alice
S → C: :Broadcast_Server 001 alice :Welcome...
```

E considerávamos aquilo "o registo a funcionar". Funcionava **para o `nc`**, porque
o `nc` não exige mais nada. Não envia `CAP`, não espera burst completo, não faz
keepalive.

Qualquer cliente IRC real faz as três coisas. Por isso o `nc` passar não era
evidência de que o servidor estivesse correcto — era apenas o teste mais tolerante
possível. A primeira falha só apareceu quando o cliente deixou de ser tolerante.

---

## 3. O handshake completo

É isto que um cliente IRC faz, pela ordem. Com o nosso servidor, hoje:

```
passo | o cliente manda                    | o servidor responde
-----+------------------------------------+-----------------------------
  1   | CAP LS 302                          | :srv CAP * LS :
  2   | PASS 123                            | (nada, guarda)
  3   | NICK vnanga                         | (nada, guarda)
  4   | USER vnanga 0 * :Victor Nanga       | 001, 002, 003, 004
  5   | PING :abc   (de tempos em tempos)   | :srv PONG abc
```

Cada passo, o que é e porque existe:

### Passo 1 — `CAP LS` (consulta de capacidades)

`CAP` significa **C**apability **N**egotiation. É uma extensão moderna do protocolo,
definida no IRCv3, que resolve um problema do IRC original: em 1996 não havia forma
de um cliente dizer "eu sei fazer isto" ao servidor. Cada cliente descobria as
funcionalidades **tentando** e vendo se recebia erro.

O `LS` significa **L**i**S**t — "lista-me o que sabes fazer". O `302` é a versão
do protocolo. O servidor responde com a lista separada por espaços, e se não
suporta nada, responde com a lista vazia:

```
C → S: CAP LS 302
S → C: :servidor CAP * LS :          ← lista vazia: não suporto nada
```

O cliente pode depois pedir capacidades concretas, e o servidor recusa:

```
C → S: CAP REQ :sasl
S → C: :servidor CAP * NAK :         ← NAK = Negative Acknowledge
```

Quando o cliente termina, diz `CAP END` e segue para o registo normal.

**Referência:** [IRCv3 capability negotiation](https://ircv3.net/specs/extensions/capability-negotiation).
O comando `CAP` também está definido no RFC 2812 §3.2 (`CAP` — Capability Negotiation),
embora a forma moderna `LS`/`REQ`/`END` seja da especificação IRCv3.

**Porquê o cliente bloqueia aqui:** um `CAP LS` sem resposta deixa o cliente à espera
até expirar o timeout. É o `Waiting for CAP LS response...` que aparecia.

### Passos 2 e 3 — `PASS` e `NICK`

O registo propriamente dito. O servidor não responde a nenhum dos dois — guarda o
estado e espera pelo `USER`. É o comportamento correcto.

### Passo 4 — `USER` e o burst de boas-vindas

O `USER` completa o registo. O servidor responde com o **burst**: quatro numerics
consecutivos.

```
S → C: :srv 001 nick :Welcome to the IRC Network nick!user@host
S → C: :srv 002 nick :Your host is srv, running ircd-1.0
S → C: :srv 003 nick :This server was created ...
S → C: :srv 004 nick srv ircd-1.0 o o
```

| Numeric | Significado |
| --- | --- |
| `001` | RPL_WELCOME — bem-vindo |
| `002` | RPL_YOURHOST — qual é o teu host |
| `003` | RPL_CREATED — quando o servidor nasceu |
| `004` | RPL_MYINFO — versão do software e modos |

**Referência:** [RFC 2812 §3.1](https://www.rfc-editor.org/rfc/rfc2812#section-3.1),
que descreve o burst `001` a `004` como parte da conclusão do registo.

O `004` tem quatro campos de além do nick: `<servername> <version> <user modes>
<channel modes>`. No nosso caso `o o` — modos de utilizador `o` (operador) e modos
de canal `o`.

**Porquê o cliente não fechava a sessão:** o `irssi` espera os **quatro**. Recebia
só o `001` e não marcava a sessão como estabelecida, daí o `Not connected to server`.

### Passo 5 — `PING`/`PONG` (keepalive)

O cliente manda `PING` de tempos em tempos para confirmar que o servidor continua
vivo. O servidor responde com `PONG` e o **mesmo token**, para o cliente saber que
a resposta é a resposta à sua pergunta e não resíduo de outra coisa.

```
C → S: PING :abc
S → C: :servidor PONG abc
```

O token é escolhido pelo cliente (tipicamente um contador ou timestamp) e tem de
voltar **idêntico**.

**Referência:** [RFC 2812 §3.7.2 (PONG)](https://www.rfc-editor.org/rfc/rfc2812#section-3.7.2)
e [RFC 1459 §4.4.1](https://datatracker.ietf.org/doc/html/rfc1459#section-4.4.1).

Isto **não tem relação com o registo.** É uma conversa contínua depois de o cliente
ter ligado.

---

## 4. Os quatro bugs

### 4.1 `CAP LS` não tinha resposta

O `dispatchCommand` só conhecia `PASS`, `NICK`, `USER` e `QUIT`. Tudo o resto caía
no `return (false)` final. O `CAP` caía lá e não recebia resposta nenhuma.

### 4.2 Comando desconhecido abortava o resto do buffer

Este é o bug mais sério dos quatro, e é a causa indirecta do `CAP` bloquear o
registo.

O `dispatchCommand` devolvia `bool`, e `false` significava **duas coisas
diferentes** ao mesmo tempo:

- "desliga o cliente" — no `QUIT`
- "comando desconhecido" — em todo o resto

Não havia forma de as distinguir. E o `handleClientData` tratava qualquer `false`
como fim de sessão:

```cpp
// antes
if (!dispatchCommand(client->second, *it))
    return;                      // sai da função, o resto do buffer é perdido
```

O `irssi` manda `CAP LS` e o registo no **mesmo pacote**. O `CAP` caía no
`return (false)` e o `PASS`, `NICK` e `USER` nunca eram processados.

Verificado contra o binário antes da correcção:

```
C → S: CAP LS 302 / NICK vnanga / USER vnanga 0 * :V / PASS 123
S → C: (nada)                     ← tudo perdido
```

Este bug não afectava apenas o `irssi`. Qualquer cliente IRC real manda comandos
que o nosso servidor não conhece — `NAMES`, `WHOIS`, `MOTD`, `LIST`, `MODE`. Todos
comeriam o resto do pacote.

### 4.3 Burst incompleto

Só se enviava o `001`. Faltavam `002`, `003` e `004`, que o RFC 2812 §3.1 define
como parte do burst de conclusão de registo.

### 4.4 `PONG` com dois espaços

Resposta errada, mas não causava o erro de ligação — apenas fazia o keepalive
parecer falhado.

A causa: o token vinha de `data.substr(4)`, que é tudo o que segue a palavra
`PING`. Em `PING :server1` isso dá `:server1` — com o `:` e o espaço à frente. A
limpeza posterior só tirava `\r` e `\n`, pelo que sobrava o espaço:

```
C → S: PING :server1
S → C: PONG  :server1             ← dois espaços
```

Faltava também o prefixo do servidor. A forma correcta:

```
C → S: PING :server1
S → C: :servidor PONG server1
```

---

## 5. O que mudou no código

### 5.1 `dispatchCommand` devolve um enum

A correcção de raiz do bug 4.2. O `bool` ambíguo passou a ser:

```cpp
enum DispatchResult { DISPATCH_OK, DISPATCH_QUIT, DISPATCH_UNKNOWN };
```

E o `handleClientData` passou a reagir a cada caso:

```cpp
DispatchResult result = dispatchCommand(client->second, *it);
if (result == DISPATCH_QUIT)
    return;                                      // só o QUIT sai
if (result == DISPATCH_UNKNOWN)
    sendNumericReply(client->second, "421", it->first + " :Unknown command");
```

Um comando desconhecido recebe agora `421` e o processing continua:

```
C → S: PASS 123 / FOOBAR x / NICK a / USER a 0 * :A
S → C: :srv 421 * FOOBAR :Unknown command
S → C: :srv 001 a :Welcome...
```

**Referência:** [RFC 2812 §3.3 (421 ERR_UNKNOWNCOMMAND)](https://www.rfc-editor.org/rfc/rfc2812#section-3.3).

### 5.2 Novo handler de `CAP`

`srcs/Server.cpp`, função `handleCap()`. Responde ao `LS` com lista vazia e ao
`REQ` com `NAK`.

Para o `CAP` chegar ao handler com o subcomando identificável, foi acrescentado à
tabela do parser (`srcs/Parser.cpp`):

```cpp
{ "CAP",  1, { PP_TARGET }, false },
{ "PING", 1, { PP_TARGET }, false }
```

O `PING` também foi acrescentado, porque deixou de ser interceptado fora do fluxo
para passar a ser tratado no `dispatchCommand` como qualquer outro comando.

### 5.3 Burst completo

`welcomeMessage()` passou a enviar os quatro numerics num único `send()`, num
único pacote TCP. Isto é melhor que quatro `send()` separados porque o cliente
recebe o burst de uma vez.

Os três `sendNumericReply(..., "001", ...)` duplicados em `Registration.cpp` — um
em cada handler — passaram a chamar `welcomeMessage()`.

O `001` passou também a incluir o prefixo `<nick>!<user>@<host>`, que o RFC 2812
descreve na mensagem de boas-vindas.

### 5.4 `PONG` correcto

O `PING` deixou de ser interceptado à bruta no `handleClientData` e passou pelo
ciclo normal: o parser lê o token, o `dispatchCommand` encaminha para o
`pongmessage()`, que o devolve com o prefixo do servidor e sem espaços a mais.

### 5.5 Log sem duplicação de linha

`(srcs/Server.cpp`, no `handleClientData`)

O log imprimia os dados crus do cliente, que já trazem o `\r\n` do protocolo, e
depois acrescentava um `\n` do `endl`. Resultado: uma linha em branco a mais por
cada comando.

```
antes:  Client <4> Data: NICK v<CR><LF>
                                      ← linha em branco
        Client <4> Data: PASS 123<CR><LF>
                                      ← linha em branco

agora:  Client <4> Data: NICK v<CR><LF>
        Client <4> Data: PASS 123<CR><LF>
```

O `\r` e o `\n` passaram a ser mostrados como `<CR>` e `<LF>` em vez de impressos
como caracteres de controlo.

---

## 6. Tabela de resposta rápida

| Mensagem do cliente | Resposta correcta | Referência |
| --- | --- | --- |
| `CAP LS 302` | `:srv CAP * LS :` | IRCv3 |
| `CAP REQ :x` | `:srv CAP * NAK :` | IRCv3 |
| `CAP END` | (nada) | IRCv3 |
| `PING :tok` | `:srv PONG tok` | RFC 2812 §3.7.2 |
| `PASS x` | (nada) | RFC 2812 §3.1 |
| `NICK n` | (nada, ou `433` se em uso) | RFC 2812 §3.1 |
| `USER u 0 * :r` | `001`, `002`, `003`, `004` | RFC 2812 §3.1 |
| comando desconhecido | `:srv 421 * CMD :Unknown command` | RFC 2812 §3.3 |
| `QUIT :motivo` | `ERROR :motivo`, depois fecha | RFC 2812 §3.1 |

---

## 7. Referências do protocolo

Todas acessíveis publicamente:

- **RFC 2812** — Internet Relay Client Protocol, a especificação que substituiu a
  RFC 1459. [rfc-editor.org/rfc/rfc2812](https://www.rfc-editor.org/rfc/rfc2812)
  - §3.1 — registo, burst de boas-vindas `001`–`004`, `PASS`/`NICK`/`USER`/`QUIT`
  - §3.3 — numerics de erro, incluindo `421`
  - §3.7.2 — `PING` e `PONG`
  - §2.3 — formato de mensagens, prefixos `<prefix> <command> <params>`

- **RFC 1459** — o protocolo original. Ainda útil para a Framing (§2.3),
  incluindo o limite de 512 bytes por linha.
  [datatracker.ietf.org/doc/html/rfc1459](https://datatracker.ietf.org/doc/html/rfc1459)

- **IRCv3 capability negotiation** — a especificação do `CAP`, com os três
  subcomandos `LS`, `REQ` e `END`. [ircv3.net/specs/extensions/capability-negotiation](https://ircv3.net/specs/extensions/capability-negotiation)

---

## 8. O que não está tratado

Isto é o que um cliente real ainda encontra, e vale a pena saber antes de
achar que o servidor está pronto:

- **Sem `JOIN` funcional.** Existe código no `Server.cpp`, mas `dispatchCommand`
  não encaminha o `JOIN`, logo nunca chega a ser executado. O mesmo para `PART` e
  `PRIVMSG`.
- **Sem `MOTD`, `LUSERS`, `NAMES`, `WHOIS`, `LIST`, `MODE`.** Todos caem no `421`.
  O irssi costuma ignorar o `MOTD` e mostrar "no motd", mas outros clientes
  podem bloquear.
- **Sem `NICK` change propagation.** Quando um cliente muda de nick, os outros não
  são avisados.
- **Sem SSL/TLS.** O irssi ligado em claro funciona, mas qualquer cliente moderno
  tenta TLS primeiro.
- **Sem rate limiting** nem tecto de clientes.