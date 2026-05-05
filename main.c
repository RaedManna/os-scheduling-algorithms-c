/*
 * CPU Scheduling Algorithms Simulator
 *
 * This is a small operating systems project written in C.
 * It simulates common CPU scheduling algorithms and compares their results
 * using completion time, turnaround time, waiting time, and response time.
 *
 * Implemented algorithms:
 *   1. First Come First Serve (FCFS)
 *   2. Shortest Job First - Non-preemptive (SJF)
 *   3. Shortest Remaining Time First - Preemptive SJF (SRTF)
 *   4. Priority Scheduling - Non-preemptive
 *   5. Round Robin (RR)
 *
 * The program uses a built-in process set by default, so anyone can compile
 * and run it immediately without needing extra input files.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>

#define MAX_PROCESSES 20
#define MAX_TIMELINE 1000

typedef struct {
    int pid;
    int arrival_time;
    int burst_time;
    int priority;

    int remaining_time;
    int start_time;
    int completion_time;
    int turnaround_time;
    int waiting_time;
    int response_time;
    int started;
} Process;

typedef struct {
    int pid;
    int start;
    int end;
} TimelineEntry;

typedef struct {
    TimelineEntry entries[MAX_TIMELINE];
    int count;
} Timeline;

/* ---------- Utility helpers ---------- */

void add_timeline_entry(Timeline *timeline, int pid, int start, int end) {
    if (start == end) {
        return;
    }

    /*
     * Merge adjacent blocks belonging to the same process.
     * This keeps the Gantt chart cleaner, especially for non-preemptive algorithms.
     */
    if (timeline->count > 0) {
        TimelineEntry *last = &timeline->entries[timeline->count - 1];
        if (last->pid == pid && last->end == start) {
            last->end = end;
            return;
        }
    }

    if (timeline->count < MAX_TIMELINE) {
        timeline->entries[timeline->count].pid = pid;
        timeline->entries[timeline->count].start = start;
        timeline->entries[timeline->count].end = end;
        timeline->count++;
    }
}

void copy_processes(Process dest[], const Process src[], int n) {
    for (int i = 0; i < n; i++) {
        dest[i] = src[i];
        dest[i].remaining_time = src[i].burst_time;
        dest[i].start_time = -1;
        dest[i].completion_time = 0;
        dest[i].turnaround_time = 0;
        dest[i].waiting_time = 0;
        dest[i].response_time = 0;
        dest[i].started = 0;
    }
}

void finalize_metrics(Process processes[], int n) {
    for (int i = 0; i < n; i++) {
        processes[i].turnaround_time = processes[i].completion_time - processes[i].arrival_time;
        processes[i].waiting_time = processes[i].turnaround_time - processes[i].burst_time;
        processes[i].response_time = processes[i].start_time - processes[i].arrival_time;
    }
}

void print_gantt_chart(const Timeline *timeline) {
    printf("\nGantt Chart:\n");

    for (int i = 0; i < timeline->count; i++) {
        if (timeline->entries[i].pid == -1) {
            printf("| IDLE ");
        } else {
            printf("| P%d ", timeline->entries[i].pid);
        }
    }
    printf("|\n");

    if (timeline->count > 0) {
        printf("%d", timeline->entries[0].start);
        for (int i = 0; i < timeline->count; i++) {
            printf("    %d", timeline->entries[i].end);
        }
        printf("\n");
    }
}

void print_results(const char *title, Process processes[], int n, const Timeline *timeline) {
    double total_waiting = 0.0;
    double total_turnaround = 0.0;
    double total_response = 0.0;

    printf("\n============================================================\n");
    printf("%s\n", title);
    printf("============================================================\n");

    print_gantt_chart(timeline);

    printf("\n%-8s %-8s %-8s %-8s %-8s %-8s %-8s\n",
           "PID", "Arrival", "Burst", "Prio", "Finish", "Turn", "Wait");

    for (int i = 0; i < n; i++) {
        printf("P%-7d %-8d %-8d %-8d %-8d %-8d %-8d\n",
               processes[i].pid,
               processes[i].arrival_time,
               processes[i].burst_time,
               processes[i].priority,
               processes[i].completion_time,
               processes[i].turnaround_time,
               processes[i].waiting_time);

        total_waiting += processes[i].waiting_time;
        total_turnaround += processes[i].turnaround_time;
        total_response += processes[i].response_time;
    }

    printf("\nAverage waiting time    : %.2f", total_waiting / n);
    printf("\nAverage turnaround time : %.2f", total_turnaround / n);
    printf("\nAverage response time   : %.2f\n", total_response / n);
}

