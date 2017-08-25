EXE_DIR := ./bin
INC_DIR := ./inc
SRC_DIR := ./src

TARGET := $(EXE_DIR)/main
OBJS   := $(wildcard $(SRC_DIR)/*.c)
CFLAGS := -I$(INC_DIR) -D_REENTRANT

$(TARGET):$(OBJS)
	gcc -g -o $@ $(OBJS) $(CFLAGS)

clean:
	rm -rf $(TARGET)
