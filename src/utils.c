#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <errno.h>
#include <limits.h>

int is_valid_pid(const char *arg) {
    char *end;
    errno = 0;

    long pid = strtol(arg, &end, 10);

    if (errno = 0 || *end != '\0' || end == arg)
        return 0;

    if (pid <= 0 || pid > INT_MAX)
        return 0;

    if(kill((pid_t)pid, 0) == 0) 
        return 1;

    return errno == EPERM;
}

