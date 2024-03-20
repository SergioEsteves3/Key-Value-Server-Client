# Server and Client Applications

This repository contains server and client applications for storing `<Key, Value>` pairs from clients. The applications are designed for Linux systems and use Google Protocol Buffers for data serialization. Additionally, the server application employs Zookeeper for redundancy using the Chain Replication method and supports multiple clients concurrently.

## Installation Requirements

Before compiling the applications, ensure the following dependencies are installed:

- [Google Protocol Buffers](https://developers.google.com/protocol-buffers)
- [Zookeeper](https://zookeeper.apache.org/)

## Compilation

The applications can be compiled using the provided Makefile. Use the following command to compile both the server and client applications:

```bash
make all
```

## Usage

Usage for server:

```bash
./table_server <port> <n_lists> <zookeeper_IP:port>
```
- <port>: The port number on which the server will listen for connections.
- <n_lists>: The number of lists the server will manage.
- <zookeeper_IP:port>: The IP address and port number of the Zookeeper server for coordination.

Usage for client:

```bash
./table_client <zookeeper_ip>:<zookeeper_port>
```

- <zookeeper_ip>: The IP address of the Zookeeper server.
- <zookeeper_port>: The port number of the Zookeeper server.

## Server Application

The server application is responsible for storing <Key, Value> pairs received from clients. It supports redundancy using the Chain Replication method with Zookeeper for distributed coordination. The server can handle multiple clients concurrently and utilizes Google Protocol Buffers for data serialization.
Client Application

The client application interacts with the server to store <Key, Value> pairs. It communicates with the server using Google Protocol Buffers.
Usage

Once compiled, you can start the server application and multiple client instances. Clients can then send <Key, Value> pairs to the server for storage.

## Notes

- Ensure Zookeeper is properly configured and running before starting the server application.
- Google Protocol Buffers must be installed and available in the system path for compilation to work.

## Disclaimer

This application was designed and tested on Linux systems. Compatibility with other operating systems may vary.
