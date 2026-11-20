#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include "memory.h"
#include "process.h"
#include "utils.h"

int main(int argc, char **argv) {
    if (argc != 2) {
        fprintf(stderr, "Usage: %s <pid/binary>\n", argv[0]);
        return 1;
    }

    if (is_valid_pid(argv[1])) {
        pid_t target = (pid_t) strtol(argv[1], NULL, 10);

        inspect_process(target);
    } else if (access(argv[1], X_OK) == 0) {
       inspect_symbols(argv[1]); 
    } else {
        fprintf(stderr, "%s: neither a running pid nor an executable file\n",
                argv[1]);
        return 1;
    }

    // TESTS 
    // uint64_t res = asm_add(10,20);
    // printf("ASM Addition : %llu", (unsigned long long) res);
    /*
        size_t size = 16;
        uint8_t *memory_zone = malloc(16);
       for (size_t i = 0; i < size; i++) {
        memory_zone[i] = (uint8_t)(i + 65); 
        } 
        inspector_dump(memory_zone, size);
        free(memory_zone);
    */ 

    return 0;
}
