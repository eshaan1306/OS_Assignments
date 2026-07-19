# Operating Systems Laboratory Assignments

## Overview

This repository contains the implementations of five programming assignments completed as part of the **Operating Systems Laboratory**. The assignments progressively build fundamental operating system concepts including executable loading, shell implementation, process scheduling, virtual memory management, and multithreaded programming.

The work is implemented primarily in **C** and **C++** using Linux system calls and POSIX APIs.

---

## Assignment 1 – Simple ELF Loader

### Objective

Implement a basic ELF loader capable of loading and executing a 32-bit ELF executable without relying on the operating system's program loader.

### Key Features

- Read ELF headers and program headers
- Identify the loadable segment containing the entry point
- Allocate executable memory using `mmap()`
- Load executable segments into memory
- Execute the `_start` function
- Clean up allocated resources

### Concepts Covered

- ELF file format
- Program headers
- Virtual memory
- Memory mapping
- Function pointers
- Linux system calls

---

## Assignment 2 – Mini Unix Shell

### Objective

Develop a basic Unix shell capable of executing Linux commands and pipelines while maintaining execution history.

### Key Features

- Execute commands using `fork()` and `execvp()`
- Support command pipelines
- Built-in `history` command
- Track command execution statistics
- Signal handling using `SIGINT`
- Display execution history before termination

### Concepts Covered

- Process creation
- Parent-child synchronization
- Pipes
- File descriptor redirection
- Signal handling
- Command parsing

---

## Assignment 3 – Round Robin Job Scheduler

### Objective

Extend the shell from Assignment 2 by implementing a Round Robin scheduler for submitted jobs.

### Key Features

- Built-in `submit` command
- Separate scheduler process
- Round Robin scheduling
- Configurable number of CPUs
- Configurable time slice
- Ready queue implementation
- Process suspension and resumption using signals
- Scheduler summary on termination

### Concepts Covered

- CPU scheduling
- Ready queues
- Inter-process communication
- Pipes
- Process control
- Zombie process handling
- UNIX signals

---

## Assignment 4 – Simple Smart Loader

### Objective

Implement a demand-paging ELF loader that loads executable pages only when they are accessed.

### Key Features

- Lazy loading of executable pages
- Page fault handling using `SIGSEGV`
- Demand paging using `mmap()`
- Automatic page allocation
- Internal fragmentation calculation
- Page fault statistics
- Memory cleanup using `munmap()`

### Concepts Covered

- Demand paging
- Virtual memory
- Page faults
- Signal handling
- Memory mapping
- ELF executable format

---

## Assignment 5 – Simple Multithreader

### Objective

Develop a lightweight multithreading library supporting generic parallel loops using POSIX threads.

### Key Features

- Generic 1D `parallel_for`
- Generic 2D `parallel_for`
- Static workload partitioning
- POSIX thread creation
- Execution time measurement
- Parallel vector addition
- Parallel matrix multiplication

### Concepts Covered

- POSIX Threads (Pthreads)
- Parallel programming
- Thread synchronization
- Workload distribution
- Performance measurement
- Reusable multithreading library design

---

## Technologies Used

- C
- C++
- POSIX Threads (Pthreads)
- Linux System Calls
- ELF (Executable and Linkable Format)
- POSIX Signals
- Memory Mapping (`mmap`)
- Process Control APIs

---

## Linux System Calls and APIs Used

- `fork()`
- `execvp()`
- `execlp()`
- `wait()`
- `waitpid()`
- `pipe()`
- `dup2()`
- `kill()`
- `sigaction()`
- `mmap()`
- `munmap()`
- `open()`
- `read()`
- `write()`
- `lseek()`
- `close()`
- `pthread_create()`
- `pthread_join()`

---

## Concepts Learned

Throughout these assignments, the following Operating System concepts were explored:

- ELF executable loading
- Memory management
- Virtual memory
- Demand paging
- Page fault handling
- Shell implementation
- Process creation and execution
- Process scheduling
- Round Robin scheduling
- Inter-process communication
- Signal handling
- Pipes and file descriptor management
- Zombie process cleanup
- POSIX multithreading
- Parallel programming
- Performance measurement

---

## Repository Structure

```
Assignment-1/
├── loader.c
├── loader.h
└── README.md

Assignment-2/
├── shell.c
└── README.md

Assignment-3/
├── shell.c
├── scheduler.c
├── dummy_main.h
└── README.md

Assignment-4/
├── loader.c
├── loader.h
└── README.md

Assignment-5/
├── simple-multithreader.h
├── vector.cpp
├── matrix.cpp
└── README.md
```

---

## Summary

These assignments collectively demonstrate the implementation of several core Operating Systems concepts, beginning with executable loading and process management, progressing through shell and scheduler development, extending into virtual memory with demand paging, and concluding with parallel programming using POSIX threads. Together, they provide practical experience with low-level Linux programming and reinforce the relationship between operating system abstractions and their underlying implementations.