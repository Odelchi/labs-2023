obj-m += mac-address.o

all:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) modules
	sudo insmod mac_address.ko
clean:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) clean
stop:
	sudo rmmod mac_address.ko
