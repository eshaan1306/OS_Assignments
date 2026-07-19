# Mini Unix Shell

## Overview

This project implements a simple Unix-like command shell in C. The shell accepts user commands, creates child processes to execute them, supports command pipelines using pipes, maintains command history, and records execution details such as process ID, start time, and execution duration.

The shell also handles the `SIGINT` signal (`Ctrl+C`) to display the execution history before terminating.

---

## Features

- Execute standard Linux commands.
- Create child processes using `fork()`.
- Execute commands using `execvp()`.
- Wait for child processes using `wait()`.
- Support multiple piped commands.
- Maintain command history.
- Record:
  - Command executed
  - Process ID (PID)
  - Start time
  - Execution duration
- Handle `Ctrl+C` gracefully by displaying execution history.
- Built-in `history` command.

---

## Files

| File | Description |
|------|-------------|
| `shell.c` | Main implementation of the shell |
| `README.md` | Project documentation |

---

## Working

### 1. Interactive Shell

The shell repeatedly prompts the user for a command:

```text
Enter your command:~$
```

Commands are read using `fgets()`.

---

### 2. Command Parsing

Commands are tokenized into arguments using `strtok()`.

Example:

```bash
ls -l
```

becomes

```text
args[0] = "ls"
args[1] = "-l"
args[2] = NULL
```

---

### 3. Command Execution

For normal commands, the shell:

1. Creates a child process using `fork()`.
2. Executes the command using `execvp()`.
3. Parent waits until execution completes.

---

### 4. Pipe Support

Commands containing the `|` operator are executed as a pipeline.

Example:

```bash
cat file.txt | grep hello | wc -l
```

For each pipe:

- A pipe is created using `pipe()`.
- Child processes redirect input/output using `dup2()`.
- Commands are executed with `execvp()`.

---

### 5. Command History

The shell stores every entered command.

The built-in command

```bash
history
```

prints all previously entered commands.

---

### 6. Execution Statistics

For every executed command, the shell stores:

- Command string
- Process ID
- Start time
- Execution duration

Timing is measured using `gettimeofday()`.

---

### 7. Signal Handling

The shell registers a handler for `SIGINT`.

When the user presses

```text
Ctrl+C
```

the shell prints execution history in the format

```text
PID
Command
Start Time
Execution Duration
```

before exiting.

---

## Compilation

Compile using

```bash
gcc shell.c -o shell
```

or use the provided Makefile if available.

---

## Running

```bash
./shell
```

Example session:

```text
Enter your command:~$ ls
Enter your command:~$ pwd
Enter your command:~$ history
Command 1: ls
Command 2: pwd
Command 3: history
```

Example pipeline:

```text
Enter your command:~$ cat input.txt | grep hello | wc -l
```

---

## System Calls Used

- `fork()`
- `execvp()`
- `wait()`
- `pipe()`
- `dup2()`
- `close()`
- `gettimeofday()`
- `sigaction()`

---

## Built-in Commands

### history

Displays all commands entered during the current shell session.

Example:

```bash
history
```

---

## Signal Support

### Ctrl+C (`SIGINT`)

Pressing `Ctrl+C` causes the shell to display the execution history including:

- PID
- Command
- Start time
- Execution time

before terminating.

---

## Data Structures

### HistoryEntry

```c
typedef struct {
    char command[1024];
    pid_t pid;
    time_t start_time;
    double duration;
} HistoryEntry;
```

Stores execution information for each command.

---

## Limitations

- Maximum of 100 commands stored.
- Maximum command length is 1024 characters.
- No support for:
  - Background execution (`&`)
  - Input/output redirection (`<`, `>`, `>>`)
  - Environment variable expansion
  - Command aliases
  - Auto-completion
- Pipeline execution records only basic timing information.
- History is maintained only for the current session.

---

## Learning Outcomes

This assignment demonstrates the implementation of several fundamental operating system concepts:

- Process creation using `fork()`
- Program execution with `execvp()`
- Parent-child synchronization using `wait()`
- Inter-process communication using pipes
- File descriptor redirection with `dup2()`
- Signal handling using `sigaction()`
- Measuring process execution time
- Building a basic Unix command interpreter