#include <stdio.h>
#include <ctype.h>
#include <mach/mach.h>
#include <mach/mach_vm.h>
#include "process.h"

#define PREVIEW 16

static void print_bytes(mach_port_t task, mach_vm_address_t address) {
    uint8_t buf[PREVIEW] = {0};
    mach_vm_size_t read = 0;

    kern_return_t kr = read_process_memory(
        task, address, buf, PREVIEW, &read);

    if (kr != KERN_SUCCESS) {
        printf("  <lecture impossible>");
        return;
    }

    printf("  ");
    for (mach_vm_size_t i = 0; i < read; i++)
        printf("%02x ", buf[i]);

    printf(" |");
    for (mach_vm_size_t i = 0; i < read; i++)
        printf("%c", isprint(buf[i]) ? buf[i] : '.');
    printf("|");
}

void memory_map(mach_port_t task) {
    mach_vm_address_t address = 0;

    printf("\n --- MEMORY MAP ---\n");
    printf("%-18s %-12s %-6s %s\n",
        "START ADDRESS", "SIZE", "PROT", "BYTES");

    while (1) {
        mach_vm_size_t size = 0;

        vm_region_basic_info_data_64_t info;
        mach_msg_type_number_t count = VM_REGION_BASIC_INFO_COUNT_64;

        mach_port_t object_name = MACH_PORT_NULL;

        kern_return_t kr = mach_vm_region(
            task,
            &address,
            &size,
            VM_REGION_BASIC_INFO_64,
            (vm_region_info_t) &info,
            &count,
            &object_name
        );

        if (kr != KERN_SUCCESS)
            break;

        char prot[4];
        prot[0] = (info.protection & VM_PROT_READ)    ? 'r' : '-';
        prot[1] = (info.protection & VM_PROT_WRITE)   ? 'w' : '-';
        prot[2] = (info.protection & VM_PROT_EXECUTE) ? 'x' : '-';
        prot[3] = '\0';

        printf("0x%016llx  %-12llu %-6s",
            (unsigned long long)address,
            (unsigned long long)size,
            prot);

        if (info.protection & VM_PROT_READ)
            print_bytes(task, address);

        printf("\n");

        address += size;
    }
}
