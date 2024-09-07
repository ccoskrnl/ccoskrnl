# libk/Makefile
ARCH!=../../conf/uname.sh
include ../../conf/target.conf

BUILD_DIR = ../../build/libk
SRC_DIRS = . stdlib string bitmap list ctype rbtree

SRCS = $(foreach dir, $(SRC_DIRS), $(wildcard $(dir)/*.c))
OBJS = $(patsubst %.c,$(BUILD_DIR)/%.o,$(SRCS))

TARGET = $(BUILD_DIR)/libk.a

all: $(TARGET)

$(TARGET): $(ASM_OBJS) $(OBJS)
	$(shell mkdir -p $(dir $@))
	@echo "AR objects to $@"
	@$(AR) $(ARFLAGS) -o $@ $^

$(BUILD_DIR)/%.o: %.c
	$(shell mkdir -p $(dir $@))
	@echo "	Compiling $<"
	@$(CC) $(CFLAGS) -c -o $@ $<

$(BUILD_DIR)/%.o: string/%.c
	$(shell mkdir -p $(dir $@))
	@echo "	Compiling $<"
	@$(CC) $(CFLAGS) -c -o $@ $<

$(BUILD_DIR)/%.o: list/%.c
	$(shell mkdir -p $(dir $@))
	@echo "	Compiling $<"
	@$(CC) $(CFLAGS) -c -o $@ $<

$(BUILD_DIR)/%.o: rbtree/%.c
	$(shell mkdir -p $(dir $@))
	@echo "	Compiling $<"
	@$(CC) $(CFLAGS) -c -o $@ $<

$(BUILD_DIR)/%.o: bitmap/%.c
	$(shell mkdir -p $(dir $@))
	@echo "	Compiling $<"
	@$(CC) $(CFLAGS) -c -o $@ $<

$(BUILD_DIR)/%.o: stdlib/%.c
	$(shell mkdir -p $(dir $@))
	@echo "	Compiling $<"
	@$(CC) $(CFLAGS) -c -o $@ $<

$(BUILD_DIR)/%.o: ctype/%.c
	$(shell mkdir -p $(dir $@))
	@echo "	Compiling $<"
	@$(CC) $(CFLAGS) -c -o $@ $<

clean:
	@rm -vf $(OBJS) $(TARGET) $(ASM_OBJS)


.PHONY: clean all