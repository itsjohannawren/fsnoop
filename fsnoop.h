#ifndef __FSNOOP_H
#define __FSNOOP_H

#define LOG_CRITICAL 0
#define LOG_ERROR 1
#define LOG_WARNING 2
#define LOG_NOTICE 3
#define LOG_INFO 4
#define LOG_VERBOSE 5
#define LOG_DEBUG1 6
#define LOG_DEBUG2 7
#define LOG_DEBUG3 8

int lprintf (const int level, const char *format, ...);

#endif
