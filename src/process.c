#include "process.h"
#include <libproc.h>
#include <stdio.h>

kern_return_t read_process_memory(
    mach_port_t task,
    mach_vm_address_t remote_address,
    void *local_buffer,
    mach_vm_size_t size,
    mach_vm_size_t *bytes_read) {
    return mach_vm_read_overwrite(
        task,
        remote_address,
        size,
        (mach_vm_address_t) local_buffer,
        bytes_read);
}

int inspect_process(pid_t pid) {
    char name[256] = {0};
    char path[PROC_PIDPATHINFO_MAXSIZE] = {0};
    kern_return_t kern_return = 0;
    mach_port_t task = 0; 

    int name_len = proc_name(pid, name, sizeof(name));
    int path_len = proc_pidpath(pid, path, sizeof(path));

    if (name_len <= 0) {
        printf("[!] Retrieve process name error\n");
        return 1;
    }

    kern_return = task_for_pid(mach_task_self(), pid, &task);

    printf("\nProcess info\n");
    printf("PID: %d\n", pid);
    printf("Name: %s\n", name);
    if (kern_return != KERN_SUCCESS) {
        printf("task_for_pid failed : %s\n", mach_error_string( kern_return )); 
        return 1;
    }

    printf("Port Task: %u\n", task);

    if (path_len > 0)
        printf("Path: %s\n", path);

    memory_map(task);
    mach_port_deallocate(mach_task_self(), task);
    printf("-----"); 

    return 0;
}
