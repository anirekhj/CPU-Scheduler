#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>

typedef struct {
  char id[4];
  int arrival_time;
  int burst_time;
  int burst_left;
  int wait_time;
  int count;
} process;

void resetProcessMeta(process *p, const int count) {
  int i;
  for (i = 0; i < count; i++) {
    p[i].burst_left = p[i].burst_time;
    p[i].wait_time = 0.0;
  }
}

void resetBurstArr(process *p, int *burst_arr, const int count) {
  int i;
  for (i = 0; i < count; i++) {
    burst_arr[i] = p[i].burst_time;
  }
}

float avgWaitTime(process *p, const int count) {
  int i;
  float avg_wait_time = 0.0;
  for (i = 0; i < count; i++) {
    avg_wait_time = avg_wait_time + (float)p[i].wait_time/count - (float)p[i].arrival_time/count;
  }
  return avg_wait_time;
}
// https://en.wikipedia.org/wiki/Qsort
int compareInts(const void *p, const void *q) {
    int x = *(const int *)p;
    int y = *(const int *)q;

    /* Avoid return x - y, which can cause undefined behaviour
       because of signed integer overflow. */
    if (x < y)
        return -1;  // Return -1 if you want ascending, 1 if you want descending order.
    else if (x > y)
        return 1;   // Return 1 if you want ascending, -1 if you want descending order.

    return 0;
}

int main(int argc, const char *argv[]) {
  // Open the input file
  FILE *txt_file = fopen(argv[1], "r");
  int *burst_arr = (int*) malloc(sizeof(int));
  int capacity = 2;
  int count = 0;
  char line[256];
  char delim[] = ",\n";
  process *p = (process*) malloc(sizeof(process));
  int i = 0;
  float avg_wait_time;

  // Check the number of args
  if (argc != 2) {
    fprintf(txt_file, "Usage: %s, <file_name>\n");
    return 0;
  }

  if (txt_file == NULL) {
    fprintf(txt_file, "Error: can't open the file");
  }
  else {
    // read till EOF and store in integer array. Realloc the size dynamically.
    while (fgets(line, sizeof(line), txt_file)) {
      if (capacity-1 == i) {
        capacity = capacity * 2;
        p = (process*) realloc(p, sizeof(process)*capacity);
        burst_arr = (int*) realloc(burst_arr, sizeof(int)*capacity);
      }
      char *ptr = strtok(line, delim);
      if (ptr != NULL) {
        strcpy(p[i].id, line);
      }
      ptr = strtok(NULL, delim);
      if (ptr != NULL) {
        p[i].arrival_time = atoi(ptr);
      }
      ptr = strtok(NULL, delim);
      if (ptr != NULL) {
        p[i].burst_time = atoi(ptr);
        p[i].burst_left = burst_arr[i] = p[i].burst_time;
      }
      i++;
      count++;
    }
  }

  txt_file = fopen("Output.txt", "w");
  fprintf(txt_file, "FCFS:\n");
  int start = 0;
  for (i = 0; i < count; i++) {
    fprintf(txt_file, "%s\t%d\t", p[i].id, start);
    p[i].wait_time = start;
    start = start + p[i].burst_time;
    fprintf(txt_file, "%d\n", start);
  }
  fprintf(txt_file, "Average Waiting Time: %.2f\n", avgWaitTime(p, count));

  resetProcessMeta(p, count);

  fprintf(txt_file, "\nRR:\n");
  start = 0;

  qsort(burst_arr, count, sizeof *burst_arr, compareInts);

  // int eighty_percent_rule_time_quantum_index = 0.8*count - 1;
  // int time_quantum = burst_arr[eighty_percent_rule_time_quantum_index];
  int time_quantum = 4;

  int rr_flag = count;
  i = 0;
  while (rr_flag != 0) {
    if (i == count) {
      i = 0;
    }
    if (p[i].burst_left == 0) {
      i++;
      continue;
    }
    fprintf(txt_file, "%s\t%d\t", p[i].id, start);
    if (p[i].burst_left <= time_quantum) {
      p[i].wait_time = start - (p[i].burst_time - p[i].burst_left);
      start = start + p[i].burst_left;
      fprintf(txt_file, "%d\n", start);
      p[i].burst_left = 0;
      rr_flag--;
      i++;
    }
    else {
      start = start + time_quantum;
      fprintf(txt_file, "%d\n", start);
      p[i].burst_left = p[i].burst_left - time_quantum;
      i++;
    }
  }
  fprintf(txt_file, "Average Waiting Time: %.2f\n", avgWaitTime(p, count));

  resetProcessMeta(p, count);

  fprintf(txt_file, "\nNSJF:\n");
  int burst_array[count];
  for (i = 0; i < count ; i++) {
    burst_array[i] = INT_MAX;
  }
  int time_state = 0;
  int j;

  for (i = 0; i < count; i++) {
    for (j = 0; j < count; j++) {
      burst_array[j] = INT_MAX;
    }
    for (j = 0; j < count; j++) {
      if ((p[j].arrival_time <= time_state) && (p[j].burst_left != 0)) {
        burst_array[j] = p[j].burst_time; // [8]
      }
    }
    qsort(burst_array, count, sizeof *burst_array, compareInts);
    for (j = 0; j < count; j++) {
      if (p[j].burst_time == burst_array[0]) {
        fprintf(txt_file, "%s\t%d\t", p[j].id, time_state);
        p[j].wait_time = time_state;
        time_state = time_state + p[j].burst_time;
        p[j].burst_left = 0;
        fprintf(txt_file, "%d\n", time_state);
      }
    }
  }

  fprintf(txt_file, "Average Waiting Time: %.2f\n", avgWaitTime(p, count));

  resetProcessMeta(p, count);

  fprintf(txt_file, "\nPSJF:\n");
  char flag[4] = "";
  for (i = 0; i < count ; i++) {
    burst_array[i] = INT_MAX;
  }
  int burst_array_size; time_state = 0;
  start = i = j = burst_array_size = 1;
  int psjf_flag = count;

  while (psjf_flag != -1 && start < 35) {
    for (i = 0; i < count ; i++) {
      burst_array[i] = INT_MAX;
    }
    for (i = 0; i < count; i++) {
      if ((p[i].arrival_time <= time_state) && (p[i].burst_left != 0)) {
        burst_array[i] = p[i].burst_left;
      }
    }

    qsort(burst_array, count, sizeof *burst_array, compareInts);
    for (i = 0; i < count; i++) {
      if (p[i].burst_left == burst_array[0]) {
        if (strcmp(flag, "") == 0) {
          fprintf(txt_file, "%s\t%d\t", p[i].id, time_state);
        }
        else if (strcmp(flag, p[i].id) != 0) {
          fprintf(txt_file, "%d\n", time_state);
          fprintf(txt_file, "%s\t%d\t", p[i].id, time_state);
        }
        --p[i].burst_left;
        break;
      }
    }
    strcpy(flag, p[i].id);
    if (p[i].burst_left == 0) {
      psjf_flag--;
      p[i].wait_time = time_state-p[i].burst_time+1;
    }
    time_state++;
  }
  fprintf(txt_file, "%d\n", time_state-1);
  fprintf(txt_file, "Average Waiting Time: %.2f\n", avgWaitTime(p, count));
  fclose(txt_file);
  free(burst_arr);
  free(p);
}
