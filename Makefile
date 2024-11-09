obj-m += retardnet.o

CFLAGS="-DFIX_PLR_5"

CFLAGS_KERNEL += $(CFLAGS)

all:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) modules

clean:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) clean

