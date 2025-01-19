# Compiler and flags
CC = gcc
CFLAGS = -Wall -pthread -lrt

# Define the target programs
SERVER = server
CLIENT = client

# Define the source files for server and client
SERVER_SRC = server.c
CLIENT_SRC = client.c

# Default target (build both client and server)
all: $(SERVER) $(CLIENT)

# Rule to compile the server program
$(SERVER): $(SERVER_SRC)
	$(CC) $(CFLAGS) -o $(SERVER) $(SERVER_SRC)

# Rule to compile the client program
$(CLIENT): $(CLIENT_SRC)
	$(CC) $(CFLAGS) -o $(CLIENT) $(CLIENT_SRC)

# Clean target to remove compiled binaries and objects
clean:
	rm -f $(SERVER) $(CLIENT)
