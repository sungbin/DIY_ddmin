if ! [ -d "./libxml2" ]; then
	git clone https://gitlab.gnome.org/GNOME/libxml2.git
fi

if ! [ -d "./_$1_libxml2" ]; then

	cp -r "./libxml2" "./_$1_libxml2"
	cd "./_$1_libxml2"

	if [ "$1" == "243" ] ; then
		git checkout 5465a8e57fe620ceb8efa534e1d6790f423d6bba # issue 243
		CC='clang' CFLAGS="-g -fsanitize=address -fsanitize-coverage=edge,trace-pc-guard" ./configure --enable-static --disable-shared
		make 
	fi
	if [ "$1" == "311" ] ; then
		git checkout dea91c97debeac7c1aaf9c19f79029809e23a353 # issue 311
		CC='clang' CFLAGS="-g -O0 -fsanitize=address -fsanitize-coverage=edge,trace-pc-guard" ./configure --prefix=$(pwd)/ --enable-static --disable-shared
		make
	fi
	if [ "$1" == "350" ] ; then
		git checkout 4adb7677635b2c8d764767f6b484de1b26217f5d # issue 350
		CC='clang' CFLAGS="-g -fsanitize=address -fsanitize-coverage=edge,trace-pc-guard" ./configure --enable-static --disable-shared --with-icu
		make
	fi

	./autogen.sh
	cd ..
fi

if [ -d "./$1_libxml2" ]; then
	rm -r "./$1_libxml2"
fi
cp -r "./_$1_libxml2" "./$1_libxml2"
echo "Generated: ./$1_libxml2"

# 2. add crash input on test/


# 3. read Makefile, and write echo the line after executing ./xmllint
# it needs to regex, and parsing test module name for naming .result

# 4. run 16 tests

# 5. read written file
