# Simple Smart Loader (Demand Paging ELF Loader)

## Overview

This project implements a **Simple Smart Loader** for 32-bit ELF executables. Unlike the basic ELF loader from Assignment 1, this loader **does not load executable segments into memory beforehand**. Instead, it uses **demand paging**, where pages are allocated only when they are accessed.

The loader installs a `SIGSEGV` handler to catch page faults. Upon a page fault, it determines whether the faulting address belongs to a valid loadable segment. If so, it allocates the required page, loads the appropriate data from the executable, and resumes execution transparently.

The implementation also reports statistics such as the total number of page faults, page allocations, and internal fragmentation.

---

## Features

- Loads 32-bit ELF executables.
- Implements demand paging using `SIGSEGV`.
- Allocates memory pages only when required.
- Loads executable pages lazily.
- Maps pages using `mmap()`.
- Tracks allocated pages for cleanup.
- Reports:
  - Total page faults
  - Total page allocations
  - Internal fragmentation
- Cleans up all allocated resources before exiting.

---

## Files

| File | Description |
|------|-------------|
| `loader.c` | Smart loader implementation |
| `loader.h` | Header file (provided) |
| `README.md` | Project documentation |

---

## Working

### 1. Open ELF File

The loader opens the executable in read-only mode and reads the ELF header.

Information extracted includes:

- Entry point
- Program header offset
- Number of program headers

---

### 2. Read Program Headers

The complete Program Header Table is loaded into memory.

Only segments of type `PT_LOAD` are considered valid executable segments.

---

### 3. Register SIGSEGV Handler

A custom page fault handler is installed using

```c
sigaction(SIGSEGV, ...)
```

Every invalid memory access generated while executing the ELF is intercepted by this handler.

---

### 4. Execute the Program

Instead of loading segments manually, execution jumps directly to the ELF entry point:

```c
_start();
```

Since no pages are mapped initially, the first memory access generates a page fault.

---

### 5. Demand Paging

Whenever a page fault occurs:

1. Obtain the faulting virtual address.
2. Identify the corresponding loadable segment.
3. Compute the page boundary.
4. Allocate one page using

```c
mmap()
```

with

- Read
- Write
- Execute

permissions.

5. Read only the required portion of the executable into the page.
6. Resume program execution.

Only pages that are actually accessed are loaded into memory.

---

### 6. Internal Fragmentation

For the final page of each segment, unused bytes remaining after the segment ends are counted as internal fragmentation.

The total fragmentation is accumulated and reported in kilobytes.

---

### 7. Cleanup

Before termination the loader

- unmaps every allocated page
- closes the executable
- frees allocated headers

---

## Demand Paging Workflow

```
Program Starts
       |
       v
Jump to Entry Point
       |
       v
SIGSEGV Generated
       |
       v
Locate Loadable Segment
       |
       v
Allocate Required Page
       |
       v
Load Data From ELF
       |
       v
Resume Execution
       |
       v
Repeat Until Program Finishes
```

---

## Compilation

Compile using

```bash
gcc loader.c -o loader
```

or use the provided Makefile if available.

---

## Running

```bash
./loader <ELF Executable>
```

Example

```bash
./loader fib
```

Sample Output

```text
55

--- SimpleSmartLoader Statistics ---
Total Page Faults: 5
Total Page Allocations: 5
Total Internal Fragmentation: 2.75 kilobytes
```

---

## System Calls Used

- `open()`
- `read()`
- `lseek()`
- `mmap()`
- `munmap()`
- `close()`
- `sigaction()`

---

## Data Structures

### ELF Header

Stores executable metadata including

- Entry point
- Program header offset
- Number of program headers

---

### Program Headers

Each `Elf32_Phdr` describes a loadable segment.

The loader checks:

- Virtual address
- File offset
- File size
- Memory size

---

### Page Tracking

An array stores all mapped pages so they can be unmapped during cleanup.

---

## Statistics Collected

### Total Page Faults

Number of page faults handled by the loader.

---

### Total Page Allocations

Number of pages allocated using `mmap()`.

---

### Internal Fragmentation

Unused memory within the final allocated page of each segment.

Reported in kilobytes.

---

## Error Handling

The loader detects and reports:

- Failure to open executable
- Invalid ELF without program headers
- Memory allocation failure
- `mmap()` failure
- `munmap()` failure
- `lseek()` failure
- Read failures
- Genuine segmentation faults outside any loadable segment

Invalid accesses outside executable segments terminate execution with an error message.

---

## Limitations

- Supports only **32-bit ELF** executables.
- Uses a fixed page size of **4096 bytes**.
- Supports only statically linked executables.
- No support for relocation or dynamic linking.
- Maximum of 10,000 mapped pages are tracked.
- Assumes Linux virtual memory behavior.

---

## Learning Outcomes

This assignment demonstrates several important Operating System concepts:

- Demand Paging
- Lazy Loading
- Virtual Memory Management
- Page Fault Handling
- Signal Handling using `SIGSEGV`
- Memory Mapping using `mmap()`
- ELF Executable Format
- Page Allocation and Deallocation
- Internal Fragmentation
- User-level implementation of a smart program loader