int all_completed(Process processes[], int n) {
    for (int i = 0; i < n; i++) {
        if (processes[i].remaining_time > 0) {
            return 0;
        }
    }
    return 1;
}

/* ---------- Scheduling algorithms ---------- */

void schedule_fcfs(const Process original[], int n) {
    Process processes[MAX_PROCESSES];
    Timeline timeline = { .count = 0 };
    copy_processes(processes, original, n);

    int completed[MAX_PROCESSES] = {0};
    int completed_count = 0;
    int time = 0;

    while (completed_count < n) {
        int selected = -1;
        int best_arrival = INT_MAX;

        /*
         * Choose the arrived process with the earliest arrival time.
         * If there is a tie, use the smaller PID for stable output.
         */
        for (int i = 0; i < n; i++) {
            if (!completed[i] && processes[i].arrival_time <= time) {
                if (processes[i].arrival_time < best_arrival ||
                    (processes[i].arrival_time == best_arrival &&
                     (selected == -1 || processes[i].pid < processes[selected].pid))) {
                    best_arrival = processes[i].arrival_time;
                    selected = i;
                }
            }
        }

        if (selected == -1) {
            add_timeline_entry(&timeline, -1, time, time + 1);
            time++;
            continue;
        }

        processes[selected].start_time = time;
        time += processes[selected].burst_time;
        processes[selected].completion_time = time;
        processes[selected].remaining_time = 0;
        completed[selected] = 1;
        completed_count++;

        add_timeline_entry(&timeline, processes[selected].pid,
                           processes[selected].start_time,
                           processes[selected].completion_time);
    }

    finalize_metrics(processes, n);
    print_results("First Come First Serve (FCFS)", processes, n, &timeline);
}

void schedule_sjf_non_preemptive(const Process original[], int n) {
    Process processes[MAX_PROCESSES];
    Timeline timeline = { .count = 0 };
    copy_processes(processes, original, n);

    int completed[MAX_PROCESSES] = {0};
    int completed_count = 0;
    int time = 0;

    while (completed_count < n) {
        int selected = -1;
        int shortest_burst = INT_MAX;

        for (int i = 0; i < n; i++) {
            if (!completed[i] && processes[i].arrival_time <= time) {
                if (processes[i].burst_time < shortest_burst ||
                    (processes[i].burst_time == shortest_burst &&
                     (selected == -1 || processes[i].arrival_time < processes[selected].arrival_time))) {
                    shortest_burst = processes[i].burst_time;
                    selected = i;
                }
            }
        }

        if (selected == -1) {
            add_timeline_entry(&timeline, -1, time, time + 1);
            time++;
            continue;
        }

        processes[selected].start_time = time;
        time += processes[selected].burst_time;
        processes[selected].completion_time = time;
        processes[selected].remaining_time = 0;
        completed[selected] = 1;
        completed_count++;

        add_timeline_entry(&timeline, processes[selected].pid,
                           processes[selected].start_time,
                           processes[selected].completion_time);
    }

    finalize_metrics(processes, n);
    print_results("Shortest Job First - Non-preemptive", processes, n, &timeline);
}

