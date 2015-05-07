// gcc --static -o snoop snoop.c

#include <errno.h>
#include <fcntl.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/fanotify.h>
#include <sys/stat.h>

#include <unistd.h>

#include "resolve.h"
#include "cache.h"


// ============================================================================
unsigned char optCache = 0;
char* optFormat = "%p %e %a (%m): %P";
char* optPath = "/";
// ============================================================================

void usage (int code) {
	exit (code);
}

int main (int argc, char** argv) {
	int opt, argi;

	int fan, bufferLen;
	char buffer [4096], mask [256], path [PATH_MAX + 1], exe [PATH_MAX + 1], cmdline [8192];
	struct fanotify_event_metadata* metadata;

	// Process arguments
	opterr = 0;
	while ((opt = getopt (argc, argv, "hcf:")) != -1) {
		switch (opt) {
			case 'h':
				usage (0);
				break;
			case 'c':
				optCache = 1;
				break;
			case 'f':
				optFormat = optarg;
			case '?':
				switch (optopt) {
					case 'c':
						fprintf (stderr, "Error: Option -%c requires an argument. See usage (-h) for more info\n", optopt);
						return (1);
						break;
					default:
						fprintf (stderr, "Warning: Ignoring unknown option -%c\n", optopt);
						break;
				}
			default:
				fprintf (stderr, "Warning: Ignoring unknown option -%c\n", opt);
				break;
		}
	}
	for (argi = optind; argi < argc; argi++) {
		optPath = argv [argi];
		break;
	}

	// Are we using the cache?
	if (optCache) {
		// Init the PID cache
		pidCacheInit ();
	}

	// Init fanotify structure
	fan = fanotify_init (FAN_CLASS_NOTIF, O_RDONLY);

	// Instruct fanotify to watch for events, but be mindful if it fails
	if (fanotify_mark (
		fan,
		FAN_MARK_ADD | FAN_MARK_MOUNT,
		FAN_ACCESS | FAN_OPEN | FAN_CLOSE_WRITE | FAN_CLOSE_NOWRITE | FAN_ONDIR | FAN_MODIFY, // | FAN_OPEN_PERM | FAN_ACCESS_PERM, // | FAN_EVENT_ON_CHILD,
		-1,
		optPath
	) == -1) {
		// Well, that didn't work
		fprintf (stderr, "Error: Failed to listen for events on %s: %s\n", optPath, strerror (errno));
		// All done
		return (1);
	}

	// Loop for(ever)
	for (;;) {
		// Read from the fanotify fd
		bufferLen = read (fan, buffer, sizeof (buffer));
		// Extract the metadata from what we just read
		metadata = (struct fanotify_event_metadata*) &buffer;

		// Process fanotify events
		while (FAN_EVENT_OK (metadata, bufferLen)) {
			// Overflow?
			if (metadata->mask & FAN_Q_OVERFLOW) {
				// Shit. Print a warning and move on
				printf ("Warning: Queue overflow\n");
				continue;
			}

			// Get the name of the file by reading the link of the passed FD
			fileByFD (metadata->fd, path, sizeof (path));

			// Get the name of the process by reading the exe link for the passed PID
			exeByPID (metadata->pid, exe, sizeof (exe));

			// Get the command line arguments for the PID
			cmdlineByPID (metadata->pid, cmdline, sizeof (cmdline));

			// Build a textual version of the operations mask
			sprintmask (metadata->mask, mask);

			// Print out a line
			//printf ("%5d %s (%s = %016llX): %s\n", metadata->pid, exe, mask, metadata->mask, path);
			printf ("%5d %s %s%s(%s): %s\n", metadata->pid, exe, cmdline, (strlen (cmdline) > 0 ? " " : ""), mask, path);

			// Close the FD
			close (metadata->fd);
			// Move on to the next event
			metadata = FAN_EVENT_NEXT (metadata, bufferLen);
		}
	}
}
