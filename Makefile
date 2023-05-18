obj-m += chardev.o

all:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) modules
	sudo insmod chardev.ko
	gcc -o chardev userspace.c
clean:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) clean