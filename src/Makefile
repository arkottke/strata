Makefile.qmake:
	. /apps/environ/.setup; \
	use -e -r qt-4.8.0; \
	use -e -r qwt-6.0.0; \
	cd ../; \
	qmake -o Makefile.qmake

all: Makefile.qmake
	. /apps/environ/.setup; \
	use -e -r qt-4.8.0; \
	use -e -r qwt-6.0.0; \
	make -f Makefile.qmake -C ../ all;

install: all
	make -f Makefile.qmake -C ../ install
	install --mode 0755 -D ../release/strata ../bin/

clean: Makefile.qmake
	make -f Makefile.qmake -C ../ clean

distclean: clean
	make -f Makefile.qmake -C ../ distclean
	rm -rf ../release ../debug
