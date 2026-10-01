# Tarefas pendentes

Lista do que falta completar e melhorar. O que já está feito não está aqui.

## Onde estamos

Servidor de IRC em C++98, sem dependências, `poll()` único, sem fork. Compila com
`make` (`-Wall -Wextra -Werror -std=c++98`) e liga o binário `ircserv`.

Implementado: `PASS`, `NICK`, `USER`, `QUIT`, com registo completo, validação de
parâmetros por `ParsedParamId` e respostas numéricas.

O parser vive em `srcs/Parser.cpp` (`class Parser`), com a tabela de comandos e as
funções auxiliares num `namespace` anónimo. `Commands` ficou só com o esqueleto da classe.

Histórico recente:

```text
1615f8b  Refatoração dos comandos IRC para utilizar CommandPairVector
a691789  controle de registro de cliente e registro completo
66db84c  remover Outbuff e adicionar constantes de limites de parâmetros
```

Por rever, além deste documento:

```text
 M Makefile
 M includes/Commands.hpp
 M srcs/Commands.cpp
 M srcs/Server.cpp
 M srcs/commands_files/Registration.cpp
?? includes/Parser.hpp
?? srcs/Parser.cpp
```

**Preservar sempre:** o comentário de exemplo de formato em
`srcs/commands_files/Registration.cpp:51`, que é anterior a todo o trabalho recente.

## Prioridade 0 — Regressão a corrigir primeiro

### R1 — `substr` sem `:` devolve a string inteira

**Isto está no código agora. Não é uma tarefa para quando houver tempo — é uma
regressão activa que já se vê no binário.**

Em `srcs/Parser.cpp:95`, o `push_back` do trailing corre sempre que o comando aceita
trailing, mesmo sem haver `:` na linha:

```cpp
if (spec->hasTrailing)
    params.push_back(make_pair(spec->ids[positionalCount], parameters.substr(colon + 1)));
```

Quando não há `:` o `colon` fica `string::npos`, e `npos + 1` transborda o `size_t` para
`0`. O `substr(0)` devolve a string inteira em vez de uma coisa vazia:

```text
USER alice 0 *     ->  PP_REALNAME = "alice 0 *"
QUIT               ->  PP_REASON = ""
```

**Efeito observável no binário:** `USER alice 0 *` não responde nada, quando devia responder
`461`. O `handleUser` (`srcs/commands_files/Registration.cpp:120-153`) só verifica se
`PP_REALNAME` existe, e agora existe sempre, por isso a validação passa e o comando é aceite
como se estivesse completo. Antes desta alteração devolvia `461` correctamente.

```text
enviado  b'USER alice 0 *\r\n'
recebido b''                          devia ser ':... 461 * USER :Not enough parameters'
```

A mesma causa em `QUIT` produz `PP_REASON = ""`, que é inofensivo porque o `handleQuit` cai
no `PP_UNKNOWN`, mas continua a ser lixo.

**Origem:** ao remover a função `extractTrailing` perdeu-se o `return (parameters.size())`
que protegia o caso "sem `:`", e o `push_back` ficou sem guarda.

**Correcção:** a guarda mínima,

```cpp
if (spec->hasTrailing && colon != string::npos)
```

ou, o que é preferível, repor a função `extractTrailing` que existia antes, porque devolvia
o `limit` já correcto e fazia o `push_back` só quando havia `:`.

**Três dos 33 casos do parser e um dos 22 de integração falham por isto.** Convém ter testes
no repositório antes de mexer, senão é fácil introduced outra da mesma.

## Prioridade 1 — Segurança

Nenhum destes exige autenticação. Um cliente que acabou de abrir o socket já os dispara.

### V1 — O motivo do `QUIT` não é validado

`srcs/commands_files/Registration.cpp:155-166`. O `reason` vai do parser directo para a
resposta, sem passar por `Verify::checkText` nem `Verify::checkError`.

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
clientes, o problema escala.

O motivo também não tem limite de tamanho. `QUIT :x` seguido de 20 000 caracteres produz uma
resposta de 20 010 bytes, e o `send()` ignora quantos bytes escreveu, pelo que uma escrita
parcial daria uma linha truncada.

**Correcção:** passar o motivo por `checkText` com um `MAX_QUIT_LEN`. `checkText` rejeita
`\r`, `\n` e `\0` — os três são rejectados por serem caracteres de controlo — e deixa passar
o espaço de que o motivo precisa. A injecção de `\n` não é possível, porque o
`extractLine` já partiu aí.

