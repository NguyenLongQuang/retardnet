obj-m += retardnet.o

CFLAGS='-DFIX_PLR_5 \
-UFIX_PLR_5 \
-DQUANGNL_FIX_WINDOW_COUNTER \
-UQUANGNL_FIX_WINDOW_COUNTER \
-DQUANGNL_TOKEN_BUCKET'

all:
	make CFLAGS_MODULE=$(CFLAGS) -C /lib/modules/$(shell uname -r)/build M=$(PWD) modules

clean:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) clean

