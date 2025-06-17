CC=gcc
CFLAGS=-Wall
LDFLAGS=-lcurl 

SRC=screen_read.c api_endpoints.c functions.c
TARGET=screen_read

all:
	$(CC) $(CFLAGS) -o $(TARGET) $(SRC) $(LDFLAGS)

clean:
	rm -f $(TARGET)

