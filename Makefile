IMGFILE = pistachio.img
ROOTFOLDER = pistachiosource
DRIVE = hd1
MODULE = mypingpong
CONFIGOPTIONS = 
#MODULEPATH = l4ka-pistachio/x86-x36-user-build/apps/bench/

all: module config image 
	qemu pistachio.img

image:
	rm -f $(IMGFILE) mtoolsrc bmap
	dd if=/dev/zero of=$(IMGFILE) bs=512 count=2880
	echo 'drive a: file="$(IMGFILE)"'> mtoolsrc
	MTOOLSRC=./mtoolsrc mformat -f 1440 a:
	MTOOLSRC=./mtoolsrc mmd a:/boot
	MTOOLSRC=./mtoolsrc mmd a:/boot/grub
	MTOOLSRC=./mtoolsrc mcopy $(ROOTFOLDER)/boot/grub/stage1 a:/boot/grub
	MTOOLSRC=./mtoolsrc mcopy $(ROOTFOLDER)/boot/grub/stage2 a:/boot/grub
	MTOOLSRC=./mtoolsrc mcopy $(ROOTFOLDER)/boot/grub/fat_stage1_5 a:/boot/grub
	MTOOLSRC=./mtoolsrc mcopy $(ROOTFOLDER)/boot/grub/menu.lst a:/boot/grub/
	MTOOLSRC=./mtoolsrc mcopy $(ROOTFOLDER)/x86-kernel a:/
	MTOOLSRC=./mtoolsrc mcopy $(ROOTFOLDER)/kickstart a:/
	MTOOLSRC=./mtoolsrc mcopy $(ROOTFOLDER)/sigma0 a:/
	MTOOLSRC=./mtoolsrc mcopy $(ROOTFOLDER)/$(MODULE) a:/

	echo "($(DRIVE))  $(IMGFILE)" > bmap
	printf "root ($(DRIVE),0) \n setup ($(DRIVE),0)\n quit\n" | /usr/sbin/grub --batch --device-map=bmap
	rm -f Makefile~
	rm -f mtoolsrc bmap


config:
	cp l4ka-pistachio/x86-kernel-build/x86-kernel l4ka-pistachio/x86-x32-user-install/libexec/l4/
	find pistachiosource/ -maxdepth 1 -type f -delete
	cp l4ka-pistachio/x86-kernel-build/x86-kernel pistachiosource/
	cp l4ka-pistachio/x86-x32-user-install/libexec/l4/* pistachiosource/
	cp -rf $(MODULE)/$(MODULE) pistachiosource/ 

module:
	make clean -C mypingpong
	make -C mypingpong
	



