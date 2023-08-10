ccflags-y := -Wall -std=gnu99 -Wno-declaration-after-statement

obj-m := remate.o
remate-objs := kmutex.o remate-impl.o

KDIR  := /lib/modules/$(shell uname -r)/build
PWD   := $(shell pwd)

#include $(KDIR)/.config

default:
	$(MAKE) -C $(KDIR) M=$(PWD) modules

prodcons-impl.o kmutex.o: kmutex.h

clean:
	$(MAKE) -C $(KDIR) M=$(PWD) clean
