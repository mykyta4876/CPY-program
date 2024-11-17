import random
import sys

if len(sys.argv) != 2:
    print("Usage: python generate_tasks.py <task_count>")
    sys.exit(1)

try:
    task_count = int(sys.argv[1])
    if task_count < 1:
        raise ValueError
except ValueError:
    print("Task count must be a positive integer")
    sys.exit(1)

with open('TaskFile', 'w') as f:
    for i in range(1, task_count + 1):  # Generate task_count tasks
        burst_time = random.randint(1, 5)
        f.write(f"{i} {burst_time}\n")
