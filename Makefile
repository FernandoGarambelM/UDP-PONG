CC = gcc
CFLAGS = -Wall -Wextra -std=c11 -O2 -Iinclude
LIBS = -lncurses -lm

# Directorios
SRC_DIR = src
INC_DIR = include
BIN_DIR = bin
OBJ_DIR = build

# Archivos fuente
SERVER_SRC = $(SRC_DIR)/pong_server.c
CLIENT_SRC = $(SRC_DIR)/pong_client.c
COMMON_SRC = $(SRC_DIR)/utils.c $(SRC_DIR)/stats.c

# Archivos objeto
SERVER_OBJ = $(OBJ_DIR)/pong_server.o $(OBJ_DIR)/utils.o $(OBJ_DIR)/stats.o
CLIENT_OBJ = $(OBJ_DIR)/pong_client.o $(OBJ_DIR)/utils.o $(OBJ_DIR)/stats.o

# Binarios
SERVER_BIN = $(BIN_DIR)/pong_server
CLIENT_BIN = $(BIN_DIR)/pong_client

# Targets principales
all: $(BIN_DIR) $(OBJ_DIR) $(SERVER_BIN) $(CLIENT_BIN)

# Crear directorios
$(BIN_DIR):
	mkdir -p $(BIN_DIR)

$(OBJ_DIR):
	mkdir -p $(OBJ_DIR)

# Servidor
$(SERVER_BIN): $(SERVER_OBJ)
	$(CC) $(CFLAGS) -o $@ $^ $(LIBS)

# Cliente
$(CLIENT_BIN): $(CLIENT_OBJ)
	$(CC) $(CFLAGS) -o $@ $^ $(LIBS)

# Compilar archivos objeto
$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c
	$(CC) $(CFLAGS) -c $< -o $@

# Limpiar archivos compilados
clean:
	rm -rf $(OBJ_DIR) $(BIN_DIR)

# Ejecutar servidor
run-server: $(SERVER_BIN)
	$(SERVER_BIN)

# Ejecutar cliente
run-client: $(CLIENT_BIN)
	$(CLIENT_BIN)

# Ayuda
help:
	@echo "Comandos disponibles:"
	@echo "  make          - Compilar todo"
	@echo "  make clean    - Limpiar archivos compilados"
	@echo "  make run-server - Compilar y ejecutar servidor"
	@echo "  make run-client - Compilar y ejecutar cliente"

.PHONY: all clean run-server run-client help
