#ifndef __CACHE_H
#define __CACHE_H

struct cacheEntry {
	struct cacheEntry* next;
	struct cacheEntry* prev;
	unsigned int pid;
	char* exe;
	char* cmdline;
};

int pidCacheInit ();
int pidCacheSet (int pid, char* exe, char* cmdline);
int pidCacheGet (int pid, char* exe, ssize_t exeLen, char* cmdline, ssize_t cmdlineLen);
void pidCacheRemove (int pid);

#endif
