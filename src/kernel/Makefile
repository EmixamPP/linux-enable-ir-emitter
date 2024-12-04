ifneq ($(KERNELRELEASE),)
obj-m := linux-enable-ir-emitter.o
else
KDIR := /run/host/usr/lib/modules/$(shell uname -r)/build
PWD := $(shell pwd)
BUILD_DIR := $(PWD)/build

all:
	$(MAKE) -C $(KDIR) M=$(PWD) modules

clean:
	$(MAKE) -C $(KDIR) M=$(PWD) clean

install:
	$(MAKE) -C $(KDIR) M=$(PWD) modules_install
endif
