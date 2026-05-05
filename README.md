# Operating Systems Scheduling Algorithms in C

A clean C project that simulates and compares common CPU/process scheduling algorithms used in operating systems.

The project includes a single runnable `main.c` file with multiple scheduling algorithms, Gantt chart output, and performance metrics.

---

## Implemented Algorithms

- First Come First Serve (FCFS)
- Shortest Job First - Non-preemptive (SJF)
- Shortest Remaining Time First - Preemptive SJF (SRTF)
- Priority Scheduling - Non-preemptive
- Round Robin (RR)

---

## What the Program Shows

For each algorithm, the simulator prints:

- Gantt chart
- Completion time
- Turnaround time
- Waiting time
- Response time
- Average waiting time
- Average turnaround time
- Average response time

---

## Features

- Built-in demo process set
- Optional custom process input
- Round Robin time quantum input
- CPU idle-time handling
- Clean process metrics calculation
- Timeline/Gantt chart generation
- Stable tie-breaking for predictable output
- Well-commented C implementation

---

## How to Compile and Run

Using GCC:

```bash
gcc main.c -o scheduler
./scheduler
```

On Windows with MinGW:

```bash
gcc main.c -o scheduler.exe
scheduler.exe
```

---

## Example Usage

When the program starts, choose:

```text
1. Use built-in demo process set
2. Enter custom process set
```

Choosing option `1` runs all algorithms on a built-in process set immediately.

---

## Concepts Practiced

- CPU scheduling
- Process arrival time
- Burst time
- Completion time
- Turnaround time
- Waiting time
- Response time
- Preemptive scheduling
- Non-preemptive scheduling
- Round Robin queue behavior
- Gantt chart simulation
- C structs and arrays

---

## Author

**Raed H. Manna**  
Computer Engineering Graduate | Junior Full-Stack Developer  

- GitHub: [RaedManna](https://github.com/RaedManna)
- LinkedIn: [raedhmanna](https://www.linkedin.com/in/raedhmanna)
