BUILD_DIR =../build
ESP_DIR =../esp
ARCH!=../conf/uname.sh

include ../conf/target.conf
include ../conf/simulator/qemu.conf

SUBDIRS = arch/$(ARCH) kernel hal libk

TARGET = $(BUILD_DIR)/krnl

# all of objs that the kernel needed
OBJS = \
       $(BUILD_DIR)/kernel/kernel.o \
       $(BUILD_DIR)/arch/$(ARCH)/arch.o \
       $(BUILD_DIR)/hal/hal.o \
       $(BUILD_DIR)/libk/libk.a \

EFI = ../esp/EFI/BOOT/BOOTX64.EFI
LINK_SCRIPT = arch/$(ARCH)/linker.ld

all: $(TARGET) 

$(TARGET): $(SUBDIRS) $(LINK_SCRIPT) 
	@echo "Linking objects to $@"
	@$(LD) $(LDFLAGS) -T $(LINK_SCRIPT) -o $@ $(OBJS)

	@if [ -f $(ESP_DIR)/krnl ]; then \
		rm $(ESP_DIR)/krnl; \
	fi

	@cp -v $(TARGET) $(ESP_DIR)
	@echo ""
	@echo "Krnl Creation Completed at $@"

$(SUBDIRS):
	$(MAKE) -C $@

run: $(TARGET) $(EFI)
	@rm -vf $(BUILD_DIR)/esp/NvVars
	$(QEMU) $(QEMU_OPTIONS)


clean:
	@for dir in $(SUBDIRS); do \
		$(MAKE) -C $$dir clean; \
	done
	@rm -vf $(TARGET) $(ESP_DIR)/NvVars $(ESP_DIR)/krnl

.PHONY: clean run all $(SUBDIRS) $(TARGET)
