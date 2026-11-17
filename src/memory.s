.global _asm_add
.global _asm_read_byte
.align 2

_asm_add:
    add x0, x0, x1
    ret

// uint8_t asm_read_byte(const uint8_t *address)
//
// X0 = represents the address, [x0] represents the content of the address
// W0 = read and stock byte
// LDRB = sys call to read byte @ given address
// ret returns W0

_asm_read_byte:
    ldrb w0, [x0]
    ret
