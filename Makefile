obj-m += retardnet.o

CFLAGS="-DFIX_PLR_5"

all:
	make CFLAGS_MODULE=$(CFLAGS) -C /lib/modules/$(shell uname -r)/build M=$(PWD) modules

clean:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) clean

