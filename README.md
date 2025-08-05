# BaseLayer

This is my base layer for C programming.

## Utilities

### OS Detection

- if `OS_WIN` macro is defined the OS is Windows.
- if `OS_LINUX` macro is defined the OS is Linux.
- if `OS_MAC` macro is defined the OS is MacOS.

### Compiler Detection

- if `CC_CALNG` macro is defined the compiler is clang.
- if `CC_GCC` macro is defined the compiler is gcc.
- if `CC_MSVC` macro is defined the compiler is msvc.

### Architecture Detection

- if `ARCH_X64` macro is defined the CPU is 64-bit x86 (amd64).
- if `ARCH_X86` macro is defined the CPU is 32-bit x86 (i386).
- if `ARCH_ARM64` macro is defined the CPU is 64-bit arm (aarch64).
- if `ARCH_ARM` macro is defined the CPU is 32-bit arm.

### Macros

- `Stringify` convers a token into a C string.
- `Glue` glues two tokens together.
- `Member` gets the struct member of nullptr.
- `MemberOffset` gets the offset of a struct member.
- `MemberSize` gets the size of a struct member.

## OS Wrapper

### Memory

- `OS_MemoryReserve` reserves address space.
- `OS_MemoryCommit` commits physical memory to the reserved address space.
- `OS_MemoryDecommit` decommits physical memory.
- `OS_MemoryRelease` releases reserved address space.

### Info

- `OS_GetExecutablePath` returnes a path to the current executable.

### Dynamic Linking

- `OS_LibraryLoad` loads a dynamic library and returnes a handle to it.
- `OS_LibraryGetFunction` gets a function pointer from the library.
- `OS_LibraryFree` unloads the dynamic library.

### Multithreading

- `OS_ThreadCreate` creates a thread with and starts it at a given function.
- `OS_ThreadJoin` waits for the thread to exit and returnes the exit code.

### Networking

- `OS_NetStartup` enables the networking API.
- `OS_NetCleanup` disables the networking API.
- `OS_NetAddressResolve` resolves the network address.
- `OS_NetOpenDatagramSocket` opens a UDP socket.
- `OS_NetOpenServer` opens a TCP server socket.
- `OS_NetConnect` opens a TCP client socket.
- `OS_NetClose` closes the socket.
- `OS_NetSend` sends data with TCP socket.
- `OS_NetSendTo` sends data with UDP socket to a given address.
- `OS_NetReceive` recieves data from TCP socket.
- `OS_NetReceiveFrom` recieves data from UDP socket and yields the address.