### V6 — O servidor bloqueia a escrever no stdout

`srcs/Server.cpp:230` escreve uma linha de log por `recv()`, e a linha **inclui os dados
crus do cliente**:

```cpp
cout << YEL << "Client <" << fd << "> Data: " << WHI << data << endl;
```

O volume de log é controlado pelo atacante. Se o `stdout` não for drenado, o `cout` bloqueia
e bloqueia o loop de `poll` inteiro. Confirmado:

```text
stdout ligado a um PIPE que ninguém lê, antes:      001 OK
cliente envia 300 000 bytes em 300 pacotes (~400 kB de log)
depois:                                              BLOQUEADO, sem recuperar
```

Um `PIPE` do Linux tem 64 kB de buffer e cada `recv()` de 1000 bytes escreve mais de 1000
bytes de log, por isso bastam ~65 pacotes para encher.

Não é exactamente um ataque de rede — depende de como o `stdout` está ligado. Num terminal
interactivo é menos provável, mas num supervisor que não drene a saída o servidor fica
indisponível. E há um vector de consumo de disco na mesma linha, sem que o servidor faça
mais trabalho de parsing.

**Correcção:** o log de dados crus é útil para depurar mas não pode ser a única saída. Ou
limita-se o que se escreve por `recv()`, ou escreve-se para `stderr` e deixa-se o `stdout`
livre, ou tira-se o `endl`, que faz `flush` a cada linha, e deixa-se o buffer decidir.

### V2 — `Client::Inbuff` cresce sem limite

`srcs/Server.cpp:229` faz `appendInput(data)` e só extrai linhas completas. Se um cliente
enviar dados sem `\n`, nada é extraído e o buffer cresce em cada ciclo de `poll`. Medido:

```text
3 900 000 bytes enviados sem nenhum \n
RSS 3592 kB -> 7852 kB   (+4260 kB, mais ou menos 1:1)
depois de o cliente fechar, volta a 3848 kB
```

Não é um leak — `removeClients` apaga o `Client` do map e a memória liberta-se. É um cliente
que consegue segurar tantos megabytes quantos conseguir enviar, com um só socket. Não há
limite de tamanho de linha nem de buffer em lado nenhum do projecto.

Isto agrava com o B3: um cliente que fique a enviar dados fica sempre no fim de `Fds`, e o
`erase` a meio do loop de `poll` faz com que outros sockets sejam saltados.

**Correcção:** tecto em `Inbuff` — `MAX_LINE_LEN`, que pelo RFC 1459 §2.3 é de 512 bytes — e
desligar o cliente com `465` quando for excedido.

### V4 — Número de clientes sem limite

`MAXPENDCONN 10` (`includes/Server.hpp:7`) é apenas a fila de espera do `listen()`. O
`addNewClient` (`srcs/Server.cpp:145`) aceita sem limite: não há tecto em `Clients.size()`
nem em `Fds.size()`. Medido:

```text
inicio                 : 4 fds, 3600 kB
300 ligações abertas   : 304 fds, 3800 kB   (delta 208 kB)
depois de fechar todas : 4 fds
```

Não é uma explosão de memória — cada `Client` é barato. O problema é o `poll()`, que é
chamado com o vector inteiro a cada ciclo e é linear no número de fds. Com 10 000 clientes
o servidor fica lento para todos. O `ulimit -n` desta máquina é 1 048 576, portanto o
processo não bate no limite do sistema antes de o `poll()` ficar inviável.

**Correcção:** tecto em `Clients.size()` e fechar a ligação imediatamente quando for
atingido, com um `MAX_CLIENTS` em `includes/Server.hpp`.

### V3 — Fuga temporal na comparação da password

`srcs/commands_files/Registration.cpp:73` usa `Password != parameters`, ou seja
`std::string::operator==`, que compara `size()` e depois o conteúdo e **desvia assim que
encontra uma diferença**. O tempo de resposta depende de quantos bytes iniciais coincidem, e
com tentativas suficientes a password pode ser reconstruída byte a byte.

O bypass do NUL que existia antes já está resolvido, porque `checkError` apanha o `\0` antes
da comparação. O que fica é a fuga temporal.

`Verify::constantTimeEquals` existia exactamente para isto, no mesmo ficheiro, e foi removida
na reescrita do `handlePass`. Como o servidor é um processo único a diferença é pequena, mas
é mensurável em rede local.

**Correcção:** restaurar a comparação em tempo constante e voltar a usá-la no `handlePass`.

