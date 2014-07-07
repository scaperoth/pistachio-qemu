IMGFILE = pistachio.img
ROOTFOLDER = pistachiosource
DRIVE = fd0

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
	MTOOLSRC=./mtoolsrc mcopy $(ROOTFOLDER)/pingpong a:/
	MTOOLSRC=./mtoolsrc mcopy $(ROOTFOLDER)/mypingpong a:/
	echo "($(DRIVE))  $(IMGFILE)" > bmap
	printf "setup ($(DRIVE)) \n root ($(DRIVE)) \n setup ($(DRIVE))\n quit\n" | /usr/sbin/grub --batch --device-map=bmap
	rm -f Makefile~
	rm -f mtoolsrc bmap

run:	
	qemu-system-i386 -serial stdio -fda pistachio.img -boot c
