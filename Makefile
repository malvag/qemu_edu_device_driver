
LINUX_VERSION = 5.15.5
KDIR = $(PWD)/../linux-$(LINUX_VERSION)

DESTDIR = $(PWD)/../buildroot/output/target
DESTLIBDIR = $(PWD)/../buildroot/output/target/lib
DESTHOMEDIR = $(PWD)/../buildroot/output/target/root
obj-m += eduv7.o 

eduv7-objs := mod.o driver.o e7cdev.o

all: modules prepare install build_user_library user_test
	make -C $(PWD)/../buildroot

install: prepare
	make -C $(KDIR) M=$(PWD) INSTALL_MOD_PATH=$(DESTDIR) modules_install

prepare: modules install_path
	make -C $(KDIR) M=$(PWD) INSTALL_MOD_PATH=$(DESTDIR) modules_prepare

modules:
	make -C $(KDIR) M=$(PWD) INSTALL_MOD_PATH=$(DESTDIR) modules

clean:
	make -C $(KDIR) M=$(PWD) INSTALL_MOD_PATH=$(DESTDIR) clean
	rm -rf *.o
	rm -rf *.ko
	rm -rf *.so
	rm -rf user_test

install_path:
	mkdir -p $(DESTDIR)/lib/modules/$(LINUX_VERSION)
	cp test.sh $(DESTDIR)/root

build_user_library: 
	#gcc --prefix=../buildroot/output/target -c ulib_evu_v7.c
	gcc -c ulib_evu_v7.c
	gcc -shared -o ulib_edu_v7.so ulib_evu_v7.o
	cp ./ulib_edu_v7.so $(DESTLIBDIR)

user_test: user_test.c
	gcc --prefix=../buildroot/output/target user_test.c ulib_evu_v7.c -o user_test
	cp ./user_test $(DESTHOMEDIR)
