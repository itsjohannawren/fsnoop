#include <errno.h>
#include <fcntl.h>
#include <getopt.h>
#include <limits.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/fanotify.h>
#include <sys/stat.h>
#include <unistd.h>

#include "fsnoop.h"
#include "resolve.h"
#include "cache.h"
#include "stubs.h"

// =============================================================================

// Options
int optFlagDontFollow = 0;
int optFlagOnlydir = 0;
int optFlagMount = 0;
int optFlagFilesystem = 0;
int optMaskAccess = 0;
int optMaskModify = 0;
int optMaskClose = 0;
int optMaskCloseWrite = 0;
int optMaskCloseNowrite = 0;
int optMaskOpen = 0;
int optMaskOpenExec = 0;
int optMaskAttrib = 0;
int optMaskCreate = 0;
int optMaskDelete = 0;
int optMaskDeleteSelf = 0;
int optMaskMove = 0;
int optMaskMovedFrom = 0;
int optMaskMovedTo = 0;
int optMaskQOverflow = 0;
int optMaskOpenPerm = 0;
int optMaskOpenExecPerm = 0;
int optMaskAccessPerm = 0;
int optMaskOndir = 0;
int optMaskEventOnChild = 0;

int optVerbose = LOG_INFO;
int optCache = 1;
char **optPaths = NULL;

// =============================================================================

int lprintf (const int level, const char *format, ...) {
	int printed = 0;
	va_list arguments;
	va_start (arguments, format);

	if (level <= optVerbose) {
		switch (level) {
			case LOG_CRITICAL:
				printf ("Critical: ");
				break;
			case LOG_ERROR:
				printf ("Error:    ");
				break;
			case LOG_WARNING:
				printf ("Warning:  ");
				break;
			case LOG_NOTICE:
				printf ("Notice:   ");
				break;
			case LOG_INFO:
				printf ("Info:     ");
				break;
			case LOG_VERBOSE:
				printf ("Verbose:  ");
				break;
			case LOG_DEBUG1:
				printf ("Debug1:   ");
				break;
			case LOG_DEBUG2:
				printf ("Debug2:   ");
				break;
			case LOG_DEBUG3:
				printf ("Debug3:   ");
				break;
			default:
				printf ("Unknown:  ");
				break;
		}

		printed += 11;
		printed += vprintf (format, arguments);
		printed += printf ("\n");
	}

	return (printed);
}

// =============================================================================

unsigned int faBuildMarkFlags () {
	unsigned int faMarkFlags = 0;

	if (optFlagDontFollow == 1) {
		if (FAN_MARK_DONT_FOLLOW == 0) {
			lprintf (LOG_WARNING, "FAN_MARK_DONT_FOLLOW is not available");
		}
		faMarkFlags |= FAN_MARK_DONT_FOLLOW;
	}
	if (optFlagOnlydir == 1) {
		if (FAN_MARK_ONLYDIR == 0) {
			lprintf (LOG_WARNING, "FAN_MARK_ONLYDIR is not available");
		}
		faMarkFlags |= FAN_MARK_ONLYDIR;
	}
	if (optFlagMount == 1) {
		if (FAN_MARK_MOUNT == 0) {
			lprintf (LOG_WARNING, "FAN_MARK_MOUNT is not available");
		}
		faMarkFlags |= FAN_MARK_MOUNT;
	}
	if (optFlagFilesystem == 1) {
		if (FAN_MARK_FILESYSTEM == 0) {
			lprintf (LOG_WARNING, "FAN_MARK_FILESYSTEM is not available");
		}
		faMarkFlags |= FAN_MARK_FILESYSTEM;
	}

	return (faMarkFlags);
}

