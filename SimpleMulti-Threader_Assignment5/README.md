# Simple Multithreader Library

## Overview

This project implements a lightweight multithreading library in C++ using **POSIX Threads (Pthreads)**. The library provides a generic `parallel_for` construct for executing one-dimensional and two-dimensional loops in parallel by dividing the workload among multiple threads.

Two applications are provided to demonstrate the library:

- Parallel Vector Addition
- Parallel Matrix Multiplication

The implementation measures and reports the execution time of each parallel computation.

---

## Features

- Generic 1D `parallel_for`
- Generic 2D `parallel_for`
- Thread creation using POSIX Threads
- Automatic workload partitioning
- Lambda function support
- Parallel vector addition
- Parallel matrix multiplication
- Execution time measurement using `std::chrono`

---

## Files

| File | Description |
|------|-------------|
| `simple-multithreader.h` | Multithreading library implementation |
| `vector.cpp` | Parallel vector addition program |
| `matrix.cpp` | Parallel matrix multiplication program |
| `README.md` | Project documentation |

---

## Library Design

The library provides two overloaded versions of `parallel_for`.

### 1D Parallel Loop

```cpp
parallel_for(low, high, lambda, numThreads);
```

Executes

```cpp
for(int i = low; i < high; i++)
```

using multiple threads.

---

### 2D Parallel Loop

```cpp
parallel_for(low1, high1,
             low2, high2,
             lambda,
             numThreads);
```

Executes

```cpp
for(int i = low1; i < high1; i++)
    for(int j = low2; j < high2; j++)
```

in parallel.

---

## Working

### 1. Workload Partitioning

The iteration space is divided into equal-sized chunks.

For a loop

```text
low ... high
```

each thread receives

```
chunk = (high - low) / numThreads
```

The final thread executes any remaining iterations.

---

### 2. Thread Creation

Each chunk is assigned to a separate POSIX thread using

```cpp
pthread_create()
```

Each thread receives its iteration range through a structure.

---

### 3. Thread Execution

For the 1D version, every thread executes

```cpp
for(i = low; i < high; i++)
    lambda(i);
```

For the 2D version

```cpp
for(i = low1; i < high1; i++)
    for(j = low2; j < high2; j++)
        lambda(i, j);
```

---

### 4. Thread Synchronization

The main thread waits for all worker threads using

```cpp
pthread_join()
```

Execution continues only after every thread completes.

---

### 5. Performance Measurement

Execution time is measured using

```cpp
std::chrono::high_resolution_clock
```

The elapsed time is printed after every `parallel_for` invocation.

---

## Parallel Vector Addition

The vector application

1. Allocates three vectors
2. Initializes the input vectors
3. Adds corresponding elements in parallel

```
C[i] = A[i] + B[i]
```

4. Verifies correctness using

```cpp
assert(C[i] == 2);
```

---

## Parallel Matrix Multiplication

The matrix application

1. Allocates three matrices
2. Initializes matrices A and B
3. Computes

```
C = A × B
```

using parallel nested loops

```cpp
C[i][j] += A[i][k] * B[k][j];
```

The result is verified using

```cpp
assert(C[i][j] == size);
```

---

## Parallel Execution Model

### 1D

```
Iterations

0 ---------------------------- N

Thread 1
Thread 2
Thread 3
...
Thread n
```

Each thread processes a contiguous block of iterations.

---

### 2D

```
Rows

0
|
|
v

Thread 1
Thread 2
Thread 3
...
Thread n
```

Rows are divided among threads while each thread processes all assigned columns.

---

## Compilation

Compile the vector addition program

```bash
g++ vector.cpp -pthread -o vector
```

Compile the matrix multiplication program

```bash
g++ matrix.cpp -pthread -o matrix
```

---

## Running

### Vector Addition

```bash
./vector <numThreads> <vectorSize>
```

Example

```bash
./vector 4 48000000
```

---

### Matrix Multiplication

```bash
./matrix <numThreads> <matrixSize>
```

Example

```bash
./matrix 4 1024
```

---

## Output

Example

```text
====== Welcome to Assignment-5 of the CSE231(A) ======

[parallel_for 1D] execution time = 0.018 seconds

Test Success

====== Hope you enjoyed CSE231(A) ======
```

For matrix multiplication

```text
[parallel_for 2D] execution time = 0.842 seconds

Test Success
```

---

## System Libraries Used

- POSIX Threads (`pthread`)
- C++ Standard Library
- `<functional>`
- `<chrono>`
- `<cassert>`

---

## Thread Argument Structures

### 1D

```cpp
thread_args1d
```

Stores

- Starting index
- Ending index

---

### 2D

```cpp
thread_args2d
```

Stores

- Row range
- Column range

---

## Synchronization

The implementation uses

- `pthread_create()`
- `pthread_join()`

No additional synchronization primitives (mutexes or semaphores) are required because each thread operates on a distinct portion of the data.

---

## Limitations

- Static workload partitioning.
- Assumes independent loop iterations.
- No dynamic load balancing.
- No thread pool implementation.
- Shared lambda functions are stored globally.
- Performance depends on balanced workload distribution.

---

## Learning Outcomes

This assignment demonstrates several important concepts in parallel programming:

- POSIX Threads (Pthreads)
- Parallel loop execution
- Static workload partitioning
- Thread creation and synchronization
- Lambda functions in C++
- Parallel vector addition
- Parallel matrix multiplication
- Performance measurement using `std::chrono`
- Designing a reusable multithreading library