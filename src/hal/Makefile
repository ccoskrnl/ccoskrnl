# hal/Makefile
ARCH!=../../conf/uname.sh
include ../../conf/target.conf

BUILD_DIR = ../../build/hal
SRC_DIRS = . acpi devices

SRCS = $(foreach dir, $(SRC_DIRS), $(wildcard $(dir)/*.c))
OBJS = $(patsubst %.c,$(BUILD_DIR)/%.o,$(SRCS))

TARGET = $(BUILD_DIR)/hal.o
all: $(TARGET)

$(TARGET): $(OBJS)
	$(shell mkdir -p $(dir $@))
	@echo "Linking objects to $@"
	@$(LD) $(LDFLAGS) --relocatable -o $@ $^

$(BUILD_DIR)/%.o: %.c
	$(shell mkdir -p $(dir $@))
	@echo "	Compiling $<"
	@$(CC) $(CFLAGS) -c -o $@ $<

$(BUILD_DIR)/%.o: devices/%.c
	$(shell mkdir -p $(dir $@))
	@echo "	Compiling $<"
	@$(CC) $(CFLAGS) -c -o $@ $<
	
$(BUILD_DIR)/%.o: acpi/%.c
	$(shell mkdir -p $(dir $@))
	@echo "	Compiling $<"
	@$(CC) $(CFLAGS) -c -o $@ $<

clean:
	@rm -vf $(OBJS) $(TARGET)

.PHONY: clean all
