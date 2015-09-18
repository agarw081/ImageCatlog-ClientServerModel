CFLAGS += -Wall
LDFLAGS += -lcrypto

SRC := server.c 

MD5_SRC := md5sum.c
MD5_OBJ := $(MD5_SRC:.c=.o)

TARGET := $(SRC:.c=)

.PHONY: all clean

all: server client

server: server.o md5sum.o
	gcc -o $@ $^ $(LDFLAGS)

client: client.o md5sum.o
	gcc -o $@ $^ $(LDFLAGS)

server.o: server.c 
	gcc -c $^

client.o: client.c
	gcc -c $^

md5sum.o: md5sum.c md5sum.h
	gcc -c $^

clean:
	-$(RM) $(TARGET) $(MD5_OBJ) server client
