cmd_scripts/selinux/genheaders/genheaders := gcc -Wp,-MD,scripts/selinux/genheaders/.genheaders.d -Iscripts/selinux/genheaders -Wall -Wmissing-prototypes -Wstrict-prototypes -O2 -fomit-frame-pointer -std=gnu89  -I../security/selinux/include -Isecurity/selinux/include -o scripts/selinux/genheaders/genheaders ../scripts/selinux/genheaders/genheaders.c  

source_scripts/selinux/genheaders/genheaders := ../scripts/selinux/genheaders/genheaders.c

deps_scripts/selinux/genheaders/genheaders := \
  /usr/include/stdc-predef.h \
  /usr/include/stdio.h \
  /usr/include/features.h \
  /usr/include/bits/alltypes.h \
  /usr/include/stdlib.h \
  /usr/include/alloca.h \
  /usr/include/unistd.h \
  /usr/include/bits/posix.h \
  /usr/include/string.h \
  /usr/include/strings.h \
  /usr/include/errno.h \
  /usr/include/bits/errno.h \
  /usr/include/ctype.h \
  ../security/selinux/include/classmap.h \
  ../security/selinux/include/initial_sid_to_string.h \

scripts/selinux/genheaders/genheaders: $(deps_scripts/selinux/genheaders/genheaders)

$(deps_scripts/selinux/genheaders/genheaders):
