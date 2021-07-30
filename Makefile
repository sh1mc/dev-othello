
# Set your following environment variable properly before build a module.
# KERN_DIR := /path/to/kernel/src/root

obj-m := othello.o

PWD := $(shell pwd)

default:
	$(MAKE) -C $(KERN_DIR) M=$(PWD) modules

clean:
	rm -f *.o *.ko *.mod *.mod.c Module.symvers modules.order
