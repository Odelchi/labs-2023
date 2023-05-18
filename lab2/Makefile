obj-m += driver.o

all:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) modules
	sudo insmod driver.ko
	gcc -o driver counter_reader.c
clean:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) clean
