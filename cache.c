#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include "cache.h"
#include "dll.h"

struct cacheEntry* PID_CACHE;

int pidCacheInit () {
	// Allocate space for the root entry
	PID_CACHE = malloc (sizeof (struct cacheEntry));
	// We got the space, right?
	if (PID_CACHE == NULL) {
		// We didn't
		return (errno);
	}
	// Clear the root node
	memset (PID_CACHE, 0, sizeof (struct cacheEntry));

	// Return happiness
	return (0);
}

void pidCacheFree (struct cacheEntry* entry) {
	if (entry != NULL) {
		if (entry->exe != NULL) {
			free (entry->exe);
		}
		if (entry->cmdline != NULL) {
			free (entry->cmdline);
		}
		free (entry);
	}
}

int pidCacheSet (int pid, char* exe, char* cmdline) {
	struct cacheEntry* entry;

	for (entry = PID_CACHE->next; entry != NULL; entry = entry->next) {
		if (entry->pid == pid) {
			break;
		}
	}

	if (entry == NULL) {
		entry = malloc (sizeof (struct cacheEntry));
		if (entry == NULL) {
			return (errno);
		}
		memset (entry, 0, sizeof (struct cacheEntry));
		entry->pid = pid;
		if (dllPrepend (PID_CACHE, entry) != 0) {
			pidCacheFree (entry);
			return (-1);
		}
	} else {
		dllRemove (PID_CACHE, entry);
		if (dllPrepend (PID_CACHE, entry) != 0) {
			pidCacheFree (entry);
			return (-1);
		}
	}

	if (exe != NULL) {
		if ((entry->exe == NULL) || (strcmp (entry->exe, exe) != 0)) {
			entry->exe = realloc (entry->exe, strlen (exe) + 1);
			if (entry->exe == NULL) {
				dllRemove (PID_CACHE, entry);
				pidCacheFree (entry);
				return (errno);
			}
			strcpy (entry->exe, exe);
		}
	}

	if (cmdline != NULL) {
		if ((entry->cmdline == NULL) || (strcmp (entry->cmdline, cmdline) != 0)) {
			entry->cmdline = realloc (entry->cmdline, strlen (cmdline) + 1);
			if (entry->cmdline == NULL) {
				dllRemove (PID_CACHE, entry);
				pidCacheFree (entry);
				return (errno);
			}
			strcpy (entry->cmdline, cmdline);
		}
	}

	return (0);
}

int pidCacheGet (int pid, char* exe, ssize_t exeLen, char* cmdline, ssize_t cmdlineLen) {
	struct cacheEntry* entry;

	if (exe != NULL) {
		memset (exe, 0, exeLen);
	}
	if (cmdline != NULL) {
		memset (cmdline, 0, cmdlineLen);
	}

	for (entry = PID_CACHE->next; entry != NULL; entry = entry->next) {
		if (entry->pid == pid) {
			break;
		}
	}

	if (entry == NULL) {
		return (-1);
	}

	if ((exe != NULL) && (entry->exe != NULL)) {
		strncpy (exe, entry->exe, exeLen - 1);
	}

	if ((cmdline != NULL) && (entry->cmdline != NULL)) {
		strncpy (cmdline, entry->cmdline, cmdlineLen - 1);
	}

	return (0);
}
