#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include "memory.h"
#include "process.h"

int main(int argc, char **argv) {
    if (argc != 2) {
        fprintf(stderr, "Usage: %s <pid>\n", argv[0]);
        return 1;
    }
    pid_t target = (pid_t) strtol(argv[1], NULL, 10);

    uint64_t res = asm_add(10,20);
    printf("ASM Addition : %llu", (unsigned long long) res);
    
    size_t size = 16;
    uint8_t *memory_zone = malloc(16);

    if (memory_zone == NULL)
        return 1;

    for (size_t i = 0; i < size; i++) {
        memory_zone[i] = (uint8_t)(i + 65); 
    }
    
    inspector_dump(memory_zone, size);
    inspect_process(target);

    free(memory_zone);

    return 0;
}
