Makefile.qmake:
	. /apps/environ/.setup; \
	use -e -r qt-4.6.3; \
	use -e -r gsl-1.14; \
	cd ../; \
	qmake -o Makefile.qmake

all: Makefile.qmake
	. /apps/environ/.setup; \
	use -e -r qt-4.6.3; \
	use -e -r gsl-1.14; \
	make -f Makefile.qmake -C ../ all;

install: all
	make -f Makefile.qmake -C ../ install

clean: Makefile.qmake
	make -f Makefile.qmake -C ../ clean

distclean: clean
	make -f Makefile.qmake -C ../ distclean
	rm -rf ../release ../debug
