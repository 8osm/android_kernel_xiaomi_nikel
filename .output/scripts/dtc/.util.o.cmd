cmd_scripts/dtc/util.o := gcc -Wp,-MD,scripts/dtc/.util.o.d -Iscripts/dtc -Wall -Wmissing-prototypes -Wstrict-prototypes -O2 -fomit-frame-pointer -std=gnu89  -I../scripts/dtc -Iscripts/dtc  -I../scripts/dtc/libfdt -Iscripts/dtc/libfdt -c -o scripts/dtc/util.o ../scripts/dtc/util.c

source_scripts/dtc/util.o := ../scripts/dtc/util.c

deps_scripts/dtc/util.o := \
  /usr/include/stdc-predef.h \
  /usr/include/ctype.h \
  /usr/include/features.h \
  /usr/include/bits/alltypes.h \
  /usr/include/stdio.h \
  /usr/include/stdlib.h \
  /usr/include/alloca.h \
  /usr/include/stdarg.h \
  /usr/include/string.h \
  /usr/include/strings.h \
  /usr/include/assert.h \
  /usr/include/errno.h \
  /usr/include/bits/errno.h \
  /usr/include/fcntl.h \
  /usr/include/bits/fcntl.h \
  /usr/include/unistd.h \
  /usr/include/bits/posix.h \
  ../scripts/dtc/libfdt/libfdt.h \
  ../scripts/dtc/libfdt/libfdt_env.h \
  /usr/include/stddef.h \
  /usr/include/stdint.h \
  /usr/include/bits/stdint.h \
  ../scripts/dtc/libfdt/fdt.h \
  ../scripts/dtc/util.h \
  /usr/include/getopt.h \
  ../scripts/dtc/version_gen.h \

scripts/dtc/util.o: $(deps_scripts/dtc/util.o)

$(deps_scripts/dtc/util.o):
