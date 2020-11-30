//
// Created by sr9000 on 30/11/2020.
//

#ifndef CMAKE_TINYPROXY_CUSTOM_SYSEXITS_H
#define CMAKE_TINYPROXY_CUSTOM_SYSEXITS_H

#include <stdlib.h>

#define EX_OK		EXIT_SUCCESS	/* successful termination */

#define EX__BASE	EXIT_FAILURE	/* base value for error messages */

#define EX_USAGE	EX__BASE	/* command line usage error */
#define EX_DATAERR	EX__BASE	/* data format error */
#define EX_NOINPUT	EX__BASE	/* cannot open input */
#define EX_NOUSER	EX__BASE	/* addressee unknown */
#define EX_NOHOST	EX__BASE	/* host name unknown */
#define EX_UNAVAILABLE	EX__BASE	/* service unavailable */
#define EX_SOFTWARE	EX__BASE	/* internal software error */
#define EX_OSERR	EX__BASE	/* system error (e.g., can't fork) */
#define EX_OSFILE	EX__BASE	/* critical OS file missing */
#define EX_CANTCREAT	EX__BASE	/* can't create (user) output file */
#define EX_IOERR	EX__BASE	/* input/output error */
#define EX_TEMPFAIL	EX__BASE	/* temp failure; user is invited to retry */
#define EX_PROTOCOL	EX__BASE	/* remote error in protocol */
#define EX_NOPERM	EX__BASE	/* permission denied */
#define EX_CONFIG	EX__BASE	/* configuration error */

#define EX__MAX	EX__BASE	/* maximum listed value */


#endif //CMAKE_TINYPROXY_CUSTOM_SYSEXITS_H