void schedule_srtf(const Process original[], int n) {
    Process processes[MAX_PROCESSES];
    Timeline timeline = { .count = 0 };
    copy_processes(processes, original, n);

    int time = 0;

    while (!all_completed(processes, n)) {
        int selected = -1;
        int best_remaining = INT_MAX;

        /*
         * At every time unit, choose the arrived process with the shortest
         * remaining burst time. This is the preemptive version of SJF.
         */
        for (int i = 0; i < n; i++) {
            if (processes[i].arrival_time <= time && processes[i].remaining_time > 0) {
                if (processes[i].remaining_time < best_remaining ||
                    (processes[i].remaining_time == best_remaining &&
                     (selected == -1 || processes[i].arrival_time < processes[selected].arrival_time))) {
                    best_remaining = processes[i].remaining_time;
                    selected = i;
                }
            }
        }

        if (selected == -1) {
            add_timeline_entry(&timeline, -1, time, time + 1);
            time++;
            continue;
        }

        if (!processes[selected].started) {
            processes[selected].start_time = time;
            processes[selected].started = 1;
        }

        add_timeline_entry(&timeline, processes[selected].pid, time, time + 1);
        processes[selected].remaining_time--;
        time++;

        if (processes[selected].remaining_time == 0) {
            processes[selected].completion_time = time;
        }
    }

    finalize_metrics(processes, n);
    print_results("Shortest Remaining Time First (Preemptive SJF)", processes, n, &timeline);
}

void schedule_priority_non_preemptive(const Process original[], int n) {
    Process processes[MAX_PROCESSES];
    Timeline timeline = { .count = 0 };
    copy_processes(processes, original, n);

    int completed[MAX_PROCESSES] = {0};
    int completed_count = 0;
    int time = 0;

    while (completed_count < n) {
        int selected = -1;
        int best_priority = INT_MAX;

        /*
         * Lower priority number means higher priority.
         * Example: priority 1 runs before priority 3.
         */
        for (int i = 0; i < n; i++) {
            if (!completed[i] && processes[i].arrival_time <= time) {
                if (processes[i].priority < best_priority ||
                    (processes[i].priority == best_priority &&
                     (selected == -1 || processes[i].arrival_time < processes[selected].arrival_time))) {
                    best_priority = processes[i].priority;
                    selected = i;
                }
            }
        }

        if (selected == -1) {
            add_timeline_entry(&timeline, -1, time, time + 1);
            time++;
            continue;
        }

        processes[selected].start_time = time;
        time += processes[selected].burst_time;
        processes[selected].completion_time = time;
        processes[selected].remaining_time = 0;
        completed[selected] = 1;
        completed_count++;

        add_timeline_entry(&timeline, processes[selected].pid,
                           processes[selected].start_time,
                           processes[selected].completion_time);
    }

    finalize_metrics(processes, n);
    print_results("Priority Scheduling - Non-preemptive", processes, n, &timeline);
}

void schedule_round_robin(const Process original[], int n, int quantum) {
    Process processes[MAX_PROCESSES];
    Timeline timeline = { .count = 0 };
    copy_processes(processes, original, n);

    int queue[MAX_TIMELINE];
    int in_queue[MAX_PROCESSES] = {0};
    int front = 0;
    int rear = 0;
    int time = 0;
    int completed_count = 0;

    while (completed_count < n) {
        /*
         * Add every newly arrived process that is not already waiting.
         */
        for (int i = 0; i < n; i++) {
            if (processes[i].arrival_time <= time &&
                processes[i].remaining_time > 0 &&
                !in_queue[i]) {
                queue[rear++] = i;
                in_queue[i] = 1;
            }
        }

        if (front == rear) {
            add_timeline_entry(&timeline, -1, time, time + 1);
            time++;
            continue;
        }

        int current = queue[front++];
        in_queue[current] = 0;

        if (!processes[current].started) {
            processes[current].start_time = time;
            processes[current].started = 1;
        }

        int run_time = processes[current].remaining_time < quantum
                       ? processes[current].remaining_time
                       : quantum;

        add_timeline_entry(&timeline, processes[current].pid, time, time + run_time);

        /*
         * Run the process. During this period, new processes may arrive.
         * We add them after advancing time, before re-queueing the current one.
         */
        time += run_time;
        processes[current].remaining_time -= run_time;

        for (int i = 0; i < n; i++) {
            if (processes[i].arrival_time <= time &&
                processes[i].remaining_time > 0 &&
                !in_queue[i] &&
                i != current) {
                queue[rear++] = i;
                in_queue[i] = 1;
            }
        }

        if (processes[current].remaining_time > 0) {
            queue[rear++] = current;
            in_queue[current] = 1;
        } else {
            processes[current].completion_time = time;
            completed_count++;
        }
    }

    finalize_metrics(processes, n);
    print_results("Round Robin", processes, n, &timeline);
}

