# Simple ELF Loader

## Overview

This project implements a simple ELF (Executable and Linkable Format) loader in C. The loader reads a 32-bit ELF executable, identifies the loadable segment containing the program entry point, maps it into memory using `mmap()`, transfers control to the loaded program, prints its return value, and performs cleanup.

The implementation demonstrates the basic principles of executable loading without relying on the operating system's program loader.

---

## Features

- Opens and validates an ELF executable.
- Reads the ELF header (`Elf32_Ehdr`).
- Reads the Program Header Table (`Elf32_Phdr`).
- Searches for the `PT_LOAD` segment containing the entry point.
- Allocates executable memory using `mmap()`.
- Copies the loadable segment into memory.
- Computes the offset of the entry point.
- Executes the loaded program by invoking its `_start` function.
- Prints the return value.
- Frees allocated resources and closes file descriptors.

---

## Files

| File | Description |
|------|-------------|
| `loader.c` | Implementation of the ELF loader |
| `loader.h` | Header file containing declarations and required includes |
| `Makefile` | Builds the loader executable |
| `README.md` | Documentation |

---

## Working

### 1. Open ELF File

The loader opens the ELF executable in read-only mode.

```c
fd = open(argv[1], O_RDONLY);
```

---

### 2. Read ELF Header

The ELF header is read into memory to obtain:

- Entry point address
- Program header offset
- Number of program headers
- Program header size

---

### 3. Read Program Headers

The loader iterates through each program header and checks:

- Whether it is of type `PT_LOAD`
- Whether the entry point lies inside the segment

```text
entry >= p_vaddr
entry < p_vaddr + p_memsz
```

---

### 4. Allocate Memory

Once the correct loadable segment is found, memory is allocated using:

```c
mmap()
```

with permissions:

- Read
- Write
- Execute

---

### 5. Load Segment

The loader seeks to the segment offset in the ELF file and copies the contents into the mapped memory.

---

### 6. Locate Entry Point

The offset inside the mapped segment is computed as

```
offset = entry - p_vaddr
```

The entry point address becomes

```
mapped_memory + offset
```

which is typecast into a function pointer.

---

### 7. Execute Program

The loader transfers execution by calling

```c
_start();
```

and prints the returned value.

---

### 8. Cleanup

Finally,

- allocated memory structures are freed
- file descriptor is closed

---

## Compilation

Build using:

```bash
make
```

or

```bash
gcc loader.c -o loader
```

depending on the provided Makefile.

---

## Running

```bash
./loader <ELF Executable>
```

Example:

```bash
./loader fib
```

Output:

```text
User _start return value = 55
```

---

## Error Handling

The loader checks for the following conditions:

- Failure to open ELF file
- Memory allocation failure
- Read failures
- Invalid ELF with no program headers
- Segment where `p_memsz < p_filesz`
- Entry point not inside any loadable segment
- `mmap()` failure
- `lseek()` failure

If any of these occur, an appropriate error message is printed and execution terminates.

---

## System Calls Used

- `open()`
- `read()`
- `lseek()`
- `mmap()`
- `close()`

---

## Memory Management

Allocated resources:

- ELF Header (`malloc`)
- Program Header (`malloc`)
- File descriptor (`open`)

Released by:

- `free()`
- `close()`

---

## Limitations

- Supports only **32-bit ELF** executables.
- Loads only the segment containing the entry point.
- Does not implement demand paging.
- Does not perform relocation or dynamic linking.
- Assumes a statically linked executable.

---

## Learning Outcomes

Through this assignment, the following concepts are demonstrated:

- ELF executable structure
- ELF headers and program headers
- Process memory mapping
- Executable loading
- Function pointers
- Memory management using `mmap`
- Low-level Linux system calls