### V5 — `iscntrl` depende do locale (latente)

`iscntrl` classifica segundo o locale activo. O projecto nunca chama `setlocale`, portanto
está no locale "C" e o comportamento é determinista: `0x00`–`0x1F` e `0x7F`.

Risco latente, não bug actual: se alguém acrescentar `setlocale(LC_ALL, "")` para suportar
acentos, em alguns locales os bytes `0x80`–`0x9F` passam a contar como imprimíveis e
`Verify::checkError` passaria a aceitá-los. O `AGENTS.md` não menciona locale.

**Correcção:** ou fixar `setlocale(LC_CTYPE, "C")` no arranque, ou classificar por tabela
própria em vez de depender de `<cctype>`.

## Prioridade 2 — Protocolo

### D2 — O `001` está incompleto

`srcs/commands_files/Registration.cpp:84`, `:116` e `:151`. A mensagem é só
`":Welcome to the IRC Network"`, falta o `<nick>!<username>@<host>`.

```text
actual  b':Broadcast_Server 001 alice :Welcome to the IRC Network'
esperado b':Broadcast_Server 001 alice :Welcome to the IRC Network alice!alice@127.0.0.1'
```

O RFC 2812 §3.1 pede o mesmo. Os dados já estão todos no `Client` — `getNick()`,
`getUsername()`, `getIpAddr()` — por isso é uma concatenação, e são três linhas repetidas em
três handlers.

**Vale a pena criar já um acessor** para o prefixo, porque o #14 abaixo precisa dele e não
deve haver três copias da mesma concatenação.

### #14 — Broadcast da mudança de nickname

A única verificação da especificação de `PASS`/`NICK`/`USER` que continua em falta. Das 18
que a especificação lista, 17 estão conformes.

O RFC 1459 §3.1 diz que depois de um `NICK` bem-sucedido o servidor tem de enviar o `NICK`
ao próprio cliente e a todos os que partilham um canal com ele:

```text
S -> C (para ti)                    :<nick-antigo>!<user>@<host> NICK :<nick-novo>
S -> C (outros nos mesmos canais)  :<nick-antigo>!<user>@<host> NICK :<nick-novo>
```

O `handleNick` hoje muda o nick em silêncio.

**Bloqueio resolvido:** o `B1` está corrigido, por isso o `Client` copiado para dentro de
`Channel::Clients` já preserva o nick. Resta um cuidado: o `Client` dentro do canal é uma
cópia (`includes/Channel.hpp:15`), portanto o broadcast tem de iterar os canais e usar o nick
novo, não o que estiver guardado.

### B8 — Comando desconhecido aborta o resto do buffer

`srcs/Server.cpp:188-210`. O `dispatchCommand` devolve `false` tanto para o `QUIT`, que quer
desligar o cliente, como para comandos desconhecidos, e o `handleClientData` trata qualquer
`false` como fim de sessão. Confirmado contra o binário:

```text
enviado  b'PASS secret\r\nFOOBAR x\r\nNICK alice\r\nUSER alice 0 * :Alice\r\n'
recebido b''
```

O `PASS` é processado, o `FOOBAR` devolve `false` e o resto do buffer é abandonado. O
cliente nunca recebe o `001` e nunca recebe o `421` que o `IMPLEMENTACAO.md` dá como
implementado.

**Precisa de uma decisão:** corrigir exige decidir se o `421` passa a existir. A alternativa
mínima é distinguir "desligar" de "desligar por comando desconhecido" sem inventar numeric —
por exemplo, `dispatchCommand` a devolver um enum, e o `421` a ficar para uma tarefa
própria.

## Prioridade 3 — Robustez

Nenhum destes é alcançável remotamente; são bugs de manutenção.

### B3 — O loop de `poll()` salta fds

`srcs/Server.cpp:256` itera `for (size_t i = 0; i < Fds.size(); i++)` e chama
`handleClientData`, que pode chegar a `removeClients` e apagar um elemento de `Fds`
(`srcs/Server.cpp:97`). O `erase` desloca os elementos seguintes e o `i++` salta um.
Verificado com `Fds = {3, 10, 11, 12}`: ao fechar o fd 10 o ciclo visita `3, 10, 12` e o
**11 nunca é processado**.

Impacto: atraso, não perda — o `Client::Inbuff` mantém-se. Pior quando vários clientes
fecham no mesmo ciclo, porque o salto acumula-se. Interage mal com o V2.

