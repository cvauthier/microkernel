#!/bin/sh
set -e
. ./build.sh

mkdir -p isodir/boot/grub
tar -cf root.tar sysroot
./rdmaker/rdmaker root.tar

cp sysroot/boot/myos.kernel isodir/boot/myos.kernel
mv root.bin isodir/boot/root.bin

cat > isodir/boot/grub/grub.cfg << EOF
menuentry "myos" {
	multiboot /boot/myos.kernel
	module /boot/root.bin
}
EOF
grub-mkrescue -o myos.iso isodir
