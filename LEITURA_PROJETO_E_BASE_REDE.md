# Leitura do projeto `ft_irc`

## 1. Objetivo do projeto

Este repositório é a base de um servidor IRC chamado `ircserv`, escrito em C++98 para o projeto `ft_irc` da 42. A ideia geral é aceitar vários clientes TCP ao mesmo tempo e, depois, interpretar comandos IRC como `PASS`, `NICK`, `USER`, `JOIN`, `PART`, `PRIVMSG`, `KICK`, `INVITE`, `TOPIC` e `MODE`.

O servidor não deve criar um processo ou uma thread por cliente. A arquitetura prevista é baseada em:

- um socket TCP de escuta;
- sockets TCP individuais para os clientes;
- um único `poll()` para vigiar o socket de escuta e todos os sockets de clientes;
- sockets em modo não bloqueante (`O_NONBLOCK`);
- uma camada posterior para ler, acumular e interpretar comandos IRC.

Este documento concentra-se apenas na primeira base de rede: socket de escuta, `poll()`, `accept()` e configuração não bloqueante. A intenção é que a Pessoa A entenda essa base antes de adicionar autenticação, registro, canais e mensagens.

---

## 2. Estado atual do repositório

### Arquivos de compilação e entrada

- `Makefile`: compila os arquivos em C++98 com `-Wall -Wextra -Werror` e `-Iincludes`.
- `srcs/main.cpp`: valida os argumentos de linha de comando, cria um `Server` e verifica se o socket de escuta foi criado. O laço existente é apenas um placeholder e termina imediatamente.
- `includes/irc.hpp`: reúne includes gerais de C++, sockets, `fcntl`, sinais e outras APIs de sistema.

### Classe `Server`

A classe está declarada em `includes/Server.hpp` e possui:

- `listen_port`: porta escolhida pelo usuário;
- `password`: senha do servidor;
- `listen_fd`: descritor do socket de escuta;
- `listen_socket(int)`: método privado que prepara o socket;
- `run()`: ponto previsto para o laço principal;
- getters para porta e descritor.

Em `srcs/Server.cpp`, `listen_socket()` já faz parte da preparação básica:

1. chama `socket(AF_INET, SOCK_STREAM, 0)`;
2. habilita `SO_REUSEADDR` com `setsockopt()`;
3. preenche uma estrutura `sockaddr_in`;
4. associa o socket à porta com `bind()`;
5. configura o descritor como não bloqueante com `fcntl()`;
6. coloca o socket em modo de escuta com `listen()`;
7. retorna o descritor criado ou `-1` em caso de erro.

O método `run()` ainda não executa o loop de eventos. Atualmente ele apenas ignora `listen_fd`.

### Classe `Client`

`Client` possui um descritor, um nickname e um username. Já existem construtor, cópia, atribuição, destrutor e getters básicos.

Ainda não existem, nesta classe, funções para:

- receber dados;
- enviar respostas;
- acumular dados incompletos;
- controlar o estado de registro;
- guardar senha, nickname e username de forma completa;
- detectar ou marcar desconexão.

### Classe `Channel`

`Channel` possui nome e um vetor de `Client`. Porém `addClient()` e `removeClient()` ainda não alteram o vetor: são placeholders.

### Comandos

`Commands` contém apenas campos para comando, parâmetros e prefixo, além de alguns métodos declarados. Os arquivos sob `srcs/commands_files/` estão vazios no estado atual. Portanto, os comandos IRC descritos no `README.md` ainda não estão implementados neste checkout.

### Divergência importante com o `README.md`

O `README.md` lista várias funcionalidades como concluídas, incluindo o loop único de `poll()`, múltiplos clientes e comandos IRC. O código atual não corresponde a essa lista: o socket de escuta está parcialmente preparado, mas o loop de eventos, o `accept()` integrado ao `poll()` e os comandos ainda precisam ser construídos.

Para trabalhar corretamente, deve-se considerar o código-fonte como a fonte de verdade atual e o README como uma descrição do objetivo pretendido.

---

## 3. Conceitos fundamentais

### 3.1 Descritor de arquivo

Um socket é representado pelo sistema operacional por um inteiro chamado file descriptor, ou `fd`.

Neste projeto haverá pelo menos:

