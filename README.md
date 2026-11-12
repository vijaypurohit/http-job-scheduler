# Job Scheduler

This is an in-process job scheduler in C++. Jobs go into a priority queue, and a background worker thread runs them in priority order. Jobs with the same priority run in submission order. Each job's status (`PENDING` → `RUNNING` → `COMPLETED`) is tracked by job id.

## Build & run

```bash
clang++ -std=c++17 -pthread JobScheduler.cpp -o JobScheduler
./JobScheduler
```

## Expected output

```
job 1 status: 1
Executing Job 1 HIGH
job 2 status: 1
Executing Job 2 MEDIUM
job 3 status: 1
Executing Job 3 LOW
job 1 final status: 2
job 2 final status: 2
job 3 final status: 2
```