unsigned int faBuildMarkMask () {
	unsigned int faMarkMask = 0;

	if (optMaskAccess == 1) {
		if (FAN_ACCESS == 0) {
			lprintf (LOG_WARNING, "FAN_ACCESS is not available");
		}
		faMarkMask |= FAN_ACCESS;
	}
	if (optMaskModify == 1) {
		if (FAN_MODIFY == 0) {
			lprintf (LOG_WARNING, "FAN_MODIFY is not available");
		}
		faMarkMask |= FAN_MODIFY;
	}
	if (optMaskClose == 1) {
		if (FAN_CLOSE == 0) {
			lprintf (LOG_WARNING, "FAN_CLOSE is not available");
		}
		faMarkMask |= FAN_CLOSE;
	}
	if (optMaskCloseWrite == 1) {
		if (FAN_CLOSE_WRITE == 0) {
			lprintf (LOG_WARNING, "FAN_CLOSE_WRITE is not available");
		}
		faMarkMask |= FAN_CLOSE_WRITE;
	}
	if (optMaskCloseNowrite == 1) {
		if (FAN_CLOSE_NOWRITE == 0) {
			lprintf (LOG_WARNING, "FAN_CLOSE_NOWRITE is not available");
		}
		faMarkMask |= FAN_CLOSE_NOWRITE;
	}
	if (optMaskOpen == 1) {
		if (FAN_OPEN == 0) {
			lprintf (LOG_WARNING, "FAN_OPEN is not available");
		}
		faMarkMask |= FAN_OPEN;
	}
	if (optMaskOpenExec == 1) {
		if (FAN_OPEN_EXEC == 0) {
			lprintf (LOG_WARNING, "FAN_OPEN_EXEC is not available");
		}
		faMarkMask |= FAN_OPEN_EXEC;
	}
	if (optMaskAttrib == 1) {
		if (FAN_ATTRIB == 0) {
			lprintf (LOG_WARNING, "FAN_ATTRIB is not available");
		}
		faMarkMask |= FAN_ATTRIB;
	}
	if (optMaskCreate == 1) {
		if (FAN_CREATE == 0) {
			lprintf (LOG_WARNING, "FAN_CREATE is not available");
		}
		faMarkMask |= FAN_CREATE;
	}
	if (optMaskDelete == 1) {
		if (FAN_DELETE == 0) {
			lprintf (LOG_WARNING, "FAN_DELETE is not available");
		}
		faMarkMask |= FAN_DELETE;
	}
	if (optMaskDeleteSelf == 1) {
		if (FAN_DELETE_SELF == 0) {
			lprintf (LOG_WARNING, "FAN_DELETE_SELF is not available");
		}
		faMarkMask |= FAN_DELETE_SELF;
	}
	if (optMaskMove == 1) {
		if (FAN_MOVE == 0) {
			lprintf (LOG_WARNING, "FAN_MOVE is not available");
		}
		faMarkMask |= FAN_MOVE;
	}
	if (optMaskMovedFrom == 1) {
		if (FAN_MOVED_FROM == 0) {
			lprintf (LOG_WARNING, "FAN_MOVED_FROM is not available");
		}
		faMarkMask |= FAN_MOVED_FROM;
	}
	if (optMaskMovedTo == 1) {
		if (FAN_MOVED_TO == 0) {
			lprintf (LOG_WARNING, "FAN_MOVED_TO is not available");
		}
		faMarkMask |= FAN_MOVED_TO;
	}
	if (optMaskQOverflow == 1) {
		if (FAN_Q_OVERFLOW == 0) {
			lprintf (LOG_WARNING, "FAN_Q_OVERFLOW is not available");
		}
		faMarkMask |= FAN_Q_OVERFLOW;
	}
	if (optMaskOpenPerm == 1) {
		if (FAN_OPEN_PERM == 0) {
			lprintf (LOG_WARNING, "FAN_OPEN_PERM is not available");
		}
		faMarkMask |= FAN_OPEN_PERM;
	}
	if (optMaskOpenExecPerm == 1) {
		if (FAN_OPEN_EXEC_PERM == 0) {
			lprintf (LOG_WARNING, "FAN_OPEN_EXEC_PERM is not available");
		}
		faMarkMask |= FAN_OPEN_EXEC_PERM;
	}
	if (optMaskAccessPerm == 1) {
		if (FAN_ACCESS_PERM == 0) {
			lprintf (LOG_WARNING, "FAN_ACCESS_PERM is not available");
		}
		faMarkMask |= FAN_ACCESS_PERM;
	}
	if (optMaskOndir == 1) {
		if (FAN_ONDIR == 0) {
			lprintf (LOG_WARNING, "FAN_ONDIR is not available");
		}
		faMarkMask |= FAN_ONDIR;
	}
	if (optMaskEventOnChild == 1) {
		if (FAN_EVENT_ON_CHILD == 0) {
			lprintf (LOG_WARNING, "FAN_EVENT_ON_CHILD is not available");
		}
		faMarkMask |= FAN_EVENT_ON_CHILD;
	}

	return (faMarkMask);
}