### B2 — `Server::closeFds()` percorre o `map` por índice

`srcs/Server.cpp:36-48` faz `for (size_t i = 0; i < this->Clients.size(); i++)` e acede a
`this->Clients[i]`. `Clients` é um `map<int, Client>` cujas chaves são fds, por isso as
chaves `0` a `fd_máximo` não existem e o `operator[]` **cria** entradas novas com um `Client`
por defeito, cujo `Fd` é `-1`. Verificado com um cliente no fd 9: o map passa de 1 para 10
entradas, com as chaves 0 a 8 a reportar `getFd() == -1`.

Não é loop infinito — termina em `fd_máximo + 1` iterações, porque a partir daí `Clients[i]`
já existe e o map deixa de crescer, e o fd real acaba por ser fechado. O que está errado é o
map ficar poluído com entradas fantasma, cada uma provocar um `close(-1)` que falha com
`EBADF`, e o `cout` imprimir `Client <-1> Disconnected` várias vezes. Como `closeFds()` é
`public` (`includes/Server.hpp:46`), se for chamada com o servidor vivo os clientes reais
ficam estruturados num map cujas chaves 0 a 8 não são sockets.

**Correcção:** iterar com iteradores, como já é feito em `removeClients`.

### B4 — `Server` não copia os containers

`srcs/Server.cpp:14-17`. O construtor de cópia só inicializa `Listen_port`, `Password`,
`Listen_fd` e `Name`; `Fds`, `Clients` e `Channels` ficam como construídos por defeito. O
`operator=` (`:19-29`) também não os copia. Verificado: um `Server` com 1 cliente e 1 fd
produz uma cópia com 0 e 0. O construtor de cópia é público (`includes/Server.hpp:40`),
portanto o `Server` é copiável e enganoso.

### B5 — `Channel` não copia `Settings` nem `InvestedUsers`

`includes/Channel.hpp:17-18` declara os dois vectors. O construtor de cópia
(`srcs/Channel.cpp:11-14`) e o `operator=` (`:16-25`) não os copiam, e há zero ocorrências em
`srcs/Channel.cpp` — hoje não são usados em lado nenhum, por isso ainda não tem efeito.

### B6 — Declarações mortas em `Commands`

`includes/Commands.hpp:24-30` declara `TOPIC`, `MODE`, `JOIN`, `INVITE`, `PART`, `PRIVMSG` e
`KICK` como métodos de instância. Nenhuma tem definição em qualquer `.cpp` e nenhuma é
chamada. Compila porque nunca são usadas, mas a primeira chamada passa a ser erro de link.

Depois da extracção do `Parser`, o `Commands` ficou só com este esqueleto: os membros
`Command`, `Parameters` e `Prefix`, o construtor de 3 argumentos, e estas sete declarações.
`getCommand()`, `getParameters()` e `getPrefix()` não são chamadas em lado nenhum, e os
handlers reais vivem no `Server`, não aqui. **A extracção não resolve o B6** — isso depende
de quem mexer no `Commands`.

## Parser — o que falta para os comandos por implementar

O `buildParams()` em `srcs/Parser.cpp` já mapeia os onze comandos para os seus
`ParsedParamId`. O que falta é o que só faz sentido quando os comandos existirem:

- **`getParams`**: hoje só há `getParam`, que devolve a primeira ocorrência. Um id
  repetido precisa de uma variante que itere todos os valores.
- **Separação por vírgulas**: `JOIN #a,#b` dá `PP_CHANNEL = "#a,#b"`, uma string só, e
  `KICK #a u1,u2` idem. É uma linha em `buildParams` mais o `getParams` acima.
- **`PP_MODE_ARGS`**: o `MODE` precisa de um id para os argumentos de modo, que são
  variáveis em número (`MODE #a +ooo a b c`). Hoje o `MODE` tem só `PP_CHANNEL` e
  `PP_MODE`, e o que sobra vai para `PP_UNKNOWN`.

**Contrato a respeitar:** tokens que não correspondem a nenhum slot esperado vão todos para
um `PP_UNKNOWN` único, com o espaçamento original intacto. Os handlers usam
`hasParam(command, PP_UNKNOWN)` para os rejeitar. Vale a pena preservar: foi o que impediu
que `NICK bad nick` fosse aceite com nick `bad` quando o parser começou a truncar tokens em
silêncio.

## Testes