/* ---------- Input and demo data ---------- */

int read_processes_from_user(Process processes[]) {
    int n;

    printf("Enter number of processes (1-%d): ", MAX_PROCESSES);
    if (scanf("%d", &n) != 1 || n < 1 || n > MAX_PROCESSES) {
        printf("Invalid number of processes.\n");
        return -1;
    }

    printf("\nEnter process data as: arrival_time burst_time priority\n");
    printf("Lower priority number means higher priority.\n\n");

    for (int i = 0; i < n; i++) {
        processes[i].pid = i + 1;

        printf("P%d: ", processes[i].pid);
        if (scanf("%d %d %d",
                  &processes[i].arrival_time,
                  &processes[i].burst_time,
                  &processes[i].priority) != 3) {
            printf("Invalid input.\n");
            return -1;
        }

        if (processes[i].arrival_time < 0 || processes[i].burst_time <= 0) {
            printf("Arrival time must be >= 0 and burst time must be > 0.\n");
            return -1;
        }
    }

    return n;
}

int load_demo_processes(Process processes[]) {
    /*
     * Built-in demo data.
     * This gives enough variation to show preemption, queueing, and priority effects.
     */
    Process demo[] = {
        {1, 0, 7, 3, 0, -1, 0, 0, 0, 0, 0},
        {2, 2, 4, 1, 0, -1, 0, 0, 0, 0, 0},
        {3, 4, 1, 4, 0, -1, 0, 0, 0, 0, 0},
        {4, 5, 4, 2, 0, -1, 0, 0, 0, 0, 0},
        {5, 6, 6, 5, 0, -1, 0, 0, 0, 0, 0}
    };

    int n = sizeof(demo) / sizeof(demo[0]);

    for (int i = 0; i < n; i++) {
        processes[i] = demo[i];
    }

    return n;
}

void print_process_set(const Process processes[], int n) {
    printf("\nProcess Set:\n");
    printf("%-8s %-8s %-8s %-8s\n", "PID", "Arrival", "Burst", "Priority");

    for (int i = 0; i < n; i++) {
        printf("P%-7d %-8d %-8d %-8d\n",
               processes[i].pid,
               processes[i].arrival_time,
               processes[i].burst_time,
               processes[i].priority);
    }
}

int main(void) {
    Process processes[MAX_PROCESSES];
    int n;
    int choice;
    int quantum = 3;

    printf("CPU Scheduling Algorithms Simulator\n");
    printf("-----------------------------------\n");
    printf("1. Use built-in demo process set\n");
    printf("2. Enter custom process set\n");
    printf("Choose option: ");

    if (scanf("%d", &choice) != 1) {
        printf("Invalid input.\n");
        return 1;
    }

    if (choice == 2) {
        n = read_processes_from_user(processes);
        if (n == -1) {
            return 1;
        }

        printf("Enter Round Robin time quantum: ");
        if (scanf("%d", &quantum) != 1 || quantum <= 0) {
            printf("Invalid time quantum.\n");
            return 1;
        }
    } else {
        n = load_demo_processes(processes);
        quantum = 3;
    }

    print_process_set(processes, n);

    schedule_fcfs(processes, n);
    schedule_sjf_non_preemptive(processes, n);
    schedule_srtf(processes, n);
    schedule_priority_non_preemptive(processes, n);
    schedule_round_robin(processes, n, quantum);

    printf("\nSimulation finished.\n");
    return 0;
}
