#include <errno.h>
#include <fcntl.h>
#include <linux/limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/fanotify.h>
#include <sys/types.h>
#include <unistd.h>

#include "fsnoop.h"
#include "cache.h"
#include "stubs.h"

extern unsigned char optCache;

ssize_t sprintmask (const unsigned long long mask, char* dst) {
	char* dest = dst;
	int retlen = 0, len = 0;

	if (FAN_CREATE && (mask & FAN_CREATE)) {
		len = sprintf (dest, "create,");
		dest += len;
		retlen += len;
	}

	if (FAN_OPEN && (mask & FAN_OPEN)) {
		len = sprintf (dest, "open,");
		dest += len;
		retlen += len;
	}
	if (FAN_OPEN_PERM && (mask & FAN_OPEN_PERM)) {
		len = sprintf (dest, "open+permcheck,");
		dest += len;
		retlen += len;
	}
	if (FAN_OPEN_EXEC && (mask & FAN_OPEN_EXEC)) {
		len = sprintf (dest, "open+exec,");
		dest += len;
		retlen += len;
	}
	if (FAN_OPEN_EXEC_PERM && (mask & FAN_OPEN_EXEC_PERM)) {
		len = sprintf (dest, "open+exec+permcheck,");
		dest += len;
		retlen += len;
	}

	if (FAN_ACCESS && (mask & FAN_ACCESS)) {
		len = sprintf (dest, "access,");
		dest += len;
		retlen += len;
	}
	if (FAN_ACCESS_PERM && (mask & FAN_ACCESS_PERM)) {
		len = sprintf (dest, "access+permcheck,");
		dest += len;
		retlen += len;
	}
	if (FAN_MODIFY && (mask & FAN_MODIFY)) {
		len = sprintf (dest, "modify,");
		dest += len;
		retlen += len;
	}

	if (FAN_CLOSE && (mask & FAN_CLOSE)) {
		len = sprintf (dest, "close,");
		dest += len;
		retlen += len;
	}
	if (FAN_CLOSE_WRITE && (mask & FAN_CLOSE_WRITE)) {
		len = sprintf (dest, "close+write,");
		dest += len;
		retlen += len;
	}
	if (FAN_CLOSE_NOWRITE && (mask & FAN_CLOSE_NOWRITE)) {
		len = sprintf (dest, "close+nowrite,");
		dest += len;
		retlen += len;
	}

	if (FAN_DELETE && (mask & FAN_DELETE)) {
		len = sprintf (dest, "delete,");
		dest += len;
		retlen += len;
	}
	if (FAN_DELETE_SELF && (mask & FAN_DELETE_SELF)) {
		len = sprintf (dest, "deleteself,");
		dest += len;
		retlen += len;
	}

	if (FAN_MOVE && (mask & FAN_MOVE)) {
		len = sprintf (dest, "move,");
		dest += len;
		retlen += len;
	}
	if (FAN_MOVED_FROM && (mask & FAN_MOVED_FROM)) {
		len = sprintf (dest, "movedfrom,");
		dest += len;
		retlen += len;
	}
	if (FAN_MOVED_TO && (mask & FAN_MOVED_TO)) {
		len = sprintf (dest, "movedto,");
		dest += len;
		retlen += len;
	}

	if (FAN_ATTRIB && (mask & FAN_ATTRIB)) {
		len = sprintf (dest, "attrib,");
		dest += len;
		retlen += len;
	}

	if (FAN_Q_OVERFLOW && (mask & FAN_Q_OVERFLOW)) {
		len = sprintf (dest, "qoverload,");
		dest += len;
		retlen += len;
	}
	if (FAN_ONDIR && (mask & FAN_ONDIR)) {
		len = sprintf (dest, "directory,");
		dest += len;
		retlen += len;
	}
	if (FAN_EVENT_ON_CHILD && (mask & FAN_EVENT_ON_CHILD)) {
		len = sprintf (dest, "child,");
		dest += len;
		retlen += len;
	}

	// Did we write anything?
	if (dest != dst) {
		// Aww yissss! Move dest back one byte so it's on the last comma
		dest--;
		// Shorten the length by one because we backed up
		retlen--;
	}
	// Set the last byte to NULL
	*dest = '\0';

	// Return the length
	return (retlen);
}
void fileByFD (const int fd, char* dst, ssize_t len) {
	char fdPath [PATH_MAX], filePath [PATH_MAX];
	ssize_t filePathLen;

	// Clear all the buffers
	memset (fdPath, 0, sizeof (fdPath));
	memset (filePath, 0, sizeof (filePath));
	memset (dst, 0, len);

	// Build the fd path buffer
	sprintf (fdPath, "/proc/self/fd/%d", fd);
	// Read the fd symlink
	filePathLen = readlink (fdPath, filePath, sizeof (filePath) - 1);
	// Did we get a path?
	if (filePathLen == -1) {
		// No :-(
		strncpy (dst, "(unknown)", len - 1);
	} else {
		// Yes!
		strncpy (dst, filePath, len - 1);
	}
}
void exeByPID (const int pid, char* dst, ssize_t len) {
	char exePath [PATH_MAX], filePath [PATH_MAX];
	ssize_t filePathLen;

	// Clear all the buffers
	memset (exePath, 0, sizeof (exePath));
	memset (filePath, 0, sizeof (filePath));
	memset (dst, 0, len);

	// Build the PID path buffer
	sprintf (exePath, "/proc/%d/exe", pid);
	// Read the exe symlink
	filePathLen = readlink (exePath, filePath, sizeof (filePath) - 1);
	// Did we get a path?
	if (filePathLen == -1) {
		// No, Are we using the cache?
		if (optCache) {
			// Gimme the cash!
			pidCacheGet (pid, filePath, PATH_MAX, NULL, 0);
			// Did the cache have anything?
			if (strlen (filePath) == 0) {
				// No :-(
				strncpy (dst, "(unknown)", len - 1);
			} else {
				// Sure did!
				strncpy (dst, filePath, len - 1);
			}
		} else {
			// No cache
			strncpy (dst, "(unknown)", len - 1);
		}
	} else {
		// Yes!
		strncpy (dst, filePath, len - 1);
		// Set the PID->exe in the cache
		pidCacheSet (pid, filePath, NULL);
	}
}
int cmdlineByPID (const int pid, char* dst, ssize_t len) {
	char cmdlinePath [PATH_MAX];
	char* buffer;
	ssize_t bufferLen = 0;
	int fd, offset;

	// Clear all the buffers
	memset (cmdlinePath, 0, sizeof (cmdlinePath));
	memset (dst, 0, len);

	// Build the PID path buffer
	snprintf (cmdlinePath, sizeof (cmdlinePath), "/proc/%d/cmdline", pid);

	// Open the cmdline file
	fd = open (cmdlinePath, O_RDONLY);
	// Did it open?
	if (fd == -1) {
		// No
		return (0);
	}

	buffer = malloc (4096);
	if (buffer == NULL) {
		close (fd);
		return (-1);
	}
	bufferLen = 4096;
	memset (buffer, 0, bufferLen);

	// Read in the file
	bufferLen = read (fd, buffer, bufferLen);
	// Close it
	close (fd);

	if (bufferLen == -1) {
		return (bufferLen);
	}

	// Move through every byte in the file
	for (offset = 0; offset < bufferLen - 1; offset++) {
		// If it's a NULL...
		if (*(buffer + offset) == '\0') {
			// Change it to a space
			*(buffer + offset) = ' ';
		}
	}
	for (offset = bufferLen; offset >= 0; offset--) {
		// If it's a NULL...
		if ((*(buffer + offset) == '\0') || (*(buffer + offset) == ' ')) {
			// Change it to a space
			*(buffer + offset) = '\0';
		} else {
			break;
		}
	}

	// Copy upto len bytes from the cmdline to the return buffer
	strncpy (dst, buffer, len);

	// Free the buffer
	free (buffer);

	return (0);
}
