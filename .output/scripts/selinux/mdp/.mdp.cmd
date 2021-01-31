cmd_scripts/selinux/mdp/mdp := gcc -Wp,-MD,scripts/selinux/mdp/.mdp.d -Iscripts/selinux/mdp -Wall -Wmissing-prototypes -Wstrict-prototypes -O2 -fomit-frame-pointer -std=gnu89  -I../security/selinux/include -Isecurity/selinux/include -o scripts/selinux/mdp/mdp ../scripts/selinux/mdp/mdp.c  

source_scripts/selinux/mdp/mdp := ../scripts/selinux/mdp/mdp.c

deps_scripts/selinux/mdp/mdp := \
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
  ../security/selinux/include/classmap.h \
  ../security/selinux/include/initial_sid_to_string.h \

scripts/selinux/mdp/mdp: $(deps_scripts/selinux/mdp/mdp)

$(deps_scripts/selinux/mdp/mdp):
