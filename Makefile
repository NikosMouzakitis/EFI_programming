
#compilation of a linux kernel module for our guest.

obj-m=efi_com.o

all:
	make -C /home/nicko/implementations/x86/buildroot/output/build/linux-5.15 M=$(PWD) modules
clean:
	rm modules.order Module.symvers efi_com.ko efi_com.mod* efi_com.o
