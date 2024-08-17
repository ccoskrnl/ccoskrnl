# arch/x86_64/Makefile
ARCH!=../../../conf/uname.sh

include ../../../conf/target.conf

BUILD_DIR = ../../../build/arch/$(ARCH)

SRC_DIRS = . intr mm cpu io

SRCS = $(foreach dir, $(SRC_DIRS), $(wildcard $(dir)/*.c))
OBJS = $(patsubst %.c,$(BUILD_DIR)/%.o,$(SRCS))

ASM_OBJS = $(BUILD_DIR)/io/io.o
ASM_OBJS += $(BUILD_DIR)/intr/pit.o
ASM_OBJS += $(BUILD_DIR)/cpu/cpu_flush.o

TARGET = $(BUILD_DIR)/arch.o

all: $(TARGET)

$(TARGET): $(ASM_OBJS) $(OBJS)
	@mkdir -p $(@D)
	@echo "Linking objects to $@"
	@$(LD) $(LDFLAGS) --relocatable -o $@ $^


$(BUILD_DIR)/%.o: %.c
	@echo "	Compiling $<"
	@$(CC) $(CFLAGS) -c -o $@ $<

$(BUILD_DIR)/mm/%.o: mm/%.c
	@mkdir -p $(@D)
	@echo "	Compiling $<"
	@$(CC) $(CFLAGS) -c -o $@ $<

$(BUILD_DIR)/io/%.o: io/%.asm
	@mkdir -p $(@D)
	@echo "	Compiling $<"
	@nasm -f elf64 -o $@ $<

$(BUILD_DIR)/intr/%.o: intr/%.asm
	@mkdir -p $(@D)
	@echo "	Compiling $<"
	@nasm -f elf64 -o $@ $<

$(BUILD_DIR)/intr/%.o: intr/%.c
	$(shell mkdir -p $(dir $@))
	@echo "	Compiling $<"
	@$(CC) $(CFLAGS) -c -o $@ $<

$(BUILD_DIR)/cpu/%.o: cpu/%.c
	$(shell mkdir -p $(dir $@))
	@echo "	Compiling $<"
	@$(CC) $(CFLAGS) -c -o $@ $<

$(BUILD_DIR)/cpu/%.o: cpu/%.asm
	@mkdir -p $(@D)
	@echo "	Compiling $<"
	@nasm -f elf64 -o $@ $<

clean:
	@rm -vf $(OBJS) $(TARGET) $(ASM_OBJS)

.PHONY: all clean $(TARGET)
