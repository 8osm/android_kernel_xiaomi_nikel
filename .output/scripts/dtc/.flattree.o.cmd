cmd_scripts/dtc/flattree.o := gcc -Wp,-MD,scripts/dtc/.flattree.o.d -Iscripts/dtc -Wall -Wmissing-prototypes -Wstrict-prototypes -O2 -fomit-frame-pointer -std=gnu89  -I../scripts/dtc -Iscripts/dtc  -I../scripts/dtc/libfdt -Iscripts/dtc/libfdt -c -o scripts/dtc/flattree.o ../scripts/dtc/flattree.c

source_scripts/dtc/flattree.o := ../scripts/dtc/flattree.c

deps_scripts/dtc/flattree.o := \
  /usr/include/stdc-predef.h \
  ../scripts/dtc/dtc.h \
  /usr/include/stdio.h \
  /usr/include/features.h \
  /usr/include/bits/alltypes.h \
  /usr/include/string.h \
  /usr/include/strings.h \
  /usr/include/stdlib.h \
  /usr/include/alloca.h \
  /usr/include/stdint.h \
  /usr/include/bits/stdint.h \
  /usr/include/stdbool.h \
  /usr/include/stdarg.h \
  /usr/include/assert.h \
  /usr/include/ctype.h \
  /usr/include/errno.h \
  /usr/include/bits/errno.h \
  /usr/include/unistd.h \
  /usr/include/bits/posix.h \
  ../scripts/dtc/libfdt/libfdt_env.h \
  /usr/include/stddef.h \
  ../scripts/dtc/libfdt/fdt.h \
  ../scripts/dtc/util.h \
  /usr/include/getopt.h \
  ../scripts/dtc/srcpos.h \

scripts/dtc/flattree.o: $(deps_scripts/dtc/flattree.o)

$(deps_scripts/dtc/flattree.o):