- um `fd` para o socket de escuta;
- um `fd` para cada cliente conectado.

O `fd` não é uma conexão em si. Ele é um identificador usado nas chamadas `poll()`, `accept()`, `recv()`, `send()` e `close()`.

É importante que o servidor seja responsável pelo ciclo de vida de cada descritor:

1. criar;
2. configurar;
3. inserir no conjunto observado;
4. usar quando houver evento;
5. remover quando a conexão fechar;
6. fechar exatamente uma vez.

### 3.2 Socket de escuta

O socket criado com:

```cpp
socket(AF_INET, SOCK_STREAM, 0)
```

é um socket TCP IPv4.

As etapas têm papéis diferentes:

- `socket()`: cria o descritor;
- `setsockopt(SO_REUSEADDR)`: permite reutilizar rapidamente um endereço local após uma reinicialização;
- `bind()`: associa o socket a um endereço e a uma porta;
- `listen()`: transforma o socket em socket passivo, capaz de receber pedidos de conexão;
- `accept()`: retira uma conexão pendente da fila e devolve um novo descritor para conversar com aquele cliente.

O socket de escuta não deve ser usado para trocar mensagens IRC. Ele apenas recebe novas conexões. Cada chamada bem-sucedida a `accept()` produz um novo socket de cliente.

### 3.3 `INADDR_ANY`

O código usa `htonl(INADDR_ANY)`. Isso faz o servidor escutar em todas as interfaces IPv4 disponíveis na máquina, e não apenas em `127.0.0.1`.

`htons()` e `htonl()` convertem valores para a ordem de bytes da rede. A porta deve ser convertida com `htons()` antes de ser colocada em `sockaddr_in`.

### 3.4 Socket não bloqueante

Em modo bloqueante, uma chamada como `accept()` ou `recv()` pode esperar indefinidamente. Isso seria incompatível com um servidor que precisa cuidar de muitos clientes usando um único fluxo de execução.

Em modo não bloqueante, uma operação que não pode avançar imediatamente retorna sem travar o processo. O servidor consulta primeiro o estado dos descritores com `poll()` e só trabalha nos descritores marcados pelo evento.

A intenção geral é:

- o socket de escuta ser não bloqueante;
- cada socket retornado por `accept()` também ser configurado como não bloqueante;
- nenhuma leitura ou escrita bloquear o loop principal.

O código atual já chama `fcntl()` para o socket de escuta. A implementação da Pessoa A deverá aplicar a mesma ideia aos sockets aceitos.

Uma forma conceitualmente cuidadosa de configurar a flag é preservar as flags existentes e adicionar `O_NONBLOCK`:

```text
flags = fcntl(fd, F_GETFL, 0)
fcntl(fd, F_SETFL, flags | O_NONBLOCK)
```

O código atual usa diretamente `F_SETFL` com `O_NONBLOCK`. Isso pode funcionar neste caso, mas substitui as flags de status existentes. É um ponto para a Pessoa A entender e revisar ao implementar a base definitiva.

### 3.5 Fila de conexões e `listen(backlog)`

`listen(lfd, 10)` cria uma fila de conexões pendentes com backlog 10. Isso não limita o número total de clientes do servidor; representa a quantidade de conexões que podem aguardar atendimento antes de serem aceitas, sujeita às regras do sistema operacional.

O socket de escuta fica legível para `poll()` quando existe pelo menos uma conexão pendente. Nesse momento, o servidor chama `accept()`.

---

## 4. O único `poll()` do servidor

O loop principal deve ter um único `poll()` que observe:

1. o socket de escuta;
2. todos os sockets de clientes ativos.

A estrutura típica é um vetor de `struct pollfd`. Cada entrada associa um descritor a eventos desejados e eventos encontrados:

```text
pollfd.fd       -> descritor observado
pollfd.events   -> eventos que interessam ao servidor
pollfd.revents  -> eventos devolvidos pelo kernel
```

Para o socket de escuta, o evento principal é `POLLIN`, que significa que há algo para ler do ponto de vista do socket passivo: uma nova conexão está aguardando.

Para um socket de cliente, inicialmente o evento necessário costuma ser `POLLIN`, porque o servidor quer saber quando o cliente enviou dados ou fechou a conexão. `POLLOUT` só deve ser habilitado quando houver dados realmente aguardando envio, caso contrário o socket pode aparecer constantemente como gravável e ocupar o loop sem necessidade.

