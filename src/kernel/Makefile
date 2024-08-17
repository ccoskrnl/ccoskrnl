# kernel/Makefile
ARCH!=../../conf/uname.sh
include ../../conf/target.conf

BUILD_DIR = ../../build/kernel
SRC_DIRS = . kernel

# 获取所有的 .c 源文件
SRCS = $(foreach dir, $(SRC_DIRS), $(wildcard $(dir)/*.c))
OBJS = $(patsubst %.c,$(BUILD_DIR)/%.o,$(SRCS))

TARGET = $(BUILD_DIR)/kernel.o
all: $(TARGET)

$(TARGET): $(OBJS)
	$(shell mkdir -p $(dir $@))
	@echo "Linking objects to $@"
	@$(LD) $(LDFLAGS) --relocatable -o $@ $^


$(BUILD_DIR)/%.o: %.c
	$(shell mkdir -p $(dir $@))
	@echo "	Compiling $<"
	@$(CC) $(CFLAGS) -c -o $@ $<

clean:
	@rm -vf $(OBJS) $(TARGET)

.PHONY: clean all
