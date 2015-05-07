#ifndef __RESOLVE_H
#define __RESOLVE_H

ssize_t sprintmask (const unsigned long long mask, char* dst);
void fileByFD (int fd, char* dst, ssize_t len);
void exeByPID (int pid, char* dst, ssize_t len);
void cmdlineByPID (int pid, char* dst, ssize_t len);

#endif
