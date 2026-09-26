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
