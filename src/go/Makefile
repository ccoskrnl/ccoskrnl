# go/Makefile
ARCH!=../../conf/uname.sh
include ../../conf/target.conf

BUILD_DIR = ../../build/go
SRC_DIRS = . font output window

SRCS = $(foreach dir, $(SRC_DIRS), $(wildcard $(dir)/*.c))
OBJS = $(patsubst %.c,$(BUILD_DIR)/%.o,$(SRCS))

TARGET = $(BUILD_DIR)/go.o
all: $(TARGET)

$(TARGET): $(OBJS)
	$(shell mkdir -p $(dir $@))
	@echo "Linking objects to $@"
	@$(LD) $(LDFLAGS) --relocatable -o $@ $^

$(BUILD_DIR)/%.o: %.c
	$(shell mkdir -p $(dir $@))
	@echo "	Compiling $<"
	@$(CC) $(CFLAGS) -c -o $@ $<

$(BUILD_DIR)/%.o: font/%.c
	$(shell mkdir -p $(dir $@))
	@echo "	Compiling $<"
	@$(CC) $(CFLAGS) -c -o $@ $<

$(BUILD_DIR)/%.o: output/%.c
	$(shell mkdir -p $(dir $@))
	@echo "	Compiling $<"
	@$(CC) $(CFLAGS) -c -o $@ $<

$(BUILD_DIR)/%.o: window/%.c
	$(shell mkdir -p $(dir $@))
	@echo "	Compiling $<"
	@$(CC) $(CFLAGS) -c -o $@ $<

clean:
	@rm -vf $(OBJS) $(TARGET)

.PHONY: clean all
