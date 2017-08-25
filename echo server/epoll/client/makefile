EXE_DIR := ./bin
INC_DIR := ./inc ./inc/net ./inc/connect
SRC_DIR := ./src ./src/net ./src/connect

TARGET := $(EXE_DIR)/main
OBJS   := $(wildcard ./src/*.c) $(wildcard ./src/net/*.c) $(wildcard ./src/connect/*.c)

LIBS := -lpthread

CC := gcc
CFLAGS := -g -D_REENTRANT $(addprefix -I, $(INC_DIR))

$(TARGET):$(OBJS)
	$(CC) -o $@ $(OBJS) $(CFLAGS)

clean:
	rm -rf $(TARGET)
