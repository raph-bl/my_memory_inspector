#ifndef PROCESS_H
#define PROCESS_H

#include <sys/types.h>
#include <mach/mach.h>
#include <mach/mach_vm.h>

kern_return_t read_process_memory(
        mach_port_t task,
        mach_vm_address_t remote_address,
        void *local_buffer,
        mach_vm_size_t size,
        mach_vm_size_t *bytes_read
);

void inspect_process(pid_t);
void memory_map(mach_port_t task);

#endif
