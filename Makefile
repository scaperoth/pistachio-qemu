IMGFILE = pistachio.img
ROOTFOLDER = pistachiosource
DRIVE = hd0
MODULE = mypingpong
CONFIGOPTIONS = 
#MODULEPATH = l4ka-pistachio/x86-x36-user-build/apps/bench/

all: clean module config image-no-root


image-root:
	mkdir -p /mnt/fda

	dd if=/dev/zero of=$(IMGFILE) bs=512 count=3200

	/sbin/losetup /dev/loop0 $(IMGFILE)
	/sbin/mke2fs /dev/loop0
	mount /dev/loop0 -o loop /mnt/fda
	chmod 777 /mnt/fda
	cp -aR $(ROOTFOLDER)/* /mnt/fda
	printf "umount /mnt/fda \n"

	printf "root ($(DRIVE)) \n setup ($(DRIVE))\n quit\n" | /usr/sbin/grub --batch --device-map=/dev/null 

	/sbin/losetup -d /dev/loop0

	rm -f Makefile~

image-no-root:
	
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
	printf "root ($(DRIVE)) \n setup ($(DRIVE))\n quit\n" | /usr/sbin/grub --batch --device-map=bmap
	rm -f Makefile~
	rm -f mtoolsrc bmap



config:
	cp l4ka-pistachio/x86-kernel-build/x86-kernel l4ka-pistachio/x86-x32-user-install/libexec/l4/
	find pistachiosource/ -maxdepth 1 -type f -delete
	cp l4ka-pistachio/x86-kernel-build/x86-kernel pistachiosource/
	cp l4ka-pistachio/x86-x32-user-install/libexec/l4/* pistachiosource/
	cp -rf $(MODULE)/$(MODULE) pistachiosource/ 

module:
	make -C $(MODULE)

qemu:
	make
	qemu-system-i386 $(IMGFILE)

clean:
	make clean -C $(MODULE)
	rm -f $(IMGFILE) mtoolsrc bmap


