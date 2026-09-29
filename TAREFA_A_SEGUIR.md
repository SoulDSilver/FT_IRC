# Tarefa 8 — Registo completo

## Objectivo

Completar o registo de um cliente depois de `PASS`, `NICK` e `USER`.

O servidor só deve considerar um cliente registado quando os três comandos tiverem sido processados correctamente. Depois do registo, novos `PASS` e `USER` devem ser recusados com `462`.

Este documento cobre todo o passo 8. A implementação será feita em etapas pequenas, sem alterar o `Server::run()`.

## Estado actual

O projecto já possui:

- buffer de entrada e extracção de linhas em `Client`;
- parser `vector<pair<string, string>>` em `Commands`;
- dispatch de `PASS`, `NICK`, `USER` e `QUIT`;
- estado `PasswordAccepted`, `HasNick` e `HasUsername` no `Client`;
- handlers correspondentes em `Registration.cpp`.

Ainda não existe:

- o estado `Registered`;
- a resposta de boas-vindas `001`;
- a verificação de re-registo `462`.

## 8.1 — Adicionar o estado `Registered`

### Ficheiros

- `includes/Client.hpp`
- `srcs/Client.cpp`

### Alterações

Adicionar o atributo:

```cpp
bool Registered;
```

Inicializá-lo a `false` no construtor por defeito e copiá-lo no construtor de cópia e no operador de atribuição.

Adicionar:

```cpp
void setRegistered();
bool isRegistered() const;
```

O valor deve ser alterado apenas quando o registo for concluído.

## 8.2 — Completar o registo e enviar `001`

### Ficheiro

- `includes/Server.hpp`
- `srcs/commands_files/Registration.cpp`
- `srcs/Server.cpp`

Adicionar uma função de `Server` responsável por verificar:

```text
PasswordAccepted == true
HasNick == true
HasUsername == true
Registered == false
```

Se as três condições estiverem satisfeitas:

1. marcar `Registered = true`;
2. enviar uma única resposta `001`.

Formato esperado:

```text
:<servidor> 001 <nick> :Welcome to the IRC Network <nick>!<username>@<host>\r\n
```

A verificação deve ser chamada depois de cada comando que pode completar o registo:

- depois de um `PASS` válido;
- depois de um `NICK` válido;
- depois de um `USER` válido.

Assim, a ordem dos comandos pode variar:

```text
PASS secret
NICK alice
USER alice 0 * :Alice
```

ou:

```text
NICK alice
USER alice 0 * :Alice
PASS secret
```

A resposta `001` só deve ser enviada uma vez.

## 8.3 — Impedir re-registo

Depois de `Registered == true`:

- `PASS` deve responder `462` e não alterar a password;
- `USER` deve responder `462` e não alterar username nem realname;
- `NICK` deve continuar permitido, porque a mudança de nickname é uma operação separada.

Resposta esperada:

```text
:<servidor> 462 <nick> :You may not reregister\r\n
```

## Erros relacionados

- `PASS` sem parâmetro: `461`;
- `PASS` incorrecto: `464`;
- `NICK` inválido: `432`;
- `NICK` já usado: `433`;
- `USER` com parâmetros insuficientes: `461`.

## Como testar

Terminal 1:

```sh
make
./ircserv 6667 secret
```

Terminal 2:

```sh
nc 127.0.0.1 6667
```

Fluxo principal:

```text
PASS secret
NICK alice
USER alice 0 * :Alice
```

Depois da implementação, o servidor deve responder:

```text
:Broadcast_Server 001 alice :Welcome to the IRC Network alice!alice@127.0.0.1
```

Testar novamente:

```text
PASS secret
USER other 0 * :Other
```

Depois do registo, ambos devem responder `462`.

## Verificação de código

```sh
make -B objs/srcs/Client.o
make -B objs/srcs/Server.o objs/srcs/commands_files/Registration.o
make
```

O passo 8 só está concluído quando o build passa e o fluxo `PASS + NICK + USER` produz `001` apenas uma vez.

---

# Diagnóstico (2026-09-29)

Análise do estado real do projecto face ao que está documentado. Os bugs B1 a B10 e as
vulnerabilidades V1 a V5 foram reproduzidos com programas isolados em `/tmp`, sem alterar o
projecto.

## Resumo do estado

| # | Assunto | Estado |
| --- | --- | --- |
| — | Bloqueio de build pelos typedefs de `irc.hpp` | **corrigido** |
| — | Adaptação do parser à estrutura nova | **corrigido**, secção 1 |
| B1 | `Client` não copia `Nick`/`Inbuff`/`Outbuff` | **corrigido** |
| B2 | `closeFds` percorre o `map` por índice | **aberto** |
| B3 | O loop de `poll()` salta fds | **aberto** |
| B4 | `Server` não copia os containers | **aberto** |
| B5 | `Channel` não copia `Settings`/`InvestedUsers` | **aberto**, sem efeito hoje |
| B6 | Declarações mortas em `Commands` | **aberto** |
| B7 | `MAX_NICK_LEN` / `MAX_USER_LEN` inexistentes | **corrigido** |
| B8 | Comando desconhecido aborta o resto do buffer | **aberto**, confirmado contra o binário |
| B9 | `432` envia um campo de nick partido | **corrigido** pelo refactor do parser |
| B10 | Bypass do NUL no `PASS` | **corrigido**, ver V3 para o resto |
| — | Validação assimétrica entre `PASS`, `NICK` e `USER` | **corrigido**, secção 7 |
| 8.1 | Estado `Registered` no `Client` | **feito** |
| 8.2 | Registo completo e `001` | **parcial** — D1 e D3 corrigidos, D2 aberto, secção 8 |
| 8.3 | `462` contra re-registo | **feito** |
| D1 | `001` enviado duas vezes | **corrigido** |
| D2 | Texto do `001` incompleto | **aberto** |
| D3 | `433` para username duplicado | **corrigido** |
| V1 | Motivo do `QUIT` sem validação | **aberto**, secção 9 |
| V2 | `Inbuff` cresce sem limite | **aberto**, secção 9 |
| V3 | Fuga temporal na comparação da password | **aberto**, secção 9 |
| V4 | Número de clientes sem limite | **aberto**, secção 9 |
| V5 | `iscntrl` depende do locale | **latente**, secção 9 |
| V6 | O servidor bloqueia a escrever no stdout | **aberto**, secção 9 |

