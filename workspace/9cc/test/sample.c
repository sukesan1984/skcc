//#define _POSIX_C_SOURCE 200809L
//#ifndef __x86_64__
//#define __x86_64__
//#endif
//#ifndef __LP64__
//#define __LP64__
//#endif
//#ifndef __GNUC__
//#define __GNUC__ 3
//#endif
//#ifndef __STDC__
//#define __STDC__
//#endif
//struct stat {
//      char _[512];
//};
////#include <stdnoreturn.h>
//#include <stddef.h>
//
//#include <stdio.h>
//#include <stdlib.h>
//#nclude <stddef.h>
//#include <stdint.h>
//#include <string.h>
//#include <sys/cdefs.h>
//#include <features.h>
//#include <endian.h>
//#include <sys/types.h>
//#include <sys/stat.h>
//#include <stdarg.h>
//#include <ctype.h>
//#include <errno.h>
//#define __LITTLE_ENDIAN 1234
//#define __BYTE_ORDER __LITTLE_ENDIAN
//#if __BYTE_ORDER == __LITTLE_ENDIAN
//# define __LONG_LONG_PAIR(HI, LO) LO, HI
//#elif __BYTE_ORDER == __BIG_ENDIAN
//# define __LONG_LONG_PAIR(HI, LO) HI, LO
//#endif

//int fuga() {
//    return 4;
//}

#include "9cc.h"
int main () {
//    int (*hoge)();
//    hoge = fuga;
    printf("hello world\n");
    return 0L;
}
