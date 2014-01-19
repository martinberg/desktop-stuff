# lsx - list executables
#   (C)opyright MMVI Anselm R. Garbe

include config.mk

SRC = lsx.c
OBJ = ${SRC:.c=.o}

all: options lsx
	@echo finished

options:
	@echo lsx build options:
	@echo "CFLAGS   = ${CFLAGS}"
	@echo "LDFLAGS  = ${LDFLAGS}"
	@echo "CC       = ${CC}"

.c.o:
	@echo CC $<
	@${CC} -c ${CFLAGS} $<

lsx: ${OBJ}
	@echo LD $@
	@${CC} -o $@ ${OBJ} ${LDFLAGS}
	@strip $@

clean:
	@echo cleaning
	@rm -f lsx ${OBJ} lsx-${VERSION}.tar.gz

dist: clean
	@echo creating dist tarball
	@mkdir -p lsx-${VERSION}
	@cp -R LICENSE Makefile README config.mk ${SRC} lsx-${VERSION}
	@tar -cf lsx-${VERSION}.tar lsx-${VERSION}
	@gzip lsx-${VERSION}.tar
	@rm -rf lsx-${VERSION}

install: all
	@echo installing executable file to ${DESTDIR}${PREFIX}/bin
	@mkdir -p ${DESTDIR}${PREFIX}/bin
	@cp -f lsx ${DESTDIR}${PREFIX}/bin
	@chmod 755 ${DESTDIR}${PREFIX}/bin/lsx

uninstall:
	@echo removing executable file from ${DESTDIR}${PREFIX}/bin
	@rm -f ${DESTDIR}${PREFIX}/bin/lsx

.PHONY: all options clean dist install uninstall
