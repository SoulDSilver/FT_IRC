*This project has been created as part of the 42 curriculum by <login1>, <login2>, <login3>.*

# ft_irc

## Description

`ircserv` is a custom IRC server written in C++98, compliant with the `ft_irc` subject.
It handles multiple clients simultaneously using a single `poll()` loop, non-blocking
I/O, client authentication, nickname/username registration, channel join/part,
private and channel messaging, and the channel-operator commands `KICK`, `INVITE`,
`TOPIC` and `MODE` (`i`, `t`, `k`, `o`, `l`).

Server-to-server communication and a built-in IRC client are explicitly out of scope
(forbidden by the subject).

## Instructions

### Build

```
make
```

Produces the `ircserv` executable. Other Makefile targets: `clean`, `fclean`, `re`.

### Run

```
./ircserv <port> <password>
```

- `port`: TCP port to listen on.
- `password`: password required by clients to authenticate (`PASS`).

### Connect

Use any standard IRC client (our reference client: **_TODO: name it here_**), e.g.:

```
/server 127.0.0.1 <port> <password>
```

or test manually with `nc`:

```
nc 127.0.0.1 <port>
PASS <password>
NICK alice
USER alice 0 * :Alice
JOIN #general
PRIVMSG #general :hello!
```

## Feature list (mandatory part)

- [x] TCP/IP server, non-blocking sockets, single `poll()`
- [x] Multi-client handling, no forking
- [x] Buffer aggregation for commands split across several `recv()` calls
- [x] `PASS`, `NICK`, `USER` authentication/registration
- [x] `JOIN`, `PART`, `PRIVMSG` (user and channel)
- [x] Operators and regular users
- [x] `KICK`, `INVITE`, `TOPIC`, `MODE` (i/t/k/o/l)
- [ ] `RPL_NAMREPLY` / `RPL_ENDOFNAMES` on JOIN (TODO, see ChannelCommands.cpp)
- [ ] Cleanup of a client from all channels on QUIT/disconnect (TODO, see Server.cpp)
- [ ] Erase empty channels after the last member leaves (TODO, see ChannelCommands.cpp)

## Technical choices

- One `Server` class owns the listening socket, the single `poll()` loop, the
  `std::map<int, Client>` of connected clients and the `std::map<std::string, Channel>`
  of channels.
- Each IRC command is a free function declared in `includes/Commands.hpp` and
  implemented in one file under `srcs/commands/`, split by topic so three people
  can work in parallel without touching the same file:
  - `Registration.cpp` — `PASS`, `NICK`, `USER`, `PING`, `QUIT`
  - `ChannelCommands.cpp` — `JOIN`, `PART`, `TOPIC`
  - `OperatorCommands.cpp` — `KICK`, `INVITE`, `MODE`
  - `Messaging.cpp` — `PRIVMSG`
- Numeric replies are centralized in `includes/replies.hpp`.
- Never read/write on a socket outside of a `poll()`-flagged event, and never
  branch on `errno` after a `recv`/`send`, per the subject's constraints.

## Resources

- RFC 1459 — Internet Relay Chat Protocol
- RFC 2812 — Internet Relay Chat: Client Protocol
- `man poll`, `man socket`, `man fcntl`
- modernIRC (https://modern.ircdocs.horse/) — up-to-date reference for numeric replies and command syntax

### AI usage

_TODO (fill in per teammate as the project progresses), e.g.:_
- Used an AI assistant to scaffold the initial project structure (Makefile, class
  skeletons for `Server`/`Client`/`Channel`, command dispatch table) so the team
  could split work from a shared, compiling baseline.
- Every generated function was read, discussed between teammates, and tested
  manually (see the `nc`/fragmented-packet test in the subject) before being
  considered "done".
- <name the parts each of you asked AI for help with, and what you changed/verified>
