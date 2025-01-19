# Hash Table Server and Client with Concurrent Operations

## Overview

This project implements a **Server** and a **Client** that communicate using **POSIX shared memory** (`shm`) and support concurrent operations (insert, get, delete) on a **hash table** with **reader-writer locks** for thread safety.

- **Server**: Initializes a hash table, processes operations, and communicates with the client via shared memory.
- **Client**: Sends requests (insert, get, delete) to the server for hash table operations via shared memory.

## Requirements

- POSIX Shared Memory (`shm`)
- `pthread` library for multithreading
- GCC compiler

## Files

- `server.c`: The server program
- `client.c`: The client program
- `Makefile`: The build script

## Instructions

### 1. **Compiling the Programs**

To compile the server and client programs, run:

```bash
make
```

### 2. Running the Server

To run the server, specify the size of the hash table as a command-line argument. For example, to run the server with a hash table size of 10, use:
```bash
./server 10
```

### 3. **Running the Client**

To interact with the server, you can send a series of operations via the client. 

For example, to insert, get, and delete data from the hash table, use the following commands:

```bash
./client insert key1 value1 get key1 delete key1 get key1
```