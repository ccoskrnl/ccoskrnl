REFLIBS_DIR = ../reflibs
BUILD_DIR =../build
ESP_DIR =../esp/EFI/ChengChengOS
ARCH!=../conf/uname.sh

include ../conf/target.conf
include ../conf/simulator/qemu.conf

SUBDIRS = arch/$(ARCH) go kernel hal libk

TARGET = $(BUILD_DIR)/ccoskrnl

# all of objs that the kernel needed
OBJS = \
       $(BUILD_DIR)/kernel/kernel.o \
       $(BUILD_DIR)/arch/$(ARCH)/arch.o \
       $(BUILD_DIR)/go/go.o \
       $(BUILD_DIR)/hal/hal.o \
       $(BUILD_DIR)/libk/libk.a \
	   $(REFLIBS_DIR)/libm.a \

EFI = ../esp/EFI/BOOT/BOOTX64.EFI
LINK_SCRIPT = ./linker.ld

all: $(TARGET) 

$(TARGET): $(SUBDIRS) $(LINK_SCRIPT) 
	@echo "Linking objects to $@"
	@$(LD) $(LDFLAGS) -T $(LINK_SCRIPT) -o $@ $(OBJS)

	@if [ -f $(ESP_DIR)/ccoskrnl ]; then \
		rm $(ESP_DIR)/ccoskrnl; \
	fi

	@echo ""
	@cp -v $(TARGET) $(ESP_DIR)
	@echo ""
	@echo "ccoskrnl creation completed at $@"
	@echo ""
	@nm -n $(TARGET) > $(BUILD_DIR)/ccoskrnl.sym



$(SUBDIRS):
	$(MAKE) -C $@

run: $(TARGET) $(EFI)
	@rm -vf ../esp/NvVars
	$(QEMU) $(QEMU_OPTIONS)


clean:
	@echo "Cleaning up build artifacts..."
	@for dir in $(SUBDIRS); do \
		$(MAKE) -C $$dir clean; \
	done
	@rm -vf $(TARGET) ../esp/NvVars $(ESP_DIR)/ccoskrnl

.PHONY: clean run all $(SUBDIRS) $(TARGET)
