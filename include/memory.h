#ifndef MEMORY_H
#define MEMORY_H

#include <stdint.h>

uint64_t asm_add(uint64_t a, uint64_t b);
uint8_t asm_read_byte(const uint8_t *address); 
void inspector_dump(const uint8_t *memory, size_t size);

#endif
