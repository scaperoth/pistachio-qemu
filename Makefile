IMGFILE = pistachio.img

image:	
	rm -f $(IMGFILE) mtoolsrc bmap
	dd if=/dev/zero of=$(IMGFILE) bs=512 count=2880
	echo 'drive a: file="$(IMGFILE)"'> mtoolsrc
	MTOOLSRC=./mtoolsrc mformat -f 1440 a:
	MTOOLSRC=./mtoolsrc mmd a:/boot
	MTOOLSRC=./mtoolsrc mmd a:/boot/grub
	MTOOLSRC=./mtoolsrc mcopy fdsource/boot/grub/stage1 a:/boot/grub
	MTOOLSRC=./mtoolsrc mcopy fdsource/boot/grub/stage2 a:/boot/grub
	MTOOLSRC=./mtoolsrc mcopy fdsource/boot/grub/fat_stage1_5 a:/boot/grub
	MTOOLSRC=./mtoolsrc mcopy fdsource/boot/grub/menu.lst a:/boot/grub/
	MTOOLSRC=./mtoolsrc mcopy fdsource/x86-kernel a:/
	MTOOLSRC=./mtoolsrc mcopy fdsource/kickstart a:/
	MTOOLSRC=./mtoolsrc mcopy fdsource/sigma0 a:/
	MTOOLSRC=./mtoolsrc mcopy fdsource/pingpong a:/
	echo "(fd0)  $(IMGFILE)" > bmap
	printf "setup (fd0) \n root (fd0) \n setup (fd0)\n quit\n" | /usr/sbin/grub --batch --device-map=bmap
	rm -f Makefile~
	rm -f mtoolsrc bmap