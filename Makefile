BIN_DIR = binary
INC_DIR = include
LIB_DIR = lib
OBJ_DIR = object
SRC_DIR = source
DEP_DIR = dependencies

CC = gcc
CFLAGS = -Wall -D THREADED -lzookeeper_mt -g -lpthread -MMD -MP -MF $(DEP_DIR)/$*.d -I $(INC_DIR) -L $(LIB_DIR) -ltable

LIB = $(OBJ_DIR)/data.o $(OBJ_DIR)/entry.o $(OBJ_DIR)/list.o $(OBJ_DIR)/table.o
CLIENT = $(OBJ_DIR)/client_stub.o $(OBJ_DIR)/sdmessage.pb-c.o $(OBJ_DIR)/network_client.o $(OBJ_DIR)/table_client.o $(OBJ_DIR)/message.o $(OBJ_DIR)/stats.o
TABLE = $(OBJ_DIR)/network_server.o $(OBJ_DIR)/sdmessage.pb-c.o $(OBJ_DIR)/table_skel.o $(OBJ_DIR)/table_server.o $(OBJ_DIR)/message.o $(OBJ_DIR)/stats.o $(OBJ_DIR)/server.o $(OBJ_DIR)/client_stub.o $(OBJ_DIR)/network_client.o
PROTO_LIB = -L/usr/lib/x86_64-linux-gnu/ -lprotobuf-c

$(OBJ_DIR)/sdmessage.pb-c.o: sdmessage.proto

all: proto table client

table: proto libtable $(TABLE)
	$(CC) $(TABLE) $(CFLAGS) $(PROTO_LIB) -o $(BIN_DIR)/table_server

client: proto libtable $(CLIENT)
	$(CC) $(CLIENT) $(CFLAGS) $(PROTO_LIB) -o $(BIN_DIR)/table_client

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c
	$(CC) $(CFLAGS) -c $< -o $@

-include $(wildcard $(DEP_DIR)/*.d)

clean:
	rm -rf $(DEP_DIR)/* $(CLIENT) $(TABLE) $(OBJ_DIR)/sdmessage.pb-c.o $(BIN_DIR)/* $(LIB_DIR)/*

libtable: $(LIB)
	ar -rcs $(LIB_DIR)/libtable.a $(LIB)

proto:
	protoc-c --c_out=. sdmessage.proto
	$(CC) -c sdmessage.pb-c.c
	mv sdmessage.pb-c.c $(SRC_DIR)
	mv sdmessage.pb-c.h $(INC_DIR)
	mv sdmessage.pb-c.o $(OBJ_DIR)