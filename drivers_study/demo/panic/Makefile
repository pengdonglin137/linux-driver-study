ifneq ($(KERNELRELEASE),)

obj-m := panic.o

else

PWD  := $(shell pwd)
ARCH=arm
CROSS_COMPILE=arm-linux-gnueabi-
KDIR := /home/pengdl/work/study/qemu_study/kernel/linux-3-14-y
OBJ := /home/pengdl/work/study/qemu_study/kernel/linux-3-14-y/out_vexpress_3_14
INSTALL_MOD_PATH := /home/pengdl/work/study/qemu_study/kernel/modules_install

all:
	$(MAKE) -C $(KDIR) M=$(PWD) O=$(OBJ) ARCH=$(ARCH) CROSS_COMPILE=$(CROSS_COMPILE) \
		INSTALL_MOD_PATH=$(INSTALL_MOD_PATH)  modules
	#make V=1 modules_install

modules_install:
	$(MAKE) -C $(KDIR) M=$(PWD) O=$(OBJ) ARCH=$(ARCH) CROSS_COMPILE=$(CROSS_COMPILE) \
		INSTALL_MOD_PATH=$(INSTALL_MOD_PATH)  modules_install

clean:
	@rm -rf .*.cmd *.o *.mod.c *.ko .tmp_versions *.order *.symvers

endif

