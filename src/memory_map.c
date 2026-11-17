#include <stdio.h> 
#include <mach/mach.h>
#include <mach/mach_vm.h>

void memory_map(mach_port_t task) {
    mach_vm_address_t address = 0;

    printf("\n --- MEMORY MAP ---\n"); 
    printf("%-18s %-12s %-15s\n", "START ADDRESS", "SIZE", "PROTECTION");

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

        printf("0x%016llx  %-12llu ",
            (unsigned long long)address,
            (unsigned long long)size);

        if (info.protection & VM_PROT_READ)
            printf("R");

        if (info.protection & VM_PROT_WRITE)
            printf("W");

        if (info.protection & VM_PROT_EXECUTE)
            printf("X");

        printf("\n");

        address += size;
    }
}
