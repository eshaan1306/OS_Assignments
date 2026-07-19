# Round Robin Job Scheduler Shell

## Overview

This project extends the Mini Unix Shell developed in Assignment 2 by introducing a **Round Robin Process Scheduler**. The shell allows users to submit executable jobs, while a separate scheduler process manages their execution using **time slicing** and a configurable number of CPUs.

The scheduler and shell communicate through a pipe, enabling asynchronous job submission and scheduling.

---

## Features

- Interactive Unix-like shell.
- Execute normal Linux commands.
- Support command pipelines.
- Maintain command history.
- Built-in `submit` command for scheduling jobs.
- Separate scheduler process.
- Round Robin scheduling algorithm.
- Configurable:
  - Number of CPUs (`NCPU`)
  - Time slice (`TSLICE`)
- Inter-process communication using pipes.
- Process suspension and resumption using signals.
- Graceful shutdown displaying scheduling statistics.

---

## Files

| File | Description |
|------|-------------|
| `shell.c` | Interactive shell with job submission support |
| `scheduler.c` | Round Robin scheduler implementation |
| `dummy_main.h` | Wrapper header used for scheduling submitted programs |
| `README.md` | Project documentation |

---

## Architecture

```
                +----------------+
                |     Shell      |
                +----------------+
                        |
             submit <program>
                        |
                  Pipe Communication
                        |
                        v
              +--------------------+
              |   Scheduler Process |
              +--------------------+
                        |
             Ready Queue (Round Robin)
                        |
          SIGSTOP / SIGCONT Scheduling
                        |
                  Executable Jobs
```

---

## Working

### 1. Starting the Shell

The shell is launched with

```bash
./shell <NCPU> <TSLICE_ms>
```

Example

```bash
./shell 2 100
```

where

- `NCPU` = number of jobs that may run simultaneously
- `TSLICE` = scheduling quantum in milliseconds

---

### 2. Scheduler Creation

At startup the shell

- creates a pipe
- forks a scheduler process
- passes
  - number of CPUs
  - time slice
  - pipe descriptor

to the scheduler using `execlp()`.

---

### 3. Job Submission

Jobs are submitted using

```bash
submit ./program
```

The shell

- forks a child process
- executes the requested program
- writes the child PID to the scheduler through the pipe
- stores job information for statistics

---

### 4. Ready Queue

The scheduler maintains a FIFO ready queue implemented as a linked list.

Each submitted job is inserted into the queue until scheduled.

---

### 5. Round Robin Scheduling

At every scheduling interval:

1. Running jobs receive `SIGSTOP`.
2. They are placed back into the ready queue.
3. Newly submitted jobs are added.
4. Up to `NCPU` jobs are selected.
5. Selected jobs receive `SIGCONT`.

This cycle repeats until the shell terminates.

---

### 6. Process Control

Scheduling uses UNIX signals:

| Signal | Purpose |
|---------|---------|
| `SIGSTOP` | Suspend running process |
| `SIGCONT` | Resume scheduled process |
| `SIGTERM` | Terminate scheduler |
| `SIGINT` | Graceful shutdown |

---

### 7. Zombie Cleanup

The shell periodically calls

```c
waitpid(..., WNOHANG)
```

to reap completed child processes and prevent zombie processes.

---

### 8. Scheduler Summary

When the user presses

```text
Ctrl+C
```

the shell

- waits for all submitted jobs to complete
- terminates the scheduler
- prints a summary table containing

- Job Name
- PID
- Completion Time
- Waiting Time

---

## Queue Implementation

The scheduler uses a linked-list queue.

Supported operations:

- Create Queue
- Enqueue
- Dequeue
- Free Queue

The queue stores process IDs representing runnable jobs.

---

## Inter-Process Communication

The shell and scheduler communicate through an anonymous pipe.

```
Shell
   |
write(pid)
   |
Pipe
   |
read(pid)
   |
Scheduler
```

Every submitted job's PID is written into the pipe and inserted into the scheduler's ready queue.

---

## Compilation

Compile the scheduler

```bash
gcc scheduler.c -o scheduler
```

Compile the shell

```bash
gcc shell.c -o shell
```

Compile user programs (example)

```bash
gcc fib.c -o fib
```

---

## Running

Start the shell

```bash
./shell 2 100
```

Submit jobs

```bash
submit ./fib
submit ./matrix
submit ./sort
```

Run normal commands

```bash
ls
pwd
history
```

Exit

```text
Ctrl+C
```

---

## System Calls Used

- `fork()`
- `execvp()`
- `execlp()`
- `wait()`
- `waitpid()`
- `pipe()`
- `dup2()`
- `kill()`
- `sigaction()`
- `read()`
- `write()`
- `fcntl()`
- `usleep()`

---

## Built-in Commands

### submit

Submits an executable to the scheduler.

Example

```bash
submit ./fib
```

---

### history

Displays all commands entered during the current shell session.

Example

```bash
history
```

---

## Scheduling Algorithm

The scheduler implements **Round Robin Scheduling**.

Algorithm:

1. Receive new jobs.
2. Insert into ready queue.
3. Run up to `NCPU` jobs.
4. Allow execution for one time slice.
5. Stop running jobs.
6. Reinsert unfinished jobs.
7. Repeat.

---

## Data Structures

### Ready Queue

Implemented using a linked list.

Stores runnable process IDs.

---

### JobInfo

Stores

- Job name
- Process ID
- Completion status
- Completion time
- Waiting time

---

### HistoryEntry

Stores shell command history including

- Command
- PID
- Start time
- Execution duration

---

## Limitations

- Maximum of 100 submitted jobs.
- Maximum command length is 1024 characters.
- Supports only Round Robin scheduling.
- Waiting time is maintained in a simplified form.
- No process priorities.
- No dynamic CPU allocation.
- No support for background jobs (`&`).
- No I/O redirection (`<`, `>`, `>>`).
- History is maintained only during the current session.

---

## Learning Outcomes

This assignment demonstrates several important Operating System concepts:

- Round Robin Scheduling
- Ready Queue management
- Process scheduling
- Process control using signals
- Process suspension and resumption
- Inter-process communication using pipes
- Zombie process handling
- Shell implementation
- Concurrent process management
- UNIX system programming