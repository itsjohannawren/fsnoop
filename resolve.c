#include <errno.h>
#include <linux/limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/fanotify.h>
#include <unistd.h>

#include "cache.h"

extern unsigned char optCache;

ssize_t sprintmask (const unsigned long long mask, char* dst) {
	char* dest = dst;
	int retlen = 0, len = 0;

	// Was this a FAN_ACCESS?
	if (mask & FAN_ACCESS) {
		// Sure enough. Append some nice output
		len = sprintf (dest, "Access,");
		// Move the dest pointer to the new end
		dest += len;
		// Make the length longerererer
		retlen += len;
	}
	// Was this a FAN_MODIFY?
	if (mask & FAN_MODIFY) {
		// Sure enough. Append some nice output
		len = sprintf (dest, "Modify,");
		// Move the dest pointer to the new end
		dest += len;
		// Make the length longerererer
		retlen += len;
	}
	// Was this a FAN_CLOSE_WRITE?
	if (mask & FAN_CLOSE_WRITE) {
		// Sure enough. Append some nice output
		len = sprintf (dest, "Close+Write,");
		// Move the dest pointer to the new end
		dest += len;
		// Make the length longerererer
		retlen += len;
	}
	// Was this a FAN_CLOSE_NOWRITE?
	if (mask & FAN_CLOSE_NOWRITE) {
		// Sure enough. Append some nice output
		len = sprintf (dest, "Close,");
		// Move the dest pointer to the new end
		dest += len;
		// Make the length longerererer
		retlen += len;
	}
	// Was this a FAN_OPEN?
	if (mask & FAN_OPEN) {
		// Sure enough. Append some nice output
		len = sprintf (dest, "Open,");
		// Move the dest pointer to the new end
		dest += len;
		// Make the length longerererer
		retlen += len;
	}
	// Was this a FAN_Q_OVERFLOW?
	if (mask & FAN_Q_OVERFLOW) {
		// Sure enough. Append some nice output
		len = sprintf (dest, "Overflow,");
		// Move the dest pointer to the new end
		dest += len;
		// Make the length longerererer
		retlen += len;
	}
	// Was this a FAN_OPEN_PERM?
	if (mask & FAN_OPEN_PERM) {
		// Sure enough. Append some nice output
		len = sprintf (dest, "Open+PermissionCheck,");
		// Move the dest pointer to the new end
		dest += len;
		// Make the length longerererer
		retlen += len;
	}
	// Was this a FAN_ACCESS_PERM?
	if (mask & FAN_ACCESS_PERM) {
		// Sure enough. Append some nice output
		len = sprintf (dest, "Access+PermissionCheck,");
		// Move the dest pointer to the new end
		dest += len;
		// Make the length longerererer
		retlen += len;
	}
	// Was this a FAN_ONDIR?
	if (mask & FAN_ONDIR) {
		// Sure enough. Append some nice output
		len = sprintf (dest, "Directory,");
		// Move the dest pointer to the new end
		dest += len;
		// Make the length longerererer
		retlen += len;
	}
	// Was this a FAN_EVENT_ON_CHILD?
	if (mask & FAN_EVENT_ON_CHILD) {
		// Sure enough. Append some nice output
		len = sprintf (dest, "ChildInfo,");
		// Move the dest pointer to the new end
		dest += len;
		// Make the length longerererer
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
void fileByFD (int fd, char* dst, ssize_t len) {
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
void exeByPID (int pid, char* dst, ssize_t len) {
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
			pidCacheGet (pid, filePath, PATH_MAX);
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
		pidCacheSet (pid, filePath);
	}
}
void cmdlineByPID (int pid, char* dst, ssize_t len) {
	char cmdlinePath [PATH_MAX];
	char* buffer;
	ssize_t bufferLen = 0;
	int offset;
	FILE* fd;

	// Clear all the buffers
	memset (cmdlinePath, 0, sizeof (cmdlinePath));
	memset (dst, 0, len);

	// Build the PID path buffer
	sprintf (cmdlinePath, "/proc/%d/cmdline", pid);

	// Open the cmdline file
	fd = fopen (cmdlinePath, "r");
	// Did it open?
	if (fd == NULL) {
		// No
		return;
	}

	// Move to the end of the cmdline
	fseek (fd, 0, SEEK_END);
	// Get the current position to see how long the file is
	bufferLen = ftell (fd);
	// Move back to the beginning so we can read
	fseek (fd, 0, SEEK_SET);

	// Allocate a buffer to fit the cmdline file
	buffer = malloc (bufferLen + 1);
	// Allocated?
	if (buffer == NULL) {
		// Shizzle
		fclose (fd);
		// Bail
		return;
	}

	// Read in the file
	fread (buffer, bufferLen, 1, fd);
	// Close it
	fclose (fd);

	// Move through every byte in the file
	for (offset = 0; offset < bufferLen - 1; offset++) {
		// If it's a NULL...
		if (*(buffer + offset) == '\0') {
			// Change it to a space
			*(buffer + offset) = ' ';
		}
	}

	// Copy upto len bytes from the cmdline to the return buffer
	strncpy (dst, buffer, len);

	// Free the buffer
	free (buffer);
}
