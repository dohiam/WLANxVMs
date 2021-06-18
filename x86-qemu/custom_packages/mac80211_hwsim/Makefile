KF=mac80211_hwsim
KDIR=${CLFSPROJ}/build/linux
ARCH=x86_64
SYSROOT=${XTOOLS}/x86_64-unknown-linux-gnu/x86_64-unknown-linux-gnu/sysroot


obj-m += ${KF}.o

all:
	$(MAKE) -C $(KDIR) SUBDIRS=$(PWD) modules
#	$(MAKE) CFLAGS=$(CFLAGS) ARCH=$(ARCH) SYSROOT=$(SYSROOT) -C $(KDIR) SUBDIRS=$(PWD) modules

clean:
	rm -rf *.o *.ko *.mod.* *.cmd .module* modules* Module* .*.cmd .tmp*	
	
install:
	cp $(KF).ko ${CLFSPROJ}/targetfs/root