int faInit () {
	int fanotify, pathi;

	// Init fanotify structure
	fanotify = fanotify_init (FAN_CLASS_NOTIF, O_RDONLY);
	if (fanotify == -1) {
		return (fanotify);
	}

	for (pathi = 0; optPaths [pathi] != NULL; pathi++) {
		if (fanotify_mark (fanotify, FAN_MARK_ADD | faBuildMarkFlags (), faBuildMarkMask (), AT_FDCWD, optPaths [pathi]) == -1) {
			return (-1);
		}
	}

	return (fanotify);
}

void faLoop (const int fanotify) {
	int buffer_len;
	char buffer [4096], mask [256], path [PATH_MAX + 1], exe [PATH_MAX + 1], cmdline [8192];
	struct fanotify_event_metadata* metadata;

	// Loop for(ever)
	for (;;) {
		// Read from the fanotify fd
		buffer_len = read (fanotify, buffer, sizeof (buffer));
		// Extract the metadata from what we just read
		metadata = (struct fanotify_event_metadata*) &buffer;

		// Process fanotify events
		while (FAN_EVENT_OK (metadata, buffer_len)) {
			// Overflow?
			if (metadata->mask & FAN_Q_OVERFLOW) {
				// Shit. Print a warning and move on
				lprintf (LOG_WARNING, "Queue overflow");
				continue;
			}

			// Get the name of the file by reading the link of the passed FD
			fileByFD (metadata->fd, path, sizeof (path));

			// Get the name of the process by reading the exe link for the passed PID
			exeByPID (metadata->pid, exe, sizeof (exe));

			// Get the command line arguments for the PID
			if (cmdlineByPID (metadata->pid, cmdline, sizeof (cmdline)) == -1) {
				lprintf (LOG_WARNING, "Could not read commandline for %d: %s", metadata->pid, strerror (errno));
			}

			// Build a textual version of the operations mask
			sprintmask (metadata->mask, mask);

			// Print out a line
			//printf ("%5d %s (%s = %016llX): %s\n", metadata->pid, exe, mask, metadata->mask, path);
			printf (
				"pid:%d exe:%s %s%s%sevents:[%s]: %s\n",
				metadata->pid,
				exe,
				(strlen (cmdline) > 0 ? "args:[" : ""),
				cmdline,
				(strlen (cmdline) > 0 ? "] " : ""),
				mask,
				path
			);

			// Close the FD
			close (metadata->fd);
			// Move on to the next event
			metadata = FAN_EVENT_NEXT (metadata, buffer_len);
		}
	}
}

// =============================================================================

void usage (int exit_status) {
	printf ("Usage: %s [OPTIONS] <PATH> [<PATH>...]\n", "fsnoop");
	printf ("\n");
	printf ("Options:\n");
	printf ("   --help             -h   This help\n");
	printf ("   --verbose          -v   Increased output verbosity\n");
	printf ("\n");
	printf ("   --cache            -C   Enable caching of PID:Path information so that closes\n");
	printf ("                           have path information (default)\n");
	printf ("   --no-cache         -c   Disable caching of PID:Path information\n");
	printf ("\n");
	printf ("Path flags:\n");
	printf ("   --dont-follow           Do not follow symlinks specified as paths\n");
	printf ("   --only-dir              Throw an error if a specified path is not a directory\n");
	printf ("   --mount                 If specified paths are not mount points, change them\n");
	printf ("                           to the mount points containing the specified path\n");
	printf ("   --filesystem            Add the filesystems containing the specified paths,\n");
	printf ("                           including all mount points for the filesystems\n");
	printf ("\n");
	printf ("Events:\n");
	printf ("   --create                File or directory created\n");
	printf ("   --open                  File or directory opened\n");
	printf ("   --open-exec             File opened with intent to execute\n");
	printf ("   --access                File or directory accessed (read)\n");
	printf ("   --modify                File modified\n");
	printf ("   --close                 File or directory closed\n");
	printf ("   --close-write           Writable file closed\n");
	printf ("   --close-no-write        Read-only file or directory closed\n");
	printf ("   --attrib                File or directory metadata modified\n");
	printf ("   --delete                File or directory deleted\n");
	printf ("   --delete-self           File or directory specified as path delete\n");
	printf ("   --move                  File or directory moved\n");
	printf ("   --moved-from            File or directory moved from a specified path\n");
	printf ("   --moved-to              File or directory moved into a specified path\n");
	printf ("   --ondir                 Apply events to directories as well as files\n");
	//printf ("\n");
	//printf ("   --open-perm             File or directory asking to be opened\n");
	//printf ("   --open-exec_perm        File asking to be opened with intent to execute\n");
	//printf ("   --access-perm           File or directory asking to be accessed (read)\n");
	printf ("\n");
	exit (exit_status);
}

