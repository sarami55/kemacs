#include <stdio.h>
#include "kanji.h"
#include "Cstrings.h"

#include "../econfig.h"
#if BSD_LEGACY
# include <strings.h>
#else
# include <string.h>
#endif

struct KS {
    char *buf;
    int ptr;
    int len;
};

static int copen(), cclose(), cput();

char *
cfromC(C)
    register Char *C;
{
    int len;
    register KSTREAM *ksp;
    struct KS ks;
    static int blen = 0;
    static char *buf;

    if (!C) return NULL;
    if (blen <= (len = Cstrlen(C)*3)) {
	/* more space will be needed */
	len = (len | 255) + 1;
	if (blen) free(buf);
	buf = malloc((unsigned)len*sizeof(char));
	if (!buf) {
	    blen = 0;
	    return NULL;
	}
	blen = len;
    }
    ks.buf = buf;
    ks.ptr = 0;
    ks.len = blen;
    if (!(ksp = kalloc((caddr_t)&ks, copen, cclose, (int(*)())NULL, cput, KS_BINMODE)))
	return NULL;
    while (*C) (void)kputc(*C++, ksp);
    (void)kputc(' ', ksp); /* reset state of stream */
    blen = ks.len;
    buf = ks.buf;
    buf[ks.ptr-1] = '\0'; /* trim last char */
    kfree(ksp);
    return buf;
}

/*ARGSUSED*/
static
copen(id)
    struct KS *id;
{
    return 0;
}

static
cclose(id)
    register struct KS *id;
{
    id->buf[id->ptr+1] = '\0';
    return 0;
}

static
cput(id, buf, len)
    register struct KS *id;
    char *buf;
    int len;
{
    while (id->ptr+len >= id->len)
	id->buf = realloc(id->buf, (unsigned)(id->len += 256));
    (void)strncpy(&id->buf[id->ptr], buf, len);
    id->ptr += len;
    return len;
}
