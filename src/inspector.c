#include <stdio.h>
#include <stdlib.h>
#include "memory.h"

void inspector_dump(const uint8_t *memory, size_t size) {
    printf("\nMEM INSPECTOR \n");

    for (size_t i = 0; i < size; i++) {
        uint8_t val = asm_read_byte(&memory[i]);

        printf("Addr: %p | Val: %u | Hex: 0x%02x\n",
                (const void *)&memory[i],
                (unsigned int) val,
                (unsigned int) val);
    }

    printf("------------"); 
}
