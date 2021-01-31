cmd_scripts/basic/fixdep := gcc -Wp,-MD,scripts/basic/.fixdep.d -Iscripts/basic -Wall -Wmissing-prototypes -Wstrict-prototypes -O2 -fomit-frame-pointer -std=gnu89 -o scripts/basic/fixdep ../scripts/basic/fixdep.c  

source_scripts/basic/fixdep := ../scripts/basic/fixdep.c

deps_scripts/basic/fixdep := \
    $(wildcard include/config/his/driver.h) \
    $(wildcard include/config/my/option.h) \
    $(wildcard include/config/.h) \
    $(wildcard include/config/foo.h) \
    $(wildcard include/config/boom.h) \
  /usr/include/stdc-predef.h \
  /usr/include/sys/types.h \
  /usr/include/features.h \
  /usr/include/bits/alltypes.h \
  /usr/include/endian.h \
  /usr/include/sys/select.h \
  /usr/include/sys/stat.h \
  /usr/include/bits/stat.h \
  /usr/include/sys/mman.h \
  /usr/include/bits/mman.h \
  /usr/include/unistd.h \
  /usr/include/bits/posix.h \
  /usr/include/fcntl.h \
  /usr/include/bits/fcntl.h \
  /usr/include/string.h \
  /usr/include/strings.h \
  /usr/include/stdlib.h \
  /usr/include/alloca.h \
  /usr/include/stdio.h \
  /usr/include/limits.h \
  /usr/include/bits/limits.h \
  /usr/include/ctype.h \
  /usr/include/arpa/inet.h \
  /usr/include/netinet/in.h \
  /usr/include/inttypes.h \
  /usr/include/stdint.h \
  /usr/include/bits/stdint.h \
  /usr/include/sys/socket.h \
  /usr/include/bits/socket.h \

scripts/basic/fixdep: $(deps_scripts/basic/fixdep)

$(deps_scripts/basic/fixdep):
