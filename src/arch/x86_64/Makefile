# arch/x86/Makefile
ARCH!=../../../conf/uname.sh

include ../../../conf/target.conf

BUILD_DIR = ../../../build/arch/$(ARCH)

SRC_DIRS = . intr mm cpu cpu/power io lib intr/isr

SRCS = $(foreach dir, $(SRC_DIRS), $(wildcard $(dir)/*.c))
OBJS = $(patsubst %.c,$(BUILD_DIR)/%.o,$(SRCS))


ASM_OBJS = $(BUILD_DIR)/io/io.o

ASM_OBJS += $(BUILD_DIR)/intr/pit.o
ASM_OBJS += $(BUILD_DIR)/intr/isr/kbd.o
ASM_OBJS += $(BUILD_DIR)/intr/isr/intr_handlers_entry.o

ASM_OBJS += $(BUILD_DIR)/cpu/cpu_flush.o

ASM_OBJS += $(BUILD_DIR)/lib/cmpxchg.o
ASM_OBJS += $(BUILD_DIR)/lib/memcpy_avx.o
ASM_OBJS += $(BUILD_DIR)/lib/memcpy_sse2.o
ASM_OBJS += $(BUILD_DIR)/lib/memzero_avx.o
ASM_OBJS += $(BUILD_DIR)/lib/memzero_sse2.o



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

$(BUILD_DIR)/lib/%.o: lib/%.asm
	@mkdir -p $(@D)
	@echo "	Compiling $<"
	@nasm -f elf64 -o $@ $<

$(BUILD_DIR)/lib/%.o: lib/%.c
	$(shell mkdir -p $(dir $@))
	@echo "	Compiling $<"
	@$(CC) $(CFLAGS) -c -o $@ $<

$(BUILD_DIR)/intr/isr/%.o: intr/isr/%.asm
	@mkdir -p $(@D)
	@echo "	Compiling $<"
	@nasm -f elf64 -o $@ $<

$(BUILD_DIR)/intr/isr/%.o: intr/isr/%.c
	$(shell mkdir -p $(dir $@))
	@echo "	Compiling $<"
	@$(CC) $(CFLAGS) -c -o $@ $<

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

$(BUILD_DIR)/cpu/power/%.o: cpu/power/%.c
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
