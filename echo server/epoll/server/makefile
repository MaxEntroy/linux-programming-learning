EXE_DIR := ./bin
INC_DIR := ./inc ./inc/net ./inc/epoll
SRC_DIR := ./src ./src/net ./src/epoll

TARGET := $(EXE_DIR)/main
OBJS   := $(wildcard ./src/*.c) $(wildcard ./src/net/*.c) $(wildcard ./src/epoll/*.c)

LIBS := -lpthread

CC := gcc
CFLAGS := -Wall -g -D_REENTRANT $(addprefix -I, $(INC_DIR)) 

$(TARGET):$(OBJS)
	$(CC) -o $@ $(OBJS) $(CFLAGS)

clean:
	rm -rf $(TARGET)
