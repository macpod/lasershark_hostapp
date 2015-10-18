#include "getopt_portable.h"

/*
* Copyright (c) 1987, 1993, 1994
*      The Regents of the University of California.  All rights reserved.
*
* Redistribution and use in source and binary forms, with or without
* modification, are permitted provided that the following conditions
* are met:
* 1. Redistributions of source code must retain the above copyright
*    notice, this list of conditions and the following disclaimer.
* 2. Redistributions in binary form must reproduce the above copyright
*    notice, this list of conditions and the following disclaimer in the
*    documentation and/or other materials provided with the distribution.
* 3. All advertising materials mentioning features or use of this software
*    must display the following acknowledgement:
*      This product includes software developed by the University of
*      California, Berkeley and its contributors.
* 4. Neither the name of the University nor the names of its contributors
*    may be used to endorse or promote products derived from this software
*    without specific prior written permission.
*
* THIS SOFTWARE IS PROVIDED BY THE REGENTS AND CONTRIBUTORS ``AS IS'' AND
* ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
* IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
* ARE DISCLAIMED.  IN NO EVENT SHALL THE REGENTS OR CONTRIBUTORS BE LIABLE
* FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
* DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
* OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
* HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
* LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
* OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
* SUCH DAMAGE.
*/

#include <string.h>
#include <stdio.h>

int     opterr_portable = 1,             /* if error message should be printed */
        optind_portable = 1,             /* index into parent argv vector */
        optopt_portable,                 /* character checked for validity */
        optreset_portable;               /* reset getopt */
char    *optarg_portable;                /* argument associated with option */

#define BADCH   (int)'?'
#define BADARG  (int)':'
#define EMSG    ""

/*
 * getopt_portable --
 *      Parse argc/argv argument vector.
 */
int getopt_portable(int nargc, char * const nargv[], const char *ostr)
{
    static char *place = EMSG;              /* option letter processing */
    const char *oli;                              /* option letter list index */

    if (optreset_portable || !*place) {              /* update scanning pointer */
        optreset_portable = 0;
        if (optind_portable >= nargc || *(place = nargv[optind_portable]) != '-') {
            place = EMSG;
            return (-1);
        }
        if (place[1] && *++place == '-') {      /* found "--" */
            ++optind_portable;
            place = EMSG;
            return (-1);
        }
    }                                       /* option letter okay? */
    if ((optopt_portable = (int)*place++) == (int)':' ||
            !(oli = strchr(ostr, optopt_portable))) {
        /*
        * if the user didn't specify '-' as an option,
        * assume it means -1.
        */
        if (optopt_portable == (int)'-')
            return (-1);
        if (!*place)
            ++optind_portable;
        if (opterr_portable && *ostr != ':')
            (void)printf("illegal option -- %c\n", optopt_portable);
        return (BADCH);
    }
    if (*++oli != ':') {                    /* don't need argument */
        optarg_portable = NULL;
        if (!*place)
            ++optind_portable;
    }
    else {                                  /* need an argument */
        if (*place)                     /* no white space */
            optarg_portable = place;
        else if (nargc <= ++optind_portable) {   /* no arg */
            place = EMSG;
            if (*ostr == ':')
                return (BADARG);
            if (opterr_portable)
                (void)printf("option requires an argument -- %c\n", optopt_portable);
            return (BADCH);
        }
        else                            /* white space */
            optarg_portable = nargv[optind_portable];
        place = EMSG;
        ++optind_portable;
    }
    return (optopt_portable);                        /* dump back option letter */
}
