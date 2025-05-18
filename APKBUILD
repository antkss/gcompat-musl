#!/bin/bash
pkgname=gcompat-musl
pkgver=1.2.5
pkgrel=1
pkgdesc="A dynamic tiling Wayland compositor based on wlroots that doesn't sacrifice on its looks."
arch=noarch
license="BSD"
# depends="libwnck3-dev gobject-introspection-dev meson ninja pkgconf gtk+3.0-dev gtk-layer-shell-dev json-c-dev "
url="google.com"
export bdir=$srcdir/..

# export pack=( $(find "$bdir" -type d -maxdepth 1 -not -name ".") )
# echo $pack


prepare() {
	cd $bdir
	if [ -d $bdir/musl$pkgver ]; then
		echo "skip cloning source "
	else
		if [ ! -f $bdir/musl-$pkgver.tar.gz ]; then
			wget "https://musl.libc.org/releases/musl-$pkgver.tar.gz"
			tar -xf musl-$pkgver.tar.gz
		else
			echo "building folder exist, skip download ..."
		fi
	fi
}

build() {
	cd $bdir
	if [ ! -f $bdir/musl-$pkgver/src/libgcompat ]; then
		ln -sr $bdir/libgcompat $bdir/musl-$pkgver/src
	fi 
	cd $bdir/musl-$pkgver
	# clean visibility 
	attribute=($(grep -rh --include='*.c' --include='*.h' "visibility" | grep hidden | grep attribute | grep define -v))
	for i in ${attribute[@]}; do
		grep -rl $i | xargs sed -i -e 's/__attribute__((__visibility__("hidden")))//g'
	done
	mkdir build; cd build
	export CFLAGS="-static-pie -static -g -Wl,--gc-sections"
	../configure --prefix=$pkgdir
	make -j$(nproc)
}

package() {
	build_dir=$bdir/musl-$pkgver/build
	cd $build_dir
	../tools/install.sh -D lib/libc.so $pkgdir/usr/lib/libgcompat.so
	../tools/install.sh -D -l libgcompat.so $pkgdir/usr/lib/ld-linux-x86-64.so.2

	../tools/install.sh -D -l ../lib/libgcompat.so $pkgdir/usr/lib64/ld-linux-x86-64.so.2
	../tools/install.sh -D -l ../usr/lib/libgcompat.so $pkgdir/lib64/ld-linux-x86-64.so.2
	../tools/install.sh -D -l ../usr/lib/libgcompat.so $pkgdir/lib/ld-linux-x86-64.so.2

	../tools/install.sh -D -l ../lib/libgcompat.so $pkgdir/usr/lib64/libgcompat.so
	../tools/install.sh -D -l ../usr/lib/libgcompat.so $pkgdir/lib64/libgcompat.so
	../tools/install.sh -D -l ../usr/lib/libgcompat.so $pkgdir/lib/libgcompat.so
}