**Ordem sugerida por dependência:** V1 (exige rede, sem autenticação) → D2 (protocolo,
e cria o prefixo de que o #14 precisa) → V2 → #14, já desbloqueado pelo B1 → B8 → B9 → V3 →
B2, B3, B4, B5, B6.

## 1. Bloqueio de build — corrigido

> **Estado: corrigido.** `make` passa e o `ircserv` liga. A estrutura de `irc.hpp` é a
> authoritative.

O `HEAD` não compilava: o commit `7cffb30` trocou os typedefs sem adaptar o parser, e o
`Commands::parse` continuava a fazer `push_back` de um `pair<string,string>` para dentro de um
`vector<CommandPairVector>`. O `HEAD~1` também não compilava, por causa do `>>` do C++11 num
projecto `-std=c++98`.

## 2. Parser adaptado à estrutura nova — feito

> **Estado: corrigido.** Ver secção 1.

### A estrutura final

`includes/irc.hpp` fica assim:

```cpp
enum ParsedParamId
{
	PP_UNKNOWN = 0,
	PP_USER = 1, PP_NICK = 2, PP_MODE = 3, PP_CHANNEL = 4, PP_TARGET = 5,
	PP_MESSAGE = 6, PP_REASON = 7, PP_TOPIC = 8, PP_PASSWORD = 9, PP_REALNAME = 10,
	PP_USER_MODE = 11, PP_UNUSED = 12, PP_MODE_ARGS = 13
};

typedef pair<ParsedParamId, string> CommandPair;             // (id, valor)
typedef pair<string, vector<CommandPair> > CommandPairVector; // (comando, parâmetros)
typedef vector<CommandPairVector> CommandList;
```

O enum ficou **antes** dos typedef, que é a ordem obrigatória. `CommandPair` passou a usar o
enum directamente em vez de `int`, o que torna as comparações type-safe.

### Ids que faltavam

O enum não cobria os comandos implementados. Acrescentados:

| Id | Comando | Porquê faltava |
| --- | --- | --- |
| `PP_PASSWORD` | `PASS` | não havia id nenhum para a senha |
| `PP_REALNAME` | `USER` | o realname não tinha id |
| `PP_USER_MODE` | `USER` | o `<mode>` do `USER` não tinha id |
| `PP_UNUSED` | `USER` | o `<unused>` não tinha id |

Ficou de fora `PP_MODE_ARGS`, para os argumentos do `MODE`. Seriam variáveis em número, o que
obrigava a um campo extra na estrutura do parser; o `MODE` não está implementado, e quando
o estiver é uma entrada no `if/else` e uma linha no `getParam`.

Corrigido também um comentário errado: `PP_MESSAGE` estava anotado como `PRIVMSG/QUIT`, mas o
`QUIT` usa `PP_REASON`.

### Como o parse identifica cada parâmetro

Não há tabela, nem struct, nem namespace. `srcs/Commands.cpp` tem uma função
`buildParams()` com um `if/else` por comando, e lê-se um comando de cada vez:

```cpp
static vector<CommandPair> buildParams(const string &command,
    const string &parameters)
{
    ParsedParamId ids[4];
    size_t count = 0;
    bool trailing = false;

    if (command == "PASS")
        ids[count++] = PP_PASSWORD;
    else if (command == "NICK")
        ids[count++] = PP_NICK;
    else if (command == "USER")
    {
        ids[count++] = PP_USER;
        ids[count++] = PP_USER_MODE;
        ids[count++] = PP_UNUSED;
        ids[count++] = PP_REALNAME;
        trailing = true;      // o ultimo e' o que comeca no ':'
    }
    ...
    else
        return (vector<CommandPair>());
```

`ids[]` são os ids por ordem. `trailing` a `true` diz que o último é o texto livre atrás do
`:`. O `if/else` é só a declaração — não há tabela para ir consultar, e cada comando está
escrito à mão com os seus nomes.

O `push_back` que o `parse` faz é o do resultado:

```cpp
commands.push_back(make_pair(command, buildParams(command, parameters)));
```

ou seja o primeiro elemento é o nome do comando e o segundo é o vector de
`(ParsedParamId, valor)`.

A parte a seguir, dentro da mesma função, é a separação: o trailing recolhe tudo a partir do
primeiro `:`, depois preenchem-se os slots posicionais, e o que sobrar vai para
`PP_UNKNOWN`.

### Corolários

- **Tokens a mais** vão todos para um `PP_UNKNOWN` com o espaçamento original intacto. É o que
  permite detectar `NICK bad nick` e `PASS a b` sem estrutura extra. Verificado contra o
  binário: `432` e `461` respectivamente.
- **`QUIT` sem `:`** põe o resto inteiro no `PP_UNKNOWN` (`QUIT a b c` → `"a b c"`), e o
  `handleQuit` cai nele numa linha. Preserva o comportamento leniente que o código tinha.
- **Um id por linha, sem excepção.** `JOIN #a,#b` dá `PP_CHANNEL = "#a,#b"`, uma string só.

### Handlers

`dispatchCommand` e os quatro handlers passaram a receber `const CommandPairVector &` em vez de
`const string &`, e cada um lê os ids de que precisa:

```cpp
void Server::handleNick(Client &client, const CommandPairVector &command)
{
	string nickname = Commands::getParam(command, PP_NICK);
	if (Commands::hasParam(command, PP_UNKNOWN)
		|| Verify::checkError(nickname, MAX_NICK_LEN) != Verify::RETURN_OK)
	{
		string value = nickname.empty() ? "*" : nickname;
		sendNumericReply(client, "432", value + " :Erroneous nickname");
		return;
	}
	...
```

`Verify::parseUserParameters` foi **removida**: o parser faz agora esse trabalho, e tê-la
duplicava a lógica.

### Testes

**Parser**, 33 verificações, todas a passar, ligadas ao código real. Cobrem o mapeamento de cada
comando, os `:` finais, listas por vírgula, `PP_MODE_ARGS`, ausência de parâmetros obrigatórios,
presença de `PP_UNKNOWN` e comandos em minúsculas.

**Integração**, contra o binário: fluxo completo e nas três ordens possíveis de `PASS`/`NICK`/
`USER` com exactamente um `001` cada; `462` duas vezes no re-registo; `NICK` pós-registo sem
`462`; 12 casos de validação; 5 casos de tokens a mais; `QUIT` com e sem dois pontos; linha
partida entre dois pacotes; vários comandos num só pacote; NUL na password.

A única falha é o B8, que é pré-existente e ficou agora confirmado contra o binário.

## 3. Lacunas da tarefa 8 (secções 8.1 a 8.3)

- **8.1** — o passo manda copiar `Registered` no construtor de cópia e no `operator=`, mas
  esse mesmo construtor já não copia `Nick`, `Inbuff` nem `Outbuff` (ver B1). A instrução
  é correcta, só não resulta em nada enquanto B1 existir.
- **8.2** — lista `srcs/Server.cpp` nos ficheiros a alterar, mas a implementação não
  precisa de mexer no `dispatchCommand`. Ou a assinatura do parser muda no mesmo passo
  (ver secção 2), ou `srcs/Server.cpp` deve sair da lista.
- **8.2** — não diz qual é o `<servidor>` nem o `<host>` do `001`. No código, o servidor é
  `Name`, definido como `"Broadcast_Server"` em `srcs/Server.cpp:11`, e o host é
  `client.getIpAddr()`, que dá `127.0.0.1`.
- **8.2 / 8.3** — não especificam a ordem entre a guarda `462` e as validações `461`/`464`
  no `PASS`. Ordem correcta: o `462` primeiro, porque um cliente já registado não deve
  receber `461` nem `464`.
- **8.3** — diz que `NICK` continua permitido, mas não diz se a mudança deve ser anunciada
  aos canais. Hoje `handleNick` não notifica ninguém.
- **Erros relacionados** — não menciona `421` nem `451`, que o `IMPLEMENTACAO.md` dá como
  implementados e que não existem no código.
- **Verificação de código** — pede `make`, impossível enquanto o bloqueio da secção 1 não
  for resolvido. Alternativa enquanto isso: `make -B objs/srcs/Client.o` e
  `make -B objs/srcs/commands_files/Registration.o`, que compilam.

## 4. Bugs encontrados e verificados

### B1 — `Client` não copiava tudo

> **Estado: corrigido.** O construtor de cópia e o `operator=` (`srcs/Client.cpp:6-25`) copiam
> agora todos os atributos, e `Outbuff` foi removido do `Client` por completo. Verificado:
> com `nick='alice'` e `inbuff='PING\r\n'`, tanto a cópia como a atribuição preservam os dois.
> **Isto desbloqueia o #14** (broadcast da mudança de nick), que dependia deste bug.

O construtor de cópia e o `operator=` copiam `Fd`, `IpAddr`, `Username`, `RealName` e os
quatro `bool`, mas **não** copiam `Nick`, `Inbuff` nem `Outbuff`.

Impacto que tinha: `Channel` guarda `map<int, Client>` por valor (`includes/Channel.hpp:15`),
por isso o canal criado em `Server::createChannel` (`srcs/Server.cpp:139-143`) nascia com o
cliente já sem nick, e o mesmo valia para a cópia feita em `Server::addNewClient`
(`srcs/Server.cpp:174`).

Nota: a remoção de `Outbuff` torna obsoletas as afirmações de que o buffer de saída "está
ainda por usar" que constam do `AGENTS.md` e do `IMPLEMENTACAO.md`.

### B2 — `Server::closeFds()` percorre o `map` por índice

`srcs/Server.cpp:36-48` faz `for (size_t i = 0; i < this->Clients.size(); i++)` e acede a
`this->Clients[i]`. `Clients` é um `map<int, Client>` cujas chaves são fds, portanto as
chaves `0` a `fd_máximo` não existem e o `operator[]` **cria** entradas novas com um `Client`
por defeito, cujo `Fd` é `-1`.

Verificado com um cliente no fd 9: o `map` passa de 1 para 10 entradas, com as chaves 0 a 8
a reportar `getFd() == -1`.

Ressalva importante: **não é um loop infinito**. Termina em `fd_máximo + 1` iterações, porque
a partir daí `Clients[i]` já existe e o `map` deixa de crescer. O fd real acaba por ser
fechado, por coincidência, na iteração em que `i` igual à chave.

O que está errado na mesma: o `map` fica poluído com entradas fantasma, cada uma provoca um
`close(-1)` que falha com `EBADF`, e o `cout` imprime `Client <-1> Disconnected` várias
vezes. Como `closeFds()` é `public` (`includes/Server.hpp:46`), se for chamada com o servidor
vivo, os clientes reais ficam estruturados num `map` cujas chaves 0 a 8 não são sockets.

Correcção: iterar com iteradores, como já é feito em `removeClients`
(`srcs/Server.cpp:76-90`).

### B3 — o loop de `poll()` salta fds

`srcs/Server.cpp:249-257` itera `for (size_t i = 0; i < Fds.size(); i++)` e chama
`handleClientData`, que pode chegar a `removeClients` e apagar um elemento de `Fds`
(`srcs/Server.cpp:92-100`). O `erase` desloca os elementos seguintes para a esquerda e o
`i++` salta um.

Verificado com `Fds = {3, 10, 11, 12}`: ao processar e fechar o fd 10, o ciclo visita
`3, 10, 12`. O **fd 11 nunca é processado**.

Impacto: atraso, não perda — o `Client::Inbuff` mantém-se, por isso os dados são tratados no
`poll()` seguinte. Pior quando vários clientes fecham no mesmo ciclo, porque o salto
acumula-se.

### B4 — `Server` não copia os containers

`srcs/Server.cpp:14-17`: o construtor de cópia só inicializa `Listen_port`, `Password`,
`Listen_fd` e `Name`. `Fds`, `Clients` e `Channels` ficam como construídos por defeito. O
`operator=` (`srcs/Server.cpp:19-29`) também não os copia.

Verificado: um `Server` com 1 cliente e 1 fd produz uma cópia com 0 e 0. O construtor de
cópia é público (`includes/Server.hpp:40`), portanto o `Server` é copiável e enganoso.

### B5 — `Channel` não copia `Settings` nem `InvestedUsers`

`includes/Channel.hpp:17-18` declara os dois vectors. O construtor de cópia
(`srcs/Channel.cpp:7-9`) e o `operator=` (`srcs/Channel.cpp:13-23`) não os copiam. Há zero
ocorrências de `Settings` e `InvestedUsers` em `srcs/Channel.cpp`: hoje não são usados em
lado nenhum, pelo que o bug ainda não tem efeito.

### B6 — Declarações mortas em `Commands`

`includes/Commands.hpp:24-30` declara `TOPIC`, `MODE`, `JOIN`, `INVITE`, `PART`, `PRIVMSG` e
`KICK` como métodos de instância. Nenhuma tem definição em qualquer `.cpp` e nenhuma é
chamada. Compila porque nunca são usadas, mas a primeira chamada passa a ser erro de link.

### B7 — `MAX_NICK_LEN` e `MAX_USER_LEN` não existiam

> **Estado: corrigido.** Passaram a estar definidas em `includes/irc.hpp`:
> `MAX_NICK_LEN 30`, `MAX_USER_LEN 30`, `MAX_REAL_LEN 255`, `MAX_PASS_LEN 128`.

O `IMPLEMENTACAO.md` (linhas 78 e 85) citava-os como constantes do projecto. Não havia
nenhuma definição em `includes/` nem em `srcs/`, e o `handleNick` não validava comprimento
nenhum.

### B8 — Comando desconhecido aborta o resto do buffer

`Server::dispatchCommand` (`srcs/Server.cpp:180-203`) devolve `false` tanto para o `QUIT`,
que quer desligar o cliente, como para comandos desconhecidos:

```cpp
if (command.first == "QUIT") { handleQuit(...); return (false); }
return (false);   // comando desconhecido cai aqui
```

`handleClientData` (`srcs/Server.cpp:233-235`) trata qualquer `false` como fim de sessão.
Um `PASS secret\r\nFOO\r\nNICK alice\r\n` processa o `PASS`, devolve `false` no `FOO` e
aborta: o `NICK alice` nunca é processado. O cliente fica ligado e nunca recebe o `421` que o
`IMPLEMENTACAO.md` dá como implementado.

A distinção entre "desligar o cliente" e "comando desconhecido" não cabe num único `bool`.
Ou `dispatchCommand` devolve um enum, ou a comparação passa a ser feita em
`handleClientData` antes de chamar o handler.

### B9 — `432` enviava um campo de nick partido

> **Estado: corrigido, por via indirecta.** O refactor do parser passou a cortar a linha em
> tokens, e o `432` deixou de ecoar o resto inteiro.

Antes, o `handleNick` metia o nicktentado completo no campo do nick, e com
`NICK bad nick` saía `:Broadcast_Server 432 * bad nick :Erroneous nickname`, quando o
RFC 2812 §3.2 define `432 <nick> :Erroneous nickname` com `<nick>` a ser um único token.

Confirmado contra o binário depois do refactor:

```text
enviado  b'NICK bad nick\r\n'
recebido b':Broadcast_Server 432 * bad :Erroneous nickname\r\n'
```

O campo do nick é agora `bad`, um token só, que é o que a RFC pede. **Não foi uma decisão**,
foi um efeito colateral de o parser já vir partido: o `handleNick` recebe `PP_NICK = "bad"` e
nunca viu o `"nick"`, que ficou em `PP_UNKNOWN`. Vale a pena fixar isto com um teste, porque
uma alteração futura ao parser pode voltar a juntar os tokens e ninguém dá por isso — é a
mesma armadilha do `checkError` para espaços, que só apareceu quando passou a haver binário.

### B10 — `PASS` aceitava passwords com NUL embebido

> **Estado: bypass corrigido, fuga temporal por corrigir.** O `handlePass` chama
> `Verify::checkToken()` antes da comparação, e o NUL é um carácter de controlo, pelo que a
> entrada é rejeitada com `461` e nunca chega ao `strcmp`. **Fica por fazer:** o handler
> continua a usar `Verify::compareWithStrcmp`, logo a fuga temporal de timing mantém-se. A
> troca para `Verify::constantTimeEquals`, que já está implementada no mesmo ficheiro e nunca
> é chamada, é uma linha.

O `handlePass` comparava com `Verify::compareWithStrcmp`, que usa
`strcmp(provided.c_str(), ...)`. O `c_str()` **trunca no primeiro byte NUL**.

Verificado:

```text
PASS secret\0garbage -> size=14  c_str_len=6
  compareWithStrcmp  = ACEITA
  constantTimeEquals = rejeita
```

Isto é, uma password correcta seguida de um `\0` e de lixo passa a autenticação.

Agravante: `Verify::constantTimeEquals` (`srcs/commands_files/Registration.cpp:9-28`) está
implementado e **nunca é chamado**. A função que trata corretamente do tamanho e não sofre
fuga temporal existe no ficheiro, mas o `handlePass` usa o `strcmp`, que sofre dos dois
problemas.

## 5. Divergências entre a documentação e o código

- `IMPLEMENTACAO.md` dá `421` e `451` como implementados: não existem em lado nenhum.
- `IMPLEMENTACAO.md` dá `462` como já implementado: passou a existir com o passo 8.3.
- `IMPLEMENTACAO.md` dá limites `MAX_NICK_LEN`/`MAX_USER_LEN` que não existem (ver B7).
- `IMPLEMENTACAO.md` dá a propagação de mudança de `NICK` como feita: `handleNick` não
  notifica ninguém.
- `IMPLEMENTACAO.md:57` descreve `parse()` como devolvendo `pair<string, string>`: já não
  corresponde aos typedefs.
- A secção "Implementation cautions" do `AGENTS.md` repete a mesma descrição antiga do
  `parse()`. **A agravar:** o `parse()` agora produz `CommandPairVector`, ou seja
  `(comando, vector<(ParsedParamId, valor)>)`, e `getParam`/`getParams` acendem por id.

## 6. Estado da árvore de trabalho

`a691789` é a tarefa 8, já commitada. O refactor do parser está em *stage*, por commitear.

```text
a691789  feat: adicionar controle de registro de cliente no servidor e registro completo
66db84c  refatorar: remover variável Outbuff e adicionar constantes para limites
72a2668  refatorar: ajustar formatação do typedef CommandPairVector
```

| Ficheiro | Onde está | Alteração |
| --- | --- | --- |
| `includes/Client.hpp`, `srcs/Client.cpp` | `66db84c` | `Registered`; `Outbuff` removido; cópia completa (B1) |
| `includes/Server.hpp`, `srcs/Server.cpp`, `srcs/commands_files/Registration.cpp` | `a691789` | `checkClientRegistered`, `001`, `462` (D1 corrigido) |
| `includes/irc.hpp` | stage | enum antes dos typedefs, `CommandPair` tipada, 4 ids novos |
| `includes/Commands.hpp` | stage | `hasParam`, `getParam` |
| `includes/Server.hpp` | stage | `dispatchCommand` e os quatro handlers em `CommandPairVector` |
| `srcs/Commands.cpp` | stage | `buildParams()` e o `parse()` reescrito |
| `srcs/Server.cpp` | stage | `dispatchCommand` adaptado |
| `srcs/commands_files/Registration.cpp` | stage | handlers lêem por id, `parseUserParameters` removida (D3 corrigido) |
| `TAREFA_A_SEGUIR.md` | por rever | este documento |

`make re` passa e o `ircserv` liga.

### Preservar

O comentário de exemplo de formato em `srcs/commands_files/Registration.cpp:51` é teu e é
anterior a todo este trabalho. Deve sobreviver a qualquer alteração ao ficheiro:

```diff
--- a/srcs/commands_files/Registration.cpp
+++ b/srcs/commands_files/Registration.cpp
@@ -48,7 +48,7 @@ void Server::sendNumericReply(Client &client, const string &code,
 	string nickname = client.getNick();
 	if (nickname.empty())
 		nickname = "*";
-
+	// ex::irc.example.com 461 * PASS :Not enough parameters
```

### Lacuna: não há testes no repositório

Não existe um único teste no projecto — nem Makefile, nem directório, nada. Todo o
verificado neste documento foi feito com programas externos em `/tmp`, que não sobrevivem a um
reboot.

Isto não é学术界ia. A validação de `NICK bad nick` só foi apanhada **quando passou a haver
binário**; antes disso a mesma lógica estava correcta no papel e errada em código. A segunda
regressão do refactor, o `QUIT a b c`, só apareceu com o teste de integração. Sem testes no
repositório, o próximo refactor volta a passar pelo mesmo.

**Recomendação:** levar o que está em `/tmp` para dentro do projecto — o teste do parser
(33 casos, ligado aos objectos reais) e o de integração (22 casos, contra o binário) — e
acrescentar alvos ao `Makefile`. Enquanto isso não existir, o `AGENTS.md` devia dizer explicitamente que a verificação é manual.

## 7. Validação de `PASS`, `NICK` e `USER` — feito

### O problema

Só o `NICK` validava o conteúdo dos parâmetros. O `PASS` não validava nada e o `USER` só
validava o `username`.

| Validação | `NICK` | `PASS` | `USER` |
| --- | --- | --- | --- |
| Não vazio | sim | sim (`parameters.empty()`) | parcial: username sim, realname não era verificada |
| Sem espaço/tab/CR/LF/`:` | sim (`:91`) | nada | parcial: só no username (`:52`) |
| Sem caracteres de controlo | sim (`:94`, `iscntrl`) | nada | nada |
| Limite de tamanho | nada | nada | nada |
| Duplicados | sim (`433`) | n/a | n/a |
| `mode` / `unused` válidos | n/a | n/a | nada (saltados) |

Nenhum caractere de controlo é removido a montante. `recv` → `string data(buff, bytes)` e
`Client::extractLine` só remove o `\r` final, pelo que NULs, tabs e restantes bytes binários
chegam intactos aos handlers.

### O que foi feito

Duas funções em `namespace Verify`, em `srcs/commands_files/Registration.cpp`, usadas pelos
três handlers:

| Função | Para | Rejeita |
| --- | --- | --- |
| `checkToken(value, maxLength)` | campos de token único: nick, username, password | vazio, ` \t\r\n:`, caracteres de controlo, acima do limite |
| `checkText(value, maxLength)` | parâmetros livres atrás de `:`: realname | vazio, caracteres de controlo, acima do limite |

Devolvem `TOKEN_OK`, `TOKEN_EMPTY`, `TOKEN_TOO_LONG` ou `TOKEN_FORBIDDEN_CHAR`, e cada
handler traduz para o seu próprio numeric.

```cpp
// handlePass
if (Verify::checkToken(parameters, MAX_PASS_LEN) != Verify::TOKEN_OK)
    sendNumericReply(client, "461", "PASS :Not enough parameters");

// handleNick
if (Verify::checkToken(nickname, MAX_NICK_LEN) != Verify::TOKEN_OK)
    sendNumericReply(client, "432", value + " :Erroneous nickname");

// handleUser
if (!Verify::parseUserParameters(parameters, username, realName)
    || Verify::checkToken(username, MAX_USER_LEN) != Verify::TOKEN_OK
    || Verify::checkText(realName, MAX_REAL_LEN) != Verify::TOKEN_OK)
    sendNumericReply(client, "461", "USER :Not enough parameters");
```

`parseUserParameters()` ficou só com parsing estrutural. A validação do `username` que lá
estava duplicou a do `checkToken` e foi removida; a resposta ao cliente é a mesma (`461`),
muda apenas o sítio onde é decidida.

Limites em `includes/irc.hpp`: `MAX_NICK_LEN 30`, `MAX_USER_LEN 30`, `MAX_REAL_LEN 255`,
`MAX_PASS_LEN 128`. Os dois primeiros são os valores que o `IMPLEMENTACAO.md` já afirmava.

### Porque duas funções e não uma

`checkToken` rejeita espaços e `:` porque nick, username e password são tokens únicos. O
realname é o que vem depois do `:` em `USER <user> <mode> <unused> :<realname>` e é texto
livre: por protocolo aceita espaços.

Se o realname fosse validado com `checkToken`, o exemplo da própria especificação
—`USER octa 0 * :Octavio Alicio`— levaria `461` e o cliente nunca se registaria. O mesmo
acontecia com `USER octa 0 * :Octavio:Alicio`.

A alternativa a considerar, se se quiser eliminar a duplicação, é uma função só com o
conjunto de proibidos como parâmetro:

```cpp
Verify::TokenResult checkValue(const string &value, size_t maxLength,
    const string &forbidden);
```

com `checkValue(nickname, MAX_NICK_LEN, " \t\r\n:")` nos tokens e
`checkValue(realName, MAX_REAL_LEN, "")` no realname. O loop de `iscntrl` continuaria a correr
sempre, pelo que passar `""` continua a rejeitar `\r` e `\n`. Escala melhor para o que vier a
seguir: `TOPIC`, o motivo do `PART`, do `KICK` e do `QUIT` são todos parâmetros livres atrás
de `:`. **Pendente de decisão.**

### Porque `iscntrl`

`iscntrl` vem do `<cctype>` e testa se um ponto de código é um carácter de controlo, ou seja,
um byte que não produz glifo. No locale "C" são apenas `0x00`–`0x1F` e `0x7F`. O espaço
`0x20` **não** é carácter de controlo, por isso o `checkToken` o apanha na regra dos
proibidos e deixa o `iscntrl` para os bytes invisíveis.

São três as razões pelas que interessa aqui.

**1. Fecha o bypass do NUL (B10).** O `strcmp` trabalha com strings C, que terminam no
primeiro `\0`. Em `PASS secret\0lixo`, o `c_str()` devolve `"secret"` e a comparação diz que
a password está correcta. O `\0` é um carácter de controlo, pelo que o `checkToken` rejeita a
entrada antes de a comparação acontecer.

**2. Fecha a injecção CRLF nas respostas.** `sendNumericReply` (`:94-106`) monta a linha e
acrescenta `\r\n` no fim. Se um valor do cliente trouxesse `\r\n` consigo, o servidor
escrevia mais do que uma linha no socket. Com `NICK x<CR><LF>JOIN #segredo`, o `432` saía
assim:

```text
:Broadcast_Server 432 * x<CR><LF>
JOIN #segredo :Erroneous nickname<CR><LF>
```

O cliente lê **duas** linhas: um `432` e um `JOIN #segredo`. Isto não é teórico, é a injecção
clássica de CRLF, e o `AGENTS.md` nota que o projecto não tem escrita parcial nem
backpressure, ou seja, o caminho de saída é um `send()` cru.

**3. Fecha a injecção no log.** `Server::handleClientData:222` faz
`cout << ... << data << endl` com os bytes crus do `recv()`. Com um `\r` no fim, o cliente
escolhe quando a linha do log acaba, e sequências ESC podem reposicionar o cursor na consola
de quem está a operar o servidor.

O `static_cast<unsigned char>(value[i])` não é cosmético. Neste sistema `char` é *signed*,
logo `(char)0x80` vale `-128`, e passar um valor negativo a uma função de `<cctype>` é
comportamento indefinido; o pior caso é `(char)0xFF` = `-1`, que colide com `EOF` e faz a
glibc ler a entrada errada da tabela de classificação. O `NICK` já fazia o cast certo na
validação antiga; a função nova mantém.

### Ressalva honesta

O `checkText` no realname é defesa em profundidade, não barreira. O realname não entra em
nenhuma resposta numérica hoje. Quando o `JOIN` e a propagação de `NICK` forem implementados
e o realname passar a aparecer em broadcasts, aí passa a ser barreira a sério.

### Testes

A lógica foi testada contra o código real, extraindo o `namespace Verify` do próprio
ficheiro para não testar uma cópia. 26 casos, todos a passar:

- `checkToken`: `"octa"`, `""`, `"bad nick"`, `"a:b"`, tab, `\x01`, `\0`, 30 chars, 31 chars
- bypass do NUL: `secret\0garbage` rejeitado antes de comparar
- `checkText`: `"Octavio Alicio"`, `"a:b:c"`, `""`, `\n`, ESC, 255 chars, 256 chars
- `parseUserParameters`: 8 casos de estrutura, incluindo `u 0 * :a:b` aceite

## 8. Tarefa 8 — verificação do que foi feito

8.1 e 8.3 estão correctos. 8.2 está feito mas com três defeitos. **Nada foi corrigido** — é
verificação.

### 8.1 — `Registered` no `Client` — correcto

`includes/Client.hpp:17` declara o atributo, e `setRegistered()` / `isRegistered()` estão
declarados e implementados. Confirmado com `make -B objs/srcs/Client.o`.

### 8.3 — `462` contra re-registo — correcto

`handlePass:85-89` e `handleUser:143-147` respondem `462` e não alteram nada. `handleNick` não
tem a guarda, o que está correcto segundo a especificação: a mudança de nickname é uma
operação separada.

### 8.2 — `Server::checkClientRegistered` — correcto

`srcs/Server.cpp:180-186` implementa exactamente as quatro condições da especificação:

```cpp
if (client.hasUsername() && client.hasNick() && client.isPasswordAccepted()
    && client.isRegistered() == false)
    return true;
```

O guarda `isRegistered() == false` é o que garante que o burst não se repete, e está bem
colocado. Os três handlers chamam-no no fim, depois de marcarem o seu próprio estado, pelo que
a ordem dos comandos pode variar como a especificação exige.

**Ortografia corrigida:** o nome já é `checkClientRegistered`, sem o "Registred" anterior.

Resta um ponto de estilo, não de lógica: `if (checkClientRegistered(client) == true)` em três
sítios (`Registration.cpp:102`, `:133`, `:168`). A função já devolve `bool`, o suficiente é
usá-la directamente na condição.

### D1 — O `001` era enviado duas vezes

> **Corrigido.** O `if` de guarda aninhado dentro de si mesmo foi removido.

O `handleUser` tinha:

```cpp
if (client.isRegistered())
{
    sendNumericReply(client, "001", ":Welcome to the IRC Network");
    if (client.isRegistered())
        sendNumericReply(client, "001", ":Welcome to the IRC Network");
}
```

O segundo `if` repetia uma condição que acabara de ser satisfeita, por isso o `001` saía
duplicado. Era a violação mais visível da especificação, que diz "A resposta `001` só deve ser
enviada uma vez."

**Verificação actual**, com o `namespace Verify` extraído do ficheiro real e os handlers
espelhados linha a linha:

```text
--- PASS + NICK + USER (fluxo da especificacao) ---
  S -> C: :Broadcast_Server 001 octa :Welcome to the IRC Network
  001 x1  (ok)   462 x0

--- NICK + USER + PASS (ordem invertida) ---
  S -> C: :Broadcast_Server 001 octa :Welcome to the IRC Network
  001 x1  (ok)   462 x0

--- USER + NICK + PASS (outra ordem) ---
  S -> C: :Broadcast_Server 001 octa :Welcome to the IRC Network
  001 x1  (ok)   462 x0

--- PASS/NICK/USER e depois re-registo ---
  S -> C: :Broadcast_Server 001 octa :Welcome to the IRC Network
  S -> C: :Broadcast_Server 462 octa :You may not reregister
  S -> C: :Broadcast_Server 462 octa :You may not reregister
  001 x1  (ok)   462 x2

--- mudanca de nick depois do registo ---
  S -> C: :Broadcast_Server 001 octa :Welcome to the IRC Network
  001 x1  (ok)   462 x0
```

O re-registo devolve `462` duas vezes, uma por `PASS` e outra por `USER`, que é o pretendido.
O `NICK` depois do registo muda o nick sem dar `462`, que é o pretendido, mas também não emite
o broadcast do #14, que continua por fazer.

### D2 — O texto do `001` está incompleto

> **Aberto.** A mensagem continua a ser só `":Welcome to the IRC Network"`, confirmado contra
> o binário: `:Broadcast_Server 001 alice :Welcome to the IRC Network`.

A especificação define:

```text
:<servidor> 001 <nick> :Welcome to the IRC Network <nick>!<username>@<host>
```

O que sai é `:Broadcast_Server 001 octa :Welcome to the IRC Network` — falta o
`<nick>!<username>@<host>`. Os dados já estão todos no `Client` (`getNick()`, `getUsername()`,
`getIpAddr()`), por isso é uma concatenação. O exemplo de saída da especificação é
`:Broadcast_Server 001 alice :Welcome to the IRC Network alice!alice@127.0.0.1`.

A RFC 2812 §3.1 pede o mesmo: `001 RPL_WELCOME :Welcome to the <networkname> network,
<nick>!<user>@<host>`. A parte que falta é a que identifica o utilizador, e é também o que vai
servir de prefixo no broadcast do #14, por isso vale a pena criar o acessor já agora.

### D3 — `433` para username duplicado era um desvio do protocolo

> **Corrigido.** O bloco de verificação foi removido do `handleUser`. O `433` continua a
> existir, mas só para o nickname, que é o que o RFC 2812 §3.2 define.

O RFC 2812 §3.2 define `433` como `ERR_NICKNAMEINUSE`, exclusivamente para nicknames. Não
existe numeric para username duplicado e os servidores reais não verificam unicidade de
username. A especificação também não pede esta verificação.

Para além do desvio, tinha um efeito prático: um cliente que se registasse primeiro com um
username concreto impedia qualquer outro de usar esse username, e a verificação corria **antes**
de o cliente estar autenticado. Era uma forma de bloquear registos alheios sem autenticação.

**O que foi removido**, em `handleUser`:

```cpp
for (map<int, Client>::const_iterator it = Clients.begin();
     it != Clients.end(); ++it)
{
    if (it->first != client.getFd() && it->second.getUsername() == username)
    {
        sendNumericReply(client, "433",
                         username + " :Username is already in use");
        return;
    }
}
```

**Verificação**, com o `namespace Verify` extraído do ficheiro real e dois clientes a
registar-se com o mesmo username:

```text
S -> C: :Broadcast_Server 001 alice :Welcome to the IRC Network
S -> C: :Broadcast_Server 001 bruno :Welcome to the IRC Network

cliente fd 9  registered=1 username='comum'
cliente fd 10 registered=1 username='comum'

>> ambos registaram: sim   433 enviados: 0
```

E o `433` do nickname continua intacto:

```text
S -> C: :Broadcast_Server 433 * bruno :Nickname is already in use
>> 433 de nick duplicado: ok
```

Nota: `Client::getUsername()` continua a existir e é usado no `001`, mas deixa de ter
leitor em `handleUser`. Com o B1 corrigido, o `User` do prefixo reflecte o username real de
cada cliente.

### Conformidade com a especificação

| # | Verificação | Estado |
| --- | --- | --- |
| 11 | Limite de tamanho do nick | **feito** — `MAX_NICK_LEN 30`, ver B7 |
| 13 | `432` para nick inválido | **parcial** — injecção fechada, texto ainda sai partido, ver B9 |
| 14 | Broadcast da mudança de nick | **em falta** |

O #14 tem duas dependências que hoje não existem:

- **Não há nenhuma função que construa o prefixo `nick!user@host`.** A concatenação existe
  dentro do `001`, mas é local ao `handleUser`. Vai ser preciso um acessor único, reutilizado
  pelo broadcast.
- **A cópia para o canal já não é um problema.** `Channel::Clients` é um `map<int, Client>`
  por valor (`includes/Channel.hpp:15`), o que antes do B1 significava que o cliente era
  copiado para o canal sem nick. **B1 está corrigido**, por isso a cópia já preserva o nick e
  o #14 deixa de estar bloqueado por esse lado. Resta só o outro ponto: o `Client` dentro do
  canal é uma cópia, por isso uma mudança de nickname não se propaga sozinha — o broadcast
  tem de iterar os canais e enviar usando o nick novo, não o que estiver guardado.

### Conformidade com o RFC

No geral o texto da especificação está de acordo com o RFC 1459 §3.1–3.2 e o RFC 2812
§3.1–3.2: a ordem `PASS` → `NICK` → `USER`, o `001` como último do burst, o `*` quando ainda
não há nick, e os textos de `461`/`432`/`433`/`462`/`464` estão correctos.

Pontos por decidir:

- **Limite de tamanho** — nenhum RFC impõe um valor. A especificação diz "documenta o teu" e o
  `IMPLEMENTACAO.md` afirma 30. Está agora em `includes/irc.hpp`: `MAX_NICK_LEN 30`,
  `MAX_USER_LEN 30`, `MAX_REAL_LEN 255`, `MAX_PASS_LEN 128`. Os dois últimos foram escolhidos
  sem referência na especificação e convém confirmá-los.
- **`464` diferido ou imediato** — a especificação mostra o `464` a chegar só depois de
  `NICK` e `USER`. Os servidores reais (unrealircd, inspircd) respondem `464` de imediato. O
  exemplo também é compatível com resposta imediata, porque o `*` explica-se só pela falta de
  nick, pelo que a ambiguidade está no texto e não no código. Convém fixar por escrito.
- **Conjunto de caracteres do nick** — o RFC 1459 §2.3.1 limita a letras, dígitos e
  `[]\`_^{|}-`; o RFC 2812 §2.3.1 mantém apenas `[]{}\`_` mais `-` e desaconselha começar por
  dígito. A especificação só pede "não vazio / sem espaços / sem controlo", e o código cumpre
  isso, mas aceita `NICK #x` e `NICK 1a`.
- **Reemitir o próprio nick** — o RFC manda `433` se o nick já estiver em uso, incluindo o
  próprio. `handleNick:123` exclui o próprio cliente da verificação, pelo que `NICK <o
  próprio>` volta a ser aceite em silêncio.
- **Faltam `421` e `451`** — não há porta de registo antes dos comandos, nem resposta para
  comando desconhecido. Ver B8.

## 9. Revisão de segurança

Reverificado contra o binário, depois de haver link. Os quatro primeiros não exigem
autenticação: um cliente que acabou de abrir o socket já os dispara. **Nada foi corrigido** —
é relatório.

> **Correção a uma medição anterior.** Numa passagem anterior deste documento o V2 foi
> descrito como um servidor que deixava de responder. **Estava errado**, e a culpa foi do
> dispositivo de teste: o `stdout` estava ligado a um `PIPE` que ninguém lia, o servidor
> encheu os 64 kB do pipe a escrever no log e bloqueou. Repetido com o output para ficheiro,
> o servidor mantém-se responsivo durante e depois do ataque. O que o V2 é, está descrito
> abaixo com a medição certa. Esse blocking é, no entanto, um problema por si — ver **V6**.

### V1 — O motivo do `QUIT` não é validado

`handleQuit` (`srcs/commands_files/Registration.cpp:155-166`) não chama `checkError` nem
`checkText`. O `reason` vai do parser directo para a resposta:

```cpp
string reply = "ERROR :" + reason + "\r\n";
send(client.getFd(), reply.c_str(), reply.length(), 0);
```

`Client::extractLine` separa por `\n` e remove um `\r` final, pelo que um `\r` no meio da
linha sobrevive. Confirmado contra o binário:

```text
enviado  b'QUIT :bye\rJOIN #segredo\r\n'
recebido b'ERROR :bye\rJOIN #segredo\r\n'      2 CR, 1 LF

enviado  b'QUIT :a\rb\rc\r\n'
recebido b'ERROR :a\rb\rc\r\n'                3 CR, 1 LF

enviado  b'QUIT :a\x00b\r\n'
recebido b'ERROR :a\x00b\r\n'                  NUL passa intacto
```

Um cliente que parta as linhas por `\n` — que é exactamente o que o `extractLine` deste
projecto faz — lê ali mais do que uma linha. Se o motivo passar a ser propagado para outros
clientes, o problema escala. O `NUL` também passa, o que é o mesmo mecanismo do B10 noutro
sítio.

O motivo também não tem limite de tamanho: `QUIT :x` seguido de 20 000 caracteres produz uma
resposta de 20 010 bytes, e o `send()` ignora quantos bytes efectivamente escreveu, pelo que
uma escrita parcial daria uma linha truncada.

**Correcção:** passar o motivo por `checkText` com um `MAX_QUIT_LEN`. `checkText` rejeita `\r`
e `\n`, que é o desejado, e deixa passar o espaço de que o motivo precisa. O `NUL` também é
rejeitado por ser carácter de controlo.

### V2 — `Client::Inbuff` cresce sem limite

`handleClientData` (`srcs/Server.cpp:229`) faz `appendInput(data)` e só extrai linhas
completas. Se um cliente enviar dados sem `\n`, nada é extraído e o buffer cresce em cada
ciclo de `poll`. Medido contra o binário:

```text
3 900 000 bytes enviados sem nenhum \n
RSS antes = 3592 kB   depois = 7852 kB   delta = 4260 kB
```

Ou seja, mais ou menos 1 kB de memória por kB enviado, sem tecto nenhum. Não há limite de
tamanho de linha nem de buffer em lado nenhum do projecto.

**A memória é libertada quando o cliente desliga** — `removeClients` apaga o `Client` do map e
o `RSS` volta aos 3 848 kB. Portanto não é um leak permanente: é um cliente que consegue
segurar tantos megabytes quantos conseguir enviar, com um só socket. Com `MAXPENDCONN` e um
`poll()` linear no número de fds, meia dúzia de clientes simultâneos já é consumo
significativo.

Isto agrava com B3: um cliente que fique a enviar dados fica sempre no fim de `Fds`, e o
`erase` a meio do loop de `poll` faz com que outros sockets sejam saltados.

**Correcção:** impor um tecto em `Inbuff` — `MAX_LINE_LEN`, que pelo RFC 1459 §2.3 é de 512
bytes — e desligar o cliente com `465` quando for excedido.

### V3 — Fuga temporal na comparação da password

O bypass do NUL (B10) está corrigido: `Password != parameters` usa `std::string::operator==`,
que compara `size()` e depois o conteúdo, e o `c_str()` já não é usado. Confirmado que
`secret\0garbage` já não é aceite: o servidor responde `461`, porque o `\0` é apanha no
`checkError` antes da comparação.

Mas `operator!=` desvia assim que encontra uma diferença, pelo que o tempo de resposta depende
de quantos bytes iniciais coincidem. Com tentativas suficientes a password pode ser reconstruída
byte a byte. `Verify::constantTimeEquals`, que existia precisamente para isto e que estava no
mesmo ficheiro, foi removida na reescrita do `handlePass`.

Como o servidor é um processo único, a diferença é pequena, mas é mensurável em rede local, e
o custo de a evitar é uma função.

**Correcção:** restaurar a comparação em tempo constante e voltar a usá-la no `handlePass`.

### V4 — Número de clientes sem limite

`MAXPENDCONN 10` é apenas a fila de espera do `listen()`. `addNewClient` (`srcs/Server.cpp:145`)
aceita sem limite: não há tecto em `Clients.size()` nem em `Fds.size()`. Medido:

```text
inicio                : 4 fds, 3600 kB
300 ligações abertas  : 304 fds, 3800 kB   (delta 208 kB)
depois de fechar todas: 4 fds
```

Não é uma explosão de memória — cada `Client` é barato. O problema é o `poll()`, que é chamado
com o vector inteiro a cada ciclo e é linear no número de fds: com 10 000 clientes o servidor
fica lento para todos, e cada `Client` tem seis strings. O `ulimit -n` desta máquina é
1 048 576, portanto o processo não bate no limite do sistema antes de o `poll()` ficar
invável.

**Correcção:** tecto em `Clients.size()` e fechar a ligação imediatamente quando for atingido,
com um `MAX_CLIENTS` em `includes/Server.hpp`.

### V5 — `iscntrl` depende do locale

`iscntrl` classifica segundo o locale activo. O projecto nunca chama `setlocale`, portanto está
no locale "C" e o comportamento é determinista: `0x00`–`0x1F` e `0x7F`.

É risco latente, não bug actual: se alguém acrescentar `setlocale(LC_ALL, "")` para suportar
acentos, em alguns locales os bytes `0x80`–`0x9F` passam a contar como imprimíveis e
`checkError` passaria a aceitá-los. O `AGENTS.md` não menciona locale, por isso não está
registado em lado nenhum do projecto.

**Correcção:** ou fixar `setlocale(LC_CTYPE, "C")` no arranque, ou classificar por tabela
própria em vez de depender de `<cctype>`.

### V6 — O servidor bloqueia a escrever no stdout

Descoberto por acidente, ao construir o dispositivo de teste, e confirmado em separado.
`handleClientData` (`srcs/Server.cpp:230`) escreve uma linha de log por `recv()`, e a linha
**inclui os dados crus do cliente**:

```cpp
cout << YEL << "Client <" << fd << "> Data: " << WHI << data << endl;
```

O volume de log é portanto controlado pelo atacante. Se o `stdout` não for drenado, o
`cout` bloqueia e bloqueia o loop de `poll` inteiro. Confirmado:

```text
stdout ligado a um PIPE que ninguém lê, antes:      001 OK
cliente envia 300 000 bytes em 300 pacotes (~400 kB de log)
depois:                                              BLOQUEADO (timed out)
```

O servidor não recupera enquanto ninguém ler o pipe. Um `PIPE` do Linux tem 64 kB de buffer, e
cada `recv()` de 1000 bytes escreve mais de 1000 bytes de log, por isso bastam ~65 pacotes para
encher.

Isto não é exactamente um ataque de rede — depende de como o `stdout` está ligado. Num
terminal interactivo é menos provável, mas num supervisor que não drene a saída, ou num
`nohup` para um pipe encravado, o servidor fica indisponível e o atacante só precisa de enviar
60 pacotes.

Há ainda um vector de consumo de disco na mesma linha: o log pode ser enchido à vontade sem
que o servidor faça mais trabalho de parsing.

**Correcção:** o log de dados crus é útil para depurar, mas não pode ser a única saída. Ou
limita-se o que se escreve por `recv()`, ou escreve-se para `stderr` e deixa-se o `stdout`
livre para o que o utilizador realmente quer ver, ou tira-se o `endl` (que faz `flush` a cada
linha) e deixa-se o buffer decidir.

### Fora do âmbito, mas a registar

- **`Server::Signal`** — o signal handler só escreve num `sig_atomic_t`, que é
  async-signal-safe. Está correcto.
- **`inet_ntoa`** devolve um ponteiro para um buffer estático, mas o resultado é consumido de
  imediato por `setIpAddr`, que copia para a string. Está correcto tal como está.
- **Sem escrita parcial** — todos os `send()` ignoram o valor de retorno. Com as respostas
  actuais, curtas, o risco é baixo; o `QUIT` com motivo grande (V1) é o primeiro caso em que
  isto já pode acontecer. O `AGENTS.md` já assinala que não há backpressure.
- **Password em claro na memória** — `Server::Password` é uma `string`. Normal neste tipo de
  projecto, sem valor prático numa captura de memória pontual.
- **Porta e password na linha de comando** — visíveis em `/proc` e no `ps` de outros utilizadores
  da máquina. É a interface que o `main.cpp` exige, portanto não é um bug do servidor.