O fluxo conceitual é:

```text
criar o socket de escuta
configurar o socket de escuta como não bloqueante
colocar o socket de escuta no vetor de pollfd

repetir enquanto o servidor estiver ativo:
    chamar o único poll()

    se o socket de escuta tiver POLLIN:
        aceitar novas conexões
        configurar cada cliente como não bloqueante
        adicionar cada cliente ao vetor observado

    para cada socket de cliente que tiver evento:
        tratar leitura, fechamento ou erro
```

O vetor pode ser `std::vector<pollfd>` ou outra estrutura equivalente. Além do vetor de `pollfd`, o servidor precisará relacionar cada `fd` ao seu objeto `Client`. Essa relação pode ser feita por mapa, vetor ou outra estrutura coerente com o restante do projeto. O ponto essencial é não perder a correspondência entre o evento recebido e o cliente correto.

### Por que não usar um `poll()` por cliente?

Um `poll()` por cliente quebraria a arquitetura desejada e tornaria difícil acompanhar todos os clientes no mesmo ciclo. O objetivo é que uma única chamada informe quais descritores precisam de atenção naquele instante.

### O que `poll()` não faz

`poll()` não:

- aceita a conexão;
- lê os bytes;
- interpreta IRC;
- envia respostas;
- fecha automaticamente o cliente.

Ele apenas espera e informa eventos. O código do servidor continua responsável por executar a ação correspondente.

---

## 5. `accept()` de novas conexões

Quando `poll()` informar `POLLIN` no descritor de escuta, o servidor deverá chamar `accept()`.

A chamada retorna um novo descritor:

```text
listen_fd  -> continua sendo o socket que recebe novas conexões
client_fd  -> representa uma conexão específica com um cliente
```

O `listen_fd` nunca deve ser substituído pelo `client_fd`. O servidor precisa manter o primeiro ativo para aceitar clientes futuros.

Para cada nova conexão aceita, a sequência conceitual é:

1. chamar `accept()`;
2. verificar se o retorno é válido;
3. configurar o novo descritor com `O_NONBLOCK`;
4. criar ou registrar o `Client` correspondente;
5. inserir o `client_fd` no conjunto observado pelo único `poll()`;
6. aguardar eventos futuros para ler dados desse cliente.

Como o socket de escuta é não bloqueante, pode existir uma situação em que a conexão já não esteja disponível quando `accept()` for executado. Isso pode acontecer por uma condição de corrida do sistema ou por conexões pendentes que desapareceram. O código deve tratar a falha sem derrubar o servidor.

Também é possível que mais de uma conexão esteja pendente quando o socket de escuta for marcado como legível. A implementação deve decidir se aceita uma conexão por passagem pelo loop ou se drena todas as conexões disponíveis com chamadas repetidas a `accept()` até não haver mais nenhuma. Em ambos os casos, cada novo cliente deve ser configurado e registrado corretamente.

---

## 6. Eventos que a Pessoa A deve distinguir

### `POLLIN`

Há dados disponíveis para leitura ou, no socket de escuta, há uma conexão esperando para ser aceita.

### `POLLHUP`

O outro lado encerrou a conexão ou ocorreu uma desconexão. O servidor deve remover o cliente das estruturas internas e fechar o descritor.

### `POLLERR`

Ocorreu um erro no descritor. O cliente normalmente precisa ser encerrado e removido, depois de qualquer diagnóstico necessário.

### `POLLNVAL`

O descritor não é válido, normalmente porque foi fechado ou não foi configurado corretamente. Isso indica um problema de controle do ciclo de vida dos `fd`s.

Um único evento pode combinar mais de uma dessas flags. O código não deve assumir que `revents` terá apenas um valor.

---

## 7. Ordem e segurança do loop

O loop precisa lidar com remoções sem invalidar a iteração atual. Quando um cliente for fechado:

- o `fd` deve ser removido do vetor de `pollfd`;
- o objeto ou registro do cliente deve ser removido ou marcado como inativo;
- o descritor deve ser fechado;
- o índice da iteração deve ser ajustado se a remoção deslocar outros elementos.

Não se deve continuar usando um `fd` depois de fechá-lo. Também não se deve deixar no vetor um `pollfd` apontando para um cliente que já não existe.

