cmd_scripts/kconfig/conf.o := gcc -Wp,-MD,scripts/kconfig/.conf.o.d -Iscripts/kconfig -Wall -Wmissing-prototypes -Wstrict-prototypes -O2 -fomit-frame-pointer -std=gnu89 -DCURSES_LOC="<ncurses.h>" -DNCURSES_WIDECHAR=1 -DLOCALE -DKBUILD_NO_NLS -c -o scripts/kconfig/conf.o ../scripts/kconfig/conf.c

source_scripts/kconfig/conf.o := ../scripts/kconfig/conf.c

deps_scripts/kconfig/conf.o := \
    $(wildcard include/config/.h) \
    $(wildcard include/config/seed.h) \
    $(wildcard include/config/allconfig.h) \
    $(wildcard include/config/nosilentupdate.h) \
  /usr/include/stdc-predef.h \
  /usr/include/locale.h \
  /usr/include/features.h \
  /usr/include/bits/alltypes.h \
  /usr/include/ctype.h \
  /usr/include/stdio.h \
  /usr/include/stdlib.h \
  /usr/include/alloca.h \
  /usr/include/string.h \
  /usr/include/strings.h \
  /usr/include/time.h \
  /usr/include/unistd.h \
  /usr/include/bits/posix.h \
  /usr/include/getopt.h \
  /usr/include/sys/stat.h \
  /usr/include/bits/stat.h \
  /usr/include/sys/time.h \
  /usr/include/sys/select.h \
  /usr/include/errno.h \
  /usr/include/bits/errno.h \
  ../scripts/kconfig/lkc.h \
    $(wildcard include/config/prefix.h) \
    $(wildcard include/config/list.h) \
    $(wildcard include/config/y.h) \
  ../scripts/kconfig/expr.h \
    $(wildcard include/config/config.h) \
  /usr/include/assert.h \
  ../scripts/kconfig/list.h \
  /usr/include/stdbool.h \
  ../scripts/kconfig/lkc_proto.h \
  /usr/include/stdarg.h \

scripts/kconfig/conf.o: $(deps_scripts/kconfig/conf.o)

$(deps_scripts/kconfig/conf.o):