**Não há um único teste no repositório** — nem directório, nem alvo no `Makefile`, nada.
Tudo o que foi verificado até agora foi feito com programas externos em `/tmp`, que não
sobrevivem a um reboot. **Já aconteceu**: os 33 casos do parser e os 22 de integração que
tinham passado perderam-se numa limpeza do `/tmp`, e a próxima alteração vai ter de os
reescrever do zero para os voltar a correr.

Isto não é teoria. A validação de `NICK bad nick` só foi apanhada **quando passou a haver
binário**: antes disso a lógica estava correcta no papel e errada em código. A segunda
regressão do refactor, o `QUIT a b c` sem dois pontos, só apareceu com teste de integração.
Sem testes no repositório, o próximo refactor volta a passar pelo mesmo.

**Recomendação:** levar os dois conjuntos para dentro do projecto e acrescentar alvos ao
`Makefile`:

- **parser** — 33 casos, ligados aos objectos reais, a cobrir o mapeamento de cada
  comando, os `:` finais, a presença e ausência de parâmetros obrigatórios, o `PP_UNKNOWN` e
  comandos em minúsculas
- **integração** — 22 casos, contra o binário: fluxo completo e nas três ordens possíveis de
  `PASS`/`NICK`/`USER`, `462` no re-registo, validações, `QUIT` com e sem dois pontos, linha
  partida entre pacotes, vários comandos num só pacote, NUL na password

Enquanto isso não existir, o `AGENTS.md` devia dizer explicitamente que a verificação é
manual.

### Estado actual de cada conjunto

Reescritos e corridos depois da extracção do `Parser`, o que felizardamente não está no `/tmp`
por acaso e sim porque foram reescritos.

**Parser: 33 casos, 3 falham.** Os três pela mesma causa, o **R1**:

```text
FALHA USER octa 0 * -> PP_REALNAME presente        devia não existir
FALHA QUIT          -> PP_REASON presente          devia não existir
FALHA QUIT bye      -> PP_UNKNOWN="bye"            esperava "a b c" (o teste estava errado)
```

O terceiro é um erro do próprio teste, não do código: `QUIT bye` só tem uma palavra, e o
`PP_UNKNOWN` correcto é `"bye"`. Os outros dois são o R1.

**Integração: 22 casos, 1 falha nova.** A falha é o R1 visto de fora:

```text
FALHA USER sem realname  ->  ''
```

Tudo o resto passa, incluindo os três regimes de registo, as validações, o `QUIT` com e sem
dois pontos, e o `462`. O **B8** continua a falhar como esperado.

**Sobre o `Parser`:** os testes linkam contra `objs/srcs/Parser.o`, não `Commands.o`, depois
da extracção. Quem acrescentar um ficheiro novo tem de lembrar que o `Makefile` tem listas
explícitas — um ficheiro esquecido não dá erro nenhum, apenas não é compilado.

## Documentação desatualizada

Estes três documentos descrevem um estado que o código já não tem:

- **`IMPLEMENTACAO.md`** dá `421` e `451` como implementados — não existem. Dá `462` como
  já implementado — passou a existir com a tarefa 8. Dá limites `MAX_NICK_LEN` e
  `MAX_USER_LEN` — foram finalmente criados, com `MAX_REAL_LEN` e `MAX_PASS_LEN` que o
  documento não menciona. Dá a propagação de mudança de `NICK` como feita — é o #14.
  Descreve `parse()` a devolver `pair<string, string>` — já não é assim. Fala do `Outbuff`
  como "ainda por usar" — foi removido do `Client`.
- **`AGENTS.md`** repete a descrição antiga do `parse()`, e a do `Outbuff`.
- **`README.md`** dá `JOIN`, `PART` e `PRIVMSG` como implementados — os três ficheiros de
  comandos estão a zero bytes.

**Nota sobre os limites:** `MAX_NICK_LEN 30` e `MAX_USER_LEN 30` vieram do que o
`IMPLEMENTACAO.md` já afirmava. `MAX_REAL_LEN 255` e `MAX_PASS_LEN 128` foram escolhidos sem
referência na especificação e convém confirmá-los. Nenhum RFC impõe um valor — a
especificação de `NICK` diz só "documenta o teu".

**Por decidir, da especificação de `PASS`/`NICK`/`USER`:** o `464` é diferido ou imediato?
A especificação mostra-o a chegar só depois de `NICK` e `USER`; os servidores reais
(unrealircd, inspircd) respondem de imediato. O exemplo também é compatível com resposta
imediata, porque o `*` se explica só pela falta de nick, por isso a ambiguidade está no
texto e não no código.
