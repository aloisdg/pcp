/*
    This file is part of Pretty Curved Privacy (pcp1).

    Copyright (C) 2013 T.Linden.

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.

    You can contact me by mail: <tlinden AT cpan DOT org>.
*/


#ifndef _HAVE_PCP_PLATFORM_H
#define _HAVE_PCP_PLATFORM_H

#include "config.h"

#ifdef HAVE_ENDIAN_H
# include <endian.h>
#else /*  no endian.h */
# ifdef HAVE_SYS_ENDIAN_H
#   include <sys/types.h>
#   include <sys/endian.h>
#   ifndef HAVE_BE32TOH
#     /*  openbsd, use aliases */
#     define be16toh betoh16
#     define be32toh betoh32
#     define be64toh betoh64
#   endif
# else /*  no sys/endian.h */
#   ifdef __CPU_IS_BIG_ENDIAN
#     define be16toh(x)	(x)
#     define htobe16(x)	(x)
#     define be32toh(x)	(x)
#     define htobe32(x)	(x)
#     define be64toh(x)	(x)
#     define htobe64(x)	(x)
#   else
#     ifdef HAVE_ARPA_INET_H
#       include <arpa/inet.h>
#     else
#       ifdef HAVE_NETINET_IN_H
#         include <netinet/in.h>
#       else
#         error Need either netinet/in.h or arpa/inet.h for ntohl() and htonl()
#       endif
#     endif
#     define be16toh(x)	((u_int16_t)ntohl((u_int16_t)(x)))
#     define htobe16(x)	((u_int16_t)htonl((u_int16_t)(x)))
#     define be32toh(x)	((u_int32_t)ntohl((u_int32_t)(x)))
#     define htobe32(x)	((u_int32_t)htonl((u_int32_t)(x)))
#     define be64toh(x)	((u_int64_t)ntohl((u_int64_t)(x)))
#     define htobe64(x)	((u_int64_t)htonl((u_int64_t)(x)))
#   endif
#  endif /*  HAVE_SYS_ENDIAN_H */
#endif /*  HAVE_ENDIAN_H */


#ifndef HAVE_ARC4RANDOM
#include <sodium.h>
static inline u_int32_t arc4random() {
  return randombytes_random();
}
#endif

#ifndef HAVE_ARC4RANDOM_BUF
#include <sodium.h>
static inline void arc4random_buf(void *buf, size_t nbytes) {
  randombytes((unsigned char*)buf, nbytes);
}
#endif


#ifndef HAVE_ERR_H

#include <errno.h>
#include <string.h>
#include <stdarg.h>
#include <stdio.h>

static inline void err(int eval, const char *fmt, ...) {
  va_list ap;
  va_start(ap, fmt);
  fprintf(stderr, "pcp1");
  if (fmt != NULL) {
    fprintf(stderr, ": ");
    vfprintf(stderr, fmt, ap);
  }
  fprintf(stderr, ": %s\n", strerror(errno));
  va_end(ap);
}

#else

#include <errno.h>
#include <err.h>

#endif



#ifndef HAVE_VASPRINTF

#include <stdarg.h>
static inline
int vasprintf(char **ret, const char *format, va_list args) {
  va_list copy;
  va_copy(copy, args);

  *ret = 0;

  int count = vsnprintf(NULL, 0, format, args);
  if (count >= 0) {
    char* buffer = (char *)malloc(count + 1);
    if (buffer != NULL) {
      count = vsnprintf(buffer, count + 1, format, copy);
      if (count < 0)
	free(buffer);
      else
	*ret = buffer;
    }
  }
  va_end(copy);  /*  Each va_start() or va_copy() needs a va_end() */

  return count;
}

#endif


#ifdef _AIX_SOURCE
#define _LINUX_SOURCE_COMPAT
#endif


#ifndef HAVE_STRNLEN
static inline size_t
strnlen(const char *msg, size_t maxlen)
{
 size_t i;

 for (i=0; i<maxlen; i++)
 if (msg[i] == '\0')
 break;

 return i;
}
#endif


#ifndef HAVE_STRNSTR
/* via FreeBSD libc */
#include <string.h>
static inline char *
strnstr(const char *s, const char *find, size_t slen)
{
  char c, sc;
  size_t len;
  
  if ((c = *find++) != '\0') {
    len = strlen(find);
    do {
      do {
	if (slen-- < 1 || (sc = *s++) == '\0')
	  return (NULL);
      } while (sc != c);
      if (len > slen)
	return (NULL);
    } while (strncmp(s, find, len) != 0);
    s--;
  }
  return ((char *)s);
}
#endif

/* size_t format string */
#ifdef __LP64__
  #define FMT_SIZE_T "llu"
  #define SIZE_T_CAST long long unsigned int
#else
  #define FMT_SIZE_T "lu"
  #define SIZE_T_CAST long unsigned int
#endif

#endif /* !_HAVE_PCP_PLATFORM_H */


