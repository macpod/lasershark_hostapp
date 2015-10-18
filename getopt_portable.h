#ifndef GETOPT_PORTABLE_H
#define GETOPT_PORTABLE_H

extern int  opterr_portable,           /* if error message should be printed */
       optind_portable,                /* index into parent argv vector */
       optopt_portable,                /* character checked for validity */
       optreset_portable;              /* reset getopt */
extern char *optarg_portable;          /* argument associated with option */

int getopt_portable(int nargc, char * const nargv[], const char *ostr);

#endif

