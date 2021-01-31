cmd_scripts/kallsyms := gcc -Wp,-MD,scripts/.kallsyms.d -Iscripts -Wall -Wmissing-prototypes -Wstrict-prototypes -O2 -fomit-frame-pointer -std=gnu89  -I../../tools/include -I../tools/include -o scripts/kallsyms ../scripts/kallsyms.c  

source_scripts/kallsyms := ../scripts/kallsyms.c

deps_scripts/kallsyms := \
    $(wildcard include/config/page/offset.h) \
  /usr/include/stdc-predef.h \
  /usr/include/stdio.h \
  /usr/include/features.h \
  /usr/include/bits/alltypes.h \
  /usr/include/stdlib.h \
  /usr/include/alloca.h \
  /usr/include/string.h \
  /usr/include/strings.h \
  /usr/include/ctype.h \

scripts/kallsyms: $(deps_scripts/kallsyms)

$(deps_scripts/kallsyms):