int parseArgs (const int argc, const char **argv) {
	struct option long_opts [] = {
		// General options
		{"help", no_argument, 0, 'h'},
		{"verbose", no_argument, 0, 'v'},
		// Caching control
		{"cache", no_argument, &optCache, 1},
		{"no-cache", no_argument, &optCache, 0},
		// fanotify flags
		{"dont-follow", no_argument, &optFlagDontFollow, 1},
		{"only-dir", no_argument, &optFlagOnlydir, 1},
		{"mount", no_argument, &optFlagMount, 1},
		{"filesystem", no_argument, &optFlagFilesystem, 1},
		// fanotify masks
		{"access", no_argument, &optMaskAccess, 1},
		{"modify", no_argument, &optMaskModify, 1},
		{"close", no_argument, &optMaskClose, 1},
		{"close-write", no_argument, &optMaskCloseWrite, 1},
		{"close-no-write", no_argument, &optMaskCloseNowrite, 1},
		{"open", no_argument, &optMaskOpen, 1},
		{"open-exec", no_argument, &optMaskOpenExec, 1},
		{"attrib", no_argument, &optMaskAttrib, 1},
		{"create", no_argument, &optMaskCreate, 1},
		{"delete", no_argument, &optMaskDelete, 1},
		{"delete-self", no_argument, &optMaskDeleteSelf, 1},
		{"move", no_argument, &optMaskMove, 1},
		{"moved-from", no_argument, &optMaskMovedFrom, 1},
		{"moved-to", no_argument, &optMaskMovedTo, 1},
		{"q-overflow", no_argument, &optMaskQOverflow, 1},
		{"open-perm", no_argument, &optMaskOpenPerm, 1},
		{"open-exec_perm", no_argument, &optMaskOpenExecPerm, 1},
		{"access-perm", no_argument, &optMaskAccessPerm, 1},
		{"ondir", no_argument, &optMaskOndir, 1},
		{"event-on-child", no_argument, &optMaskEventOnChild, 1},

		//{"delete",  required_argument, 0, 'd'},
		{0, 0, 0, 0}
	};
	int opt, opt_index, path_index;

	while (1) {
		opt_index = 0;

		opt = getopt_long (argc, (char * const *) argv, "hvcC", long_opts, &opt_index);
		if (opt == -1) {
			break;
		}

		switch (opt) {
			case 0:
				/* If this option set a flag, do nothing else now. */
				if (long_opts [opt_index].flag != 0) {
					break;
				}
				//long_opts [opt_index].name
				//optarg
				break;

			case 'h':
				usage (0);
				break;

			case 'v':
				optVerbose++;
				break;

			case 'c':
				optCache = 0;
				break;

			case 'C':
				optCache = 1;
				break;

			//case 'c':
			//	printf ("option -c with value `%s'\n", optarg);
			//	break;

			case '?':
				break;

			default:
				usage (1);
				break;
		}
	}

	if (optind < argc) {
		optPaths = calloc (argc - optind + 1, sizeof (*optPaths));
		if (optPaths == NULL) {
			return (-1);
		}
		memset (optPaths, 0, (argc - optind + 1) * sizeof (*optPaths));

		for (path_index = 0; optind < argc; optind++, path_index++) {
			optPaths [path_index] = malloc (strlen (argv [optind]));
			if (optPaths [path_index] == NULL) {
				return (-1);
			}
			memset (optPaths [path_index], 0, strlen (argv [optind]) + 1);
			strcpy (optPaths [path_index], argv [optind]);
		}
	}

	return (0);
}

// =============================================================================

int main (const int argc, const char** argv) {
	int fanotify;

	if (parseArgs (argc, argv) == -1) {
		printf ("Error parsing arguments: %s\n", strerror (errno));
		return (1);
	}

	if (optCache) {
		if (pidCacheInit () == -1) {
			printf ("Error initializing cache: %s\n", strerror (errno));
			return (1);
		}
	}

	if ((fanotify = faInit ()) == -1) {
		printf ("Error configuring fanotify: %s\n", strerror (errno));
		return (1);
	}

	faLoop (fanotify);

	return (1);
}
