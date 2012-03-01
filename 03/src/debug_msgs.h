#ifndef DEBUG_MSGS_H_
#define DEBUG_MSGS_H_

#include <syslog.h>

#define STDERR 0
#define SYSLOG 1

#ifdef DEBUG
#define DBG(output, format, args...) \
	if (output == SYSLOG){\
		syslog(LOG_DEBUG,"%s:%d : "format,__FILE__,__LINE__,##args);\
	} else {\
		fprintf(stderr, "[%s:%d, %s()]"format,__FILE__, __LINE__,\
		__FUNCTION__,##args);\
	}

#else
#define DBG(format, args...)

#endif
#endif /* DEBUG_MSGS_H_ */
