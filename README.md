# my_memory_inspector 

PoC for a memory inspector on macOS. Uses the built-in Mach kernel APIs to acquire a send right to a target PID's task_port (`task_for_pid`), then reads a copy of its virtual memory (`mach_vm_read_overwrite`) and walks its regions (`mach_vm_region`).

> [!WARNING]  
On modern macOS (Apple Silicon, SIP enabled), `task_for_pid` only succeeds if the target process is signed with the `com.apple.security.get-task-allow`
entitlement and runs under the same UID (or the caller is root). System/platform binaries (e.g. top) can never be inspected while SIP is on. A self-signed test target is provided via make victim.

## Installation & Usage

### Requirements

- macOS on Apple Silicon (arm64)
- Xcode Command Line Tools (`clang`, `codesign`) — install with `xcode-select --install`

### Build

```sh
make            # builds ./memory_inspector
make victim     # builds & self-signs ./victim/victim (test target, see below)
```

> [!TIP]
Victim will print its own PID and tick every 1 second. 

### Run

```sh
./memory_inspector <pid | binary>
```

The argument is treated as a PID if it is a strictly positive integer matching a running process, otherwise as a path to an executable file.

#### Process mode (`<pid>`)

Prints the process name, path and task port, then walks its memory map: start address, size, protection (`rwx`) and a 16-byte hex/ASCII preview of every readable region.

```sh
./victim/victim &
./memory_inspector <victim_pid>
```

#### Binary mode (`<binary>`)

Parses the Mach-O file on disk (no process needed, no entitlement required) and lists its imported symbols, i.e. the external undefined entries of the `LC_SYMTAB` symbol table.

```sh
$ ./memory_inspector victim/victim
___stdoutp
_fflush
_getpid
_printf
_sleep
```

> [!NOTE]
Only thin 64-bit Mach-O files (`MH_MAGIC_64`) are supported. Universal (fat) binaries, such as most system binaries in `/bin`, are rejected with `unsupported macho format`.
