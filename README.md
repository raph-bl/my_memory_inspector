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
./memory_inspector <pid>
```
