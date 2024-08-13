# libk/Makefile
ARCH!=../../conf/uname.sh
include ../../conf/target.conf

BUILD_DIR = ../../build/libk
SRC_DIRS = . stdio stdlib string bitmap list ctype math

# 获取所有的 .c 源文件
SRCS = $(foreach dir, $(SRC_DIRS), $(wildcard $(dir)/*.c))
OBJS = $(patsubst %.c,$(BUILD_DIR)/%.o,$(SRCS))

ASM_OBJS = $(BUILD_DIR)/string/memcpy_avx.o
ASM_OBJS += $(BUILD_DIR)/string/memcpy_sse2.o
ASM_OBJS += $(BUILD_DIR)/string/memzero_avx.o
ASM_OBJS += $(BUILD_DIR)/string/memzero_sse2.o

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

$(BUILD_DIR)/string/%.o: string/%.asm
	@mkdir -p $(@D)
	@echo "	Compiling $<"
	@nasm -f elf64 -o $@ $<

$(BUILD_DIR)/%.o: string/%.c
	$(shell mkdir -p $(dir $@))
	@echo "	Compiling $<"
	@$(CC) $(CFLAGS) -c -o $@ $<

$(BUILD_DIR)/%.o: list/%.c
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

$(BUILD_DIR)/%.o: stdio/%.c
	$(shell mkdir -p $(dir $@))
	@echo "	Compiling $<"
	@$(CC) $(CFLAGS) -c -o $@ $<

$(BUILD_DIR)/%.o: ctype/%.c
	$(shell mkdir -p $(dir $@))
	@echo "	Compiling $<"
	@$(CC) $(CFLAGS) -c -o $@ $<

clean:
	rm -vf $(OBJS) $(TARGET)

.PHONY: clean all