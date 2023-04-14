Valentin-Ioan VINTILĂ (323CA)
-----------------------------

    My homework implements a simple C++ application which allows UDP clients to
connect to a server that acts like a broker, communicating with TCP subscribers.
    The implementation is written in modern C++ (C++ 17) and contains some
useful extra features, such as logging capabilities and gracefully quiting when
Ctrl+C is pressed (SIGINT).


File structure
==============

    The files are organized as presented bellow (in alphabetical order):

    > Makefile
        : A complete makefile, containing all the necessary building rules, as
        : well as testing and cleaning rules.
    > readme.txt
        : This file, a tour of the project.
    > server.cpp
        : This is the TCP/UDP server that shall act as a broker.
    > structs.hpp + struct.cpp
        : These files contain the structures that will be used to send messages
        : back and forth.
    > subscriber.cpp
        : This is the TCP client (subscriber) that shall receive the data
        : provided by the other UDP clients.
    > utils.hpp + utils.cpp
        : These files contain the logging implementation and a simple way to
        : break a string into commands.

Used structures
===============

    In order to communicate efficiently, multiple structs were defined (for more
info, please refer to the structs.hpp / structs.cpp files):

    > tcp_message_t
        : Used for server -> TCP client communication;
        : Contains details about the message itself (message_type, topic,
        : payload) and about its UDP origins (from_ip, from_port).
    > udp_message_t
        : Used for UDP client -> server communication;
        : Contains details about the topic of the broadcasted message, its type
        : and the message itself (topic, message_type, payload).
    > subscriber_t
        : This structure is meant to store some information required to easily
        : distinguish between TCP clients.
    > message_from_tcp_t
        : Used for TCP client -> server communication;
        : Contains details about the command that should be sent (command), the
        : topic (in case of (un)subscribe), the store and forward parameter (in
        : case of subscribe) and the client's unique ID.

The TCP clients
===============

    The clients connect to the server and wait for messages. When they arive,
they are imediately printed to the screen.
    Since IO multiplexing was implemented (using epoll()), various commands can
be sent at any time to the server.
    To close down the connection, the exit command can be issued or Ctrl+C can
be pressed (SIGINT is handled).

The server
==========

    The server waits for UDP/TCP messages and input from the user; IO
multiplexing was implemented using epoll().
    The messages received from the UDP clients are translated and sent to the
online TCP clients that are subscribed to their topics. In case said users are
offline, the messages are enqueued and sent later.
    To close down the server, the exit command can be issued or Ctrl+C can be
pressed (SIGINT is handled).

Special features
================

    As it was previously stated, some additional features were implemented as
well, including (but not limited to):

    > Logging capabilities
        : This allows for quick and easy development. In order to turn this
        : feature ON, please modify the Makefile by uncommenting the
        : -DLOG_ENABLE flag.
    > Ctrl+C (SIGINT) guard
        : This features disables the default behaviour of SIGINT so that it
        : can gracefully disconnect all the clients and close down the server.
    > Input sanitization
        : The user should not be able to break the server when communicating
        : with it.
    > Speed
        : To help out with the speed, epoll() is used instead of any other
        : slower alternative, such as poll() or select().
