.DEFAULT_GOAL := all

#Support variables
ROOT_DIR = $(shell dirname $(realpath $(firstword $(MAKEFILE_LIST))))
BUILD_LOG = $(ROOT_DIR)/build.log
MOUNT_DIR = $(ROOT_DIR)/img_mount_point
MOUNT_IMG = vv_ex.img
SHELL :=/bin/bash -O globstar
VPATH = $(shell find ./kernel -type d -printf "kernel/%P:")
SOURCES_C = $(shell ls kernel/*.c)
SOURCES_ASMS = $(shell ls kernel/*.S)
OBJECTS_C = $(patsubst %.c, build/%.o, $(shell ls kernel/*.c | xargs -n 1 basename))
OBJECTS_ASMS = $(patsubst %.S, build/%.o, $(shell ls kernel/*.S | xargs -n 1 basename))

#Compile programs and flags
CC = /usr/local/osdev/bin/x86_64-elf-gcc
LD = /usr/local/osdev/bin/x86_64-elf-ld
ASM = /usr/local/osdev/bin/x86_64-elf-as
DEFINES = -DPAGING_PAE \
		  -DGRAPHICS_OFF \
		  -DBITS_32
CFLAGS = $(DEFINES) -Wno-write-strings -ffreestanding -fno-omit-frame-pointer -O0 -g -I$(ROOT_DIR)/kernel/include -I$(ROOT_DIR)/libcvv/libc/include \
	-std=c11             \
    -ffreestanding       \
    -fno-stack-protector \
    -fno-stack-check     \
    -fno-lto             \
    -fno-pie             \
    -fno-pic             \
    -m64                 \
    -march=x86-64        \
    -mabi=sysv           \
    -mno-80387           \
    -mno-mmx             \
    -mno-sse             \
    -mno-sse2            \
    -mno-red-zone        \
    -mcmodel=kernel      \
    -MMD                 \
	-I
CFLAGS_END = -nostdlib -lgcc
AFLAGS = $(CFLAGS)

#Support program and flags
OBJDUMP = /usr/local/osdev/bin/x86_64-elf-objdump
QEMU = /usr/bin/qemu-system-x86_64
QEMU_COMMON = 	-drive format=raw,if=ide,file=$(ROOT_DIR)/vv_ex.img \
				-device isa-debug-exit,iobase=0xf4,iosize=0x04 \
				-nic user,ipv6=off,model=e1000,mac=52:54:98:76:54:32 \
				-m 4G \
				-serial null \
				-serial stdio \
				-serial null \
				-serial file:$(ROOT_DIR)/serial_out.txt \
				-no-reboot
QEMU_DISPLAY_NONE =	-display none
QEMU_DISPLAY_NORMAL = -vga std
QEMU_DEBUG_COMMON = -S -gdb tcp::5894 
QEMU_DEBUG_LOGGING = -D $(ROOT_DIR)/qemu_debug_log.txt -d int,cpu_reset 

# tcp:192.168.0.100:21,nodelay=on,reconnect=0
# tcp::6699,nodelay=on,server=on,wait=no

# -serial tcp:192.168.0.100:6699,nodelay=on,server=on,wait=no \
# -serial null \

export

all: debug_dump install

build/versionv_ex.bin: $(SOURCES_C) $(SOURCES_ASM) $(SOURCES_ASMS) $(OBJECTS_C) $(OBJECTS_ASM) $(OBJECTS_ASMS)
	$(LD) -nostdlib -static -m elf_x86_64 -z max-page-size=0x1000 -T kernel/linker.ld -o build/versionv_ex.bin $(OBJECTS_C) $(OBJECTS_ASM) $(OBJECTS_ASMS)
	$(OBJDUMP) -x -D -S build/versionv_ex.bin > objdump.txt
	readelf -a build/versionv_ex.bin > elfdump.txt
	@>&2 printf "[Build] Done\n"

build/%.o: %.c
	@>&2 printf "[Build] $<\n"
	$(eval OBJNAME := $(shell basename $@))
	$(CC) $(CFLAGS) $(CFLAGS_END) -c $< -o build/$(OBJNAME) >> $(BUILD_LOG)

#build/%.o: kernel/**/%.s
build/%.o: %.s
	@>&2 printf "[Build] $<\n"
	$(eval OBJNAME := $(shell basename $@))
	$(ASM) -c $< -o build/$(OBJNAME) >> $(BUILD_LOG)

build/%.o: %.S
	@>&2 printf "[Build] $<\n"
	$(eval OBJNAME := $(shell basename $@))
	$(CC) $(AFLAGS) $(CFLAGS_END) -c $< -o build/$(OBJNAME) >> $(BUILD_LOG)

debug_dump:
	@>&2 echo [Build] Makefile Debug Dump
	@make debug_dump_stage2 >> $(BUILD_LOG)

debug_dump_stage2:
	@echo "Start Debug Dump"
	@echo "----------"
	@echo "vpath:" $(VPATH)
	@echo "----------"
	@echo "*.s:" $(SOURCES_ASM)
	@echo " "
	@echo "*.o:" $(OBJECTS_ASM)
	@echo "----------"
	@echo "*.S:" $(SOURCES_ASMS)
	@echo " "
	@echo "*.o:" $(OBJECTS_ASMS)
	@echo "----------"
	@echo "*.c:" $(SOURCES_C)
	@echo " "
	@echo "*.o:" $(OBJECTS_C)
	@echo "----------"
	@echo "End Debug Dump"
	@echo " "

install:
	@make install_stage2 >> $(BUILD_LOG)
	@>&2 printf "[Install] Done\n"

#if included in fstab, use: mount $(MOUNT_DIR)
install_stage2: build/versionv_ex.bin
	@>&2 echo [Install] Installing to $(MOUNT_IMG)
	@$(eval LOOP_DRIVE := $(shell sudo losetup -f))
	@sudo losetup -fP $(MOUNT_IMG)
	@sudo mount $(LOOP_DRIVE)p1 $(MOUNT_DIR)
	@sudo cp build/versionv_ex.bin -f $(MOUNT_DIR)/versionv_ex.bin
	@sudo umount $(MOUNT_DIR) 
	@sudo losetup -d $(LOOP_DRIVE)

run: install
	$(QEMU) $(QEMU_COMMON) $(QEMU_DISPLAY_NORMAL) 

run_debug: install
	$(QEMU) $(QEMU_COMMON) $(QEMU_DISPLAY_NORMAL) $(QEMU_DEBUG_COMMON)

run_ng: install
	$(QEMU) $(QEMU_COMMON) $(QEMU_DISPLAY_NONE) && echo "Exit Code: $$?" || echo "Exit Code: $$?"

run_ng_debug: install
	$(QEMU) $(QEMU_COMMON) $(QEMU_DISPLAY_NONE) $(QEMU_DEBUG_COMMON)

gdb:
	gdb -q --command=/usr/local/osdev/versions/v/kernel/build_support/main_debug.gdb 

clean:
	@rm -rf build.log
	@make clean_stage_2 >> $(BUILD_LOG)

create_img:
	@make create_img_stage_2 >> $(BUILD_LOG)
	@>&2 echo [Create Img] Done

create_img_stage_2:
	@dd if=/dev/zero of=$(MOUNT_IMG) bs=100M count=2 >> $(BUILD_LOG)
	@$(eval LOOP_DRIVE := $(shell sudo losetup -f))
	@sudo losetup -fP $(MOUNT_IMG)
	@echo -e "g\nn\n1\n\n+100M\nn\n2\n\n\nw" | sudo fdisk $(LOOP_DRIVE) >> $(BUILD_LOG)
	@sudo mke2fs $(LOOP_DRIVE)p1
	@sudo mke2fs $(LOOP_DRIVE)p2
	@sudo mount -o noacl $(LOOP_DRIVE)p1 $(MOUNT_DIR)
	@sudo cp -r boot_files/* $(MOUNT_DIR)
	@sudo umount $(MOUNT_DIR)
	@sudo /usr/local/osdev/share/limine/limine-deploy $(LOOP_DRIVE)
	@sudo losetup -d $(LOOP_DRIVE)

clean_stage_2:
	rm -rf build/*.o 
	rm -rf build/*.bin 
	rm -rf objdump.txt 
	rm -rf versionv.iso
	@>&2 echo [Clean] Done
