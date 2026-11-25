#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

#include <mach-o/loader.h>
#include <mach-o/nlist.h>

int inspect_symbols(const char *file) {
    FILE *fp = fopen(file, "rb");
    if (!fp) {
        perror("fopen");
        return 1;
    }

    // get file size & go to start of file 
    if (fseek(fp, 0, SEEK_END) != 0) {
        perror("fseek");
        fclose(fp);
        return 1;
    }

    long end = ftell(fp);
    if (end < 0) {
        perror("ftell");
        fclose(fp);
        return 1;
    }

    size_t file_size = (size_t)end;
    rewind(fp);

    uint8_t *buffer = malloc(file_size);

    if (!buffer || fread(buffer, 1, file_size, fp) != file_size) {
        fprintf(stderr, "failed to read binary\n");
        free(buffer);
        fclose(fp);
        return 1;
    } 

    fclose(fp); 

    
    // check that the file has enough size to contains a macho header
    if (file_size < sizeof(struct mach_header_64)) {
        fprintf(stderr, "invalid macho");
        free(buffer);
        return 1;
    }

    struct mach_header_64 *header = (struct mach_header_64 *)buffer;

    // check file signature
    // MH_MAGIC_64 identifies native MACHO 64 bits 
    if (header->magic != MH_MAGIC_64) {
        fprintf(stderr, "unsupported macho format");
        free(buffer);
        return 1;
    }

    // load commands starts immediatly after the ader
    uint8_t *ptr = buffer + sizeof(struct mach_header_64); 

    // this struct will contain location of the symbols table if LC_SYMTAB is found
    struct symtab_command *symtab = NULL;

    // go through load commands, ncmds indicates if commands are presents
    for (uint32_t i = 0; i < header->ncmds; i++) {
        struct load_command *cmd = (struct load_command *) ptr;

        if (cmd->cmdsize < sizeof(struct load_command) || cmd->cmdsize > (size_t)(buffer+file_size - ptr)) {
            fprintf(stderr, "[!] Invalid load commands\n"); 
            free(buffer);
            return 1;
        }

        // LC_SYMTAB indicates
        // - table of symbols
        // - names of symbols
        if (cmd->cmd == LC_SYMTAB && cmd->cmdsize >= sizeof(struct symtab_command)) {
            symtab = (struct symtab_command *)cmd;
            break;
        }

        ptr += cmd->cmdsize;
    }

    if (!symtab) {
        fprintf(stderr, "no symbol table found\n");
        free(buffer);
        return 1;
    }

    // check for offsets & size before accessing mem
    if (symtab->symoff > file_size || symtab->stroff > file_size || symtab->nsyms > (file_size - symtab->symoff) / sizeof(struct nlist_64) ||
            symtab->strsize > file_size - symtab->stroff) {
        fprintf(stderr, "[!] Invalid symbol table offsets\n");
        free(buffer);
        return 1;
    }

    // getting table of symbols 
    struct nlist_64 *symbols = (struct nlist_64 *)(buffer+symtab->symoff);

    char *strings = (char *)(buffer + symtab->stroff);

    // iterates over all entries of symtab 
    for (uint32_t i = 0; i < symtab->nsyms; i++) {
        struct nlist_64 *sym = &symbols[i];


        // N_STAB -> debugging enties
        // ignoring
        if ((sym->n_type & N_STAB) != 0) {
            continue;
        }

        // N_TYPE extract type of symbol
        // N_UNDF tells that symbol is undefined
        if ((sym->n_type & N_TYPE) != N_UNDF) {
            continue;
        }

        if (!(sym->n_type & N_EXT))
            continue;

        uint32_t index = sym->n_un.n_strx;

        if (index == 0 || index >= symtab->strsize)
            continue;

        // Vérification qu'une terminaison '\0' existe
        // avant la fin de la String Table.
        if (!memchr(strings + index, '\0',
                    symtab->strsize - index))
            continue;

        // Affichage du nom du symbole.
        printf("%s\n", strings + index);
    }

    free(buffer);
    return 0;

} 
