#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include "memory.h"
#include "process.h"
#include "utils.h"

#define MAX_PATTERN 4096

static size_t validate_pattern(const char *arg) {
    if (!arg)
        return 1;

    size_t len = strlen(arg);

    if (len == 0) {
        fprintf(stderr, "[!] Search pattern must not be empty!\n");
        return 1;
    }

    if (len > MAX_PATTERN) {
        fprintf(stderr,"[!] Search pattern too long! (max %d bytes)", MAX_PATTERN);
        return 1;
    }

    return len;
}

int main(int argc, char **argv) {

    if (argc != 2) {
        fprintf(stderr, "Usage: %s <pid/binary>\n", argv[0]);
        return 1;
    }

    if (argc == 4 && strcmp(argv[2], "-s") == 0) {
        size_t len = validate_pattern(argv[3]);

        if (len == 0)
            return 1;

        //memory_search();
    }

    if (is_valid_pid(argv[1])) {
        pid_t target = (pid_t) strtol(argv[1], NULL, 10);
        return inspect_process(target);
    } else if (access(argv[1], X_OK) == 0) {
       return inspect_symbols(argv[1]); 
    } else {
        fprintf(stderr, "%s: neither a running pid nor an executable file\n",
                argv[1]);
        return 1;
    }

    return 0;
}
