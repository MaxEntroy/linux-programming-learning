EXE_DIR := ./bin
INC_DIR := ./inc 
SRC_DIR := ./src

TARGET := $(EXE_DIR)/main
OBJS   := $(wildcard ./src/*.c)
CFLAGS := -I$(INC_DIR) -D_REENTRANT -lpthread

CC := gcc

$(TARGET):$(OBJS)
	$(CC) -g -o $@ $(OBJS) $(CFLAGS)

clean:
	rm -rf $(TARGET)