O código deve evitar chamadas de leitura e escrita fora do momento apropriado. A regra arquitetural deste projeto é consultar os eventos de `poll()` e agir sobre os descritores indicados por `revents`.

Para a primeira etapa, o foco é aceitar conexões. A leitura de dados dos clientes pode ser deixada como uma etapa posterior, mas o desenho do vetor deve permitir adicioná-la sem criar um segundo loop de `poll()`.

---

## 8. Relação com `main()` e `Server::run()`

O construtor de `Server` atualmente chama `listen_socket(port)` e guarda o retorno em `listen_fd`.

Depois de validar que o descritor é válido, `main()` deveria entregar o controle ao método de execução do servidor. No estado atual, `main()` possui um `while` vazio que executa uma vez e termina.

A responsabilidade esperada é:

- `main()`: validar argumentos, construir o servidor e iniciar sua execução;
- `Server`: possuir o socket de escuta, o conjunto de clientes e o loop único de eventos;
- camada de cliente: guardar o estado e os dados de cada conexão;
- camada de comandos: interpretar e executar o protocolo IRC posteriormente.

A divisão exata pode evoluir, mas a Pessoa A deve evitar colocar o loop de rede dentro de cada comando. O loop de eventos é a infraestrutura que todos os comandos irão utilizar.

---

## 9. O que está pronto para esta primeira etapa

Já existe:

- criação de socket TCP IPv4;
- configuração de `SO_REUSEADDR`;
- associação à porta com `bind()`;
- configuração preliminar de não bloqueio no socket de escuta;
- chamada de `listen()`;
- armazenamento do descritor no objeto `Server`;
- fechamento do socket de escuta no destrutor;
- validação básica da porta em `main()`;
- regra de compilação para incluir `includes/`.

Ainda falta, especificamente para a base de rede:

- incluir e utilizar corretamente a API de `poll()`;
- criar o vetor ou estrutura que representa os `pollfd` ativos;
- implementar o loop único de `poll()`;
- colocar o socket de escuta nesse conjunto;
- chamar `accept()` quando houver `POLLIN` no socket de escuta;
- configurar cada socket aceito como não bloqueante;
- registrar os novos clientes junto dos seus `fd`s;
- tratar fechamento e erros sem deixar descritores inválidos;
- fazer `main()` iniciar o loop real de `Server`.

O header atual inclui `sys/select.h`, mas a infraestrutura de `poll()` normalmente deve incluir a declaração específica de `poll`, como `poll.h`. Isso é uma verificação necessária antes ou durante a implementação, não uma funcionalidade já concluída.

---

## 10. O que não deve ser implementado nesta primeira etapa

Para manter a base compreensível e permitir que as próximas pessoas trabalhem em cima dela, a Pessoa A não precisa implementar agora:

- parsing de linhas IRC;
- `PASS`, `NICK` ou `USER`;
- validação completa de registro;
- canais e operadores;
- `PRIVMSG`;
- buffer de comandos fragmentados;
- respostas numéricas IRC;
- regras de `QUIT` e remoção de clientes de canais.

Esses recursos dependem do loop de rede, mas não devem ser misturados à primeira implementação. A base deve entregar uma conexão aceita, configurada e observada pelo mesmo `poll()` que futuramente vigiará os eventos de todos os clientes.

---

## 11. Modelo mental para a Pessoa A

Pense no servidor como uma recepção central:

- o socket de escuta é a porta de entrada;
- `accept()` transforma uma chegada em uma conexão individual;
- cada `client_fd` é um telefone separado com um cliente;
- `poll()` é a central que informa qual telefone tocou;
- o loop principal atende apenas os telefones que sinalizaram atividade;
- os comandos IRC são a conversa que será construída depois.

A responsabilidade desta primeira etapa termina quando:

1. o servidor abre a porta;
2. aguarda eventos com um único `poll()`;
3. aceita novas conexões sem bloquear;
4. configura os sockets dos clientes como não bloqueantes;
5. acompanha todos os descritores pelo mesmo mecanismo;
6. consegue remover uma conexão encerrada sem corromper as estruturas internas.

Essa é a fundação sobre a qual as outras pessoas poderão adicionar leitura, buffers, autenticação, canais e mensagens.
